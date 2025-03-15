/* $Id: pfpinhole.c,v 1.30 2024/01/14 23:56:45 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2012-2024 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include "config.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#ifdef __DragonFly__
#include <net/pf/pfvar.h>
#else
#ifdef __APPLE__
#define PRIVATE 1
#endif
#include <net/pfvar.h>
#endif
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef USE_LIBPFCTL
#include <libpfctl.h>
#endif

#include "pfpinhole.h"
#include "../upnpglobalvars.h"
#include "../macros.h"
#include "../upnputils.h"
#include "rtickets.h"

/* the pass rules created by add_pinhole() are as follow :
 *
 * pass in quick on ep0 inet6 proto udp
 *   from any to dead:beef::42:42 port = 8080
 *   flags S/SA keep state
 *   label "pinhole-2 ts-4321000"
 *
 * with the label "pinhole-$uid ts-$timestamp: $description"
 */

#ifdef ENABLE_UPNPPINHOLE

/* /dev/pf when opened */
extern int dev;

static int next_uid = 1;

#define PINEHOLE_LABEL_FORMAT "pinhole-%d ts-%u: %s"
#define PINEHOLE_LABEL_FORMAT_SKIPDESC "pinhole-%d ts-%u: %*s"

#define RULE (pr.rule)

int add_pinhole(const char * ifname,
                const char * rem_host, unsigned short rem_port,
                const char * int_client, unsigned short int_port,
                int proto, const char * desc, unsigned int timestamp)
{
	int uid;
	struct pfioc_rule pr;
#ifndef PF_NEWSTYLE
	struct pfioc_pooladdr pp;
#endif

	if(dev<0) {
		syslog(LOG_ERR, "pf device is not open");
		return -1;
	}
	memset(&pr, 0, sizeof(pr));
	strlcpy(pr.anchor, anchor_name, MAXPATHLEN);

#ifndef PF_NEWSTYLE
	memset(&pp, 0, sizeof(pp));
	strlcpy(pp.anchor, anchor_name, MAXPATHLEN);
	if(ioctl(dev, DIOCBEGINADDRS, &pp) < 0) {
		syslog(LOG_ERR, "ioctl(dev, DIOCBEGINADDRS, ...): %m");
		return -1;
	} else {
		pr.pool_ticket = pp.ticket;
#else
	{
#endif
		RULE.direction = PF_IN;
		RULE.action = PF_PASS;
		RULE.af = AF_INET6;
#ifdef PF_NEWSTYLE
		RULE.nat.addr.type = PF_ADDR_NONE;
		RULE.rdr.addr.type = PF_ADDR_NONE;
#endif
#ifdef USE_IFNAME_IN_RULES
		if(ifname)
			strlcpy(RULE.ifname, ifname, IFNAMSIZ);
#endif
		RULE.proto = proto;

		RULE.quick = 1;/*(GETFLAG(PFNOQUICKRULESMASK))?0:1;*/
		RULE.log = (GETFLAG(LOGPACKETSMASK))?1:0;	/*logpackets;*/
/* see the discussion on the forum :
 * http://miniupnp.tuxfamily.org/forum/viewtopic.php?p=638 */
		RULE.flags = TH_SYN;
		RULE.flagset = (TH_SYN|TH_ACK);
#ifdef PFRULE_HAS_RTABLEID
		RULE.rtableid = -1;	/* first appeared in OpenBSD 4.0 */
#endif
#ifdef PFRULE_HAS_ONRDOMAIN
		RULE.onrdomain = -1;	/* first appeared in OpenBSD 5.0 */
#endif
		RULE.keep_state = 1;
		uid = next_uid;
		snprintf(RULE.label, PF_RULE_LABEL_SIZE,
		         PINEHOLE_LABEL_FORMAT, uid, timestamp, desc);
		if(queue)
			strlcpy(RULE.qname, queue, PF_QNAME_SIZE);
		if(tag)
			strlcpy(RULE.tagname, tag, PF_TAG_NAME_SIZE);

		if(rem_port) {
			RULE.src.port_op = PF_OP_EQ;
			RULE.src.port[0] = htons(rem_port);
		}
		if(rem_host && rem_host[0] != '\0' && rem_host[0] != '*') {
			RULE.src.addr.type = PF_ADDR_ADDRMASK;
			if(inet_pton(AF_INET6, rem_host, &RULE.src.addr.v.a.addr.v6) != 1) {
				syslog(LOG_ERR, "inet_pton(%s) failed", rem_host);
			}
			memset(&RULE.src.addr.v.a.mask.addr8, 255, 16);
		}

		RULE.dst.port_op = PF_OP_EQ;
		RULE.dst.port[0] = htons(int_port);
		RULE.dst.addr.type = PF_ADDR_ADDRMASK;
		if(inet_pton(AF_INET6, int_client, &RULE.dst.addr.v.a.addr.v6) != 1) {
			syslog(LOG_ERR, "inet_pton(%s) failed", int_client);
		}
		memset(&RULE.dst.addr.v.a.mask.addr8, 255, 16);

		if(ifname)
			strlcpy(RULE.ifname, ifname, IFNAMSIZ);

		pr.action = PF_CHANGE_GET_TICKET;
		if(ioctl(dev, DIOCCHANGERULE, &pr) < 0) {
			syslog(LOG_ERR, "ioctl(dev, DIOCCHANGERULE, ...) PF_CHANGE_GET_TICKET: %m");
			return -1;
		} else {
			pr.action = PF_CHANGE_ADD_TAIL;
			if(ioctl(dev, DIOCCHANGERULE, &pr) < 0) {
				syslog(LOG_ERR, "ioctl(dev, DIOCCHANGERULE, ...) PF_CHANGE_ADD_TAIL: %m");
				return -1;
			}
		}
	}

	if(++next_uid >= 65535) {
		next_uid = 1;
	}
	return uid;
}

int find_pinhole(const char * ifname,
                 const char * rem_host, unsigned short rem_port,
                 const char * int_client, unsigned short int_port,
                 int proto,
                 char *desc, int desc_len, unsigned int * timestamp)
{
	int uid;
	unsigned int ts, tnum;
	int i, n;
#undef RULE
#ifdef USE_LIBPFCTL
	struct pfctl_rules_info ri;
	struct pfctl_rule rule;
#define RULE (rule)
	char anchor_call[MAXPATHLEN] = "";
#else /* USE_LIBPFCTL */
	struct pfioc_rule pr;
#define RULE (pr.rule)
#endif /* USE_LIBPFCTL */
	struct in6_addr saddr;
	struct in6_addr daddr;
	UNUSED(ifname);

	if(dev<0) {
		syslog(LOG_ERR, "pf device is not open");
		return -1;
	}
	if(rem_host && (rem_host[0] != '\0')) {
		inet_pton(AF_INET6, rem_host, &saddr);
	} else {
		memset(&saddr, 0, sizeof(struct in6_addr));
	}
	inet_pton(AF_INET6, int_client, &daddr);
#ifdef USE_LIBPFCTL
	if(pfctl_get_rules_info(dev, &ri, PF_PASS, anchor_name) < 0)
	{
		syslog(LOG_ERR, "pfctl_get_rules_info: %m");
		return -1;
	}
	n = ri.nr;
#ifdef PF_RELEASETICKETS
	tnum = ri.ticket;
#endif /* PF_RELEASETICKETS */
#else /* USE_LIBPFCTL */
	memset(&pr, 0, sizeof(pr));
	strlcpy(pr.anchor, anchor_name, MAXPATHLEN);
#ifndef PF_NEWSTYLE
	RULE.action = PF_PASS;
#endif
	if(ioctl(dev, DIOCGETRULES, &pr) < 0) {
		syslog(LOG_ERR, "ioctl(dev, DIOCGETRULES, ...): %m");
		return -1;
	}
	n = pr.nr;
#ifdef PF_RELEASETICKETS
	tnum = pr.ticket;
#endif /* PF_RELEASETICKETS */
#endif /* USE_LIBPFCTL */
	for(i=0; i<n; i++) {
#ifdef USE_LIBPFCTL
		if(pfctl_get_rule(dev, i, ri.ticket, anchor_name, PF_PASS, &rule, anchor_call) < 0)
		{
			syslog(LOG_ERR, "pfctl_get_rule: %m");
			release_ticket(dev, tnum);
			return -1;
		}
#else /* USE_LIBPFCTL */
		pr.nr = i;
		if(ioctl(dev, DIOCGETRULE, &pr) < 0) {
			syslog(LOG_ERR, "ioctl(dev, DIOCGETRULE): %m");
			release_ticket(dev, tnum);
			return -1;
		}
#endif /* USE_LIBPFCTL */
		if((proto == RULE.proto) && (rem_port == ntohs(RULE.src.port[0]))
		   && (0 == memcmp(&saddr, &RULE.src.addr.v.a.addr.v6, sizeof(struct in6_addr)))
		   && (int_port == ntohs(RULE.dst.port[0])) &&
		   (0 == memcmp(&daddr, &RULE.dst.addr.v.a.addr.v6, sizeof(struct in6_addr)))) {
#ifdef USE_LIBPFCTL
			if(sscanf(RULE.label[0], PINEHOLE_LABEL_FORMAT_SKIPDESC, &uid, &ts) != 2) {
				syslog(LOG_DEBUG, "rule with label '%s' is not a IGD pinhole", RULE.label[0]);
				continue;
			}
#else /* USE_LIBPFCTL */
			if(sscanf(RULE.label, PINEHOLE_LABEL_FORMAT_SKIPDESC, &uid, &ts) != 2) {
				syslog(LOG_DEBUG, "rule with label '%s' is not a IGD pinhole", RULE.label);
				continue;
			}
#endif /* USE_LIBPFCTL */
			if(timestamp) *timestamp = ts;
			if(desc) {
#ifdef USE_LIBPFCTL
				char * p = strchr(RULE.label[0], ':');
#else /* USE_LIBPFCTL */
				char * p = strchr(RULE.label, ':');
#endif /* USE_LIBPFCTL */
				if(p) {
					p += 2;
					strlcpy(desc, p, desc_len);
				}
			}
			release_ticket(dev, tnum);
			return uid;
		}
	}
	release_ticket(dev, tnum);
	return -2;
}

int delete_pinhole(unsigned short uid)
{
	int i, n;
	unsigned int tnum;
#undef RULE
#ifdef USE_LIBPFCTL
	struct pfctl_rules_info ri;
	struct pfctl_rule rule;
#define RULE (rule)
	char anchor_call[MAXPATHLEN] = "";
#else /* USE_LIBPFCTL */
	struct pfioc_rule pr;
#define RULE (pr.rule)
#endif /* USE_LIBPFCTL */
	char label_start[PF_RULE_LABEL_SIZE];
	char tmp_label[PF_RULE_LABEL_SIZE];

	if(dev<0) {
		syslog(LOG_ERR, "pf device is not open");
		return -1;
	}
	snprintf(label_start, sizeof(label_start),
	         "pinhole-%hu", uid);
#ifdef USE_LIBPFCTL
	if(pfctl_get_rules_info(dev, &ri, PF_PASS, anchor_name) < 0)
	{
		syslog(LOG_ERR, "pfctl_get_rules_info: %m");
		return -1;
	}
	n = ri.nr;
#ifdef PF_RELEASETICKETS
	tnum = ri.ticket;
#endif /* PF_RELEASETICKETS */
#else /* USE_LIBPFCTL */
	memset(&pr, 0, sizeof(pr));
	strlcpy(pr.anchor, anchor_name, MAXPATHLEN);
#ifndef PF_NEWSTYLE
	RULE.action = PF_PASS;
#endif
	if(ioctl(dev, DIOCGETRULES, &pr) < 0) {
		syslog(LOG_ERR, "ioctl(dev, DIOCGETRULES, ...): %m");
		return -1;
	}
	n = pr.nr;
#ifdef PF_RELEASETICKETS
	tnum = pr.ticket;
#endif
#endif /* USE_LIBPFCTL */
	for(i=0; i<n; i++) {
#ifdef USE_LIBPFCTL
		if(pfctl_get_rule(dev, i, ri.ticket, anchor_name, PF_PASS, &rule, anchor_call) < 0)
		{
			syslog(LOG_ERR, "pfctl_get_rule: %m");
			return -1;
		}
		strlcpy(tmp_label, RULE.label[0], sizeof(tmp_label));
#else /* USE_LIBPFCTL */
		pr.nr = i;
		if(ioctl(dev, DIOCGETRULE, &pr) < 0) {
			syslog(LOG_ERR, "ioctl(dev, DIOCGETRULE): %m");
			return -1;
		}
		strlcpy(tmp_label, RULE.label, sizeof(tmp_label));
#endif /* USE_LIBPFCTL */
		strtok(tmp_label, " ");
		if(0 == strcmp(tmp_label, label_start)) {
#ifdef USE_LIBPFCTL
			/* TODO: convert DIOCCHANGERULE to libpfctl */
			struct pfioc_rule pr;
			pr.action = PF_CHANGE_GET_TICKET;
			memset(&pr, 0, sizeof(pr));
			pr.ticket = ri.ticket;
			pr.nr = i;
			strlcpy(pr.anchor, anchor_name, MAXPATHLEN);
#endif /* USE_LIBPFCTL */
			if(ioctl(dev, DIOCCHANGERULE, &pr) < 0) {
				syslog(LOG_ERR, "ioctl(dev, DIOCCHANGERULE, ...) PF_CHANGE_GET_TICKET: %m");
				release_ticket(dev, tnum);
				return -1;
			}
			pr.action = PF_CHANGE_REMOVE;
			pr.nr = i;
			if(ioctl(dev, DIOCCHANGERULE, &pr) < 0) {
				syslog(LOG_ERR, "ioctl(dev, DIOCCHANGERULE, ...) PF_CHANGE_REMOVE: %m");
				release_ticket(dev, tnum);
				return -1;
			}
			release_ticket(dev, tnum);
			return 0;
		}
	}
	release_ticket(dev, tnum);
	/* not found */
	return -2;
}

int
get_pinhole_info(unsigned short uid,
                 char * rem_host, int rem_hostlen, unsigned short * rem_port,
                 char * int_client, int int_clientlen, unsigned short * int_port,
                 int * proto, char * desc, int desclen,
                 unsigned int * timestamp,
                 u_int64_t * packets, u_int64_t * bytes)
{
	int i, n;
	unsigned int tnum;
#undef RULE
#ifdef USE_LIBPFCTL
	struct pfctl_rules_info ri;
	struct pfctl_rule rule;
#define RULE (rule)
	char anchor_call[MAXPATHLEN] = "";
#else /* USE_LIBPFCTL */
	struct pfioc_rule pr;
#define RULE (pr.rule)
#endif /* USE_LIBPFCTL */
	char label_start[PF_RULE_LABEL_SIZE];
	char tmp_label[PF_RULE_LABEL_SIZE];
	char * p;

	if(dev<0) {
		syslog(LOG_ERR, "pf device is not open");
		return -1;
	}
	snprintf(label_start, sizeof(label_start),
	         "pinhole-%hu", uid);
#ifdef USE_LIBPFCTL
	if(pfctl_get_rules_info(dev, &ri, PF_PASS, anchor_name) < 0)
	{
		syslog(LOG_ERR, "pfctl_get_rules_info: %m");
		return -1;
	}
	n = ri.nr;
#ifdef PF_RELEASETICKETS
	tnum = ri.ticket;
#endif /* PF_RELEASETICKETS */
#else /* USE_LIBPFCTL */
	memset(&pr, 0, sizeof(pr));
	strlcpy(pr.anchor, anchor_name, MAXPATHLEN);
#ifndef PF_NEWSTYLE
	RULE.action = PF_PASS;
#endif
	if(ioctl(dev, DIOCGETRULES, &pr) < 0) {
		syslog(LOG_ERR, "ioctl(dev, DIOCGETRULES, ...): %m");
		return -1;
	}
	n = pr.nr;
#ifdef PF_RELEASETICKETS
	tnum = pr.ticket;
#endif
#endif /* USE_LIBPFCTL */
	for(i=0; i<n; i++) {
#ifdef USE_LIBPFCTL
		if(pfctl_get_rule(dev, i, ri.ticket, anchor_name, PF_PASS, &rule, anchor_call) < 0)
		{
			syslog(LOG_ERR, "pfctl_get_rule: %m");
			release_ticket(dev, tnum);
			return -1;
		}
		strlcpy(tmp_label, RULE.label[0], sizeof(tmp_label));
#else /* USE_LIBPFCTL */
		pr.nr = i;
		if(ioctl(dev, DIOCGETRULE, &pr) < 0) {
			syslog(LOG_ERR, "ioctl(dev, DIOCGETRULE): %m");
			release_ticket(dev, tnum);
			return -1;
		}
		strlcpy(tmp_label, RULE.label, sizeof(tmp_label));
#endif /* USE_LIBPFCTL */
		p = tmp_label;
		strsep(&p, " ");
		if(0 == strcmp(tmp_label, label_start)) {
			if(rem_host && (inet_ntop(AF_INET6, &RULE.src.addr.v.a.addr.v6, rem_host, rem_hostlen) == NULL)) {
				release_ticket(dev, tnum);
				return -1;
			}
			if(rem_port)
				*rem_port = ntohs(RULE.src.port[0]);
			if(int_client && (inet_ntop(AF_INET6, &RULE.dst.addr.v.a.addr.v6, int_client, int_clientlen) == NULL)) {
				release_ticket(dev, tnum);
				return -1;
			}
			if(int_port)
				*int_port = ntohs(RULE.dst.port[0]);
			if(proto)
				*proto = RULE.proto;
			if(timestamp)
				sscanf(p, "ts-%u", timestamp);
			if(desc) {
				strsep(&p, " ");
				if(p) {
					strlcpy(desc, p, desclen);
				} else {
					desc[0] = '\0';
				}
			}
#ifdef PFRULE_INOUT_COUNTS
			if(packets)
				*packets = RULE.packets[0] + RULE.packets[1];
			if(bytes)
				*bytes = RULE.bytes[0] + RULE.bytes[1];
#else
			if(packets)
				*packets = RULE.packets;
			if(bytes)
				*bytes = RULE.bytes;
#endif
			release_ticket(dev, tnum);
			return 0;
		}
	}
	release_ticket(dev, tnum);
	/* not found */
	return -2;
}

int update_pinhole(unsigned short uid, unsigned int timestamp)
{
	/* TODO :
	 * As it is not possible to change rule label, we should :
	 * 1 - delete
	 * 2 - Add new
	 * the stats of the rule will then be reset :( */
	UNUSED(uid); UNUSED(timestamp);
	return -42; /* not implemented */
}

/* return the number of rules removed
 * or a negative integer in case of error */
int clean_pinhole_list(unsigned int * next_timestamp)
{
	int i;
#undef RULE
#ifdef USE_LIBPFCTL
	struct pfctl_rules_info ri;
	struct pfctl_rule rule;
#define RULE (rule)
	char anchor_call[MAXPATHLEN] = "";
#else /* USE_LIBPFCTL */
	struct pfioc_rule pr;
#define RULE (pr.rule)
#endif /* USE_LIBPFCTL */
	time_t current_time;
	unsigned int ts, tnum;
	int uid;
	unsigned int min_ts = UINT_MAX;
	int min_uid = INT_MAX, max_uid = -1;
	int n = 0;

	if(dev<0) {
		syslog(LOG_ERR, "pf device is not open");
		return -1;
	}
	current_time = upnp_time();
#ifdef USE_LIBPFCTL
	if(pfctl_get_rules_info(dev, &ri, PF_PASS, anchor_name) < 0)
	{
		syslog(LOG_ERR, "pfctl_get_rules_info: %m");
		return -1;
	}
	i = ri.nr - 1;
#ifdef PF_RELEASETICKETS
	tnum = ri.ticket;
#endif /* PF_RELEASETICKETS */
#else /* USE_LIBPFCTL */
	memset(&pr, 0, sizeof(pr));
	strlcpy(pr.anchor, anchor_name, MAXPATHLEN);
#ifndef PF_NEWSTYLE
	RULE.action = PF_PASS;
#endif
	if(ioctl(dev, DIOCGETRULES, &pr) < 0) {
		syslog(LOG_ERR, "ioctl(dev, DIOCGETRULES, ...): %m");
		return -1;
	}
	i = pr.nr - 1;
#ifdef PF_RELEASETICKETS
	tnum = pr.ticket;
#endif
#endif /* USE_LIBPFCTL */
	for(; i >= 0; i--) {
#ifdef USE_LIBPFCTL
		if(pfctl_get_rule(dev, i, ri.ticket, anchor_name, PF_PASS, &rule, anchor_call) < 0)
		{
			syslog(LOG_ERR, "pfctl_get_rule: %m");
			release_ticket(dev, tnum);
			return -1;
		}
		if(sscanf(RULE.label[0], PINEHOLE_LABEL_FORMAT_SKIPDESC, &uid, &ts) != 2) {
			syslog(LOG_DEBUG, "rule with label '%s' is not a IGD pinhole", RULE.label[0]);
			continue;
		}
#else /* USE_LIBPFCTL */
		pr.nr = i;
		if(ioctl(dev, DIOCGETRULE, &pr) < 0) {
			syslog(LOG_ERR, "ioctl(dev, DIOCGETRULE): %m");
			release_ticket(dev, tnum);
			return -1;
		}
		if(sscanf(RULE.label, PINEHOLE_LABEL_FORMAT_SKIPDESC, &uid, &ts) != 2) {
			syslog(LOG_DEBUG, "rule with label '%s' is not a IGD pinhole", RULE.label);
			continue;
		}
#endif /* USE_LIBPFCTL */
		if(ts <= (unsigned int)current_time) {
#ifdef USE_LIBPFCTL
			/* TODO: convert DIOCCHANGERULE to libpfctl */
			struct pfioc_rule pr;
			pr.action = PF_CHANGE_GET_TICKET;
			memset(&pr, 0, sizeof(pr));
			pr.ticket = ri.ticket;
			pr.nr = i;
			strlcpy(pr.anchor, anchor_name, MAXPATHLEN);
			syslog(LOG_INFO, "removing expired pinhole '%s'", RULE.label[0]);
#else /* USE_LIBPFCTL */
			syslog(LOG_INFO, "removing expired pinhole '%s'", RULE.label);
#endif /* USE_LIBPFCTL */
			pr.action = PF_CHANGE_GET_TICKET;
			if(ioctl(dev, DIOCCHANGERULE, &pr) < 0) {
				syslog(LOG_ERR, "ioctl(dev, DIOCCHANGERULE, ...) PF_CHANGE_GET_TICKET: %m");
				release_ticket(dev, tnum);
				return -1;
			}
			pr.action = PF_CHANGE_REMOVE;
			pr.nr = i;
			if(ioctl(dev, DIOCCHANGERULE, &pr) < 0) {
				syslog(LOG_ERR, "ioctl(dev, DIOCCHANGERULE, ...) PF_CHANGE_REMOVE: %m");
				release_ticket(dev, tnum);
				return -1;
			}
			n++;
#ifdef USE_LIBPFCTL
			if(pfctl_get_rules_info(dev, &ri, PF_PASS, anchor_name) < 0)
			{
				syslog(LOG_ERR, "pfctl_get_rules_info: %m");
				return -1;
			}
#ifdef PF_RELEASETICKETS
			tnum = ri.ticket;
#endif /* PF_RELEASETICKETS */
#else /* USE_LIBPFCTL */
#ifndef PF_NEWSTYLE
			RULE.action = PF_PASS;
#endif
			release_ticket(dev, tnum);
			if(ioctl(dev, DIOCGETRULES, &pr) < 0) {
				syslog(LOG_ERR, "ioctl(dev, DIOCGETRULES, ...): %m");
				return -1;
			}
#ifdef PF_RELEASETICKETS
			tnum = pr.ticket;
#endif
#endif /* USE_LIBPFCTL */
		} else {
			if(uid > max_uid)
				max_uid = uid;
			else if(uid < min_uid)
				min_uid = uid;
			if(ts < min_ts)
				min_ts = ts;
		}
	}
	if(next_timestamp && (min_ts != UINT_MAX))
		*next_timestamp = min_ts;
	if(max_uid > 0) {
		if(((min_uid - 32000) <= next_uid) && (next_uid <= max_uid)) {
			next_uid = max_uid + 1;
		}
		if(next_uid >= 65535) {
			next_uid = 1;
		}
	}
	release_ticket(dev, tnum);
	return n;	/* number of rules removed */
}

#endif /* ENABLE_UPNPPINHOLE */
