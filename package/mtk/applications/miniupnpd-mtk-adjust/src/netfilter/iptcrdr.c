/* $Id: iptcrdr.c,v 1.67 2020/11/11 12:09:05 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2020 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include <xtables.h>
#include <linux/netfilter/xt_DSCP.h>
#include <libiptc/libiptc.h>

#include <linux/version.h>

#include "config.h"

#ifdef IPTABLES_143
/* IPTABLES API version >= 1.4.3 */

/* added in order to compile on gentoo :
 * http://miniupnp.tuxfamily.org/forum/viewtopic.php?p=2183 */
#define BUILD_BUG_ON_ZERO(e) (sizeof(struct { int:-!!(e); }))
#define __must_be_array(a) \
	BUILD_BUG_ON_ZERO(__builtin_types_compatible_p(typeof(a), typeof(&a[0])))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))
#define LIST_POISON2  ((void *) 0x00200200 )

#if 0
#include <linux/netfilter/nf_nat.h>
#else
#include "tiny_nf_nat.h"
#endif
#define ip_nat_multi_range	nf_nat_multi_range
#define ip_nat_range		nf_nat_range
#define IPTC_HANDLE		struct iptc_handle *
#else
/* IPTABLES API version < 1.4.3 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
#include <linux/netfilter_ipv4/ip_nat.h>
#else
#if 0
#include <linux/netfilter/nf_nat.h>
#else
#include "tiny_nf_nat.h"
#endif
#endif
#define IPTC_HANDLE		iptc_handle_t
#endif

/* IPT_ALIGN was renamed XT_ALIGN in iptables-1.4.11 */
#ifndef IPT_ALIGN
#define IPT_ALIGN XT_ALIGN
#endif

#include "../macros.h"
#include "iptcrdr.h"
#include "../upnpglobalvars.h"

/* chain names to use in the nat and filter tables. */

/* iptables -t nat -N MINIUPNPD
 * iptables -t nat -A PREROUTING -i <ext_if_name> -j MINIUPNPD */
static const char * miniupnpd_nat_chain = "MINIUPNPD";

/* iptables -t nat -N MINIUPNPD-POSTROUTING
 * iptables -t nat -A POSTROUTING -o <ext_if_name> -j MINIUPNPD-POSTROUTING */
static const char * miniupnpd_nat_postrouting_chain = "MINIUPNPD-POSTROUTING";

/* iptables -t filter -N MINIUPNPD
 * iptables -t filter -A FORWARD -i <ext_if_name> ! -o <ext_if_name> -j MINIUPNPD */
static const char * miniupnpd_forward_chain = "MINIUPNPD";

/**
 * used by the core to override default chain names if specified in config file
 * @param param which string to set
 * @param string the new name to use. Do not dispose after setting (i.e. use strdup if not static).
 * @return 0 if successful
 */
int
set_rdr_name(rdr_name_type param, const char *string)
{
	if (string == NULL || strlen(string) > 30 || string[0] == '\0') {
		syslog(LOG_ERR, "%s(): invalid string argument '%s'", "set_rdr_name", string);
		return -1;
	}
	switch (param) {
		case RDR_NAT_PREROUTING_CHAIN_NAME:
			miniupnpd_nat_chain = string;
			break;
		case RDR_NAT_POSTROUTING_CHAIN_NAME:
			miniupnpd_nat_postrouting_chain = string;
			break;
		case RDR_FORWARD_CHAIN_NAME:
			miniupnpd_forward_chain = string;
			break;
		default:
			syslog(LOG_ERR, "%s(): tried to set invalid string parameter: %d", "set_rdr_name", param);
			return -2;
	}
	return 0;
}

/* local functions declarations */
static int
addnatrule(int proto, unsigned short eport,
           const char * iaddr, unsigned short iport,
           const char * rhost);

static int
add_filter_rule(int proto, const char * rhost,
                const char * iaddr, unsigned short iport);

#ifdef ENABLE_PORT_TRIGGERING
static int
addmasqueraderule(int proto,
           unsigned short eport,
           const char * iaddr, unsigned short iport,
           const char * rhost/*, const char * extif*/);
#endif /* ENABLE_PORT_TRIGGERING */

static int
addpeernatrule(int proto,
           const char * eaddr, unsigned short eport,
           const char * iaddr, unsigned short iport,
           const char * rhost, unsigned short rport);

static int
addpeerdscprule(int proto, unsigned char dscp,
           const char * iaddr, unsigned short iport,
           const char * rhost, unsigned short rport);

/* dummy init and shutdown functions
 * Only test iptc_init() */
int init_redirect(void)
{
	IPTC_HANDLE h;

	h = iptc_init("nat");
	if(!h) {
		syslog(LOG_ERR, "iptc_init() failed : %s",
		       iptc_strerror(errno));
		return -1;
	} else {
#ifdef IPTABLES_143
		iptc_free(h);
#else
		iptc_free(&h);
#endif
	}
	return 0;
}

void shutdown_redirect(void)
{
	return;
}

/* convert an ip address to string */
static int snprintip(char * dst, size_t size, uint32_t ip)
{
	return snprintf(dst, size,
	       "%u.%u.%u.%u", ip >> 24, (ip >> 16) & 0xff,
	       (ip >> 8) & 0xff, ip & 0xff);
}

/* netfilter cannot store redirection descriptions, so we use our
 * own structure to store them */
struct rdr_desc {
	struct rdr_desc * next;
	unsigned int timestamp;
	unsigned short eport;
	short proto;
	char str[];
};

/* pointer to the chained list where descriptions are stored */
static struct rdr_desc * rdr_desc_list = 0;

/* add a description to the list of redirection descriptions */
static void
add_redirect_desc(unsigned short eport, int proto,
                  const char * desc, unsigned int timestamp)
{
	struct rdr_desc * p;
	size_t l;
	/* set a default description if none given */
	if(!desc)
		desc = "miniupnpd";
	l = strlen(desc) + 1;
	p = malloc(sizeof(struct rdr_desc) + l);
	if(p)
	{
		p->next = rdr_desc_list;
		p->timestamp = timestamp;
		p->eport = eport;
		p->proto = (short)proto;
		memcpy(p->str, desc, l);
		rdr_desc_list = p;
	}
}

/* delete a description from the list */
static void
del_redirect_desc(unsigned short eport, int proto)
{
	struct rdr_desc * p, * last;
	p = rdr_desc_list;
	last = 0;
	while(p)
	{
		if(p->eport == eport && p->proto == proto)
		{
			if(!last)
				rdr_desc_list = p->next;
			else
				last->next = p->next;
			free(p);
			return;
		}
		last = p;
		p = p->next;
	}
}

/* go through the list to find the description */
static void
get_redirect_desc(unsigned short eport, int proto,
                  char * desc, int desclen,
                  unsigned int * timestamp)
{
	struct rdr_desc * p;
	for(p = rdr_desc_list; p; p = p->next)
	{
		if(p->eport == eport && p->proto == (short)proto)
		{
			if(desc)
				strncpy(desc, p->str, desclen);
			if(timestamp)
				*timestamp = p->timestamp;
			return;
		}
	}
	/* if no description was found, return miniupnpd as default */
	if(desc)
		strncpy(desc, "miniupnpd", desclen);
	if(timestamp)
		*timestamp = 0;
}

/* add_redirect_rule2() */
int
add_redirect_rule2(const char * ifname,
                   const char * rhost, unsigned short eport,
                   const char * iaddr, unsigned short iport, int proto,
				   const char * desc, unsigned int timestamp)
{
	int r;
	UNUSED(ifname);

	r = addnatrule(proto, eport, iaddr, iport, rhost);
	if(r >= 0) {
		add_redirect_desc(eport, proto, desc, timestamp);
#ifdef ENABLE_PORT_TRIGGERING
		/* we now always setup SNAT to support bidirectional mapping
		 * we cannot expect that iport == eport on all the firewall.
		 */
		/* TODO : check if this should be done only with UDP */
		r = addmasqueraderule(proto, eport, iaddr, iport, rhost/*, ifname*/);
		if(r < 0) {
			syslog(LOG_NOTICE, "add_redirect_rule2(): addmasqueraderule returned %d", r);
		}
#endif /* ENABLE_PORT_TRIGGERING */
	}
	return r;
}

/* add_peer_redirect_rule2() */
int
add_peer_redirect_rule2(const char * ifname,
                   const char * rhost, unsigned short rport,
                   const char * eaddr, unsigned short eport,
                   const char * iaddr, unsigned short iport, int proto,
                   const char * desc, unsigned int timestamp)
{
	int r;
	UNUSED(ifname);

	r = addpeernatrule(proto, eaddr, eport, iaddr, iport, rhost, rport);
	if(r >= 0)
		add_redirect_desc(eport, proto, desc, timestamp);
	return r;
}

