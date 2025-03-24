/* $Id: nftnlrdr.c,v 1.15 2024/03/11 23:28:21 nanard Exp $
 * vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2015 Tomofumi Hayashi
 * (c) 2019 Sven Auhagen
 * (c) 2019 Paul Chambers
 * (c) 2020-2024 Thomas Bernard
 *
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution.
 */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include <net/if.h>

#include <linux/version.h>

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables.h>

#include <libmnl/libmnl.h>
#include <libnftnl/table.h>
#include <libnftnl/chain.h>
#include <libnftnl/rule.h>
#include <libnftnl/expr.h>

#include "tiny_nf_nat.h"

#include "config.h"
#include "../macros.h"
#include "../commonrdr.h"
#include "nftnlrdr.h"

#include "nftnlrdr_misc.h"


#ifdef DEBUG
#define d_printf(x) do { printf x; } while (0)
#else
#define d_printf(x)
#endif

/* list to keep timestamps for port mappings having a lease duration */
struct timestamp_entry {
	struct timestamp_entry * next;
	unsigned int timestamp;
	unsigned short eport;
	short protocol;
};

static struct timestamp_entry * timestamp_list = NULL;

#define NAT_CHAIN_TYPE		"nat"
#define FILTER_CHAIN_TYPE	"filter"

/* init and shutdown functions */
int
init_redirect(void)
{
	int result;

	/* requires elevated privileges */
	result = nft_mnl_connect();

	return result;
}

void
shutdown_redirect(void)
{
	nft_mnl_disconnect();
}

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
	case RDR_TABLE_NAME:
		nft_table = string;
		break;
	case RDR_NAT_TABLE_NAME:
		nft_nat_table = string;
		break;
	case RDR_NAT_PREROUTING_CHAIN_NAME:
		nft_prerouting_chain = string;
		break;
	case RDR_NAT_POSTROUTING_CHAIN_NAME:
		nft_postrouting_chain = string;
		break;
	case RDR_FORWARD_CHAIN_NAME:
		nft_forward_chain = string;
		break;
	case RDR_FAMILY_SPLIT:
		if(strcmp(string, "yes") == 0) {
			nft_nat_family = NFPROTO_IPV4;
			nft_ipv4_family = NFPROTO_IPV4;
			nft_ipv6_family = NFPROTO_IPV6;
			syslog(LOG_INFO, "using IPv4/IPv6 Table");
		}
		break;
	default:
		syslog(LOG_ERR, "%s(): tried to set invalid string parameter: %d", "set_rdr_name", param);
		return -2;
	}
	return 0;
}

static unsigned int
get_timestamp(unsigned short eport, int proto)
{
	struct timestamp_entry * e;
	e = timestamp_list;
	while(e) {
		if(e->eport == eport && e->protocol == (short)proto) {
			syslog(LOG_DEBUG, "timestamp entry found (%hu, %d, %u)", eport, proto, e->timestamp);
			return e->timestamp;
		}
		e = e->next;
	}
	syslog(LOG_WARNING, "get_timestamp(%hu, %d) no entry found", eport, proto);
	return 0;
}

static void
remove_timestamp_entry(unsigned short eport, int proto)
{
	struct timestamp_entry * e;
	struct timestamp_entry * * p;
	p = &timestamp_list;
	e = *p;
	while(e) {
		if(e->eport == eport && e->protocol == (short)proto) {
			syslog(LOG_DEBUG, "timestamp entry removed (%hu, %d, %u)", eport, proto, e->timestamp);
			/* remove the entry */
			*p = e->next;
			free(e);
			return;
		}
		p = &(e->next);
		e = *p;
	}
	syslog(LOG_WARNING, "remove_timestamp_entry(%hu, %d) no entry found", eport, proto);
}

