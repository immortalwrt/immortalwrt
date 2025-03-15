/* $Id: obsdrdr.c,v 1.102 2023/12/07 18:56:32 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2024 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

/*
 * pf rules created (with ext_if = xl1)
 * - OpenBSD up to version 4.6 :
 *     rdr pass on xl1 inet proto udp from any to any port = 54321 \
 *         keep state label "test label" -> 192.168.0.42 port 12345
 *   or a rdr rule + a pass rule :
 *     rdr quick on xl1 inet proto udp from any to any port = 54321 \
 *         keep state label "test label" -> 192.168.0.42 port 12345
 *     pass in quick on xl1 inet proto udp from any to 192.168.0.42 port = 12345 \
 *          flags S/SA keep state label "test label"
 *
 * - OpenBSD starting from version 4.7
 *     match in on xl1 inet proto udp from any to any port 54321 \
 *            label "test label" rdr-to 192.168.0.42 port 12345
 *   or
 *     pass in quick on xl1 inet proto udp from any to any port 54321 \
 *            label "test label" rdr-to 192.168.0.42 port 12345
 *
 *
 *
 * Macros/#defines :
 * - PF_ENABLE_FILTER_RULES
 *   If set, two rules are created : rdr + pass. Else a rdr/pass rule
 *   is created.
 * - USE_IFNAME_IN_RULES
 *   If set the interface name is set in the rule.
 * - PFRULE_INOUT_COUNTS
 *   Must be set with OpenBSD version 3.8 and up, FreeBSD 7.0+, DragonFly 2.8+
 *   and OS X with pf.
 * - PFRULE_HAS_RTABLEID
 *   Must be set with OpenBSD version 4.0 and up.
 * - PF_NEWSTYLE
 *   Must be set with OpenBSD version 4.7 and up. FreeBSD/pfSense is old style.
 * - USE_LIBPFCTL
 *   libpfctl was introduced in FreeBSD 14. Starting with FreeBSD 15,
 *   several ioctl calls (such as DIOCGETRULE) are removed, so libpfctl has
 *   to be used.
 */

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

#include "../macros.h"
#include "obsdrdr.h"
#include "../upnpglobalvars.h"
#include "../getifaddr.h"
#include "rtickets.h"

#ifndef USE_PF
#error "USE_PF macro is undefined, check consistency between config.h and Makefile"
#else

/* list to keep timestamps for port mappings having a lease duration */
struct timestamp_entry {
	struct timestamp_entry * next;
	unsigned int timestamp;
	unsigned short eport;
	short protocol;
};

static struct timestamp_entry * timestamp_list = NULL;

static unsigned int
get_timestamp(unsigned short eport, int proto)
{
	struct timestamp_entry * e;
	e = timestamp_list;
	while(e) {
		if(e->eport == eport && e->protocol == (short)proto)
			return e->timestamp;
		e = e->next;
	}
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
			/* remove the entry */
			*p = e->next;
			free(e);
			return;
		}
		p = &(e->next);
		e = *p;
	}
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
	}
	else
	{
		syslog(LOG_ERR, "add_timestamp_entry() malloc(%lu) error",
		       sizeof(struct timestamp_entry));
	}
}

/* /dev/pf when opened */
extern int dev;	/* global also used in pfpinhole.c */
int dev = -1;

/* shutdown_redirect() :
 * close the /dev/pf device */
void
shutdown_redirect(void)
{
	if(close(dev)<0)
		syslog(LOG_ERR, "close(\"/dev/pf\"): %m");
	dev = -1;
}

/* open the device */
int
init_redirect(void)
{
#ifdef USE_LIBPFCTL
	struct pfctl_status *status;
#else
	struct pf_status status;
#endif
	if(dev>=0)
		shutdown_redirect();
	dev = open("/dev/pf", O_RDWR);
	if(dev<0) {
		syslog(LOG_ERR, "open(\"/dev/pf\"): %m");
		return -1;
	}
#ifdef USE_LIBPFCTL
	status = pfctl_get_status(dev);
	if (status == NULL) {
		syslog(LOG_ERR, "pfctl_get_status: %m");
		return -1;
	}
	if(!status->running) {
		pfctl_free_status(status);
		syslog(LOG_ERR, "pf is disabled");
		return -1;
	}
	pfctl_free_status(status);
#else
	if(ioctl(dev, DIOCGETSTATUS, &status)<0) {
		syslog(LOG_ERR, "DIOCGETSTATUS: %m");
		return -1;
	}
	if(!status.running) {
		syslog(LOG_ERR, "pf is disabled");
		return -1;
	}
#endif
	return 0;
}

#if TEST
/* for debug */
int
clear_redirect_rules(void)
{
	struct pfioc_trans io;
	struct pfioc_trans_e ioe;
	if(dev<0) {
		syslog(LOG_ERR, "pf device is not open");
		return -1;
	}
	memset(&ioe, 0, sizeof(ioe));
	io.size = 1;
	io.esize = sizeof(ioe);
	io.array = &ioe;
#ifndef PF_NEWSTYLE
	ioe.rs_num = PF_RULESET_RDR;
#else
	ioe.type = PF_TRANS_RULESET;
#endif
	strlcpy(ioe.anchor, anchor_name, MAXPATHLEN);
	if(ioctl(dev, DIOCXBEGIN, &io) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCXBEGIN, ...): %m");
		goto error;
	}
	if(ioctl(dev, DIOCXCOMMIT, &io) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCXCOMMIT, ...): %m");
		goto error;
	}
	return 0;
error:
	return -1;
}

int
clear_filter_rules(void)
{
#ifndef PF_ENABLE_FILTER_RULES
	return 0;
#else
	struct pfioc_trans io;
	struct pfioc_trans_e ioe;
	if(dev<0) {
		syslog(LOG_ERR, "pf device is not open");
		return -1;
	}
	memset(&ioe, 0, sizeof(ioe));
	io.size = 1;
	io.esize = sizeof(ioe);
	io.array = &ioe;
#ifndef PF_NEWSTYLE
	ioe.rs_num = PF_RULESET_FILTER;
#else
	/* ? */
	ioe.type = PF_TRANS_RULESET;
#endif
	strlcpy(ioe.anchor, anchor_name, MAXPATHLEN);
	if(ioctl(dev, DIOCXBEGIN, &io) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCXBEGIN, ...): %m");
		goto error;
	}
	if(ioctl(dev, DIOCXCOMMIT, &io) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCXCOMMIT, ...): %m");
		goto error;
	}
	return 0;
error:
	return -1;
#endif
}

int
clear_nat_rules(void)
{
	struct pfioc_trans io;
	struct pfioc_trans_e ioe;
	if(dev<0) {
		syslog(LOG_ERR, "pf device is not open");
		return -1;
	}
	memset(&ioe, 0, sizeof(ioe));
	io.size = 1;
	io.esize = sizeof(ioe);
	io.array = &ioe;
#ifndef PF_NEWSTYLE
	ioe.rs_num = PF_RULESET_NAT;
#else
	/* ? */
	ioe.type = PF_TRANS_RULESET;
#endif
	strlcpy(ioe.anchor, anchor_name, MAXPATHLEN);
	if(ioctl(dev, DIOCXBEGIN, &io) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCXBEGIN, ...): %m");
		goto error;
	}
	if(ioctl(dev, DIOCXCOMMIT, &io) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCXCOMMIT, ...): %m");
		goto error;
	}
	return 0;
error:
	return -1;
}
#endif

#ifdef ENABLE_PORT_TRIGGERING
/* add_nat_rule()
 * nat on re0 inet proto udp from 192.168.1.49 port 3074 to any -> (re0) port 3148
 * <re0> is ext_if, (re0) is ext_ip
 * should be created when a port mapping (3148 => 192.168.1.49:3074 UDP) is created.
 * for symetric NAT / UPnP IGD NAT Port Triggering */