int
add_peer_dscp_rule2(const char * ifname,
                   const char * rhost, unsigned short rport,
                   unsigned char dscp,
                   const char * iaddr, unsigned short iport, int proto,
                   const char * desc, unsigned int timestamp)
{
	int r;
	UNUSED(ifname);
	UNUSED(desc);
	UNUSED(timestamp);

	r = addpeerdscprule(proto, dscp, iaddr, iport, rhost, rport);
/*	if(r >= 0)
		add_redirect_desc(dscp, proto, desc, timestamp); */
	return r;
}

int
add_filter_rule2(const char * ifname,
                 const char * rhost, const char * iaddr,
                 unsigned short eport, unsigned short iport,
                 int proto, const char * desc)
{
	UNUSED(ifname);
	UNUSED(eport);
	UNUSED(desc);

	return add_filter_rule(proto, rhost, iaddr, iport);
}

/* get_redirect_rule()
 * returns -1 if the rule is not found */
int
get_redirect_rule(const char * ifname, unsigned short eport, int proto,
                  char * iaddr, int iaddrlen, unsigned short * iport,
                  char * desc, int desclen,
                  char * rhost, int rhostlen,
                  unsigned int * timestamp,
                  u_int64_t * packets, u_int64_t * bytes)
{
	return get_nat_redirect_rule(miniupnpd_nat_chain,
	                             ifname, eport, proto,
	                             iaddr, iaddrlen, iport,
	                             desc, desclen,
	                             rhost, rhostlen,
	                             timestamp, packets, bytes);
}

int
get_nat_redirect_rule(const char * nat_chain_name, const char * ifname, unsigned short eport, int proto,
                  char * iaddr, int iaddrlen, unsigned short * iport,
                  char * desc, int desclen,
                  char * rhost, int rhostlen,
                  unsigned int * timestamp,
                  u_int64_t * packets, u_int64_t * bytes)
{
	int r = -1;
	IPTC_HANDLE h;
	const struct ipt_entry * e;
	const struct ipt_entry_target * target;
	const struct ip_nat_multi_range * mr;
	const struct ipt_entry_match *match;
	UNUSED(ifname);

	h = iptc_init("nat");
	if(!h)
	{
		syslog(LOG_ERR, "%s() : iptc_init() failed : %s",
		       "get_nat_redirect_rule", iptc_strerror(errno));
		return -1;
	}
	if(!iptc_is_chain(nat_chain_name, h))
	{
		syslog(LOG_ERR, "chain %s not found", nat_chain_name);
	}
	else
	{
#ifdef IPTABLES_143
		for(e = iptc_first_rule(nat_chain_name, h);
		    e;
			e = iptc_next_rule(e, h))
#else
		for(e = iptc_first_rule(nat_chain_name, &h);
		    e;
			e = iptc_next_rule(e, &h))
#endif
		{
			if(proto==e->ip.proto)
			{
				match = (const struct ipt_entry_match *)&e->elems;
				if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
				{
					const struct ipt_tcp * info;
					info = (const struct ipt_tcp *)match->data;
					if(eport != info->dpts[0])
						continue;
				}
				else
				{
					const struct ipt_udp * info;
					info = (const struct ipt_udp *)match->data;
					if(eport != info->dpts[0])
						continue;
				}
				target = (void *)e + e->target_offset;
				/* target = ipt_get_target(e); */
				mr = (const struct ip_nat_multi_range *)&target->data[0];
				snprintip(iaddr, iaddrlen, ntohl(mr->range[0].min_ip));
				*iport = ntohs(mr->range[0].min.all);
				get_redirect_desc(eport, proto, desc, desclen, timestamp);
				if(packets)
					*packets = e->counters.pcnt;
				if(bytes)
					*bytes = e->counters.bcnt;
				/* rhost */
				if(e->ip.src.s_addr && rhost) {
					snprintip(rhost, rhostlen, ntohl(e->ip.src.s_addr));
				}
				r = 0;
				break;
			}
		}
	}
	if(h)
#ifdef IPTABLES_143
		iptc_free(h);
#else
		iptc_free(&h);
#endif
	return r;
}

/* get_redirect_rule_by_index()
 * return -1 when the rule was not found */
int
get_redirect_rule_by_index(int index,
                           char * ifname, unsigned short * eport,
                           char * iaddr, int iaddrlen, unsigned short * iport,
                           int * proto, char * desc, int desclen,
                           char * rhost, int rhostlen,
                           unsigned int * timestamp,
                           u_int64_t * packets, u_int64_t * bytes)
{
	int r = -1;
	int i = 0;
	IPTC_HANDLE h;
	const struct ipt_entry * e;
	const struct ipt_entry_target * target;
	const struct ip_nat_multi_range * mr;
	const struct ipt_entry_match *match;
	UNUSED(ifname);

	h = iptc_init("nat");
	if(!h)
	{
		syslog(LOG_ERR, "%s() : iptc_init() failed : %s",
		       "get_redirect_rule_by_index", iptc_strerror(errno));
		return -1;
	}
	if(!iptc_is_chain(miniupnpd_nat_chain, h))
	{
		syslog(LOG_ERR, "chain %s not found", miniupnpd_nat_chain);
	}
	else
	{
#ifdef IPTABLES_143
		for(e = iptc_first_rule(miniupnpd_nat_chain, h);
		    e;
			e = iptc_next_rule(e, h))
#else
		for(e = iptc_first_rule(miniupnpd_nat_chain, &h);
		    e;
			e = iptc_next_rule(e, &h))
#endif
		{
			if(i==index)
			{
				*proto = e->ip.proto;
				match = (const struct ipt_entry_match *)&e->elems;
				if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
				{
					const struct ipt_tcp * info;
					info = (const struct ipt_tcp *)match->data;
					*eport = info->dpts[0];
				}
				else
				{
					const struct ipt_udp * info;
					info = (const struct ipt_udp *)match->data;
					*eport = info->dpts[0];
				}
				target = (void *)e + e->target_offset;
				mr = (const struct ip_nat_multi_range *)&target->data[0];
				snprintip(iaddr, iaddrlen, ntohl(mr->range[0].min_ip));
				*iport = ntohs(mr->range[0].min.all);
				get_redirect_desc(*eport, *proto, desc, desclen, timestamp);
				if(packets)
					*packets = e->counters.pcnt;
				if(bytes)
					*bytes = e->counters.bcnt;
				/* rhost */
				if(rhost && rhostlen > 0) {
					if(e->ip.src.s_addr) {
						snprintip(rhost, rhostlen, ntohl(e->ip.src.s_addr));
					} else {
						rhost[0] = '\0';
					}
				}
				r = 0;
				break;
			}
			i++;
		}
	}
	if(h)
#ifdef IPTABLES_143
		iptc_free(h);
#else
		iptc_free(&h);
#endif
	return r;
}

/* get_peer_rule_by_index()
 * return -1 when the rule was not found */
int
get_peer_rule_by_index(int index,
                           char * ifname, unsigned short * eport,
                           char * iaddr, int iaddrlen, unsigned short * iport,
                           int * proto, char * desc, int desclen,
                           char * rhost, int rhostlen, unsigned short * rport,
                           unsigned int * timestamp,
                           u_int64_t * packets, u_int64_t * bytes)
{
	int r = -1;
	int i = 0;
	IPTC_HANDLE h;
	const struct ipt_entry * e;
	const struct ipt_entry_target * target;
	const struct ip_nat_multi_range * mr;
	const struct ipt_entry_match *match;
	UNUSED(ifname);

	h = iptc_init("nat");
	if(!h)
	{
		syslog(LOG_ERR, "%s() : iptc_init() failed : %s",
		       "get_peer_rule_by_index", iptc_strerror(errno));
		return -1;
	}
	if(!iptc_is_chain(miniupnpd_nat_postrouting_chain, h))
	{
		syslog(LOG_ERR, "chain %s not found", miniupnpd_nat_postrouting_chain);
	}
	else
	{
#ifdef IPTABLES_143
		for(e = iptc_first_rule(miniupnpd_nat_postrouting_chain, h);
		    e;
			e = iptc_next_rule(e, h))
#else
		for(e = iptc_first_rule(miniupnpd_nat_postrouting_chain, &h);
		    e;
			e = iptc_next_rule(e, &h))
#endif
		{
			if(i==index)
			{
				*proto = e->ip.proto;
				match = (const struct ipt_entry_match *)&e->elems;
				if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
				{
					const struct ipt_tcp * info;
					info = (const struct ipt_tcp *)match->data;
					if (rport)
						*rport = info->dpts[0];
					if (iport)
						*iport = info->spts[0];
				}
				else
				{
					const struct ipt_udp * info;
					info = (const struct ipt_udp *)match->data;
					if (rport)
						*rport = info->dpts[0];
					if (iport)
						*iport = info->spts[0];
				}
				target = (void *)e + e->target_offset;
				mr = (const struct ip_nat_multi_range *)&target->data[0];
				*eport = ntohs(mr->range[0].min.all);
				get_redirect_desc(*eport, *proto, desc, desclen, timestamp);
				if(packets)
					*packets = e->counters.pcnt;
				if(bytes)
					*bytes = e->counters.bcnt;
				/* rhost */
				if(rhost && rhostlen > 0) {
					if(e->ip.dst.s_addr) {
						snprintip(rhost, rhostlen, ntohl(e->ip.dst.s_addr));
					} else {
						rhost[0] = '\0';
					}
				}
				if(iaddr && iaddrlen > 0) {
					if(e->ip.src.s_addr) {
						snprintip(iaddr, iaddrlen, ntohl(e->ip.src.s_addr));
					} else {
						rhost[0] = '\0';
					}
				}
				r = 0;
				break;
			}
			i++;
		}
	}
	if(h)
#ifdef IPTABLES_143
		iptc_free(h);
#else
		iptc_free(&h);
#endif
	return r;
}

