/* $Id: ipfwrdr.c,v 1.18 2022/02/19 21:44:51 nanard Exp $ */
/*
 * MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2009 Jardel Weyrich
 * (c) 2011-2016 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution
 */

#include "config.h"
#include "../macros.h"

#include <sys/param.h>
#include <sys/types.h>
#include <sys/file.h>

/*
This is a workaround for <sys/uio.h> troubles on FreeBSD, HPUX, OpenBSD.
Needed here because on some systems <sys/uio.h> gets included by things
like <sys/socket.h>
*/
#ifndef _KERNEL
#  define ADD_KERNEL
#  define _KERNEL
#  define KERNEL
#endif
#ifdef __OpenBSD__
struct file;
#endif
#include <sys/uio.h>
#ifdef ADD_KERNEL
#  undef _KERNEL
#  undef KERNEL
#endif

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/ioctl.h>
#include <net/if.h>
#if __FreeBSD_version >= 300000
#  include <net/if_var.h>
#endif
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <stdlib.h>
#include <fcntl.h>
#include <syslog.h>
#include <stddef.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip_fw.h>
#include "ipfwaux.h"
#include "ipfwrdr.h"

#include "../upnpglobalvars.h"

/* This ipfw backend is known to work with OS X up to 10.6.
 * Some work is needed to support FreeBSD ipfw.
 */
#ifndef IP_FW_CURRENT_API_VERSION
#error "ip_fw.h does not contain supported API"
#endif

/* init and shutdown functions */

int init_redirect(void) {
	return ipfw_exec(IP_FW_INIT, NULL, 0);
}

void shutdown_redirect(void) {
	ipfw_exec(IP_FW_TERM, NULL, 0);
}

/* ipfw cannot store descriptions and timestamp for port mappings so we keep
 * our own list in memory */
struct mapping_desc_time {
	struct mapping_desc_time * next;
	unsigned int timestamp;
	unsigned short eport;
	short proto;
	char desc[];
};

static struct mapping_desc_time * mappings_list = NULL;

/* add an element to the port mappings descriptions & timestamp list */
static void
add_desc_time(unsigned short eport, int proto,
              const char * desc, unsigned int timestamp)
{
	struct mapping_desc_time * tmp;
	size_t l;
	if(!desc)
		desc = "miniupnpd";
	l = strlen(desc) + 1;
	tmp = malloc(sizeof(struct mapping_desc_time) + l);
	if(tmp) {
		/* fill the element and insert it as head of the list */
		tmp->next = mappings_list;
		tmp->timestamp = timestamp;
		tmp->eport = eport;
		tmp->proto = (short)proto;
		memcpy(tmp->desc, desc, l);
		mappings_list = tmp;
	}
}

/* remove an element to the port mappings descriptions & timestamp list */
static void
del_desc_time(unsigned short eport, int proto)
{
	struct mapping_desc_time * e;
	struct mapping_desc_time * * p;
	p = &mappings_list;
	e = *p;
	while(e) {
		if(e->eport == eport && e->proto == (short)proto) {
			*p = e->next;
			free(e);
			return;
		} else {
			p = &e->next;
			e = *p;
		}
	}
}

/* go through the list and find the description and timestamp */
static void
get_desc_time(unsigned short eport, int proto,
              char * desc, int desclen,
              unsigned int * timestamp)
{
	struct mapping_desc_time * e;

	for(e = mappings_list; e; e = e->next) {
		if(e->eport == eport && e->proto == (short)proto) {
			if(desc)
				strlcpy(desc, e->desc, desclen);
			if(timestamp)
				*timestamp = e->timestamp;
			return;
		}
	}
}

