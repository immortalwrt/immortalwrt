/* $Id: iptpinhole.c,v 1.21 2020/05/10 17:49:33 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2012-2020 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/queue.h>

#include "config.h"
#include "../macros.h"
#include "iptpinhole.h"
#include "../upnpglobalvars.h"
#include "../upnputils.h"

#ifdef ENABLE_UPNPPINHOLE

#include <xtables.h>
#include <libiptc/libip6tc.h>
#include "tiny_nf_nat.h"

#define IP6TC_HANDLE struct ip6tc_handle *

static int next_uid = 1;

static const char * miniupnpd_v6_filter_chain = "MINIUPNPD";

static LIST_HEAD(pinhole_list_t, pinhole_t) pinhole_list;

static struct pinhole_t *
get_pinhole(unsigned short uid);

struct pinhole_t {
	struct in6_addr saddr;
	struct in6_addr daddr;
	LIST_ENTRY(pinhole_t) entries;
	unsigned int timestamp;
	unsigned short sport;
	unsigned short dport;
	unsigned short uid;
	unsigned char proto;
	char desc[];
};

void init_iptpinhole(void)
{
	LIST_INIT(&pinhole_list);
}

void shutdown_iptpinhole(void)
{
	struct pinhole_t * p;
	while(pinhole_list.lh_first != NULL) {
		p = pinhole_list.lh_first;
		LIST_REMOVE(p, entries);
		free(p);
	}
}

/* return uid */
static int
add_to_pinhole_list(struct in6_addr * saddr, unsigned short sport,
                    struct in6_addr * daddr, unsigned short dport,
                    int proto, const char *desc, unsigned int timestamp)
{
	struct pinhole_t * p;

	p = calloc(1, sizeof(struct pinhole_t) + strlen(desc) + 1);
	if(!p) {
		syslog(LOG_ERR, "add_to_pinhole_list calloc() error");
		return -1;
	}
	strcpy(p->desc, desc);
	memcpy(&p->saddr, saddr, sizeof(struct in6_addr));
	p->sport = sport;
	memcpy(&p->daddr, daddr, sizeof(struct in6_addr));
	p->dport = dport;
	p->timestamp = timestamp;
	p->proto = (unsigned char)proto;
	LIST_INSERT_HEAD(&pinhole_list, p, entries);
	while(get_pinhole(next_uid) != NULL) {
		next_uid++;
		if(next_uid > 65535)
			next_uid = 1;
	}
	p->uid = next_uid;
	next_uid++;
	if(next_uid > 65535)
		next_uid = 1;
	return p->uid;
}

static struct pinhole_t *
get_pinhole(unsigned short uid)
{
	struct pinhole_t * p;

	for(p = pinhole_list.lh_first; p != NULL; p = p->entries.le_next) {
		if(p->uid == uid)
			return p;
	}
	return NULL;	/* not found */
}

/* new_match()
 * Allocate and set a new ip6t_entry_match structure
 * The caller must free() it after usage */
static struct ip6t_entry_match *
new_match(int proto, unsigned short sport, unsigned short dport)
{
	struct ip6t_entry_match *match;
	struct ip6t_tcp *info;	/* TODO : use ip6t_udp if needed */
	size_t size;
	const char * name;
	size =   XT_ALIGN(sizeof(struct ip6t_entry_match))
	       + XT_ALIGN(sizeof(struct ip6t_tcp));
	match = calloc(1, size);
	match->u.user.match_size = size;
	switch(proto) {
	case IPPROTO_TCP:
		name = "tcp";
		break;
	case IPPROTO_UDP:
		name = "udp";
		break;
	case IPPROTO_UDPLITE:
		name = "udplite";
		break;
	default:
		name = NULL;
	}
	if(name)
		strncpy(match->u.user.name, name, sizeof(match->u.user.name));
	else
		syslog(LOG_WARNING, "no name for protocol %d", proto);
	info = (struct ip6t_tcp *)match->data;
	if(sport) {
		info->spts[0] = sport;	/* specified source port */
		info->spts[1] = sport;
	} else {
		info->spts[0] = 0;		/* all source ports */
		info->spts[1] = 0xFFFF;
	}
	info->dpts[0] = dport;	/* specified destination port */
	info->dpts[1] = dport;
	return match;
}