/* delete_rule_and_commit() :
 * subfunction used in delete_redirect_and_filter_rules()
 * always call iptc_free(h) */
static int
delete_rule_and_commit(unsigned int index, IPTC_HANDLE h,
                       const char * miniupnpd_chain,
                       const char * logcaller)
{
	int r = 0;
#ifdef IPTABLES_143
	if(!iptc_delete_num_entry(miniupnpd_chain, index, h))
#else
	if(!iptc_delete_num_entry(miniupnpd_chain, index, &h))
#endif
	{
		syslog(LOG_ERR, "%s() : iptc_delete_num_entry(): %s\n",
	    	   logcaller, iptc_strerror(errno));
		r = -1;
	}
#ifdef IPTABLES_143
	else if(!iptc_commit(h))
#else
	else if(!iptc_commit(&h))
#endif
	{
		syslog(LOG_ERR, "%s() : iptc_commit(): %s\n",
	    	   logcaller, iptc_strerror(errno));
		r = -1;
	}
	if(h)
#ifdef IPTABLES_143
		iptc_free(h);
#else
		iptc_free(&h);
#endif
	return r;
}

/* delete_filter_rule()
 */
int
delete_filter_rule(const char * ifname, unsigned short port, int proto)
{
	int r = -1;
	unsigned index = 0;
	unsigned i = 0;
	IPTC_HANDLE h;
	const struct ipt_entry * e;
	const struct ipt_entry_match *match;
	UNUSED(ifname);

	if((h = iptc_init("filter")))
	{
		i = 0;
		/* we must find the right index for the filter rule */
#ifdef IPTABLES_143
		for(e = iptc_first_rule(miniupnpd_forward_chain, h);
		    e;
			e = iptc_next_rule(e, h), i++)
#else
		for(e = iptc_first_rule(miniupnpd_forward_chain, &h);
		    e;
			e = iptc_next_rule(e, &h), i++)
#endif
		{
			if(proto==e->ip.proto)
			{
				match = (const struct ipt_entry_match *)&e->elems;
				/*syslog(LOG_DEBUG, "filter rule #%u: %s %s",
				       i, match->u.user.name, inet_ntoa(e->ip.dst));*/
				if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
				{
					const struct ipt_tcp * info;
					info = (const struct ipt_tcp *)match->data;
					if(port != info->dpts[0])
						continue;
				}
				else
				{
					const struct ipt_udp * info;
					info = (const struct ipt_udp *)match->data;
					if(port != info->dpts[0])
						continue;
				}
				index = i;
				/*syslog(LOG_INFO, "Trying to delete filter rule at index %u", index);*/
				r = delete_rule_and_commit(index, h, miniupnpd_forward_chain, "delete_filter_rule");
				h = NULL;
				break;
			}
		}
	}
	if(h)
#ifdef IPTABLES_143
		iptc_free(h);
#else
		iptc_free(&h);
#endif
	return r;
}

/* delete_redirect_and_filter_rules()
 */
int
delete_redirect_and_filter_rules(unsigned short eport, int proto)
{
	int r = -1, r2 = -1;
	unsigned index = 0;
	unsigned i = 0;
	IPTC_HANDLE h;
	const struct ipt_entry * e;
	const struct ipt_entry_target * target;
	const struct ip_nat_multi_range * mr;
	const struct ipt_entry_match *match;
	unsigned short iport = 0;
	uint32_t iaddr = 0;

	h = iptc_init("nat");
	if(!h)
	{
		syslog(LOG_ERR, "%s() : iptc_init() failed : %s",
		       "delete_redirect_and_filter_rules", iptc_strerror(errno));
		return -1;
	}
	/* First step : find the right nat rule */
	if(!iptc_is_chain(miniupnpd_nat_chain, h))
	{
		syslog(LOG_ERR, "chain %s not found", miniupnpd_nat_chain);
	}
	else
	{
#ifdef IPTABLES_143
		for(e = iptc_first_rule(miniupnpd_nat_chain, h);
		    e;
			e = iptc_next_rule(e, h), i++)
#else
		for(e = iptc_first_rule(miniupnpd_nat_chain, &h);
		    e;
			e = iptc_next_rule(e, &h), i++)
#endif
		{
			if(proto==e->ip.proto)
			{
				match = (const struct ipt_entry_match *)&e->elems;
				if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
				{
					const struct ipt_tcp * info;
					info = (const struct ipt_tcp *)match->data;
					if(eport != info->dpts[0])
						continue;
				}
				else
				{
					const struct ipt_udp * info;
					info = (const struct ipt_udp *)match->data;
					if(eport != info->dpts[0])
						continue;
				}
				/* get the index, the internal address and the internal port
				 * of the rule */
				index = i;
				target = (void *)e + e->target_offset;
				mr = (const struct ip_nat_multi_range *)&target->data[0];
				iaddr = mr->range[0].min_ip;
				iport = ntohs(mr->range[0].min.all);
				r = 0;
				break;
			}
		}
	}
	if(h)
#ifdef IPTABLES_143
		iptc_free(h);
#else
		iptc_free(&h);
#endif
	if(r == 0)
	{
		syslog(LOG_INFO, "Trying to delete nat rule at index %u", index);
		/* Now delete both rules */
		/* first delete the nat rule */
		h = iptc_init("nat");
		if(h)
		{
			r = delete_rule_and_commit(index, h, miniupnpd_nat_chain, "delete_redirect_rule");
		}
		if((r == 0) && (h = iptc_init("filter")))
		{
			i = 0;
			/* we must find the right index for the filter rule */
#ifdef IPTABLES_143
			for(e = iptc_first_rule(miniupnpd_forward_chain, h);
			    e;
				e = iptc_next_rule(e, h), i++)
#else
			for(e = iptc_first_rule(miniupnpd_forward_chain, &h);
			    e;
				e = iptc_next_rule(e, &h), i++)
#endif
			{
				if(proto==e->ip.proto)
				{
					match = (const struct ipt_entry_match *)&e->elems;
					/*syslog(LOG_DEBUG, "filter rule #%u: %s %s",
					       i, match->u.user.name, inet_ntoa(e->ip.dst));*/
					if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
					{
						const struct ipt_tcp * info;
						info = (const struct ipt_tcp *)match->data;
						if(iport != info->dpts[0])
							continue;
					}
					else
					{
						const struct ipt_udp * info;
						info = (const struct ipt_udp *)match->data;
						if(iport != info->dpts[0])
							continue;
					}
					if(iaddr != e->ip.dst.s_addr)
						continue;
					index = i;
					syslog(LOG_INFO, "Trying to delete filter rule at index %u", index);
					r = delete_rule_and_commit(index, h, miniupnpd_forward_chain, "delete_filter_rule");
					h = NULL;
					break;
				}
			}
			/* in case the filter rule has not been found, delete_rule_and_commit() is not called
			 * so we neet to free h */
			if(h)
#ifdef IPTABLES_143
				iptc_free(h);
#else
				iptc_free(&h);
#endif
		}
	}

	/*delete PEER rule*/
	if((h = iptc_init("nat")))
	{
		i = 0;
		/* we must find the right index for the filter rule */
#ifdef IPTABLES_143
		for(e = iptc_first_rule(miniupnpd_nat_postrouting_chain, h);
		    e;
			e = iptc_next_rule(e, h), i++)
#else
		for(e = iptc_first_rule(miniupnpd_nat_postrouting_chain, &h);
		    e;
			e = iptc_next_rule(e, &h), i++)
#endif
		{
			if(proto==e->ip.proto)
			{
				target = (void *)e + e->target_offset;
				mr = (const struct ip_nat_multi_range *)&target->data[0];
				syslog(LOG_DEBUG, "postrouting rule #%u: %s %s %hu",
				       i, target->u.user.name, inet_ntoa(e->ip.src), ntohs(mr->range[0].min.all));
				/* target->u.user.name SNAT / MASQUERADE */
				if (eport != ntohs(mr->range[0].min.all)) {
					continue;
				}
				iaddr = e->ip.src.s_addr;
				match = (const struct ipt_entry_match *)&e->elems;
				if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
				{
					const struct ipt_tcp * info;
					info = (const struct ipt_tcp *)match->data;
					iport = info->spts[0];
				}
				else
				{
					const struct ipt_udp * info;
					info = (const struct ipt_udp *)match->data;
					iport = info->spts[0];
				}

				index = i;
				syslog(LOG_INFO, "Trying to delete peer rule at index %u", index);
				r2 = delete_rule_and_commit(index, h, miniupnpd_nat_postrouting_chain, "delete_peer_rule");
				h = NULL;
				break;
			}
		}
	}

	if(h)
#ifdef IPTABLES_143
		iptc_free(h);
#else
		iptc_free(&h);
#endif
	/*delete DSCP rule*/
	if((r2==0)&&(h = iptc_init("mangle")))
	{
		i = 0;
		index = -1;
		/* we must find the right index for the filter rule */
#ifdef IPTABLES_143
		for(e = iptc_first_rule(miniupnpd_nat_chain, h);
		    e;
			e = iptc_next_rule(e, h), i++)
#else
		for(e = iptc_first_rule(miniupnpd_nat_chain, &h);
		    e;
			e = iptc_next_rule(e, &h), i++)
#endif
		{
			if(proto==e->ip.proto)
			{
				match = (const struct ipt_entry_match *)&e->elems;
				/*syslog(LOG_DEBUG, "filter rule #%u: %s %s",
				       i, match->u.user.name, inet_ntoa(e->ip.dst));*/
				if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
				{
					const struct ipt_tcp * info;
					info = (const struct ipt_tcp *)match->data;
					if(iport != info->spts[0])
						continue;
				}
				else
				{
					const struct ipt_udp * info;
					info = (const struct ipt_udp *)match->data;
					if(iport != info->spts[0])
						continue;
				}
				if(iaddr != e->ip.src.s_addr)
					continue;
				index = i;
				syslog(LOG_INFO, "Trying to delete dscp rule at index %u", index);
				r2 = delete_rule_and_commit(index, h, miniupnpd_nat_chain, "delete_dscp_rule");
				h = NULL;
				break;
			}
		}
		if (h)
#ifdef IPTABLES_143
			iptc_free(h);
#else
			iptc_free(&h);
#endif
	}

	del_redirect_desc(eport, proto);
	return r*r2;
}