int add_nat_rule(const char * ifname,
                 const char * rhost, unsigned short eport,
                 const char * iaddr, unsigned short iport, int proto,
                 const char * desc)
{
	int r;
	struct pfioc_rule pcr;
#ifndef PF_NEWSTYLE
	struct pfioc_pooladdr pp;
	struct pf_pooladdr *a;
#endif
	const char * extaddr;
	char extaddr_buf[INET_ADDRSTRLEN];
	struct in_addr wan_addr;

	if(dev<0) {
		syslog(LOG_ERR, "pf device is not open");
		return -1;
	}
	if(getifaddr(ifname, extaddr_buf, INET_ADDRSTRLEN, &wan_addr, NULL) < 0) {
		syslog(LOG_WARNING, "failed to get address for interface %s", ifname);
		if(use_ext_ip_addr && use_ext_ip_addr[0] != '\0') {
			extaddr = use_ext_ip_addr;
		} else {
			return -1;	/* no address to use => failure */
		}
	} else {
		if (addr_is_reserved(&wan_addr)) {
			syslog(LOG_DEBUG, "WAN IP is reserved, it will be used for NAT");
			extaddr = extaddr_buf;
		} else if (use_ext_ip_addr && use_ext_ip_addr[0] != '\0') {
			extaddr = use_ext_ip_addr;
		} else {
			extaddr = extaddr_buf;
		}
	}
	syslog(LOG_DEBUG, "use external ip %s", extaddr);
	r = 0;
	memset(&pcr, 0, sizeof(pcr));
	strlcpy(pcr.anchor, anchor_name, MAXPATHLEN);

#ifndef PF_NEWSTYLE
	memset(&pp, 0, sizeof(pp));
	strlcpy(pp.anchor, anchor_name, MAXPATHLEN);
	if(ioctl(dev, DIOCBEGINADDRS, &pp) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCBEGINADDRS, ...): %m");
		r = -1;
	}
	else
	{
		pcr.pool_ticket = pp.ticket;
#else
	{
		pcr.rule.nat.addr.type = PF_ADDR_ADDRMASK;
		pcr.rule.rdr.addr.type = PF_ADDR_NONE;
#endif
		/*pcr.rule.src.addr.type = PF_ADDR_NONE;*/
		pcr.rule.src.addr.type = PF_ADDR_ADDRMASK;
		pcr.rule.dst.addr.type = PF_ADDR_ADDRMASK;

#ifndef PF_NEWSTYLE
		pcr.rule.action = PF_NAT;
#else
		pcr.rule.action = PF_PASS;	/* or PF_MATCH as we dont expect outbound packets to be blocked */
		pcr.rule.direction = PF_OUT;
#endif
		pcr.rule.af = AF_INET;
#ifdef USE_IFNAME_IN_RULES
		if(ifname)
			strlcpy(pcr.rule.ifname, ifname, IFNAMSIZ);
#endif
		pcr.rule.proto = proto;
		pcr.rule.log = (GETFLAG(LOGPACKETSMASK))?1:0;	/*logpackets;*/
#ifdef PFRULE_HAS_RTABLEID
		pcr.rule.rtableid = -1;	/* first appeared in OpenBSD 4.0 */
#endif
#ifdef PFRULE_HAS_ONRDOMAIN
		pcr.rule.onrdomain = -1;	/* first appeared in OpenBSD 5.0 */
#endif
		pcr.rule.quick = 1;
		pcr.rule.keep_state = PF_STATE_NORMAL;
		if(tag)
			strlcpy(pcr.rule.tagname, tag, PF_TAG_NAME_SIZE);
		strlcpy(pcr.rule.label, desc, PF_RULE_LABEL_SIZE);
#ifdef PFVAR_NEW_STYLE
		inet_pton(AF_INET, iaddr, &pcr.rule.src.addr.v.a.addr.v4addr.s_addr);
		pcr.rule.src.addr.v.a.mask.v4addr.s_addr = htonl(INADDR_NONE);
#else
		inet_pton(AF_INET, iaddr, &pcr.rule.src.addr.v.a.addr.v4.s_addr);
		pcr.rule.src.addr.v.a.mask.v4.s_addr = htonl(INADDR_NONE);
#endif
#ifdef __APPLE__
		pcr.rule.src.xport.range.op = PF_OP_EQ;
		pcr.rule.src.xport.range.port[0] = htons(iport);
		pcr.rule.src.xport.range.port[1] = htons(iport);
#else
		pcr.rule.src.port_op = PF_OP_EQ;
		pcr.rule.src.port[0] = htons(iport);
		pcr.rule.src.port[1] = htons(iport);
#endif
		if(rhost && rhost[0] != '\0' && rhost[0] != '*')
		{
#ifdef PFVAR_NEW_STYLE
			inet_pton(AF_INET, rhost, &pcr.rule.dst.addr.v.a.addr.v4addr.s_addr);
			pcr.rule.dst.addr.v.a.mask.v4addr.s_addr = htonl(INADDR_NONE);
#else
			inet_pton(AF_INET, rhost, &pcr.rule.dst.addr.v.a.addr.v4.s_addr);
			pcr.rule.dst.addr.v.a.mask.v4.s_addr = htonl(INADDR_NONE);
#endif
		}
#ifdef __APPLE__
		pcr.rule.dst.xport.range.op = PF_OP_NONE;
#else
		pcr.rule.dst.port_op = PF_OP_NONE;
#endif
		/* -> xxx.xxx.xxx.xxx port 1234 */
#ifndef PF_NEWSTYLE
		pcr.rule.rpool.proxy_port[0] = eport;
		pcr.rule.rpool.proxy_port[1] = eport;
		TAILQ_INIT(&pcr.rule.rpool.list);
		a = calloc(1, sizeof(struct pf_pooladdr));
#ifdef PFVAR_NEW_STYLE
		inet_pton(AF_INET, extaddr, &a->addr.v.a.addr.v4addr.s_addr);
		a->addr.v.a.mask.v4addr.s_addr = htonl(INADDR_NONE);
#else
		inet_pton(AF_INET, extaddr, &a->addr.v.a.addr.v4.s_addr);
		a->addr.v.a.mask.v4.s_addr = htonl(INADDR_NONE);
#endif
		TAILQ_INSERT_TAIL(&pcr.rule.rpool.list, a, entries);

		memcpy(&pp.addr, a, sizeof(struct pf_pooladdr));
		if(ioctl(dev, DIOCADDADDR, &pp) < 0)
		{
			syslog(LOG_ERR, "ioctl(dev, DIOCADDADDR, ...): %m");
			r = -1;
		}
		else
		{
#else
		pcr.rule.nat.proxy_port[0] = eport;
		pcr.rule.nat.proxy_port[1] = eport;
		inet_pton(AF_INET, extaddr, &pcr.rule.nat.addr.v.a.addr.v4.s_addr);
		pcr.rule.nat.addr.v.a.mask.v4.s_addr = htonl(INADDR_NONE);
		{
#endif /* PF_NEWSTYLE */
			pcr.action = PF_CHANGE_GET_TICKET;
			if(ioctl(dev, DIOCCHANGERULE, &pcr) < 0)
			{
				syslog(LOG_ERR, "ioctl(dev, DIOCCHANGERULE, ...) PF_CHANGE_GET_TICKET: %m");
				r = -1;
			}
			else
			{
				pcr.action = PF_CHANGE_ADD_TAIL;
				if(ioctl(dev, DIOCCHANGERULE, &pcr) < 0)
				{
					syslog(LOG_ERR, "ioctl(dev, DIOCCHANGERULE, ...) PF_CHANGE_ADD_TAIL: %m");
					r = -1;
				}
			}
		}
#ifndef PF_NEWSTYLE
		free(a);
#endif
	}
	return r;
}


/*
 * returns:  0 : OK
 *          -1 : ERROR
 *          -2 : Rule not found
 */
static int
delete_nat_rule(const char * ifname, unsigned short iport, int proto, in_addr_t iaddr)
{
	int i, n, r;
	unsigned int tnum;
#ifdef USE_LIBPFCTL
	struct pfctl_rules_info ri;
	struct pfctl_rule rule;
#define RULE (rule)
	char anchor_call[MAXPATHLEN] = "";
#else
	struct pfioc_rule pr;
#define RULE (pr.rule)
#endif
	UNUSED(ifname);
	if(dev<0) {
		syslog(LOG_ERR, "pf device is not open");
		return -1;
	}
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
	RULE.action = PF_NAT;
#else
	RULE.action = PF_PASS;	/* or PF_MATCH as we dont expect outbound packets to be blocked */
	RULE.direction = PF_OUT;
#endif
	if(ioctl(dev, DIOCGETRULES, &pr) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCGETRULES, ...): %m");
		return -1;
	}
	n = pr.nr;
#ifdef PF_RELEASETICKETS
	tnum = pr.ticket;
#endif /* PF_RELEASETICKETS */
#endif /* USE_LIBPFCTL */
	r = -2;	/* not found */
	for(i=0; i<n; i++)
	{
#ifdef USE_LIBPFCTL
		if(pfctl_get_rule(dev, i, ri.ticket, anchor_name, PF_PASS, &rule, anchor_call) < 0)
		{
			syslog(LOG_ERR, "pfctl_get_rule(): %m");
			r = -1;
			break;
		}
#else /* USE_LIBPFCTL */
		pr.nr = i;
		if(ioctl(dev, DIOCGETRULE, &pr) < 0)
		{
			syslog(LOG_ERR, "ioctl(dev, DIOCGETRULE): %m");
			r = -1;
			break;
		}
#endif /* USE_LIBPFCTL */
#ifdef TEST
		syslog(LOG_DEBUG, "%2d port=%hu proto=%d addr=%8x    %8x",
		       i, ntohs(RULE.src.port[0]), RULE.proto,
		       RULE.src.addr.v.a.addr.v4.s_addr, iaddr);
#endif /* TEST */
		if(iport == ntohs(RULE.src.port[0])
		 && RULE.proto == proto
		 && iaddr == RULE.src.addr.v.a.addr.v4.s_addr)
		{
#ifdef USE_LIBPFCTL
			/* to change with the libpfctl alternative to DIOCCHANGERULE */
			struct pfioc_rule pr;
			memset(&pr, 0, sizeof(pr));
			strlcpy(pr.anchor, anchor_name, MAXPATHLEN);
			pr.ticket = ri.ticket;
#endif
			pr.action = PF_CHANGE_GET_TICKET;
			if(ioctl(dev, DIOCCHANGERULE, &pr) < 0)
			{
				syslog(LOG_ERR, "ioctl(dev, DIOCCHANGERULE, ...) PF_CHANGE_GET_TICKET: %m");
				r = -1;
				break;
			}
			pr.action = PF_CHANGE_REMOVE;
			pr.nr = i;
			if(ioctl(dev, DIOCCHANGERULE, &pr) < 0)
			{
				syslog(LOG_ERR, "ioctl(dev, DIOCCHANGERULE, ...) PF_CHANGE_REMOVE: %m");
				r = -1;
			}
			else
			{
				r = 0;
			}
			break;
		}
	}
	if (r == -2)
		syslog(LOG_NOTICE, "could not find nat rule to delete iport=%hu addr=%8x", iport, ntohl(iaddr));
	release_ticket(dev, tnum);
	return r;
}
#endif