static void
add_timestamp_entry(unsigned short eport, int proto, unsigned timestamp)
{
	struct timestamp_entry * tmp;
	tmp = malloc(sizeof(struct timestamp_entry));
	if(tmp)
	{
		tmp->next = timestamp_list;
		tmp->timestamp = timestamp;
		tmp->eport = eport;
		tmp->protocol = (short)proto;
		timestamp_list = tmp;
		syslog(LOG_DEBUG, "timestamp entry added (%hu, %d, %u)", eport, proto, timestamp);
	}
	else
	{
		syslog(LOG_ERR, "add_timestamp_entry() malloc(%lu) error",
		       sizeof(struct timestamp_entry));
	}
}

int
add_redirect_rule2(const char * ifname,
		   const char * rhost, unsigned short eport,
		   const char * iaddr, unsigned short iport, int proto,
		   const char * desc, unsigned int timestamp)
{
	int ret;
	struct nftnl_rule *r;
	UNUSED(rhost);

	d_printf(("add redirect rule2(%s, %s, %u, %s, %u, %d, %s)!\n",
	          ifname, rhost, eport, iaddr, iport, proto, desc));

	r = rule_set_dnat(nft_nat_family, ifname, proto,
	                  0, eport,
	                  inet_addr(iaddr), iport,  desc, NULL);

	ret = nft_send_rule(r, NFT_MSG_NEWRULE, RULE_CHAIN_REDIRECT);
	if (ret >= 0) {
		add_timestamp_entry(eport, proto, timestamp);
	}
	return ret;
}

/*
 * This function submit the rule as following:
 * nft add rule nat miniupnpd-pcp-peer ip
 *    saddr <iaddr> ip daddr <rhost> tcp sport <iport>
 *    tcp dport <rport> snat <eaddr>:<eport>
 */
int
add_peer_redirect_rule2(const char * ifname,
			const char * rhost, unsigned short rport,
			const char * eaddr, unsigned short eport,
			const char * iaddr, unsigned short iport, int proto,
			const char * desc, unsigned int timestamp)
{
	struct nftnl_rule *r;
	UNUSED(ifname); UNUSED(timestamp);

	d_printf(("add peer redirect rule2()!\n"));


	r = rule_set_snat(nft_nat_family, proto,
	                  inet_addr(rhost), rport,
	                  inet_addr(eaddr), eport,
	                  inet_addr(iaddr), iport, desc, NULL);

	return nft_send_rule(r, NFT_MSG_NEWRULE, RULE_CHAIN_PEER);
}

/*
 * This function submit the rule as following:
 * nft add rule filter miniupnpd
 *    ip daddr <iaddr> tcp dport <iport> accept
 *
 */
int
add_filter_rule2(const char * ifname,
		 const char * rhost, const char * iaddr,
		 unsigned short eport, unsigned short iport,
		 int proto, const char * desc)
{
	struct nftnl_rule *r = NULL;
	in_addr_t rhost_addr = 0;

	d_printf(("add_filter_rule2(%s, %s, %s, %d, %d, %d, %s)\n",
	          ifname, rhost, iaddr, eport, iport, proto, desc));

	if (rhost != NULL && strcmp(rhost, "") != 0 && strcmp(rhost, "*") != 0) {
		rhost_addr = inet_addr(rhost);
	}
	r = rule_set_filter(nft_nat_family, ifname, proto,
	                    rhost_addr, inet_addr(iaddr),
	                    eport, iport, 0,
	                    desc, 0);

	return nft_send_rule(r, NFT_MSG_NEWRULE, RULE_CHAIN_FILTER);
}

/*
 * add_peer_dscp_rule2() is not supported due to nft does not support
 * dscp set.
 */
int
add_peer_dscp_rule2(const char * ifname,
		    const char * rhost, unsigned short rport,
		    unsigned char dscp,
		    const char * iaddr, unsigned short iport, int proto,
		    const char * desc, unsigned int timestamp)
{
	UNUSED(ifname); UNUSED(rhost); UNUSED(rport);
	UNUSED(dscp); UNUSED(iaddr); UNUSED(iport); UNUSED(proto);
	UNUSED(desc); UNUSED(timestamp);
	syslog(LOG_ERR, "add_peer_dscp_rule2: not supported");
	return 0;
}