/* ==================================== */
/* TODO : add the -m state --state NEW,ESTABLISHED,RELATED
 * only for the filter rule */
static struct ipt_entry_match *
get_tcp_match(unsigned short dport, unsigned short sport)
{
	struct ipt_entry_match *match;
	struct ipt_tcp * tcpinfo;
	size_t size;
	size =   IPT_ALIGN(sizeof(struct ipt_entry_match))
	       + IPT_ALIGN(sizeof(struct ipt_tcp));
	match = calloc(1, size);
	match->u.match_size = size;
	strncpy(match->u.user.name, "tcp", sizeof(match->u.user.name));
	tcpinfo = (struct ipt_tcp *)match->data;
	if (sport == 0) {
		tcpinfo->spts[0] = 0;		/* all source ports */
		tcpinfo->spts[1] = 0xFFFF;
	} else {
		tcpinfo->spts[0] = sport;	   /* specified source port */
		tcpinfo->spts[1] = sport;
	}
	if (dport == 0) {
		tcpinfo->dpts[0] = 0;	   /* all destination ports */
		tcpinfo->dpts[1] = 0xFFFF;
	} else {
		tcpinfo->dpts[0] = dport;	/* specified destination port */
		tcpinfo->dpts[1] = dport;
	}
	return match;
}

static struct ipt_entry_match *
get_udp_match(unsigned short dport, unsigned short sport)
{
	struct ipt_entry_match *match;
	struct ipt_udp * udpinfo;
	size_t size;
	size =   IPT_ALIGN(sizeof(struct ipt_entry_match))
	       + IPT_ALIGN(sizeof(struct ipt_udp));
	match = calloc(1, size);
	match->u.match_size = size;
	strncpy(match->u.user.name, "udp", sizeof(match->u.user.name));
	udpinfo = (struct ipt_udp *)match->data;
	if (sport == 0) {
		udpinfo->spts[0] = 0;	   /* all source ports */
		udpinfo->spts[1] = 0xFFFF;
	} else {
		udpinfo->spts[0] = sport;	   /* specified source port */
		udpinfo->spts[1] = sport;
	}
	if (dport == 0) {
		udpinfo->dpts[0] = 0;	   /* all destination ports */
		udpinfo->dpts[1] = 0xFFFF;
	} else {
		udpinfo->dpts[0] = dport;   /* specified destination port */
		udpinfo->dpts[1] = dport;
	}
	return match;
}

static struct ipt_entry_target *
get_dnat_target(const char * daddr, unsigned short dport)
{
	struct ipt_entry_target * target;
	struct ip_nat_multi_range * mr;
	struct ip_nat_range * range;
	size_t size;

	size =   IPT_ALIGN(sizeof(struct ipt_entry_target))
	       + IPT_ALIGN(sizeof(struct ip_nat_multi_range));
	target = calloc(1, size);
	target->u.target_size = size;
	strncpy(target->u.user.name, "DNAT", sizeof(target->u.user.name));
	/* one ip_nat_range already included in ip_nat_multi_range */
	mr = (struct ip_nat_multi_range *)&target->data[0];
	mr->rangesize = 1;
	range = &mr->range[0];
	range->min_ip = range->max_ip = inet_addr(daddr);
	range->flags |= IP_NAT_RANGE_MAP_IPS;
	range->min.all = range->max.all = htons(dport);
	range->flags |= IP_NAT_RANGE_PROTO_SPECIFIED;
	return target;
}

static struct ipt_entry_target *
get_snat_target(const char * saddr, unsigned short sport)
{
	struct ipt_entry_target * target;
	struct ip_nat_multi_range * mr;
	struct ip_nat_range * range;
	size_t size;

	size =   IPT_ALIGN(sizeof(struct ipt_entry_target))
	       + IPT_ALIGN(sizeof(struct ip_nat_multi_range));
	target = calloc(1, size);
	target->u.target_size = size;
	strncpy(target->u.user.name, "SNAT", sizeof(target->u.user.name));
	/* one ip_nat_range already included in ip_nat_multi_range */
	mr = (struct ip_nat_multi_range *)&target->data[0];
	mr->rangesize = 1;
	range = &mr->range[0];
	range->min_ip = range->max_ip = inet_addr(saddr);
	range->flags |= IP_NAT_RANGE_MAP_IPS;
	range->min.all = range->max.all = htons(sport);
	range->flags |= IP_NAT_RANGE_PROTO_SPECIFIED;
	return target;
}

static struct ipt_entry_target *
get_dscp_target(unsigned char dscp)
{
	struct ipt_entry_target * target;
	struct xt_DSCP_info * di;
	size_t size;

	size =   IPT_ALIGN(sizeof(struct ipt_entry_target))
	       + IPT_ALIGN(sizeof(struct xt_DSCP_info));
	target = calloc(1, size);
	target->u.target_size = size;
	strncpy(target->u.user.name, "DSCP", sizeof(target->u.user.name));
	/* one ip_nat_range already included in ip_nat_multi_range */
	di = (struct xt_DSCP_info *)&target->data[0];
	di->dscp=dscp;
	return target;
}

#ifdef ENABLE_PORT_TRIGGERING
static struct ipt_entry_target *
get_masquerade_target(unsigned short port)
{
	struct ipt_entry_target * target;
	struct ip_nat_multi_range * mr;
	struct ip_nat_range * range;
	size_t size;

	size =   IPT_ALIGN(sizeof(struct ipt_entry_target))
	       + IPT_ALIGN(sizeof(struct ip_nat_multi_range));
	target = calloc(1, size);
	target->u.target_size = size;
	strncpy(target->u.user.name, "MASQUERADE", sizeof(target->u.user.name));
	/* one ip_nat_range already included in ip_nat_multi_range */
	mr = (struct ip_nat_multi_range *)&target->data[0];
	mr->rangesize = 1;
	range = &mr->range[0];
	range->min.tcp.port = range->max.tcp.port = htons(port);
	/*range->min.all = range->max.all = htons(port);*/
	range->flags |= IP_NAT_RANGE_PROTO_SPECIFIED;
	return target;
}
#endif /* ENABLE_PORT_TRIGGERING */