/* add_redirect_rule2() :
 * create a rdr rule */
int
add_redirect_rule2(const char * ifname,
                   const char * rhost, unsigned short eport,
                   const char * iaddr, unsigned short iport, int proto,
                   const char * desc, unsigned int timestamp)
{
	int r;
	struct pfioc_rule pcr;
#ifndef PF_NEWSTYLE
	struct pfioc_pooladdr pp;
	struct pf_pooladdr *a;
#endif

	if(dev<0) {
		syslog(LOG_ERR, "pf device is not open");
		return -1;
	}
	r = 0;
	memset(&pcr, 0, sizeof(pcr));
	strlcpy(pcr.anchor, anchor_name, MAXPATHLEN);

#ifndef PF_NEWSTYLE
	memset(&pp, 0, sizeof(pp));
	strlcpy(pp.anchor, anchor_name, MAXPATHLEN);
	if(ioctl(dev, DIOCBEGINADDRS, &pp) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCBEGINADDRS, ...): %m");
		r = -1;
	}
	else
	{
		pcr.pool_ticket = pp.ticket;
#else
	if(1)
	{
		pcr.rule.direction = PF_IN;
		pcr.rule.nat.addr.type = PF_ADDR_NONE;
		pcr.rule.rdr.addr.type = PF_ADDR_ADDRMASK;
#endif
		/*pcr.rule.src.addr.type = PF_ADDR_NONE;*/
		pcr.rule.src.addr.type = PF_ADDR_ADDRMASK;
		pcr.rule.dst.addr.type = PF_ADDR_ADDRMASK;

#ifdef __APPLE__
		pcr.rule.dst.xport.range.op = PF_OP_EQ;
		pcr.rule.dst.xport.range.port[0] = htons(eport);
		pcr.rule.dst.xport.range.port[1] = htons(eport);
#else
		pcr.rule.dst.port_op = PF_OP_EQ;
		pcr.rule.dst.port[0] = htons(eport);
		pcr.rule.dst.port[1] = htons(eport);
#endif
#ifndef PF_NEWSTYLE
		pcr.rule.action = PF_RDR;
#ifndef PF_ENABLE_FILTER_RULES
		pcr.rule.natpass = 1;
#else
		pcr.rule.natpass = 0;
#endif
#else
#ifndef PF_ENABLE_FILTER_RULES
		pcr.rule.action = PF_PASS;
#else
		pcr.rule.action = PF_MATCH;
#endif
#endif
		pcr.rule.af = AF_INET;
#ifdef USE_IFNAME_IN_RULES
		if(ifname)
			strlcpy(pcr.rule.ifname, ifname, IFNAMSIZ);
#endif
		pcr.rule.proto = proto;
		pcr.rule.log = (GETFLAG(LOGPACKETSMASK))?1:0;	/*logpackets;*/
#ifdef PFRULE_HAS_RTABLEID
		pcr.rule.rtableid = -1;	/* first appeared in OpenBSD 4.0 */
#endif
#ifdef PFRULE_HAS_ONRDOMAIN
		pcr.rule.onrdomain = -1;	/* first appeared in OpenBSD 5.0 */
#endif
		pcr.rule.quick = 1;
		pcr.rule.keep_state = PF_STATE_NORMAL;
		if(tag)
			strlcpy(pcr.rule.tagname, tag, PF_TAG_NAME_SIZE);
		strlcpy(pcr.rule.label, desc, PF_RULE_LABEL_SIZE);
		if(rhost && rhost[0] != '\0' && rhost[0] != '*')
		{
#ifdef PFVAR_NEW_STYLE
			inet_pton(AF_INET, rhost, &pcr.rule.src.addr.v.a.addr.v4addr.s_addr);
			pcr.rule.src.addr.v.a.mask.v4addr.s_addr = htonl(INADDR_NONE);
#else
			inet_pton(AF_INET, rhost, &pcr.rule.src.addr.v.a.addr.v4.s_addr);
			pcr.rule.src.addr.v.a.mask.v4.s_addr = htonl(INADDR_NONE);
#endif
		}
#ifdef PF_SET_DST_ADDR
		/* set dst address
		 * see https://github.com/miniupnp/miniupnp/issues/231 */
		if(use_ext_ip_addr && use_ext_ip_addr[0] != '\0')
		{
#ifdef PFVAR_NEW_STYLE
			inet_pton(AF_INET, use_ext_ip_addr, &pcr.rule.dst.addr.v.a.addr.v4addr.s_addr);
			pcr.rule.dst.addr.v.a.mask.v4addr.s_addr = htonl(INADDR_NONE);
#else
			inet_pton(AF_INET, use_ext_ip_addr, &pcr.rule.dst.addr.v.a.addr.v4.s_addr);
			pcr.rule.dst.addr.v.a.mask.v4.s_addr = htonl(INADDR_NONE);
#endif
		}
#endif /* PF_SET_DST_ADDR */
#ifndef PF_NEWSTYLE
		pcr.rule.rpool.proxy_port[0] = iport;
		pcr.rule.rpool.proxy_port[1] = iport;
		TAILQ_INIT(&pcr.rule.rpool.list);
		a = calloc(1, sizeof(struct pf_pooladdr));
#ifdef PFVAR_NEW_STYLE
		inet_pton(AF_INET, iaddr, &a->addr.v.a.addr.v4addr.s_addr);
		a->addr.v.a.mask.v4addr.s_addr = htonl(INADDR_NONE);
#else
		inet_pton(AF_INET, iaddr, &a->addr.v.a.addr.v4.s_addr);
		a->addr.v.a.mask.v4.s_addr = htonl(INADDR_NONE);
#endif
		TAILQ_INSERT_TAIL(&pcr.rule.rpool.list, a, entries);

		memcpy(&pp.addr, a, sizeof(struct pf_pooladdr));
		if(ioctl(dev, DIOCADDADDR, &pp) < 0)
		{
			syslog(LOG_ERR, "ioctl(dev, DIOCADDADDR, ...): %m");
			r = -1;
		}
		else
		{
#else
		pcr.rule.rdr.proxy_port[0] = iport;
		pcr.rule.rdr.proxy_port[1] = iport;
		inet_pton(AF_INET, iaddr, &pcr.rule.rdr.addr.v.a.addr.v4.s_addr);
		pcr.rule.rdr.addr.v.a.mask.v4.s_addr = htonl(INADDR_NONE);
		if(1)
		{
#endif
			pcr.action = PF_CHANGE_GET_TICKET;
			if(ioctl(dev, DIOCCHANGERULE, &pcr) < 0)
			{
				syslog(LOG_ERR, "ioctl(dev, DIOCCHANGERULE, ...) PF_CHANGE_GET_TICKET: %m");
				r = -1;
			}
			else
			{
				pcr.action = PF_CHANGE_ADD_TAIL;
				if(ioctl(dev, DIOCCHANGERULE, &pcr) < 0)
				{
					syslog(LOG_ERR, "ioctl(dev, DIOCCHANGERULE, ...) PF_CHANGE_ADD_TAIL: %m");
					r = -1;
				}
			}
		}
#ifndef PF_NEWSTYLE
		free(a);
#endif
	}
	if(r == 0 && timestamp > 0)
		add_timestamp_entry(eport, proto, timestamp);
#ifdef ENABLE_PORT_TRIGGERING
	if(r == 0 && proto == IPPROTO_UDP)
	{
		add_nat_rule(ifname, rhost, eport, iaddr, iport, proto, desc);
		/* TODO check error */
	}
#endif
	return r;
}

/* thanks to Seth Mos for this function */
int
add_filter_rule2(const char * ifname,
                 const char * rhost, const char * iaddr,
                 unsigned short eport, unsigned short iport,
				 int proto, const char * desc)
{
#ifndef PF_ENABLE_FILTER_RULES
	UNUSED(ifname);
	UNUSED(rhost); UNUSED(iaddr);
	UNUSED(eport); UNUSED(iport);
	UNUSED(proto); UNUSED(desc);
	return 0;
#else
	int r;
	struct pfioc_rule pcr;
#ifndef PF_NEWSTYLE
	struct pfioc_pooladdr pp;
#endif
#ifndef USE_IFNAME_IN_RULES
	UNUSED(ifname);
#endif
	UNUSED(eport);
	if(dev<0) {
		syslog(LOG_ERR, "pf device is not open");
		return -1;
	}
	r = 0;
	memset(&pcr, 0, sizeof(pcr));
	strlcpy(pcr.anchor, anchor_name, MAXPATHLEN);

#ifndef PF_NEWSTYLE
	memset(&pp, 0, sizeof(pp));
	strlcpy(pp.anchor, anchor_name, MAXPATHLEN);
	if(ioctl(dev, DIOCBEGINADDRS, &pp) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCBEGINADDRS, ...): %m");
		r = -1;
	}
	else
	{
		pcr.pool_ticket = pp.ticket;
#else
	if(1)
	{
#endif
		pcr.rule.dst.port_op = PF_OP_EQ;
		pcr.rule.dst.port[0] = htons(iport);
		pcr.rule.direction = PF_IN;
		pcr.rule.action = PF_PASS;
		pcr.rule.af = AF_INET;
#ifdef USE_IFNAME_IN_RULES
		if(ifname)
			strlcpy(pcr.rule.ifname, ifname, IFNAMSIZ);
#endif
		pcr.rule.proto = proto;
		pcr.rule.quick = (GETFLAG(PFNOQUICKRULESMASK))?0:1;
		pcr.rule.log = (GETFLAG(LOGPACKETSMASK))?1:0;	/*logpackets;*/
/* see the discussion on the forum :
 * http://miniupnp.tuxfamily.org/forum/viewtopic.php?p=638 */
		pcr.rule.flags = TH_SYN;
		pcr.rule.flagset = (TH_SYN|TH_ACK);
#ifdef PFRULE_HAS_RTABLEID
		pcr.rule.rtableid = -1;	/* first appeared in OpenBSD 4.0 */
#endif
#ifdef PFRULE_HAS_ONRDOMAIN
		pcr.rule.onrdomain = -1;	/* first appeared in OpenBSD 5.0 */
#endif
		pcr.rule.keep_state = 1;
		strlcpy(pcr.rule.label, desc, PF_RULE_LABEL_SIZE);
		if(queue)
			strlcpy(pcr.rule.qname, queue, PF_QNAME_SIZE);
		if(tag)
			strlcpy(pcr.rule.tagname, tag, PF_TAG_NAME_SIZE);

		if(rhost && rhost[0] != '\0' && rhost[0] != '*')
		{
			inet_pton(AF_INET, rhost, &pcr.rule.src.addr.v.a.addr.v4.s_addr);
			pcr.rule.src.addr.v.a.mask.v4.s_addr = htonl(INADDR_NONE);
		}
		/* we want any - iaddr port = # keep state label */
		inet_pton(AF_INET, iaddr, &pcr.rule.dst.addr.v.a.addr.v4.s_addr);
		pcr.rule.dst.addr.v.a.mask.v4.s_addr = htonl(INADDR_NONE);
#ifndef PF_NEWSTYLE
		pcr.rule.rpool.proxy_port[0] = iport;
		pcr.rule.rpool.proxy_port[1] = iport;
		TAILQ_INIT(&pcr.rule.rpool.list);
#endif
		if(1)
		{
			pcr.action = PF_CHANGE_GET_TICKET;
        	if(ioctl(dev, DIOCCHANGERULE, &pcr) < 0)
			{
            	syslog(LOG_ERR, "ioctl(dev, DIOCCHANGERULE, ...) PF_CHANGE_GET_TICKET: %m");
				r = -1;
			}
			else
			{
				pcr.action = PF_CHANGE_ADD_TAIL;
				if(ioctl(dev, DIOCCHANGERULE, &pcr) < 0)
				{
					syslog(LOG_ERR, "ioctl(dev, DIOCCHANGERULE, ...) PF_CHANGE_ADD_TAIL: %m");
					r = -1;
				}
			}
		}
	}
	return r;
#endif
}

/* get_redirect_rule_count()
 * return value : -1 for error or the number of rdr rules */
int
get_redirect_rule_count(const char * ifname)
{
#ifdef USE_LIBPFCTL
	struct pfctl_rules_info ri;
#else
	struct pfioc_rule pr;
#endif
	UNUSED(ifname);

	if(dev<0) {
		syslog(LOG_ERR, "pf device is not open");
		return -1;
	}
#ifdef USE_LIBPFCTL
	if (pfctl_get_rules_info(dev, &ri, PF_RDR, anchor_name) < 0)
	{
		syslog(LOG_ERR, "pfctl_get_rules_info: %m");
		return -1;
	}
	return ri.nr;
#else
	memset(&pr, 0, sizeof(pr));
	strlcpy(pr.anchor, anchor_name, MAXPATHLEN);
#ifndef PF_NEWSTYLE
	RULE.action = PF_RDR;
#endif
	if(ioctl(dev, DIOCGETRULES, &pr) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCGETRULES, ...): %m");
		return -1;
	}
	release_ticket(dev, pr.ticket);
	return pr.nr;
#endif
}