int
delete_filter_rule(const char * ifname, unsigned short port, int proto)
{
	rule_t *p;
	struct nftnl_rule *r;
	UNUSED(ifname);

	refresh_nft_cache_filter();
	LIST_FOREACH(p, &head_filter, entry) {
		if (p->dport == port && p->proto == proto && p->type == RULE_FILTER) {
			r = rule_del_handle(p);
			nft_send_rule(r, NFT_MSG_DELRULE, RULE_CHAIN_FILTER);
			break;
		}
	}

	return 0;
}

/*
 * Clear all rules corresponding eport/proto
 */
int
delete_redirect_and_filter_rules(unsigned short eport, int proto)
{
	rule_t *p;
	struct nftnl_rule *r = NULL;
	in_addr_t iaddr = 0;
	uint16_t iport = 0;

	d_printf(("delete_redirect_and_filter_rules(%d %d)\n", eport, proto));

	refresh_nft_cache_redirect();

	// Delete Redirect Rule  eport => iaddr:iport
	LIST_FOREACH(p, &head_redirect, entry) {
		if (p->dport == eport && p->proto == proto &&
		    (p->type == RULE_NAT && p->nat_type == NFT_NAT_DNAT)) {
			iaddr = p->nat_addr;
			iport = p->nat_port;

			r = rule_del_handle(p);
			/* Todo: send bulk request */
			nft_send_rule(r, NFT_MSG_DELRULE, RULE_CHAIN_REDIRECT);
			break;
		}
	}

	if (iaddr != 0 && iport != 0) {
		refresh_nft_cache_filter();
		// Delete Forward Rule
		LIST_FOREACH(p, &head_filter, entry) {
			if (p->dport == iport && p->daddr == iaddr && p->proto == proto
 			    && p->type == RULE_FILTER)  {
				r = rule_del_handle(p);
				/* Todo: send bulk request */
				nft_send_rule(r, NFT_MSG_DELRULE, RULE_CHAIN_FILTER);
				break;
			}
		}
	}

	iaddr = 0;
	iport = 0;

	refresh_nft_cache_peer();
	// Delete Peer Rule
	LIST_FOREACH(p, &head_peer, entry) {
		if (p->nat_port == eport && p->proto == proto &&
		    (p->type == RULE_NAT && p->nat_type == NFT_NAT_SNAT)) {
			iaddr = p->daddr;
			iport = p->dport;

			r = rule_del_handle(p);
			/* Todo: send bulk request */
			nft_send_rule(r, NFT_MSG_DELRULE, RULE_CHAIN_PEER);
			break;
		}
	}

	if (iaddr != 0 && iport != 0) {
		refresh_nft_cache_filter();
		// Delete Forward Rule
		LIST_FOREACH(p, &head_filter, entry) {
			if (p->dport == iport &&
				p->daddr == iaddr && p->type == RULE_FILTER) {
				r = rule_del_handle(p);
				/* Todo: send bulk request */
				nft_send_rule(r, NFT_MSG_DELRULE, RULE_CHAIN_FILTER);
				break;
			}
		}
	}

	return 0;
}

/*
 * get peer by index as array.
 * return -1 when not found.
 */
int
get_peer_rule_by_index(int index,
		       char * ifname, unsigned short * eport,
		       char * iaddr, int iaddrlen, unsigned short * iport,
		       int * proto, char * desc, int desclen,
		       char * rhost, int rhostlen, unsigned short * rport,
		       unsigned int * timestamp,
		       u_int64_t * packets, u_int64_t * bytes)
{
	rule_t *r;
	int i = 0;

	d_printf(("get_peer_rule_by_index()\n"));
	refresh_nft_cache_peer();

	LIST_FOREACH(r, &head_peer, entry) {
		if (i++ == index) {
			if (ifname != NULL) {
				if_indextoname(r->ingress_ifidx, ifname);
			}

			if (eport != NULL) {
				*eport = r->nat_port;
			}

			if (iaddr != NULL) {
				if (inet_ntop(AF_INET, &r->daddr, iaddr, iaddrlen) == NULL) {
					syslog(LOG_ERR, "%s: inet_ntop: %m",
					       "get_peer_rule_by_index");
				}
			}

			if (iport != NULL) {
				*iport = r->dport;
			}

			if (proto != NULL) {
				*proto = r->proto;
			}

			if (rhost != NULL) {
				if (r->saddr) {
					if (inet_ntop(AF_INET, &r->saddr, rhost, rhostlen) == NULL) {
						syslog(LOG_ERR, "%s: inet_ntop: %m",
						       "get_peer_rule_by_index");
					}
				} else {
					rhost[0] = '\0';
				}
			}

			if (rport != NULL) {
				*rport = r->sport;
			}

			if (desc != NULL) {
				strncpy(desc, r->desc, desclen);
			}

			if (packets) {
				*packets = r->packets;
			}
			if (bytes) {
				*bytes = r->bytes;
			}

			if (timestamp) {
				*timestamp = get_timestamp(r->dport, r->proto);
			}
			/*
			 * TODO: Implement counter in case of add {nat,filter}
			 */
			return 0;
		}
	}

	return -1;
}