/* iptc_init_verify_and_append()
 * return 0 on success, -1 on failure */
static int
iptc_init_verify_and_append(const char * table,
                            const char * miniupnpd_chain,
                            struct ipt_entry * e,
                            const char * logcaller)
{
	IPTC_HANDLE h;
	h = iptc_init(table);
	if(!h)
	{
		syslog(LOG_ERR, "%s() : iptc_init() error : %s\n",
		       logcaller, iptc_strerror(errno));
		return -1;
	}
	if(!iptc_is_chain(miniupnpd_chain, h))
	{
		syslog(LOG_ERR, "%s() : chain %s not found",
		       logcaller, miniupnpd_chain);
		if(h)
#ifdef IPTABLES_143
			iptc_free(h);
#else
			iptc_free(&h);
#endif
		return -1;
	}
	/* iptc_insert_entry(miniupnpd_chain, e, n, h/&h) could also be used */
#ifdef IPTABLES_143
	if(!iptc_append_entry(miniupnpd_chain, e, h))
#else
	if(!iptc_append_entry(miniupnpd_chain, e, &h))
#endif
	{
		syslog(LOG_ERR, "%s() : iptc_append_entry() error : %s\n",
		       logcaller, iptc_strerror(errno));
		if(h)
#ifdef IPTABLES_143
			iptc_free(h);
#else
			iptc_free(&h);
#endif
		return -1;
	}
#ifdef IPTABLES_143
	if(!iptc_commit(h))
#else
	if(!iptc_commit(&h))
#endif
	{
		syslog(LOG_ERR, "%s() : iptc_commit() error : %s\n",
		       logcaller, iptc_strerror(errno));
		if(h)
#ifdef IPTABLES_143
			iptc_free(h);
#else
			iptc_free(&h);
#endif
		return -1;
	}
	if(h)
#ifdef IPTABLES_143
		iptc_free(h);
#else
		iptc_free(&h);
#endif
	return 0;
}

/* add nat rule
 * iptables -t nat -A MINIUPNPD -p <proto> [-s <rhost>] --dport <eport> -j DNAT --to <iaddr>:<iport>
 * */
static int
addnatrule(int proto, unsigned short eport,
           const char * iaddr, unsigned short iport,
           const char * rhost)
{
	int r = 0;
	struct ipt_entry * e;
	struct ipt_entry * tmp;
	struct ipt_entry_match *match = NULL;
	struct ipt_entry_target *target = NULL;

	e = calloc(1, sizeof(struct ipt_entry));
	if(!e) {
		syslog(LOG_ERR, "%s: calloc(%d) error", "addnatrule",
		       (int)sizeof(struct ipt_entry));
		return -1;
	}
	e->ip.proto = proto;
	if(proto == IPPROTO_TCP) {
		match = get_tcp_match(eport, 0);
	} else {
		match = get_udp_match(eport, 0);
	}
#ifdef NFC_UNKNOWN
	e->nfcache = NFC_UNKNOWN;
#endif
	target = get_dnat_target(iaddr, iport);
#ifdef NFC_IP_DST_PT
	e->nfcache |= NFC_IP_DST_PT;
#endif
	tmp = realloc(e, sizeof(struct ipt_entry)
	               + match->u.match_size
				   + target->u.target_size);
	if(!tmp) {
		syslog(LOG_ERR, "%s: realloc(%d) error", "addnatrule",
		       (int)(sizeof(struct ipt_entry) + match->u.match_size + target->u.target_size));
		free(e);
		free(match);
		free(target);
		return -1;
	}
	e = tmp;
	memcpy(e->elems, match, match->u.match_size);
	memcpy(e->elems + match->u.match_size, target, target->u.target_size);
	e->target_offset = sizeof(struct ipt_entry)
	                   + match->u.match_size;
	e->next_offset = sizeof(struct ipt_entry)
	                 + match->u.match_size
					 + target->u.target_size;
	/* remote host */
	if(rhost && (rhost[0] != '\0') && (0 != strcmp(rhost, "*"))) {
		e->ip.src.s_addr = inet_addr(rhost);
		e->ip.smsk.s_addr = INADDR_NONE;
	}

	r = iptc_init_verify_and_append("nat", miniupnpd_nat_chain, e, "addnatrule");
	free(target);
	free(match);
	free(e);
	return r;
}

/* for "Port Triggering"
 * Section 2.5.16 figure 2.2 in UPnP-gw-WANIPConnection-v2-Service.pdf
 *
 *   When a control point creates a port forwarding rule with AddPortMapping()
 *   action for inbound traffic , this rule MUST also be applied when NAT port
 *   triggering occurs for outbound traffic.
 *
 * iptables -t nat -A MINIUPNPD-POSTROUTING {-o <extif>} -s <iaddr>
 *          -p <proto> [-d <rhost>] --sport <iport> -j MASQUERADE --to-ports <eport>
 */
#ifdef ENABLE_PORT_TRIGGERING
static int
addmasqueraderule(int proto,
           unsigned short eport,
           const char * iaddr, unsigned short iport,
           const char * rhost/*, const char * extif*/)
{
	int r = 0;
	struct ipt_entry * e;
	struct ipt_entry * tmp;
	struct ipt_entry_match *match = NULL;
	struct ipt_entry_target *target = NULL;

	e = calloc(1, sizeof(struct ipt_entry));
	if(!e) {
		syslog(LOG_ERR, "%s: calloc(%d) error", "addmasqueraderule",
		       (int)sizeof(struct ipt_entry));
		return -1;
	}
	e->ip.proto = proto;
	if(proto == IPPROTO_TCP) {
		match = get_tcp_match(0, iport);
	} else {
		match = get_udp_match(0, iport);
	}
#ifdef NFC_UNKNOWN
	e->nfcache = NFC_UNKNOWN;
#endif
	target = get_masquerade_target(eport);
#ifdef NFC_IP_DST_PT
	e->nfcache |= NFC_IP_DST_PT;
#endif
	tmp = realloc(e, sizeof(struct ipt_entry)
	               + match->u.match_size
				   + target->u.target_size);
	if(!tmp) {
		syslog(LOG_ERR, "%s: realloc(%d) error", "addmasqueraderule",
		       (int)(sizeof(struct ipt_entry) + match->u.match_size + target->u.target_size));
		free(e);
		free(match);
		free(target);
		return -1;
	}
	e = tmp;
	memcpy(e->elems, match, match->u.match_size);
	memcpy(e->elems + match->u.match_size, target, target->u.target_size);
	e->target_offset = sizeof(struct ipt_entry)
	                   + match->u.match_size;
	e->next_offset = sizeof(struct ipt_entry)
	                 + match->u.match_size
					 + target->u.target_size;
#if 0
	/* do not add outiface (-o) to rule, as the MINIUPNPD-POSTROUTING chain
	 * should already have matched it */
	if(extif != NULL) {
		strncpy(e->ip.outiface, extif, sizeof(e->ip.outiface));
		memset(e->ip.outiface_mask, 0xff, strlen(e->ip.outiface) + 1);/* Include nul-terminator in match */
	}
#endif
	/* internal host */
	if(iaddr && (iaddr[0] != '\0') && (0 != strcmp(iaddr, "*")))
	{
		e->ip.src.s_addr = inet_addr(iaddr);
		e->ip.smsk.s_addr = INADDR_NONE;
	}
	/* remote host */
	if(rhost && (rhost[0] != '\0') && (0 != strcmp(rhost, "*"))) {
		e->ip.dst.s_addr = inet_addr(rhost);
		e->ip.dmsk.s_addr = INADDR_NONE;
	}

	r = iptc_init_verify_and_append("nat", miniupnpd_nat_postrouting_chain, e, "addmasqueraderule");
	free(target);
	free(match);
	free(e);
	return r;
}
#endif /* ENABLE_PORT_TRIGGERING */

/* called by add_peer_redirect_rule2()
 *
 * iptables -t nat -A MINIUPNPD-POSTROUTING -s <iaddr> -d <rhost>
 *    -p <proto> --sport <iport> --dport <rport> -j SNAT
 *    --to-source <eaddr>:<eport> */