static struct ip6t_entry_target *
get_accept_target(void)
{
	struct ip6t_entry_target * target = NULL;
	size_t size;
	size =   XT_ALIGN(sizeof(struct ip6t_entry_target))
	       + XT_ALIGN(sizeof(int));
	target = calloc(1, size);
	target->u.user.target_size = size;
	strncpy(target->u.user.name, "ACCEPT", sizeof(target->u.user.name));
	return target;
}

static int
ip6tc_init_verify_append(const char * table,
                         const char * chain,
                         struct ip6t_entry * e)
{
	IP6TC_HANDLE h;

	h = ip6tc_init(table);
	if(!h) {
		syslog(LOG_ERR, "ip6tc_init error : %s", ip6tc_strerror(errno));
		return -1;
	}
	if(!ip6tc_is_chain(chain, h)) {
		syslog(LOG_ERR, "chain %s not found", chain);
		goto error;
	}
	if(!ip6tc_append_entry(chain, e, h)) {
		syslog(LOG_ERR, "ip6tc_append_entry() error : %s", ip6tc_strerror(errno));
		goto error;
	}
	if(!ip6tc_commit(h)) {
		syslog(LOG_ERR, "ip6tc_commit() error : %s", ip6tc_strerror(errno));
		goto error;
	}
	ip6tc_free(h);
	return 0;	/* ok */
error:
	ip6tc_free(h);
	return -1;
}

/*
ip6tables -I %s %d -p %s -i %s -s %s --sport %hu -d %s --dport %hu -j ACCEPT
ip6tables -I %s %d -p %s -i %s --sport %hu -d %s --dport %hu -j ACCEPT

miniupnpd_forward_chain, line_number, proto, ext_if_name, raddr, rport, iaddr, iport

ip6tables -t raw -I PREROUTING %d -p %s -i %s -s %s --sport %hu -d %s --dport %hu -j TRACE
ip6tables -t raw -I PREROUTING %d -p %s -i %s --sport %hu -d %s --dport %hu -j TRACE
*/
int add_pinhole(const char * ifname,
                const char * rem_host, unsigned short rem_port,
                const char * int_client, unsigned short int_port,
                int proto, const char * desc, unsigned int timestamp)
{
	int uid;
	struct ip6t_entry * e;
	struct ip6t_entry * tmp;
	struct ip6t_entry_match *match = NULL;
	struct ip6t_entry_target *target = NULL;

	e = calloc(1, sizeof(struct ip6t_entry));
	if(!e) {
		syslog(LOG_ERR, "%s: calloc(%d) failed",
		       "add_pinhole", (int)sizeof(struct ip6t_entry));
		return -1;
	}
	e->ipv6.proto = proto;
	if (proto)
		e->ipv6.flags |= IP6T_F_PROTO;

	/* TODO: check if enforcing USE_IFNAME_IN_RULES is needed */
	if(ifname)
		strncpy(e->ipv6.iniface, ifname, IFNAMSIZ);
	if(rem_host && (rem_host[0] != '\0')) {
		if(inet_pton(AF_INET6, rem_host, &e->ipv6.src) < 1) {
			syslog(LOG_WARNING, "failed to parse INET6 address \"%s\"", rem_host);
		} else {
			memset(&e->ipv6.smsk, 0xff, sizeof(e->ipv6.smsk));
		}
	}
	if (inet_pton(AF_INET6, int_client, &e->ipv6.dst) < 1) {
		syslog(LOG_WARNING, "failed to parse INET6 address \"%s\"", int_client);
	} else {
		memset(&e->ipv6.dmsk, 0xff, sizeof(e->ipv6.dmsk));
	}

	/*e->nfcache = NFC_IP_DST_PT;*/
	/*e->nfcache |= NFC_UNKNOWN;*/

	match = new_match(proto, rem_port, int_port);
	target = get_accept_target();
	tmp = realloc(e, sizeof(struct ip6t_entry)
	               + match->u.match_size
	               + target->u.target_size);
	if(!tmp) {
		syslog(LOG_ERR, "%s: realloc(%d) failed",
		       "add_pinhole", (int)(sizeof(struct ip6t_entry) + match->u.match_size + target->u.target_size));
		free(e);
		free(match);
		free(target);
		return -1;
	}
	e = tmp;
	memcpy(e->elems, match, match->u.match_size);
	memcpy(e->elems + match->u.match_size, target, target->u.target_size);
	e->target_offset = sizeof(struct ip6t_entry)
	                   + match->u.match_size;
	e->next_offset = sizeof(struct ip6t_entry)
	                 + match->u.match_size
					 + target->u.target_size;
	free(match);
	free(target);

	if(ip6tc_init_verify_append("filter", miniupnpd_v6_filter_chain, e) < 0) {
		free(e);
		return -1;
	}
	uid = add_to_pinhole_list(&e->ipv6.src, rem_port,
	                          &e->ipv6.dst, int_port,
	                          proto, desc, timestamp);
	free(e);
	return uid;
}