/* get_redirect_rule()
 * return value : 0 success (found)
 * -1 = error
 * -2 = rule not found */
int
get_redirect_rule(const char * ifname, unsigned short eport, int proto,
                  char * iaddr, int iaddrlen, unsigned short * iport,
                  char * desc, int desclen,
                  char * rhost, int rhostlen,
                  unsigned int * timestamp,
                  u_int64_t * packets, u_int64_t * bytes)
{
	int i, n, r;
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
#ifndef PF_NEWSTYLE
	struct pfioc_pooladdr pp;
#endif
	UNUSED(ifname);

	if(dev<0) {
		syslog(LOG_ERR, "pf device is not open");
		return -1;
	}
#ifdef USE_LIBPFCTL
	if(pfctl_get_rules_info(dev, &ri, PF_RDR, anchor_name) < 0)
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
	RULE.action = PF_RDR;
#endif
	if(ioctl(dev, DIOCGETRULES, &pr) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCGETRULES, ...): %m");
		return -1;
	}
	n = pr.nr;
#ifdef PF_RELEASETICKETS
	tnum = pr.ticket;
#endif /* PF_RELEASETICKETS */
#endif /* USE_LIBPFCTL */
	r = -2;
	for(i=0; i<n; i++)
	{
#ifdef USE_LIBPFCTL
		if(pfctl_get_rule(dev, i, ri.ticket, anchor_name, PF_RDR, &rule, anchor_call) < 0)
		{
			syslog(LOG_ERR, "pfctl_get_rule: %m");
			r = -1;
			break;
		}
#else /* USE_LIBPFCTL */
		pr.nr = i;
		if(ioctl(dev, DIOCGETRULE, &pr) < 0)
		{
			syslog(LOG_ERR, "ioctl(dev, DIOCGETRULE): %m");
			r = -1;
			break;
		}
#endif /* USE_LIBPFCTL */
#ifdef __APPLE__
		if( (eport == ntohs(RULE.dst.xport.range.port[0]))
		  && (eport == ntohs(RULE.dst.xport.range.port[1]))
#else
		if( (eport == ntohs(RULE.dst.port[0]))
		  && (eport == ntohs(RULE.dst.port[1]))
#endif
		  && (RULE.proto == proto) )
		{
#ifndef PF_NEWSTYLE
			*iport = RULE.rpool.proxy_port[0];
#else
			*iport = RULE.rdr.proxy_port[0];
#endif
			if(desc)
#ifdef USE_LIBPFCTL
				strlcpy(desc, RULE.label[0], desclen);
#else /* USE_LIBPFCTL */
				strlcpy(desc, RULE.label, desclen);
#endif /* USE_LIBPFCTL */
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
#ifndef PF_NEWSTYLE
			memset(&pp, 0, sizeof(pp));
			strlcpy(pp.anchor, anchor_name, MAXPATHLEN);
			pp.r_action = PF_RDR;
			pp.r_num = i;
#ifdef USE_LIBPFCTL
			pp.ticket = ri.ticket;
#else /* USE_LIBPFCTL */
			pp.ticket = pr.ticket;
#endif /* USE_LIBPFCTL */
			if(ioctl(dev, DIOCGETADDRS, &pp) < 0)
			{
				syslog(LOG_ERR, "ioctl(dev, DIOCGETADDRS, ...): %m");
				r = -1;
				break;
			}
			if(pp.nr != 1)
			{
				syslog(LOG_NOTICE, "No address associated with pf rule");
				r = -1;
				break;
			}
			pp.nr = 0;	/* first */
			if(ioctl(dev, DIOCGETADDR, &pp) < 0)
			{
				syslog(LOG_ERR, "ioctl(dev, DIOCGETADDR, ...): %m");
				r = -1;
				break;
			}
#ifdef PFVAR_NEW_STYLE
			inet_ntop(AF_INET, &pp.addr.addr.v.a.addr.v4addr.s_addr,
			          iaddr, iaddrlen);
#else
			inet_ntop(AF_INET, &pp.addr.addr.v.a.addr.v4.s_addr,
			          iaddr, iaddrlen);
#endif
#else
			inet_ntop(AF_INET, &RULE.rdr.addr.v.a.addr.v4.s_addr,
			          iaddr, iaddrlen);
#endif
			if(rhost && rhostlen > 0)
			{
#ifdef PFVAR_NEW_STYLE
				if (RULE.src.addr.v.a.addr.v4addr.s_addr == 0)
#else
				if (RULE.src.addr.v.a.addr.v4.s_addr == 0)
#endif
				{
					rhost[0] = '\0'; /* empty string */
				}
				else
				{
#ifdef PFVAR_NEW_STYLE
					inet_ntop(AF_INET, &RULE.src.addr.v.a.addr.v4addr.s_addr,
					          rhost, rhostlen);
#else
					inet_ntop(AF_INET, &RULE.src.addr.v.a.addr.v4.s_addr,
					          rhost, rhostlen);
#endif
				}
			}
			if(timestamp)
				*timestamp = get_timestamp(eport, proto);
			r = 0;
			break;
		}
	}
	release_ticket(dev, tnum);
	return r;
}