static int
addpeernatrule(int proto,
           const char * eaddr, unsigned short eport,
           const char * iaddr, unsigned short iport,
           const char * rhost, unsigned short rport)
{
	int r = 0;
	struct ipt_entry * e;
	struct ipt_entry * tmp;
	struct ipt_entry_match *match = NULL;
	struct ipt_entry_target *target = NULL;

	e = calloc(1, sizeof(struct ipt_entry));
	if(!e) {
		syslog(LOG_ERR, "%s: calloc(%d) error", "addpeernatrule",
		       (int)sizeof(struct ipt_entry));
		return -1;
	}
	e->ip.proto = proto;
	/* TODO: Fill port matches and SNAT */
	if(proto == IPPROTO_TCP) {
		match = get_tcp_match(rport, iport);
	} else {
		match = get_udp_match(rport, iport);
	}
#ifdef NFC_UNKNOWN
	e->nfcache = NFC_UNKNOWN;
#endif
	target = get_snat_target(eaddr, eport);
#ifdef NFC_IP_DST_PT
	e->nfcache |= NFC_IP_DST_PT;
#endif
#ifdef NFC_IP_SRC_PT
	e->nfcache |= NFC_IP_SRC_PT;
#endif
	tmp = realloc(e, sizeof(struct ipt_entry)
	               + match->u.match_size
				   + target->u.target_size);
	if(!tmp) {
		syslog(LOG_ERR, "%s: realloc(%d) error", "addpeernatrule",
		       (int)(sizeof(struct ipt_entry) + match->u.match_size + target->u.target_size));
		free(e);
		free(match);
		free(target);
		return -1;
	}
	e = tmp;
	memcpy(e->elems, match, match->u.match_size);
	memcpy(e->elems + match->u.match_size, target, target->u.target_size);
	e->target_offset = sizeof(struct ipt_entry)
	                   + match->u.match_size;
	e->next_offset = sizeof(struct ipt_entry)
	                 + match->u.match_size
					 + target->u.target_size;

	/* internal host */
	if(iaddr && (iaddr[0] != '\0') && (0 != strcmp(iaddr, "*")))
	{
		e->ip.src.s_addr = inet_addr(iaddr);
		e->ip.smsk.s_addr = INADDR_NONE;
	}
	/* remote host */
	if(rhost && (rhost[0] != '\0') && (0 != strcmp(rhost, "*")))
	{
		e->ip.dst.s_addr = inet_addr(rhost);
		e->ip.dmsk.s_addr = INADDR_NONE;
	}

	r = iptc_init_verify_and_append("nat", miniupnpd_nat_postrouting_chain, e, "addpeernatrule");
	free(target);
	free(match);
	free(e);
	return r;
}

/* called by add_peer_dscp_rule2()
 * iptables -t mangle -A MINIUPNPD -s <iaddr> -d <rhost>
 *    -p <proto> --sport <iport> --dport <rport> -j DSCP
 *    --set-dscp 0xXXXX                   */
static int
addpeerdscprule(int proto, unsigned char dscp,
           const char * iaddr, unsigned short iport,
           const char * rhost, unsigned short rport)
{
	int r = 0;
	struct ipt_entry * e;
	struct ipt_entry * tmp;
	struct ipt_entry_match *match = NULL;
	struct ipt_entry_target *target = NULL;

	e = calloc(1, sizeof(struct ipt_entry));
	if(!e) {
		syslog(LOG_ERR, "%s: calloc(%d) error", "addpeerdscprule",
		       (int)sizeof(struct ipt_entry));
		return -1;
	}
	e->ip.proto = proto;
	/* TODO: Fill port matches and SNAT */
	if(proto == IPPROTO_TCP) {
		match = get_tcp_match(rport, iport);
	} else {
		match = get_udp_match(rport, iport);
	}
#ifdef NFC_UNKNOWN
	e->nfcache = NFC_UNKNOWN;
#endif
	target = get_dscp_target(dscp);
#ifdef NFC_IP_DST_PT
	e->nfcache |= NFC_IP_DST_PT;
#endif
#ifdef NFC_IP_SRC_PT
	e->nfcache |= NFC_IP_SRC_PT;
#endif
	tmp = realloc(e, sizeof(struct ipt_entry)
	               + match->u.match_size
				   + target->u.target_size);
	if(!tmp) {
		syslog(LOG_ERR, "%s: realloc(%d) error", "addpeerdscprule",
		       (int)(sizeof(struct ipt_entry) + match->u.match_size + target->u.target_size));
		free(e);
		free(match);
		free(target);
		return -1;
	}
	e = tmp;
	memcpy(e->elems, match, match->u.match_size);
	memcpy(e->elems + match->u.match_size, target, target->u.target_size);
	e->target_offset = sizeof(struct ipt_entry)
	                   + match->u.match_size;
	e->next_offset = sizeof(struct ipt_entry)
	                 + match->u.match_size
					 + target->u.target_size;

	/* internal host */
	if(iaddr && (iaddr[0] != '\0') && (0 != strcmp(iaddr, "*")))
	{
		e->ip.src.s_addr = inet_addr(iaddr);
		e->ip.smsk.s_addr = INADDR_NONE;
	}
	/* remote host */
	if(rhost && (rhost[0] != '\0') && (0 != strcmp(rhost, "*")))
	{
		e->ip.dst.s_addr = inet_addr(rhost);
		e->ip.dmsk.s_addr = INADDR_NONE;
	}

	r = iptc_init_verify_and_append("mangle", miniupnpd_nat_chain, e,
	                                "addpeerDSCPrule");
	free(target);
	free(match);
	free(e);
	return r;
}


/* ================================= */
static struct ipt_entry_target *
get_accept_target(void)
{
	struct ipt_entry_target * target = NULL;
	size_t size;
	size =   IPT_ALIGN(sizeof(struct ipt_entry_target))
	       + IPT_ALIGN(sizeof(int));
	target = calloc(1, size);
	target->u.user.target_size = size;
	strncpy(target->u.user.name, "ACCEPT", sizeof(target->u.user.name));
	return target;
}

/* add_filter_rule()
 * iptables -t filter -A MINIUPNPD [-s <rhost>] -p <proto> -d <iaddr> --dport <iport> -j ACCEPT */
static int
add_filter_rule(int proto, const char * rhost,
                const char * iaddr, unsigned short iport)
{
	int r = 0;
	struct ipt_entry * e;
	struct ipt_entry * tmp;
	struct ipt_entry_match *match = NULL;
	struct ipt_entry_target *target = NULL;

	e = calloc(1, sizeof(struct ipt_entry));
	if(!e) {
		syslog(LOG_ERR, "%s: calloc(%d) error", "add_filter_rule",
		       (int)sizeof(struct ipt_entry));
		return -1;
	}
	e->ip.proto = proto;
	if(proto == IPPROTO_TCP) {
		match = get_tcp_match(iport,0);
	} else {
		match = get_udp_match(iport,0);
	}
	e->ip.dst.s_addr = inet_addr(iaddr);
	e->ip.dmsk.s_addr = INADDR_NONE;
#ifdef NFC_UNKNOWN
	e->nfcache = NFC_UNKNOWN;
#endif
	target = get_accept_target();
#ifdef NFC_IP_DST_PT
	e->nfcache |= NFC_IP_DST_PT;
#endif
	tmp = realloc(e, sizeof(struct ipt_entry)
	               + match->u.match_size
				   + target->u.target_size);
	if(!tmp) {
		syslog(LOG_ERR, "%s: realloc(%d) error", "add_filter_rule",
		       (int)(sizeof(struct ipt_entry) + match->u.match_size + target->u.target_size));
		free(e);
		free(match);
		free(target);
		return -1;
	}
	e = tmp;
	memcpy(e->elems, match, match->u.match_size);
	memcpy(e->elems + match->u.match_size, target, target->u.target_size);
	e->target_offset = sizeof(struct ipt_entry)
	                   + match->u.match_size;
	e->next_offset = sizeof(struct ipt_entry)
	                 + match->u.match_size
					 + target->u.target_size;
	/* remote host */
	if(rhost && (rhost[0] != '\0') && (0 != strcmp(rhost, "*")))
	{
		e->ip.src.s_addr = inet_addr(rhost);
		e->ip.smsk.s_addr = INADDR_NONE;
	}

	r = iptc_init_verify_and_append("filter", miniupnpd_forward_chain, e, "add_filter_rule");
	free(target);
	free(match);
	free(e);
	return r;
}

/* return an (malloc'ed) array of "external" port for which there is
 * a port mapping. number is the size of the array */