/*
 * get_redirect_rule()
 * returns -1 if the rule is not found
 */
int
get_redirect_rule(const char * ifname, unsigned short eport, int proto,
		  char * iaddr, int iaddrlen, unsigned short * iport,
		  char * desc, int desclen,
		  char * rhost, int rhostlen,
		  unsigned int * timestamp,
		  u_int64_t * packets, u_int64_t * bytes)
{
	return get_nat_redirect_rule(nft_prerouting_chain,
	                             ifname, eport, proto,
	                             iaddr, iaddrlen, iport,
	                             desc, desclen,
	                             rhost, rhostlen,
	                             timestamp, packets, bytes);
}

/* get_redirect_rule_count()
 * return value : -1 for error or the number of redirection rules */
int
get_redirect_rule_count(const char * ifname)
{
	rule_t *r;
	int n = 0;
	UNUSED(ifname);

	refresh_nft_cache_redirect();
	LIST_FOREACH(r, &head_redirect, entry) {
		n++;
	}
	return n;
}

/*
 * get_redirect_rule_by_index()
 * return -1 when the rule was not found
 */
int
get_redirect_rule_by_index(int index,
			   char * ifname, unsigned short * eport,
			   char * iaddr, int iaddrlen, unsigned short * iport,
			   int * proto, char * desc, int desclen,
			   char * rhost, int rhostlen,
			   unsigned int * timestamp,
			   u_int64_t * packets, u_int64_t * bytes)
{
	rule_t *r;
	int i = 0;

	d_printf(("get_redirect_rule_by_index()\n"));
	refresh_nft_cache_redirect();

	LIST_FOREACH(r, &head_redirect, entry) {
		if (i++ == index) {
			if (ifname != NULL) {
				if_indextoname(r->ingress_ifidx, ifname);
			}

			if (eport != NULL) {
				*eport = r->dport;
			}

			if (iaddr != NULL) {
				if (inet_ntop(AF_INET, &r->nat_addr, iaddr, iaddrlen) == NULL) {
					syslog(LOG_ERR, "%s: inet_ntop: %m",
					       "get_redirect_rule_by_index");
				}
			}

			if (iport != NULL) {
				*iport = r->nat_port;
			}

			if (proto != NULL) {
				*proto = r->proto;
			}

			if (rhost != NULL) {
				if (r->saddr) {
					if (inet_ntop(AF_INET, &r->saddr, rhost, rhostlen) == NULL) {
						syslog(LOG_ERR, "%s: inet_ntop: %m",
						       "get_redirect_rule_by_index");
					}
				} else {
					rhost[0] = '\0';
				}
			}

			if (desc != NULL && r->desc) {
				strncpy(desc, r->desc, desclen);
			}

			if (timestamp != NULL) {
				*timestamp = get_timestamp(*eport, *proto);
			}

			if (packets || bytes) {
				if (packets)
					*packets = r->packets;
				if (bytes)
					*bytes = r->bytes;
			}

			/*
			 * TODO: Implement counter in case of add {nat,filter}
			 */
			return 0;
		}
	}

	return -1;
}

/*
 * return -1 not found.
 * return 0 found
 */