int
find_pinhole(const char * ifname,
             const char * rem_host, unsigned short rem_port,
             const char * int_client, unsigned short int_port,
             int proto,
             char *desc, int desc_len, unsigned int * timestamp)
{
	struct pinhole_t * p;
	struct in6_addr saddr;
	struct in6_addr daddr;
	UNUSED(ifname);

	if(rem_host && (rem_host[0] != '\0')) {
		if (inet_pton(AF_INET6, rem_host, &saddr) < 1) {
			syslog(LOG_WARNING, "Failed to parse INET6 address \"%s\"", rem_host);
			memset(&saddr, 0, sizeof(struct in6_addr));
		}
	} else {
		memset(&saddr, 0, sizeof(struct in6_addr));
	}
	if (inet_pton(AF_INET6, int_client, &daddr) < 1) {
		syslog(LOG_WARNING, "Failed to parse INET6 address \"%s\"", int_client);
		memset(&daddr, 0, sizeof(struct in6_addr));
	}
	for(p = pinhole_list.lh_first; p != NULL; p = p->entries.le_next) {
		if((proto == p->proto) && (rem_port == p->sport) &&
		   (0 == memcmp(&saddr, &p->saddr, sizeof(struct in6_addr))) &&
		   (int_port == p->dport) &&
		   (0 == memcmp(&daddr, &p->daddr, sizeof(struct in6_addr)))) {
			if(desc) strncpy(desc, p->desc, desc_len);
			if(timestamp) *timestamp = p->timestamp;
			return (int)p->uid;
		}
	}
	return -2;	/* not found */
}

int
delete_pinhole(unsigned short uid)
{
	struct pinhole_t * p;
	IP6TC_HANDLE h;
	const struct ip6t_entry * e;
	const struct ip6t_entry_match *match = NULL;
	/*const struct ip6t_entry_target *target = NULL;*/
	unsigned int index;

	p = get_pinhole(uid);
	if(!p)
		return -2;	/* not found */

	h = ip6tc_init("filter");
	if(!h) {
		syslog(LOG_ERR, "ip6tc_init error : %s", ip6tc_strerror(errno));
		return -1;
	}
	if(!ip6tc_is_chain(miniupnpd_v6_filter_chain, h)) {
		syslog(LOG_ERR, "chain %s not found", miniupnpd_v6_filter_chain);
		goto error;
	}
	index = 0;
	for(e = ip6tc_first_rule(miniupnpd_v6_filter_chain, h);
	    e;
	    e = ip6tc_next_rule(e, h)) {
		if((e->ipv6.proto == p->proto) &&
		   (0 == memcmp(&e->ipv6.src, &p->saddr, sizeof(e->ipv6.src))) &&
		   (0 == memcmp(&e->ipv6.dst, &p->daddr, sizeof(e->ipv6.dst)))) {
			const struct ip6t_tcp * info;
			match = (const struct ip6t_entry_match *)&e->elems;
			info = (const struct ip6t_tcp *)&match->data;
			if((info->spts[0] == p->sport) && (info->dpts[0] == p->dport)) {
				if(!ip6tc_delete_num_entry(miniupnpd_v6_filter_chain, index, h)) {
					syslog(LOG_ERR, "ip6tc_delete_num_entry(%s,%u,...): %s",
					       miniupnpd_v6_filter_chain, index, ip6tc_strerror(errno));
					goto error;
				}
				if(!ip6tc_commit(h)) {
					syslog(LOG_ERR, "ip6tc_commit(): %s",
					       ip6tc_strerror(errno));
					goto error;
				}
				ip6tc_free(h);
				LIST_REMOVE(p, entries);
				return 0;	/* ok */
			}
		}
		index++;
	}
	ip6tc_free(h);
	syslog(LOG_WARNING, "delete_pinhole() rule with PID=%hu not found", uid);
	LIST_REMOVE(p, entries);
	return -2;	/* not found */
error:
	ip6tc_free(h);
	return -1;
}