#define priv_delete_redirect_rule(ifname, eport, proto, iport, \
                                  iaddr, rhost, rhostlen) \
        priv_delete_redirect_rule_check_desc(ifname, eport, proto, iport, \
                                             iaddr, rhost, rhostlen, 0, NULL)
/* if check_desc is true, only delete the rule if the description differs.
 * returns : -2 : rule not found
 *           -1 : error
 *            0 : rule deleted
 *            1 : rule untouched
 */
static int
priv_delete_redirect_rule_check_desc(const char * ifname, unsigned short eport,
                          int proto, unsigned short * iport,
                          in_addr_t * iaddr, char * rhost, int rhostlen,
                          int check_desc, const char * desc)
{
	int i, n, r;
	unsigned int tnum;
	struct pfioc_rule pr;
#ifdef USE_LIBPFCTL
	struct pfctl_rules_info ri;
	struct pfctl_rule rule;
#define RULE (rule)
	char anchor_call[MAXPATHLEN] = "";
#else /* USE_LIBPFCTL */
#define RULE (pr.rule)
#endif /* USE_LIBPFCTL */
	UNUSED(ifname);

	if(dev<0) {
		syslog(LOG_ERR, "pf device is not open");
		return -1;
	}
	memset(&pr, 0, sizeof(pr));
	strlcpy(pr.anchor, anchor_name, MAXPATHLEN);
#ifndef PF_NEWSTYLE
	RULE.action = PF_RDR;
	pr.rule.action = PF_RDR;	/* used with USE_LIBPFCTL for the DIOCCHANGERULE calls */
#endif
#ifdef USE_LIBPFCTL
	if (pfctl_get_rules_info(dev, &ri, PF_RDR, anchor_name) < 0)
	{
		syslog(LOG_ERR, "pfctl_get_rules_info: %m");
		return -1;
	}
	pr.ticket = ri.ticket;
	n = ri.nr;
#else /* USE_LIBPFCTL */
	if(ioctl(dev, DIOCGETRULES, &pr) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCGETRULES, ...): %m");
		return -1;
	}
	n = pr.nr;
#ifdef PF_RELEASETICKETS
	tnum = pr.ticket;