/* --- */
int add_redirect_rule2(
	const char * ifname,
	const char * rhost,
	unsigned short eport,
	const char * iaddr,
	unsigned short iport,
	int proto,
	const char * desc,
	unsigned int timestamp)
{
	struct ip_fw rule;
	int r;

	if (ipfw_validate_protocol(proto) < 0)
		return -1;
	if (ipfw_validate_ifname(ifname) < 0)
		return -1;

	memset(&rule, 0, sizeof(struct ip_fw));
	rule.version = IP_FW_CURRENT_API_VERSION;
#if 0
	rule.fw_number = 1000; /* rule number */
	rule.context = (void *)desc; /* The description is kept in a separate list */
#endif
	rule.fw_prot = proto; /* protocol */
	rule.fw_flg |= IP_FW_F_IIFACE; /* interfaces to check */
	rule.fw_flg |= IP_FW_F_IIFNAME; /* interfaces to check by name */
	rule.fw_flg |= (IP_FW_F_IN | IP_FW_F_OUT); /* packet direction */
	rule.fw_flg |= IP_FW_F_FWD; /* forward action */
#ifdef USE_IFNAME_IN_RULES
	if (ifname != NULL) {
		strlcpy(rule.fw_in_if.fu_via_if.name, ifname, IFNAMSIZ); /* src interface */
		rule.fw_in_if.fu_via_if.unit = -1;
	}
#endif
	if (inet_aton(iaddr, &rule.fw_out_if.fu_via_ip) == 0) {
		syslog(LOG_ERR, "inet_aton(): %m");
		return -1;
	}
	memcpy(&rule.fw_dst,  &rule.fw_out_if.fu_via_ip, sizeof(struct in_addr));
	memcpy(&rule.fw_fwd_ip.sin_addr, &rule.fw_out_if.fu_via_ip, sizeof(struct in_addr));
	rule.fw_dmsk.s_addr = INADDR_BROADCAST;	/* TODO check this */
	IP_FW_SETNDSTP(&rule, 1); /* number of external ports */
	rule.fw_uar.fw_pts[0] = eport; /* external port */
	rule.fw_fwd_ip.sin_port = iport; /* internal port */
	if (rhost && rhost[0] != '\0') {
		inet_aton(rhost, &rule.fw_src);
		rule.fw_smsk.s_addr = htonl(INADDR_NONE);
	}

	r = ipfw_exec(IP_FW_ADD, &rule, sizeof(rule));
	if(r >= 0)
		add_desc_time(eport, proto, desc, timestamp);
	return r;
}

/* get_redirect_rule()
 * return value : 0 success (found)
 * -1 = error or rule not found */
int get_redirect_rule(
	const char * ifname,
	unsigned short eport,
	int proto,
	char * iaddr,
	int iaddrlen,
	unsigned short * iport,
	char * desc,
	int desclen,
	char * rhost,
	int rhostlen,
	unsigned int * timestamp,
	u_int64_t * packets,
	u_int64_t * bytes)
{
	int i, count_rules, total_rules = 0;
	struct ip_fw * rules = NULL;

	if (ipfw_validate_protocol(proto) < 0)
		return -1;
	if (ipfw_validate_ifname(ifname) < 0)
		return -1;
	if (timestamp)
		*timestamp = 0;

	do {
		count_rules = ipfw_fetch_ruleset(&rules, &total_rules, 10);
		if (count_rules < 0)
			goto error;
	} while (count_rules == 10);

	for (i=0; i<total_rules-1; i++) {
		const struct ip_fw const * ptr = &rules[i];
		if (proto == ptr->fw_prot && eport == ptr->fw_uar.fw_pts[0]) {
			if (packets != NULL)
				*packets = ptr->fw_pcnt;
			if (bytes != NULL)
				*bytes = ptr->fw_bcnt;
			if (iport != NULL)
				*iport = ptr->fw_fwd_ip.sin_port;
			if (iaddr != NULL && iaddrlen > 0) {
				/* looks like fw_out_if.fu_via_ip is zero */
				/*if (inet_ntop(AF_INET, &ptr->fw_out_if.fu_via_ip, iaddr, iaddrlen) == NULL) {*/
				if (inet_ntop(AF_INET, &ptr->fw_fwd_ip.sin_addr, iaddr, iaddrlen) == NULL) {
					syslog(LOG_ERR, "inet_ntop(): %m");
					goto error;
				}
			}
			if (rhost != NULL && rhostlen > 0) {
				if (ptr->fw_src.s_addr == 0)
					rhost[0] = '\0';
				else if (inet_ntop(AF_INET, &ptr->fw_src.s_addr, rhost, rhostlen) == NULL) {
					syslog(LOG_ERR, "inet_ntop(): %m");
					goto error;
				}
			}
			/* And what if we found more than 1 matching rule? */
			ipfw_free_ruleset(&rules);
			get_desc_time(eport, proto, desc, desclen, timestamp);
			return 0;
		}
	}

error:
	if (rules != NULL)
		ipfw_free_ruleset(&rules);
	return -1;
}