int
get_nat_redirect_rule(const char * nat_chain_name, const char * ifname,
		      unsigned short eport, int proto,
		      char * iaddr, int iaddrlen, unsigned short * iport,
		      char * desc, int desclen,
		      char * rhost, int rhostlen,
		      unsigned int * timestamp,
		      u_int64_t * packets, u_int64_t * bytes)
{
	rule_t *p;
	struct in_addr addr;
	UNUSED(nat_chain_name);
	UNUSED(ifname);
	UNUSED(packets);
	UNUSED(bytes);
	UNUSED(rhost);
	UNUSED(rhostlen);

	refresh_nft_cache_redirect();

	LIST_FOREACH(p, &head_redirect, entry) {
		if (p->proto == proto &&
		    p->dport == eport) {

			if (p->nat_addr && iaddr) {
				addr.s_addr = p->nat_addr;
				if (inet_ntop(AF_INET, &addr, iaddr, iaddrlen) == NULL) {
					syslog(LOG_ERR, "%s: inet_ntop: %m",
					       "get_nat_redirect_rule");
				}
			}

			if (desc != NULL && p->desc) {
				strncpy(desc, p->desc, desclen);
			}

			if (iport)
				*iport = p->nat_port;

			if(timestamp != NULL)
				*timestamp = get_timestamp(eport, proto);

			return 0;
		}
	}

	return -1;
}

/*
 * return an (malloc'ed) array of "external" port for which there is
 * a port mapping. number is the size of the array
 */
unsigned short *
get_portmappings_in_range(unsigned short startport, unsigned short endport,
			  int proto, unsigned int * number)
{
	uint32_t capacity;
	rule_t *p;
	unsigned short *array;
	unsigned short *tmp;

	d_printf(("get_portmappings_in_range()\n"));

	*number = 0;
	capacity = 128;
	array = calloc(capacity, sizeof(unsigned short));

	if (array == NULL) {
		syslog(LOG_ERR, "get_portmappings_in_range(): calloc error");
		return NULL;
	}

	refresh_nft_cache_redirect();

	LIST_FOREACH(p, &head_redirect, entry) {
		if (p->proto == proto &&
		    startport <= p->dport &&
		    p->dport <= endport) {

			if (*number >= capacity) {
				capacity += 128;
				tmp = realloc(array,
					      sizeof(unsigned short)*capacity);
				if (tmp == NULL) {
					syslog(LOG_ERR,
					       "get_portmappings_in_range(): "
					       "realloc(%u) error",
					       (unsigned)sizeof(unsigned short)*capacity);
					*number = 0;
					free(array);
					return NULL;
				}
				array = tmp;
			}
			array[*number] = p->dport;
			(*number)++;
		}
	}
	return array;
}

int
update_portmapping_desc_timestamp(const char * ifname,
                   unsigned short eport, int proto,
                   const char * desc, unsigned int timestamp)
{
	UNUSED(ifname);
	UNUSED(desc);
	remove_timestamp_entry(eport, proto);
	add_timestamp_entry(eport, proto, timestamp);
	return 0;
}

int
update_portmapping(const char * ifname, unsigned short eport, int proto,
                   unsigned short iport, const char * desc,
                   unsigned int timestamp)
{
	char iaddr_str[INET_ADDRSTRLEN];
	char rhost[INET_ADDRSTRLEN];
	int r;

	d_printf(("update_portmapping()\n"));

	iaddr_str[0] = '\0';
	rhost[0] = '\0';


	if (get_redirect_rule(NULL, eport, proto, iaddr_str, INET_ADDRSTRLEN, NULL, NULL, 0, rhost, INET_ADDRSTRLEN, NULL, 0, 0) < 0)
		return -1;

	r = delete_redirect_and_filter_rules(eport, proto);
	if (r < 0)
		return -1;

	if (add_redirect_rule2(ifname, rhost, eport, iaddr_str, iport, proto,
						  desc, timestamp) < 0)
		return -1;

	if (add_filter_rule2(ifname, rhost, iaddr_str, eport, iport, proto, desc) < 0)
		return -1;

	return 0;
}