#endif /* PF_RELEASETICKETS */
#endif /* USE_LIBPFCTL */
	r = -2;
	for(i=0; i<n; i++)
	{
#ifdef USE_LIBPFCTL
		if(pfctl_get_rule(dev, i, ri.ticket, anchor_name, PF_RDR, &rule, anchor_call) < 0)
#else /* USE_LIBPFCTL */
		pr.nr = i;
		if(ioctl(dev, DIOCGETRULE, &pr) < 0)
#endif /* USE_LIBPFCTL */
		{
			syslog(LOG_ERR, "ioctl(dev, DIOCGETRULE): %m");
			r = -1;
			break;
		}
#ifdef __APPLE__
		if( (eport == ntohs(RULE.dst.xport.range.port[0]))
		  && (eport == ntohs(RULE.dst.xport.range.port[1]))
#else
		if( (eport == ntohs(RULE.dst.port[0]))
		  && (eport == ntohs(RULE.dst.port[1]))
#endif
		  && (RULE.proto == proto) )
		{
			/* retrieve iport in order to remove filter rule */
#ifndef PF_NEWSTYLE
			if(iport) *iport = RULE.rpool.proxy_port[0];
			if(iaddr)
			{
				/* retrieve internal address */
				struct pfioc_pooladdr pp;
				memset(&pp, 0, sizeof(pp));
				strlcpy(pp.anchor, anchor_name, MAXPATHLEN);
				pp.r_action = PF_RDR;
				pp.r_num = i;
#ifdef USE_LIBPFCTL
				pp.ticket = ri.ticket;
#else /* USE_LIBPFCTL */
				pp.ticket = pr.ticket;
#endif /* USE_LIBPFCTL */
				if(ioctl(dev, DIOCGETADDRS, &pp) < 0)
				{
					syslog(LOG_ERR, "ioctl(dev, DIOCGETADDRS, ...): %m");
					r = -1;
					break;
				}
				if(pp.nr != 1)
				{
					syslog(LOG_NOTICE, "No address associated with pf rule");
					r = -1;
					break;
				}
				pp.nr = 0;	/* first */
				if(ioctl(dev, DIOCGETADDR, &pp) < 0)
				{
					syslog(LOG_ERR, "ioctl(dev, DIOCGETADDR, ...): %m");
					r = -1;
					break;
				}
#ifdef PFVAR_NEW_STYLE
				*iaddr = pp.addr.addr.v.a.addr.v4addr.s_addr;
#else
				*iaddr = pp.addr.addr.v.a.addr.v4.s_addr;
#endif
			}
#else
			if(iport) *iport = RULE.rdr.proxy_port[0];
			if(iaddr)
			{
				/* retrieve internal address */
				*iaddr = RULE.rdr.addr.v.a.addr.v4.s_addr;
			}
#endif
			if(rhost && rhostlen > 0)
			{
#ifdef PFVAR_NEW_STYLE
				if (RULE.src.addr.v.a.addr.v4addr.s_addr == 0)
#else
				if (RULE.src.addr.v.a.addr.v4.s_addr == 0)
#endif
					rhost[0] = '\0'; /* empty string */
				else
#ifdef PFVAR_NEW_STYLE
					inet_ntop(AF_INET, &RULE.src.addr.v.a.addr.v4addr.s_addr,
					          rhost, rhostlen);
#else
					inet_ntop(AF_INET, &RULE.src.addr.v.a.addr.v4.s_addr,
					          rhost, rhostlen);
#endif
			}
			if(check_desc) {
#ifdef USE_LIBPFCTL
				if((desc == NULL && RULE.label[0][0] == '\0') ||
				   (desc && 0 == strcmp(desc, RULE.label[0]))) {
#else /* USE_LIBPFCTL */
				if((desc == NULL && RULE.label[0] == '\0') ||
				   (desc && 0 == strcmp(desc, RULE.label))) {
#endif /* USE_LIBPFCTL */
					r = 1;
					break;
				}
			}
			pr.action = PF_CHANGE_GET_TICKET;
        	if(ioctl(dev, DIOCCHANGERULE, &pr) < 0)
			{
            	syslog(LOG_ERR, "ioctl(dev, DIOCCHANGERULE, ...) PF_CHANGE_GET_TICKET: %m");
				r = -1;
				break;
			}
			pr.action = PF_CHANGE_REMOVE;
			pr.nr = i;
			if(ioctl(dev, DIOCCHANGERULE, &pr) < 0)
			{
				syslog(LOG_ERR, "ioctl(dev, DIOCCHANGERULE, ...) PF_CHANGE_REMOVE: %m");
				r = -1;
				break;
			}
			remove_timestamp_entry(eport, proto);
			r = 0;
			break;
		}
	}
	if (r == -2)
		syslog(LOG_NOTICE, "could not find redirect rule to delete eport=%hu", eport);
	release_ticket(dev, tnum);
	return r;
}

int
delete_redirect_rule(const char * ifname, unsigned short eport,
                    int proto)
{
	return priv_delete_redirect_rule(ifname, eport, proto, NULL, NULL, NULL, 0);
}

/* returns:  0 : OK
 *          -1 : Error
 *          -2 : rule not found */
static int
priv_delete_filter_rule(const char * ifname, unsigned short iport,
                        int proto, in_addr_t iaddr)
{
#ifndef PF_ENABLE_FILTER_RULES
	UNUSED(ifname); UNUSED(iport); UNUSED(proto); UNUSED(iaddr);
	return 0;
#else
	int i, n, r;
	unsigned int tnum;
	struct pfioc_rule pr;
	UNUSED(ifname);
	if(dev<0) {
		syslog(LOG_ERR, "pf device is not open");
		return -1;
	}
	memset(&pr, 0, sizeof(pr));
	strlcpy(pr.anchor, anchor_name, MAXPATHLEN);
	RULE.action = PF_PASS;
	if(ioctl(dev, DIOCGETRULES, &pr) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCGETRULES, ...): %m");
		return -1;
	}
	n = pr.nr;
#ifdef PF_RELEASETICKETS
	tnum = pr.ticket;
#endif /* PF_RELEASETICKETS */
	r = -2;
	for(i=0; i<n; i++)
	{
		pr.nr = i;
		if(ioctl(dev, DIOCGETRULE, &pr) < 0)
		{
			syslog(LOG_ERR, "ioctl(dev, DIOCGETRULE): %m");
			r = -1;
			break;
		}
#ifdef TEST
syslog(LOG_DEBUG, "%2d port=%hu proto=%d addr=%8x",
       i, ntohs(RULE.dst.port[0]), RULE.proto,
       RULE.dst.addr.v.a.addr.v4.s_addr);
/*RULE.dst.addr.v.a.mask.v4.s_addr*/
#endif
		if( (iport == ntohs(RULE.dst.port[0]))
		  && (RULE.proto == proto) &&
		   (iaddr == 0 || iaddr == RULE.dst.addr.v.a.addr.v4.s_addr)
		  )
		{
			pr.action = PF_CHANGE_GET_TICKET;
        	if(ioctl(dev, DIOCCHANGERULE, &pr) < 0)
			{
            	syslog(LOG_ERR, "ioctl(dev, DIOCCHANGERULE, ...) PF_CHANGE_GET_TICKET: %m");
				r = -1;
				break;
			}
			pr.action = PF_CHANGE_REMOVE;
			pr.nr = i;
			if(ioctl(dev, DIOCCHANGERULE, &pr) < 0)
			{
				syslog(LOG_ERR, "ioctl(dev, DIOCCHANGERULE, ...) PF_CHANGE_REMOVE: %m");
				r = -1;
			}
			else
			{
				r = 0;
			}
			break;
		}
	}
	if (r == -2)
		syslog(LOG_NOTICE, "could not find filter rule to delete iport=%hu addr=%8x", iport, ntohl(iaddr));
	release_ticket(dev, tnum);
	return r;
#endif
}

/* returns:  0 : OK
 *          -1 : Error
 *          -2 : rule not found */
int
delete_filter_rule(const char * ifname, unsigned short port, int proto)
{
	return priv_delete_filter_rule(ifname, port, proto, 0);
}

/* returns:  0 : OK
 *          -1 : Error
 *          -2 : rule not found */
int
delete_redirect_and_filter_rules(const char * ifname, unsigned short eport,
                                 int proto)
{
	int r;
	unsigned short iport;
	in_addr_t iaddr;
	r = priv_delete_redirect_rule(ifname, eport, proto, &iport, &iaddr, NULL, 0);
	if(r == 0)
	{
#ifdef ENABLE_PORT_TRIGGERING
		if (proto == IPPROTO_UDP) {
			delete_nat_rule(ifname, iport, proto, iaddr);
		}
#endif
		r = priv_delete_filter_rule(ifname, iport, proto, iaddr);
	}
	return r;
}