unsigned short *
get_portmappings_in_range(unsigned short startport, unsigned short endport,
                          int proto, unsigned int * number)
{
	unsigned short * array;
	unsigned int capacity;
	unsigned short eport;
	IPTC_HANDLE h;
	const struct ipt_entry * e;
	const struct ipt_entry_match *match;

	*number = 0;
	capacity = 128;
	array = calloc(capacity, sizeof(unsigned short));
	if(!array)
	{
		syslog(LOG_ERR, "%s() : calloc error", "get_portmappings_in_range");
		return NULL;
	}

	h = iptc_init("nat");
	if(!h)
	{
		syslog(LOG_ERR, "%s() : iptc_init() failed : %s",
		       "get_portmappings_in_range", iptc_strerror(errno));
		free(array);
		return NULL;
	}
	if(!iptc_is_chain(miniupnpd_nat_chain, h))
	{
		syslog(LOG_ERR, "chain %s not found", miniupnpd_nat_chain);
		free(array);
		array = NULL;
	}
	else
	{
#ifdef IPTABLES_143
		for(e = iptc_first_rule(miniupnpd_nat_chain, h);
		    e;
			e = iptc_next_rule(e, h))
#else
		for(e = iptc_first_rule(miniupnpd_nat_chain, &h);
		    e;
			e = iptc_next_rule(e, &h))
#endif
		{
			if(proto == e->ip.proto)
			{
				match = (const struct ipt_entry_match *)&e->elems;
				if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
				{
					const struct ipt_tcp * info;
					info = (const struct ipt_tcp *)match->data;
					eport = info->dpts[0];
				}
				else
				{
					const struct ipt_udp * info;
					info = (const struct ipt_udp *)match->data;
					eport = info->dpts[0];
				}
				if(startport <= eport && eport <= endport)
				{
					if(*number >= capacity)
					{
						unsigned short * tmp;
						/* need to increase the capacity of the array */
						capacity += 128;
						if (capacity <= *number)
							capacity = *number + 1;
						tmp = realloc(array, sizeof(unsigned short)*capacity);
						if(!tmp)
						{
							syslog(LOG_ERR, "get_portmappings_in_range() : realloc(%u) error",
							       (unsigned)sizeof(unsigned short)*capacity);
							*number = 0;
							free(array);
							array = NULL;
							break;
						}
						array = tmp;
					}
					array[*number] = eport;
					(*number)++;
				}
			}
		}
	}
	if(h)
#ifdef IPTABLES_143
		iptc_free(h);
#else
		iptc_free(&h);
#endif
	return array;
}

int
update_portmapping_desc_timestamp(const char * ifname,
                   unsigned short eport, int proto,
                   const char * desc, unsigned int timestamp)
{
	UNUSED(ifname);
	del_redirect_desc(eport, proto);
	add_redirect_desc(eport, proto, desc, timestamp);
	return 0;
}

static int
update_rule_and_commit(const char * table, const char * chain,
                       unsigned index, const struct ipt_entry * e)
{
	IPTC_HANDLE h;
	int r = 0;

	h = iptc_init(table);
	if(!h)
	{
		syslog(LOG_ERR, "%s() : iptc_init() failed : %s",
		       "update_rule_and_commit", iptc_strerror(errno));
		return -1;
	}
#ifdef IPTABLES_143
	if(!iptc_replace_entry(chain, e, index, h))
#else
	if(!iptc_replace_entry(chain, e, index, &h))
#endif
	{
		syslog(LOG_ERR, "%s(): iptc_replace_entry: %s",
		       "update_rule_and_commit", iptc_strerror(errno));
		r = -1;
	}
#ifdef IPTABLES_143
	else if(!iptc_commit(h))
#else
	else if(!iptc_commit(&h))
#endif
	{
		syslog(LOG_ERR, "%s(): iptc_commit: %s",
		       "update_rule_and_commit", iptc_strerror(errno));
		r = -1;
	}
#ifdef IPTABLES_143
	iptc_free(h);
#else
	iptc_free(&h);
#endif
	return r;
}

int
update_portmapping(const char * ifname, unsigned short eport, int proto,
                   unsigned short iport, const char * desc,
                   unsigned int timestamp)
{
	int r = 0;
	int found = 0;
	unsigned index = 0;
	unsigned i = 0;
	IPTC_HANDLE h;
	const struct ipt_entry * e;
	struct ipt_entry * new_e = NULL;
	size_t entry_len;
	struct ipt_entry_target * target;
	struct ip_nat_multi_range * mr;
	const struct ipt_entry_match *match;
	uint32_t iaddr = 0;
	unsigned short old_iport = 0;

	h = iptc_init("nat");
	if(!h)
	{
		syslog(LOG_ERR, "%s() : iptc_init() failed : %s",
		       "update_portmapping", iptc_strerror(errno));
		return -1;
	}
	/* First step : find the right nat rule */
	if(!iptc_is_chain(miniupnpd_nat_chain, h))
	{
		syslog(LOG_ERR, "chain %s not found", miniupnpd_nat_chain);
		r = -1;
	}
	else
	{
#ifdef IPTABLES_143
		for(e = iptc_first_rule(miniupnpd_nat_chain, h);
		    e;
			e = iptc_next_rule(e, h), i++)
#else
		for(e = iptc_first_rule(miniupnpd_nat_chain, &h);
		    e;
			e = iptc_next_rule(e, &h), i++)
#endif
		{
			if(proto==e->ip.proto)
			{
				match = (const struct ipt_entry_match *)&e->elems;
				if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
				{
					const struct ipt_tcp * info;
					info = (const struct ipt_tcp *)match->data;
					if(eport != info->dpts[0])
						continue;
				}
				else
				{
					const struct ipt_udp * info;
					info = (const struct ipt_udp *)match->data;
					if(eport != info->dpts[0])
						continue;
				}
				/* we found the right rule */
				found = 1;
				index = i;
				target = (void *)e + e->target_offset;
				mr = (struct ip_nat_multi_range *)&target->data[0];
				iaddr = mr->range[0].min_ip;
				old_iport = ntohs(mr->range[0].min.all);
				entry_len = sizeof(struct ipt_entry) + match->u.match_size + target->u.target_size;
				new_e = malloc(entry_len);
				if(new_e == NULL) {
					syslog(LOG_ERR, "%s: malloc(%u) error",
					       "update_portmapping", (unsigned)entry_len);
					r = -1;
				}
				else
				{
					memcpy(new_e, e, entry_len);
				}
				break;
			}
		}
	}
#ifdef IPTABLES_143
	iptc_free(h);
#else
	iptc_free(&h);
#endif
	if(!found || r < 0)
		return -1;
	syslog(LOG_INFO, "Trying to update nat rule at index %u", index);
	target = (void *)new_e + new_e->target_offset;
	mr = (struct ip_nat_multi_range *)&target->data[0];
	mr->range[0].min.all = mr->range[0].max.all = htons(iport);
	/* first update the nat rule */
	r = update_rule_and_commit("nat", miniupnpd_nat_chain, index, new_e);
	free(new_e); new_e = NULL;
	if(r < 0)
		return r;

	/* update filter rule */
	h = iptc_init("filter");
	if(!h)
	{
		syslog(LOG_ERR, "%s() : iptc_init() failed : %s",
		       "update_portmapping", iptc_strerror(errno));
		return -1;
	}
	i = 0; found = 0;
	if(!iptc_is_chain(miniupnpd_forward_chain, h))
	{
		syslog(LOG_ERR, "chain %s not found", miniupnpd_forward_chain);
	}
	else
	{
		/* we must find the right index for the filter rule */
#ifdef IPTABLES_143
		for(e = iptc_first_rule(miniupnpd_forward_chain, h);
		    e;
			e = iptc_next_rule(e, h), i++)
#else
		for(e = iptc_first_rule(miniupnpd_forward_chain, &h);
		    e;
			e = iptc_next_rule(e, &h), i++)
#endif
		{
			if(proto!=e->ip.proto)
				continue;
			target = (void *)e + e->target_offset;
			match = (const struct ipt_entry_match *)&e->elems;
			if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
			{
				const struct ipt_tcp * info;
				info = (const struct ipt_tcp *)match->data;
				if(old_iport != info->dpts[0])
					continue;
			}
			else
			{
				const struct ipt_udp * info;
				info = (const struct ipt_udp *)match->data;
				if(old_iport != info->dpts[0])
					continue;
			}
			if(iaddr != e->ip.dst.s_addr)
				continue;
			index = i;
			found = 1;
			entry_len = sizeof(struct ipt_entry) + match->u.match_size + target->u.target_size;
			new_e = malloc(entry_len);
			if(new_e == NULL) {
				syslog(LOG_ERR, "%s: malloc(%u) error",
				       "update_portmapping", (unsigned)entry_len);
				r = -1;
			} else {
				memcpy(new_e, e, entry_len);
				target = (void *)new_e + new_e->target_offset;
				if (target->u.user.name[0] == '\0' && iptc_get_target(e, h)) {
					strncpy(target->u.user.name, iptc_get_target(e, h), sizeof(target->u.user.name));
				}
			}
			break;
		}
	}
#ifdef IPTABLES_143
	iptc_free(h);
#else
	iptc_free(&h);
#endif
	if(!found || r < 0)
		return -1;

	syslog(LOG_INFO, "Trying to update filter rule at index %u", index);
	match = (struct ipt_entry_match *)&new_e->elems;
	if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
	{
		struct ipt_tcp * info;
		info = (struct ipt_tcp *)match->data;
		info->dpts[0] = info->dpts[1] = iport;
	}
	else
	{
		struct ipt_udp * info;
		info = (struct ipt_udp *)match->data;
		info->dpts[0] = info->dpts[1] = iport;
	}
	r = update_rule_and_commit("filter", miniupnpd_forward_chain, index, new_e);
	free(new_e); new_e = NULL;
	if(r < 0)
		return r;

#ifdef ENABLE_PORT_TRIGGERING
	/* update snat rule */
	h = iptc_init("nat");
	if(!h)
	{
		syslog(LOG_ERR, "%s() : iptc_init() failed : %s",
		       "update_portmapping", iptc_strerror(errno));
		goto skip;
	}
	i = 0; found = 0;
	if(!iptc_is_chain(miniupnpd_nat_postrouting_chain, h))
	{
		syslog(LOG_ERR, "chain %s not found", miniupnpd_nat_postrouting_chain);
	}
	else
	{
		/* we must find the right index for the filter rule */
#ifdef IPTABLES_143
		for(e = iptc_first_rule(miniupnpd_nat_postrouting_chain, h);
		    e;
			e = iptc_next_rule(e, h), i++)
#else
		for(e = iptc_first_rule(miniupnpd_nat_postrouting_chain, &h);
		    e;
			e = iptc_next_rule(e, &h), i++)
#endif
		{
			if(proto==e->ip.proto)
			{
				target = (void *)e + e->target_offset;
				mr = (struct ip_nat_multi_range *)&target->data[0];
				syslog(LOG_DEBUG, "postrouting rule #%u: %s %s %hu",
						i, target->u.user.name, inet_ntoa(e->ip.src), ntohs(mr->range[0].min.all));
				/* target->u.user.name SNAT / MASQUERADE */
				if (eport != ntohs(mr->range[0].min.all)) {
					continue;
				}
				match = (const struct ipt_entry_match *)&e->elems;
				if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
				{
					const struct ipt_tcp * info;
					info = (const struct ipt_tcp *)match->data;
					if(old_iport != info->spts[0])
						continue;
				}
				else
				{
					const struct ipt_udp * info;
					info = (const struct ipt_udp *)match->data;
					if(old_iport != info->spts[0])
						continue;
				}
				if (iaddr != e->ip.src.s_addr) {
					continue;
				}
				index = i;
				found = 1;
				entry_len = sizeof(struct ipt_entry) + match->u.match_size + target->u.target_size;
				new_e = malloc(entry_len);
				if(new_e == NULL) {
					syslog(LOG_ERR, "%s: malloc(%u) error",
							"update_portmapping", (unsigned)entry_len);
					r = -1;
				} else {
					memcpy(new_e, e, entry_len);
				}
				break;
			}
		}
	}
#ifdef IPTABLES_143
	iptc_free(h);
#else
	iptc_free(&h);
#endif
	if(!found || r < 0)
		goto skip;

	syslog(LOG_INFO, "Trying to update snat rule at index %u", index);
	match = (struct ipt_entry_match *)&new_e->elems;
	if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
	{
		struct ipt_tcp * info;
		info = (struct ipt_tcp *)match->data;
		info->spts[0] = info->spts[1] = iport;
	}
	else
	{
		struct ipt_udp * info;
		info = (struct ipt_udp *)match->data;
		info->spts[0] = info->spts[1] = iport;
	}
	r = update_rule_and_commit("nat", miniupnpd_nat_postrouting_chain, index, new_e);
	free(new_e); new_e = NULL;
	if(r < 0)
		syslog(LOG_INFO, "Trying to update snat rule at index %u fail!", index);

skip:
#endif /* ENABLE_PORT_TRIGGERING */
	return update_portmapping_desc_timestamp(ifname, eport, proto, desc, timestamp);
}