int delete_redirect_rule(
	const char * ifname,
	unsigned short eport,
	int proto)
{
	int i, count_rules, total_rules = 0;
	struct ip_fw * rules = NULL;

	if (ipfw_validate_protocol(proto) < 0)
		return -1;
	if (ipfw_validate_ifname(ifname) < 0)
		return -1;

	do {
		count_rules = ipfw_fetch_ruleset(&rules, &total_rules, 10);
		if (count_rules < 0)
			goto error;
	} while (count_rules == 10);

	for (i=0; i<total_rules-1; i++) {
		const struct ip_fw const * ptr = &rules[i];
		if (proto == ptr->fw_prot && eport == ptr->fw_uar.fw_pts[0]) {
			if (ipfw_exec(IP_FW_DEL, (struct ip_fw *)ptr, sizeof(*ptr)) < 0)
				goto error;
			/* And what if we found more than 1 matching rule? */
			ipfw_free_ruleset(&rules);
			del_desc_time(eport, proto);
			return 0;
		}
	}

error:
	if (rules != NULL)
		ipfw_free_ruleset(&rules);
	return -1;
}

int add_filter_rule2(
	const char * ifname,
	const char * rhost,
	const char * iaddr,
	unsigned short eport,
	unsigned short iport,
	int proto,
	const char * desc)
{
	UNUSED(ifname);
	UNUSED(rhost);
	UNUSED(iaddr);
	UNUSED(eport);
	UNUSED(iport);
	UNUSED(proto);
	UNUSED(desc);
	return 0; /* nothing to do, always success */
}

int delete_filter_rule(
	const char * ifname,
	unsigned short eport,
	int proto)
{
	UNUSED(ifname);
	UNUSED(eport);
	UNUSED(proto);
	return 0; /* nothing to do, always success */
}

int get_redirect_rule_by_index(
	int index,
	char * ifname,
	unsigned short * eport,
	char * iaddr,
	int iaddrlen,
	unsigned short * iport,
	int * proto,
	char * desc,
	int desclen,
	char * rhost,
	int rhostlen,
	unsigned int * timestamp,
	u_int64_t * packets,
	u_int64_t * bytes)
{
	int total_rules = 0;
	struct ip_fw * rules = NULL;

	if (index < 0) /* TODO shouldn't we also validate the maximum? */
		return -1;

	if(timestamp)
		*timestamp = 0;

	ipfw_fetch_ruleset(&rules, &total_rules, index + 1);

	if (total_rules > index) {
		const struct ip_fw const * ptr = &rules[index];
		if (ptr->fw_prot == 0)	/* invalid rule */
			goto error;
		if (proto != NULL)
			*proto = ptr->fw_prot;
		if (eport != NULL)
			*eport = ptr->fw_uar.fw_pts[0];
		if (iport != NULL)
			*iport = ptr->fw_fwd_ip.sin_port;
		if (ifname != NULL)
			strlcpy(ifname, ptr->fw_in_if.fu_via_if.name, IFNAMSIZ);
		if (packets != NULL)
			*packets = ptr->fw_pcnt;
		if (bytes != NULL)
			*bytes = ptr->fw_bcnt;
		if (iport != NULL)
			*iport = ptr->fw_fwd_ip.sin_port;
		if (iaddr != NULL && iaddrlen > 0) {
			/* looks like fw_out_if.fu_via_ip is zero */
			/*if (inet_ntop(AF_INET, &ptr->fw_out_if.fu_via_ip, iaddr, iaddrlen) == NULL) {*/
			if (inet_ntop(AF_INET, &ptr->fw_fwd_ip.sin_addr, iaddr, iaddrlen) == NULL) {
				syslog(LOG_ERR, "inet_ntop(): %m");
				goto error;
			}
		}
		if (rhost != NULL && rhostlen > 0) {
			if (ptr->fw_src.s_addr == 0)
				rhost[0] = '\0';
			else if (inet_ntop(AF_INET, &ptr->fw_src.s_addr, rhost, rhostlen) == NULL) {
				syslog(LOG_ERR, "inet_ntop(): %m");
				goto error;
			}
		}
		ipfw_free_ruleset(&rules);
		get_desc_time(*eport, *proto, desc, desclen, timestamp);
		return 0;
	}

error:
	if (rules != NULL)
		ipfw_free_ruleset(&rules);
	return -1;
}