int
get_redirect_rule_by_index(int index,
                           char * ifname, unsigned short * eport,
                           char * iaddr, int iaddrlen, unsigned short * iport,
                           int * proto, char * desc, int desclen,
                           char * rhost, int rhostlen,
                           unsigned int * timestamp,
                           u_int64_t * packets, u_int64_t * bytes)
{
	int n, r;
	unsigned int tnum;
#ifdef USE_LIBPFCTL
	struct pfctl_rules_info ri;
	struct pfctl_rule rule;
#define RULE (rule)
	char anchor_call[MAXPATHLEN] = "";
#else /* USE_LIBPFCTL */
	struct pfioc_rule pr;
#define RULE (pr.rule)
#endif /* USE_LIBPFCTL */
#ifndef PF_NEWSTYLE
	struct pfioc_pooladdr pp;
#endif
	if(index < 0)
		return -1;
	if(dev<0) {
		syslog(LOG_ERR, "pf device is not open");
		return -1;
	}
#ifdef USE_LIBPFCTL
	if(pfctl_get_rules_info(dev, &ri, PF_RDR, anchor_name) < 0)
	{
		syslog(LOG_ERR, "pfctl_get_rules_info: %m");
		return -1;
	}
	n = ri.nr;
#else /* USE_LIBPFCTL */
	memset(&pr, 0, sizeof(pr));
	strlcpy(pr.anchor, anchor_name, MAXPATHLEN);
#ifndef PF_NEWSTYLE
	RULE.action = PF_RDR;
#endif
	if(ioctl(dev, DIOCGETRULES, &pr) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCGETRULES, ...): %m");
		return -1;
	}
	n = pr.nr;
#ifdef PF_RELEASETICKETS
	tnum = pr.ticket;
#endif /* PF_RELEASETICKETS */
#endif /* USE_LIBPFCTL */
	r = -1;
	if(index >= n)
		goto error;
#ifdef USE_LIBPFCTL
	if(pfctl_get_rule(dev, index, ri.ticket, anchor_name, PF_RDR, &rule, anchor_call) < 0)
	{
		syslog(LOG_ERR, "pfctl_get_rule: %m");
		goto error;
	}
#else /* USE_LIBPFCTL */
	pr.nr = index;
	if(ioctl(dev, DIOCGETRULE, &pr) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCGETRULE): %m");
		goto error;
	}
#endif /* USE_LIBPFCTL */
	*proto = RULE.proto;
#ifdef __APPLE__
	*eport = ntohs(RULE.dst.xport.range.port[0]);
#else
	*eport = ntohs(RULE.dst.port[0]);
#endif
#ifndef PF_NEWSTYLE
	*iport = RULE.rpool.proxy_port[0];
#else
	*iport = RULE.rdr.proxy_port[0];
#endif
	if(ifname)
		strlcpy(ifname, RULE.ifname, IFNAMSIZ);
	if(desc)
#ifdef USE_LIBPFCTL
		strlcpy(desc, RULE.label[0], desclen);
#else /* USE_LIBPFCTL */
		strlcpy(desc, RULE.label, desclen);
#endif /* USE_LIBPFCTL */
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
#ifndef PF_NEWSTYLE
	memset(&pp, 0, sizeof(pp));
	strlcpy(pp.anchor, anchor_name, MAXPATHLEN);
	pp.r_action = PF_RDR;
	pp.r_num = index;
#ifdef USE_LIBPFCTL
	pp.ticket = ri.ticket;
#else /* USE_LIBPFCTL */
	pp.ticket = pr.ticket;
#endif /* USE_LIBPFCTL */
	if(ioctl(dev, DIOCGETADDRS, &pp) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCGETADDRS, ...): %m");
		goto error;
	}
	if(pp.nr != 1)
	{
		syslog(LOG_NOTICE, "No address associated with pf rule");
		goto error;
	}
	pp.nr = 0;	/* first */
	if(ioctl(dev, DIOCGETADDR, &pp) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCGETADDR, ...): %m");
		goto error;
	}
#ifdef PFVAR_NEW_STYLE
	inet_ntop(AF_INET, &pp.addr.addr.v.a.addr.v4addr.s_addr,
	          iaddr, iaddrlen);
#else
	inet_ntop(AF_INET, &pp.addr.addr.v.a.addr.v4.s_addr,
	          iaddr, iaddrlen);
#endif
#else
	inet_ntop(AF_INET, &RULE.rdr.addr.v.a.addr.v4.s_addr,
	          iaddr, iaddrlen);
#endif
	if(rhost && rhostlen > 0)
	{
#ifdef PFVAR_NEW_STYLE
		if (RULE.src.addr.v.a.addr.v4addr.s_addr == 0)
#else
		if (RULE.src.addr.v.a.addr.v4.s_addr == 0)
#endif
		{
			rhost[0] = '\0'; /* empty string */
		}
		else
		{
#ifdef PFVAR_NEW_STYLE
			inet_ntop(AF_INET, &RULE.src.addr.v.a.addr.v4addr.s_addr,
			          rhost, rhostlen);
#else
			inet_ntop(AF_INET, &RULE.src.addr.v.a.addr.v4.s_addr,
			          rhost, rhostlen);
#endif
		}
	}
	if(timestamp)
		*timestamp = get_timestamp(*eport, *proto);
	r = 0;
error:
	release_ticket(dev, tnum);
	return r;
}

/* return an (malloc'ed) array of "external" port for which there is
 * a port mapping. number is the size of the array */