/* ================================ */
#ifdef DEBUG
static int
print_match(const struct ipt_entry_match *match)
{
	printf("match %s :\n", match->u.user.name);
	if(0 == strncmp(match->u.user.name, "tcp", IPT_FUNCTION_MAXNAMELEN))
	{
		struct ipt_tcp * tcpinfo;
		tcpinfo = (struct ipt_tcp *)match->data;
		printf("  srcport = %hu:%hu dstport = %hu:%hu\n",
		       tcpinfo->spts[0], tcpinfo->spts[1],
			   tcpinfo->dpts[0], tcpinfo->dpts[1]);
	}
	else if(0 == strncmp(match->u.user.name, "udp", IPT_FUNCTION_MAXNAMELEN))
	{
		struct ipt_udp * udpinfo;
		udpinfo = (struct ipt_udp *)match->data;
		printf("  srcport = %hu:%hu dstport = %hu:%hu\n",
		       udpinfo->spts[0], udpinfo->spts[1],
			   udpinfo->dpts[0], udpinfo->dpts[1]);
	}
	return 0;
}

static void
print_iface(const char * iface, const unsigned char * mask, int invert)
{
	unsigned i;
	if(mask[0] == 0)
		return;
	if(invert)
		printf("! ");
	for(i=0; i<IFNAMSIZ; i++)
	{
		if(mask[i])
		{
			if(iface[i])
				putchar(iface[i]);
		}
		else
		{
			if(iface[i-1])
				putchar('+');
			break;
		}
	}
}

static void
printip(uint32_t ip)
{
	printf("%u.%u.%u.%u", ip >> 24, (ip >> 16) & 0xff,
	       (ip >> 8) & 0xff, ip & 0xff);
}

/* for debug */
/* read the "filter" and "nat" tables */
int
list_redirect_rule(const char * ifname)
{
	IPTC_HANDLE h;
	const struct ipt_entry * e;
	const struct ipt_entry_target * target;
	const struct ip_nat_multi_range * mr;
	const char * target_str;
	char addr[16], mask[16];
	unsigned int index;
	(void)ifname;

	h = iptc_init("nat");
	if(!h)
	{
		printf("iptc_init() error : %s\n", iptc_strerror(errno));
		return -1;
	}
	if(!iptc_is_chain(miniupnpd_nat_chain, h))
	{
		printf("chain %s not found\n", miniupnpd_nat_chain);
#ifdef IPTABLES_143
		iptc_free(h);
#else
		iptc_free(&h);
#endif
		return -1;
	}
	index = 0;
#ifdef IPTABLES_143
	for(e = iptc_first_rule(miniupnpd_nat_chain, h);
		e;
		e = iptc_next_rule(e, h))
	{
		target_str = iptc_get_target(e, h);
#else
	for(e = iptc_first_rule(miniupnpd_nat_chain, &h);
		e;
		e = iptc_next_rule(e, &h))
	{
		target_str = iptc_get_target(e, &h);
#endif
		printf("=== rule #%u ===\n", index);
		inet_ntop(AF_INET, &e->ip.src, addr, sizeof(addr));
		inet_ntop(AF_INET, &e->ip.smsk, mask, sizeof(mask));
		printf("src = %s%s/%s\t", (e->ip.invflags & IPT_INV_SRCIP)?"! ":"",
		       /*inet_ntoa(e->ip.src), inet_ntoa(e->ip.smsk)*/
		       addr, mask);
		inet_ntop(AF_INET, &e->ip.dst, addr, sizeof(addr));
		inet_ntop(AF_INET, &e->ip.dmsk, mask, sizeof(mask));
		printf("dst = %s%s/%s\n", (e->ip.invflags & IPT_INV_DSTIP)?"! ":"",
		       /*inet_ntoa(e->ip.dst), inet_ntoa(e->ip.dmsk)*/
		       addr, mask);
		/*printf("in_if = %s  out_if = %s\n", e->ip.iniface, e->ip.outiface);*/
		printf("in_if = ");
		print_iface(e->ip.iniface, e->ip.iniface_mask,
		            e->ip.invflags & IPT_INV_VIA_IN);
		printf("\tout_if = ");
		print_iface(e->ip.outiface, e->ip.outiface_mask,
		            e->ip.invflags & IPT_INV_VIA_OUT);
		printf("\n");
		printf("ip.proto = %s%d\n", (e->ip.invflags & IPT_INV_PROTO)?"! ":"",
		       e->ip.proto);
		/* display matches stuff */
		if(e->target_offset)
		{
			IPT_MATCH_ITERATE(e, print_match);
			/*printf("\n");*/
		}
		printf("target = %s :\n", target_str);
		target = (void *)e + e->target_offset;
		mr = (const struct ip_nat_multi_range *)&target->data[0];
		printf("  ips ");
		printip(ntohl(mr->range[0].min_ip));
		printf(" ");
		printip(ntohl(mr->range[0].max_ip));
		printf("\n  ports %hu %hu\n", ntohs(mr->range[0].min.all),
		          ntohs(mr->range[0].max.all));
		printf("  flags = %x\n", mr->range[0].flags);
		index++;
	}
	if(h)
#ifdef IPTABLES_143
		iptc_free(h);
#else
		iptc_free(&h);
#endif
	printf("======\n");
	return 0;
}
#endif