int
update_pinhole(unsigned short uid, unsigned int timestamp)
{
	struct pinhole_t * p;

	p = get_pinhole(uid);
	if(p) {
		p->timestamp = timestamp;
		return 0;
	} else {
		return -2;	/* Not found */
	}
}

int
get_pinhole_info(unsigned short uid,
                 char * rem_host, int rem_hostlen,
                 unsigned short * rem_port,
                 char * int_client, int int_clientlen,
                 unsigned short * int_port,
                 int * proto, char * desc, int desclen,
                 unsigned int * timestamp,
                 u_int64_t * packets, u_int64_t * bytes)
{
	struct pinhole_t * p;

	p = get_pinhole(uid);
	if(!p)
		return -2;	/* Not found */
	if(rem_host && (rem_host[0] != '\0')) {
		if(inet_ntop(AF_INET6, &p->saddr, rem_host, rem_hostlen) == NULL)
			return -1;
	}
	if(rem_port)
		*rem_port = p->sport;
	if(int_client) {
		if(inet_ntop(AF_INET6, &p->daddr, int_client, int_clientlen) == NULL)
			return -1;
	}
	if(int_port)
		*int_port = p->dport;
	if(proto)
		*proto = p->proto;
	if(timestamp)
		*timestamp = p->timestamp;
	if (desc)
		strncpy(desc, p->desc, desclen);
	if(packets || bytes) {
		/* theses informations need to be read from netfilter */
		IP6TC_HANDLE h;
		const struct ip6t_entry * e;
		const struct ip6t_entry_match * match;
		h = ip6tc_init("filter");
		if(!h) {
			syslog(LOG_ERR, "ip6tc_init error : %s", ip6tc_strerror(errno));
			return -1;
		}
		for(e = ip6tc_first_rule(miniupnpd_v6_filter_chain, h);
		    e;
		    e = ip6tc_next_rule(e, h)) {
			if((e->ipv6.proto == p->proto) &&
			   (0 == memcmp(&e->ipv6.src, &p->saddr, sizeof(e->ipv6.src))) &&
			   (0 == memcmp(&e->ipv6.dst, &p->daddr, sizeof(e->ipv6.dst)))) {
				const struct ip6t_tcp * info;
				match = (const struct ip6t_entry_match *)&e->elems;
				info = (const struct ip6t_tcp *)&match->data;
				if((info->spts[0] == p->sport) && (info->dpts[0] == p->dport)) {
					if(packets)
						*packets = e->counters.pcnt;
					if(bytes)
						*bytes = e->counters.bcnt;
					break;
				}
			}
		}
		ip6tc_free(h);
	}
	return 0;
}

int get_pinhole_uid_by_index(int index)
{
	struct pinhole_t * p;

	for(p = pinhole_list.lh_first; p != NULL; p = p->entries.le_next)
		if (!index--)
			return p->uid;
	return -1;
}

int
clean_pinhole_list(unsigned int * next_timestamp)
{
	unsigned int min_ts = UINT_MAX;
	struct pinhole_t * p;
	time_t current_time;
	int n = 0;

	current_time = upnp_time();
	p = pinhole_list.lh_first;
	while(p != NULL) {
		if(p->timestamp <= (unsigned int)current_time) {
			unsigned short uid = p->uid;
			syslog(LOG_INFO, "removing expired pinhole with uid=%hu", uid);
			p = p->entries.le_next;
			if(delete_pinhole(uid) == 0)
				n++;
			else
				break;
		} else {
			if(p->timestamp < min_ts)
				min_ts = p->timestamp;
			p = p->entries.le_next;
		}
	}
	if(next_timestamp && (min_ts != UINT_MAX))
		*next_timestamp = min_ts;
	return n;
}

#endif /* ENABLE_UPNPPINHOLE */