/* upnp_get_portmappings_in_range()
 * return a list of all "external" ports for which a port
 * mapping exists */
unsigned short *
get_portmappings_in_range(unsigned short startport,
                          unsigned short endport,
                          int proto,
                          unsigned int * number)
{
	unsigned short *array = NULL, *array2 = NULL;
	unsigned int capacity = 128;
	int i, count_rules, total_rules = 0;
	struct ip_fw * rules = NULL;

	if (ipfw_validate_protocol(proto) < 0)
		return NULL;

	do {
		count_rules = ipfw_fetch_ruleset(&rules, &total_rules, 10);
		if (count_rules < 0)
			goto error;
	} while (count_rules == 10);

	array = calloc(capacity, sizeof(unsigned short));
	if(!array) {
		syslog(LOG_ERR, "get_portmappings_in_range() : calloc error");
                goto error;
	}
	*number = 0;

	for (i=0; i<total_rules-1; i++) {
		const struct ip_fw const * ptr = &rules[i];
		unsigned short eport = ptr->fw_uar.fw_pts[0];
		if (proto == ptr->fw_prot
		    && startport <= eport
		    && eport <= endport) {
			if(*number >= capacity) {
				capacity += 128;
				array2 = realloc(array, sizeof(unsigned short)*capacity);
				if(!array2) {
					syslog(LOG_ERR, "get_portmappings_in_range() : realloc(%lu) error", sizeof(unsigned short)*capacity);
					*number = 0;
					free(array);
					goto error;
				}
				array = array2;
			}
			array[*number] = eport;
			(*number)++;
		}
	}
error:
	if (rules != NULL)
		ipfw_free_ruleset(&rules);
	return array;
}

int
update_portmapping_desc_timestamp(const char * ifname,
                   unsigned short eport, int proto,
                   const char * desc, unsigned int timestamp)
{
	UNUSED(ifname);
	del_desc_time(eport, proto);
	add_desc_time(eport, proto, desc, timestamp);
	return 0;
}

int
update_portmapping(const char * ifname, unsigned short eport, int proto,
                   unsigned short iport, const char * desc,
                   unsigned int timestamp)
{
	int i, count_rules, total_rules = 0;
	struct ip_fw * rules = NULL;
	int r = -1;
	char iaddr[16];
	char rhost[16];
	int found;

	if (ipfw_validate_protocol(proto) < 0)
		return -1;
	if (ipfw_validate_ifname(ifname) < 0)
		return -1;

	do {
		count_rules = ipfw_fetch_ruleset(&rules, &total_rules, 10);
		if (count_rules < 0)
			goto error;
	} while (count_rules == 10);

	found = 0;
	iaddr[0] = '\0';
	rhost[0] = '\0';

	for (i=0; i<total_rules-1; i++) {
		const struct ip_fw const * ptr = &rules[i];
		if (proto == ptr->fw_prot && eport == ptr->fw_uar.fw_pts[0]) {
			if (inet_ntop(AF_INET, &ptr->fw_fwd_ip.sin_addr, iaddr, sizeof(iaddr)) == NULL) {
				syslog(LOG_ERR, "inet_ntop(): %m");
				goto error;
			}
			if ((ptr->fw_src.s_addr != 0) &&
			    (inet_ntop(AF_INET, &ptr->fw_src.s_addr, rhost, sizeof(rhost)) == NULL)) {
				syslog(LOG_ERR, "inet_ntop(): %m");
				goto error;
			}
			found = 1;
			if (ipfw_exec(IP_FW_DEL, (struct ip_fw *)ptr, sizeof(*ptr)) < 0)
				goto error;
			del_desc_time(eport, proto);
			break;
		}
	}
	ipfw_free_ruleset(&rules);
	rules = NULL;

	if(found)
		r = add_redirect_rule2(ifname, rhost, eport, iaddr, iport, proto, desc, timestamp);

error:
	if (rules != NULL)
		ipfw_free_ruleset(&rules);
	return r;
}