unsigned short *
get_portmappings_in_range(unsigned short startport, unsigned short endport,
                          int proto, unsigned int * number)
{
	unsigned short * array;
	unsigned int capacity, tnum;
	int i, n;
	unsigned short eport;
#ifdef USE_LIBPFCTL
	struct pfctl_rules_info ri;
	struct pfctl_rule rule;
#define RULE (rule)
#else /* USE_LIBPFCTL */
	struct pfioc_rule pr;
#define RULE (pr.rule)
#endif

	*number = 0;
	if(dev<0) {
		syslog(LOG_ERR, "pf device is not open");
		return NULL;
	}
	capacity = 128;
	array = calloc(capacity, sizeof(unsigned short));
	if(!array)
	{
		syslog(LOG_ERR, "get_portmappings_in_range() : calloc error");
		return NULL;
	}
#ifdef USE_LIBPFCTL
	if (pfctl_get_rules_info(dev, &ri, PF_RDR, anchor_name) < 0)
	{
		syslog(LOG_ERR, "pfctl_get_rules_info: %m");
		free(array);
		return NULL;
	}
	n = ri.nr;
#else /* USE_LIBPFCTL */
	memset(&pr, 0, sizeof(pr));
	strlcpy(pr.anchor, anchor_name, MAXPATHLEN);
#ifndef PF_NEWSTYLE
	RULE.action = PF_RDR;
#endif
	if(ioctl(dev, DIOCGETRULES, &pr) < 0)
	{
		syslog(LOG_ERR, "ioctl(dev, DIOCGETRULES, ...): %m");
		free(array);
		return NULL;
	}
	n = pr.nr;
#ifdef PF_RELEASETICKETS
	tnum = pr.ticket;
#endif /* PF_RELEASETICKETS */
#endif /* USE_LIBPFCTL */
	for(i=0; i<n; i++)
	{
#ifdef USE_LIBPFCTL
#else /* USE_LIBPFCTL */
		pr.nr = i;
		if(ioctl(dev, DIOCGETRULE, &pr) < 0)
		{
			syslog(LOG_ERR, "ioctl(dev, DIOCGETRULE): %m");
			continue;
		}
#endif /* USE_LIBPFCTL */
#ifdef __APPLE__
		eport = ntohs(RULE.dst.xport.range.port[0]);
		if( (eport == ntohs(RULE.dst.xport.range.port[1]))
#else
		eport = ntohs(RULE.dst.port[0]);
		if( (eport == ntohs(RULE.dst.port[1]))
#endif
		  && (RULE.proto == proto)
		  && (startport <= eport) && (eport <= endport) )
		{
			if(*number >= capacity)
			{
				/* need to increase the capacity of the array */
				unsigned short * tmp;
				capacity += 128;
				tmp = realloc(array, sizeof(unsigned short)*capacity);
				if(!tmp)
				{
					syslog(LOG_ERR, "get_portmappings_in_range() : realloc(%lu) error", sizeof(unsigned short)*capacity);
					*number = 0;
					free(array);
					release_ticket(dev, tnum);
					return NULL;
				}
				array = tmp;
			}
			array[*number] = eport;
			(*number)++;
		}
	}
	release_ticket(dev, tnum);
	return array;
}

/* update the port mapping internal port, description and timestamp
 * returns:  0 : OK
 *          -1 : Error */
int
update_portmapping(const char * ifname, unsigned short eport, int proto,
                   unsigned short iport, const char * desc,
                   unsigned int timestamp)
{
	unsigned short old_iport;
	in_addr_t iaddr;
	char iaddr_str[16];
	char rhost[32];

	if(priv_delete_redirect_rule(ifname, eport, proto, &old_iport, &iaddr, rhost, sizeof(rhost)) < 0)
		return -1;
	if (priv_delete_filter_rule(ifname, old_iport, proto, iaddr) < 0)
		return -1;

	if (inet_ntop(AF_INET, &iaddr, iaddr_str, sizeof(iaddr_str)) == NULL) {
		syslog(LOG_ERR, "inet_ntop(AF_INET, ...): %m");
		return -1;
	}

	if(add_redirect_rule2(ifname, rhost, eport, iaddr_str, iport, proto,
	                      desc, timestamp) < 0)
		return -1;
	if(add_filter_rule2(ifname, rhost, iaddr_str, eport, iport, proto, desc) < 0)
		return -1;

	return 0;
}

/* update the port mapping description and timestamp */
int
update_portmapping_desc_timestamp(const char * ifname,
                   unsigned short eport, int proto,
                   const char * desc, unsigned int timestamp)
{
	unsigned short iport;
	in_addr_t iaddr;
	char iaddr_str[16];
	char rhost[32];
	int r;

	r = priv_delete_redirect_rule_check_desc(ifname, eport, proto, &iport, &iaddr, rhost, sizeof(rhost), 1, desc);
	if(r < 0)
		return -1;
	if(r == 1) {
		/* only change timestamp */
		remove_timestamp_entry(eport, proto);
		add_timestamp_entry(eport, proto, timestamp);
		return 0;
	}
	if (priv_delete_filter_rule(ifname, iport, proto, iaddr) < 0)
		return -1;

	if (inet_ntop(AF_INET, &iaddr, iaddr_str, sizeof(iaddr_str)) == NULL) {
		syslog(LOG_ERR, "inet_ntop(AF_INET, ...): %m");
		return -1;
	}

	if(add_redirect_rule2(ifname, rhost, eport, iaddr_str, iport, proto,
	                      desc, timestamp) < 0)
		return -1;
	if(add_filter_rule2(ifname, rhost, iaddr_str, eport, iport, proto, desc) < 0)
		return -1;

	return 0;
}


/* this function is only for testing */
#if TEST
void
list_rules(void)
{
	char buf[32];
	char buf2[32];
	int i, n;
	unsigned int tnum;
#ifdef USE_LIBPFCTL
	struct pfctl_rules_info ri;
	struct pfctl_rule rule;
#define RULE (rule)
	char anchor_call[MAXPATHLEN] = "";
#else /* USE_LIBPFCTL */
	struct pfioc_rule pr;
#define RULE (pr.rule)
#endif /* USE_LIBPFCTL */
#ifndef PF_NEWSTYLE
	struct pfioc_pooladdr pp;
#endif

	if(dev<0)
	{
		perror("pf dev not open");
		return ;
	}

#ifdef USE_LIBPFCTL
	if (pfctl_get_rules_info(dev, &ri, PF_RDR, anchor_name) < 0)
		perror("pfctl_get_rules_info");
	printf("ticket = %d, nr = %d\n", ri.ticket, ri.nr);
	n = ri.nr;
#else /* USE_LIBPFCTL */
	memset(&pr, 0, sizeof(pr));
	strlcpy(pr.anchor, anchor_name, MAXPATHLEN);
	RULE.action = PF_RDR;
	if(ioctl(dev, DIOCGETRULES, &pr) < 0)
		perror("DIOCGETRULES");
	printf("ticket = %d, nr = %d\n", pr.ticket, pr.nr);
	n = pr.nr;
#endif /* USE_LIBPFCTL */
	for(i=0; i<n; i++)
	{
		printf("-- rule %d --\n", i);
#ifdef USE_LIBPFCTL
		if(pfctl_get_rule(dev, i, ri.ticket, anchor_name, PF_RDR, &rule, anchor_call) < 0)
			perror("pfctl_get_rule");
#else /* USE_LIBPFCTL */
		pr.nr = i;
		if(ioctl(dev, DIOCGETRULE, &pr) < 0)
			perror("DIOCGETRULE");
#endif /* USE_LIBPFCTL */
		printf(" %s %s %d:%d -> %s %d:%d  proto %d keep_state=%d action=%d\n",
			RULE.ifname,
			inet_ntop(AF_INET, &RULE.src.addr.v.a.addr.v4.s_addr, buf, 32),
			(int)ntohs(RULE.dst.port[0]),
			(int)ntohs(RULE.dst.port[1]),
			inet_ntop(AF_INET, &RULE.dst.addr.v.a.addr.v4.s_addr, buf2, 32),
#ifndef PF_NEWSTYLE
			(int)RULE.rpool.proxy_port[0],
			(int)RULE.rpool.proxy_port[1],
#else
			(int)RULE.rdr.proxy_port[0],
			(int)RULE.rdr.proxy_port[1],
#endif
			(int)RULE.proto,
			(int)RULE.keep_state,
			(int)RULE.action);
#ifdef USE_LIBPFCTL
		printf("  description: \"%s\"\n", RULE.label[0]);
#else /* USE_LIBPFCTL */
		printf("  description: \"%s\"\n", RULE.label);
#endif /* USE_LIBPFCTL */
#ifndef PF_NEWSTYLE
		memset(&pp, 0, sizeof(pp));
		strlcpy(pp.anchor, anchor_name, MAXPATHLEN);
		pp.r_action = PF_RDR;
		pp.r_num = i;
#ifdef USE_LIBPFCTL
		pp.ticket = ri.ticket;
#else
		pp.ticket = pr.ticket;
#endif
		if(ioctl(dev, DIOCGETADDRS, &pp) < 0)
			perror("DIOCGETADDRS");
		printf("  nb pool addr = %d ticket=%d\n", pp.nr, pp.ticket);
		/*if(ioctl(dev, DIOCGETRULE, &pr) < 0)
			perror("DIOCGETRULE"); */
		pp.nr = 0;	/* first */
		if(ioctl(dev, DIOCGETADDR, &pp) < 0)
			perror("DIOCGETADDR");
		/* addr.v.a.addr.v4.s_addr */
		printf("  %s\n", inet_ntop(AF_INET, &pp.addr.addr.v.a.addr.v4.s_addr, buf, 32));
#else
		printf("  rule_flag=%08x action=%d direction=%d log=%d logif=%d "
		       "quick=%d ifnot=%d af=%d type=%d code=%d rdr.port_op=%d rdr.opts=%d\n",
		       RULE.rule_flag, RULE.action, RULE.direction,
		       RULE.log, RULE.logif, RULE.quick, RULE.ifnot,
		       RULE.af, RULE.type, RULE.code,
		       RULE.rdr.port_op, RULE.rdr.opts);
		printf("  %s\n", inet_ntop(AF_INET, &RULE.rdr.addr.v.a.addr.v4.s_addr, buf, 32));
#endif
	}
	release_ticket(dev, tnum);
}
#endif /* TEST */

#endif /* USE_PF */
