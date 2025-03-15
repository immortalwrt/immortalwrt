/* $Id: ipfrdr.c,v 1.20 2020/05/10 22:26:04 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2007 Darren Reed
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/file.h>
/*
 * This is a workaround for <sys/uio.h> troubles on FreeBSD, HPUX, OpenBSD.
 * Needed here because on some systems <sys/uio.h> gets included by things
 * like <sys/socket.h>
 */
#ifndef _KERNEL
# define ADD_KERNEL
# define _KERNEL
# define KERNEL
#endif
#ifdef __OpenBSD__
struct file;
#endif
#include <sys/uio.h>
#ifdef ADD_KERNEL
# undef _KERNEL
# undef KERNEL
#endif
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/ioctl.h>
#include <net/if.h>
#if __FreeBSD_version >= 300000
# include <net/if_var.h>
#endif
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#ifndef	TCP_PAWS_IDLE	/* IRIX */
# include <netinet/tcp.h>
#endif
#include <netinet/udp.h>

#include <arpa/inet.h>

#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <stdlib.h>
#include <fcntl.h>
#include <syslog.h>
#include <stddef.h>
#include <stdio.h>
#if !defined(__SVR4) && !defined(__svr4__) && defined(sun)
# include <strings.h>
#endif
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "netinet/ipl.h"
#include "netinet/ip_compat.h"
#include "netinet/ip_fil.h"
#include "netinet/ip_nat.h"
#include "netinet/ip_state.h"

#include "../macros.h"

#ifndef __P
# ifdef __STDC__
#  define	__P(x)	x
# else
#  define	__P(x)	()
# endif
#endif
#ifndef __STDC__
# undef		const
# define	const
#endif

#ifndef	U_32_T
# define	U_32_T	1
# if defined(__NetBSD__) || defined(__OpenBSD__) || defined(__FreeBSD__) || \
	defined(__sgi)
typedef	u_int32_t	u_32_t;
# else
#  if defined(__alpha__) || defined(__alpha) || defined(_LP64)
typedef unsigned int	u_32_t;
#  else
#   if SOLARIS2 >= 6
typedef uint32_t	u_32_t;
#   else
typedef unsigned int	u_32_t;
#   endif
#  endif
# endif /* __NetBSD__ || __OpenBSD__ || __FreeBSD__ || __sgi */
#endif /* U_32_T */


#if defined(__NetBSD__) || defined(__OpenBSD__) ||			\
        (_BSDI_VERSION >= 199701) || (__FreeBSD_version >= 300000) ||	\
	SOLARIS || defined(__sgi) || defined(__osf__) || defined(linux)
# include <stdarg.h>
typedef	int	(* ioctlfunc_t) __P((int, ioctlcmd_t, ...));
#else
typedef	int	(* ioctlfunc_t) __P((dev_t, ioctlcmd_t, void *));
#endif
typedef	void	(* addfunc_t) __P((int, ioctlfunc_t, void *));
typedef	int	(* copyfunc_t) __P((void *, void *, size_t));


/*
 * SunOS4
 */
#if defined(sun) && !defined(__SVR4) && !defined(__svr4__)
extern	int	ioctl __P((int, int, void *));
#endif

#include "../upnpglobalvars.h"

/* group name */
static const char group_name[] = "miniupnpd";

static int dev = -1;
static int dev_ipl = -1;

/* IPFilter cannot store redirection descriptions, so we use our
 * own structure to store them */
struct rdr_desc {
	struct rdr_desc * next;
	unsigned short eport;
	int proto;
	unsigned int timestamp;
	char str[];
};

/* pointer to the chained list where descriptions are stored */
static struct rdr_desc * rdr_desc_list;

static void
add_redirect_desc(unsigned short eport, int proto,
                  unsigned int timestamp, const char * desc)
{
	struct rdr_desc * p;
	size_t l;

	if (desc != NULL) {
		l = strlen(desc) + 1;
		p = malloc(sizeof(struct rdr_desc) + l);
		if (p) {
			p->next = rdr_desc_list;
			p->eport = eport;
			p->proto = proto;
			p->timestamp = timestamp;
			memcpy(p->str, desc, l);
			rdr_desc_list = p;
		}
	}
}

static void
del_redirect_desc(unsigned short eport, int proto)
{
	struct rdr_desc * p, * last;

	last = NULL;
	for (p = rdr_desc_list; p; p = p->next) {
		if(p->eport == eport && p->proto == proto) {
			if (last == NULL)
				rdr_desc_list = p->next;
			else
				last->next = p->next;
			free(p);
			return;
		}
	}
}

static void
get_redirect_desc(unsigned short eport, int proto, char * desc, int desclen, unsigned int * timestamp)
{
	struct rdr_desc * p;

	if (desc == NULL || desclen == 0)
		return;
	for (p = rdr_desc_list; p; p = p->next) {
		if (p->eport == eport && p->proto == proto)
		{
			strncpy(desc, p->str, desclen);
			*timestamp = p->timestamp;
			return;
		}
	}
	return;
}

int init_redirect(void)
{

	dev = open(IPNAT_NAME, O_RDWR);
	if (dev < 0) {
		syslog(LOG_ERR, "open(\"%s\"): %m", IPNAT_NAME);
		return -1;
	}
	dev_ipl = open(IPL_NAME, O_RDWR);
	if (dev_ipl < 0) {
		syslog(LOG_ERR, "open(\"%s\"): %m", IPL_NAME);
		return -1;
	}
	return 0;
}

void shutdown_redirect(void)
{

	if (dev >= 0) {
		close(dev);
		dev = -1;
	}
	if (dev_ipl >= 0) {
		close(dev_ipl);
		dev = -1;
	}
	return;
}

int
add_redirect_rule2(const char * ifname, const char * rhost,
    unsigned short eport, const char * iaddr, unsigned short iport,
    int proto, const char * desc, unsigned int timestamp)
{
	struct ipnat ipnat;
	struct ipfobj obj;
	int r;

	if (dev < 0) {
		syslog(LOG_ERR, "%s not open", IPNAT_NAME);
		return -1;
	}

	memset(&obj, 0, sizeof(obj));
	memset(&ipnat, 0, sizeof(ipnat));

	ipnat.in_redir = NAT_REDIRECT;
#if IPFILTER_VERSION >= 5000000
	ipnat.in_pr[0] = proto;
	ipnat.in_pr[1] = proto;
#else
	ipnat.in_p = proto;
#endif
	if (proto == IPPROTO_TCP)
		ipnat.in_flags = IPN_TCP;
	if (proto == IPPROTO_UDP)
		ipnat.in_flags = IPN_UDP;
	ipnat.in_dcmp = FR_EQUAL;
#if IPFILTER_VERSION >= 5000000
	ipnat.in_dpmin = htons(eport);
	ipnat.in_dpmax = htons(eport);
	ipnat.in_dpnext = htons(iport);
	ipnat.in_v[0] = 4;
	ipnat.in_v[1] = 4;
#else
	ipnat.in_pmin = htons(eport);
	ipnat.in_pmax = htons(eport);
	ipnat.in_pnext = htons(iport);
	ipnat.in_v = 4;
#endif
	strlcpy(ipnat.in_tag.ipt_tag, group_name, IPFTAG_LEN);

#ifdef USE_IFNAME_IN_RULES
	if (ifname) {
#if IPFILTER_VERSION >= 5000000
		/* XXX check for stack overflow ! */
		ipnat.in_ifnames[0] = 0;
		ipnat.in_ifnames[1] = 0;
		strlcpy(ipnat.in_names, ifname, IFNAMSIZ);
		ipnat.in_namelen = strlen(ipnat.in_names) + 1;
#else
		strlcpy(ipnat.in_ifnames[0], ifname, IFNAMSIZ);
		strlcpy(ipnat.in_ifnames[1], ifname, IFNAMSIZ);
#endif
	}
#endif

	if(rhost && rhost[0] != '\0' && rhost[0] != '*')
	{
#if IPFILTER_VERSION >= 5000000
		inet_pton(AF_INET, rhost, &ipnat.in_nsrc.na_addr[0].in4); /* in_nsrcip */
		ipnat.in_nsrc.na_addr[1].in4.s_addr = 0xffffffff;	/* in_nsrcmsk */
#else
		inet_pton(AF_INET, rhost, &ipnat.in_src[0].in4);
		ipnat.in_src[1].in4.s_addr = 0xffffffff;
#endif
	}

#if IPFILTER_VERSION >= 5000000
	inet_pton(AF_INET, iaddr, &ipnat.in_ndst.na_addr[0].in4); /* in_ndstip */
	ipnat.in_ndst.na_addr[1].in4.s_addr = 0xffffffff; /* in_ndstmsk */
#else
	inet_pton(AF_INET, iaddr, &ipnat.in_in[0].in4);
	ipnat.in_in[1].in4.s_addr = 0xffffffff;
#endif

	obj.ipfo_rev = IPFILTER_VERSION;
	obj.ipfo_size = sizeof(ipnat);
	obj.ipfo_ptr = &ipnat;
	obj.ipfo_type = IPFOBJ_IPNAT;

	r = ioctl(dev, SIOCADNAT, &obj);
	if (r == -1)
		syslog(LOG_ERR, "ioctl(SIOCADNAT): %m");
	else
		add_redirect_desc(eport, proto, timestamp, desc);
	return r;
}

/* get_redirect_rule()
 * return value : 0 success (found)
 * -1 = error or rule not found */
int
get_redirect_rule(const char * ifname, unsigned short eport, int proto,
    char * iaddr, int iaddrlen, unsigned short * iport,
    char * desc, int desclen,
    char * rhost, int rhostlen,
    unsigned int * timestamp,
    u_int64_t * packets, u_int64_t * bytes)
{
	ipfgeniter_t iter;
	ipfobj_t obj;
	ipnat_t ipn;
	int r;
	UNUSED(ifname);

	memset(&obj, 0, sizeof(obj));
	obj.ipfo_rev = IPFILTER_VERSION;
	obj.ipfo_type = IPFOBJ_GENITER;
	obj.ipfo_size = sizeof(iter);
	obj.ipfo_ptr = &iter;

	iter.igi_type = IPFGENITER_IPNAT;
#if IPFILTER_VERSION > 4011300
	iter.igi_nitems = 1;
#endif
	iter.igi_data = &ipn;

	if (dev < 0) {
		syslog(LOG_ERR, "%s not open", IPNAT_NAME);
		return -1;
	}

	r = -1;
	do {
		if (ioctl(dev, SIOCGENITER, &obj) == -1) {
			syslog(LOG_ERR, "ioctl(dev, SIOCGENITER): %m");
			break;
		}
#if IPFILTER_VERSION >= 5000000
		if (eport == ntohs(ipn.in_dpmin) &&
		    eport == ntohs(ipn.in_dpmax) &&
		    strcmp(ipn.in_tag.ipt_tag, group_name) == 0 &&
		    ipn.in_pr[0] == proto)
#else
		if (eport == ntohs(ipn.in_pmin) &&
		    eport == ntohs(ipn.in_pmax) &&
		    strcmp(ipn.in_tag.ipt_tag, group_name) == 0 &&
		    ipn.in_p == proto)
#endif
		{
			strlcpy(desc, "", desclen);
			if (packets != NULL)
				*packets = 0;
			if (bytes != NULL)
				*bytes = 0;
			if (iport != NULL)
#if IPFILTER_VERSION >= 5000000
				*iport = ntohs(ipn.in_dpnext);
#else
				*iport = ntohs(ipn.in_pnext);
#endif
			if ((desc != NULL) && (timestamp != NULL))
				get_redirect_desc(eport, proto, desc, desclen, timestamp);
			if ((rhost != NULL) && (rhostlen > 0))
#if IPFILTER_VERSION >= 5000000
				inet_ntop(AF_INET, &ipn.in_nsrc.na_addr[0].in4, rhost, rhostlen);	/* in_nsrcip */
#else
				inet_ntop(AF_INET, &ipn.in_src[0].in4, rhost, rhostlen);
#endif
#if IPFILTER_VERSION >= 5000000
			inet_ntop(AF_INET, &ipn.in_ndst.na_addr[0].in4, iaddr, iaddrlen);	/* in_ndstip */
#else
			inet_ntop(AF_INET, &ipn.in_in[0].in4, iaddr, iaddrlen);
#endif
			r = 0;
		}
	} while (ipn.in_next != NULL);
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
	ipfgeniter_t iter;
	ipfobj_t obj;
	ipnat_t ipn;
	int n, r;

	if (index < 0)
		return -1;

	if (dev < 0) {
		syslog(LOG_ERR, "%s not open", IPNAT_NAME);
		return -1;
	}

	memset(&obj, 0, sizeof(obj));
	obj.ipfo_rev = IPFILTER_VERSION;
	obj.ipfo_ptr = &iter;
	obj.ipfo_size = sizeof(iter);
	obj.ipfo_type = IPFOBJ_GENITER;

	iter.igi_type = IPFGENITER_IPNAT;
#if IPFILTER_VERSION > 4011300
	iter.igi_nitems = 1;
#endif
	iter.igi_data = &ipn;

	n = 0;
	r = -1;
	do {
		if (ioctl(dev, SIOCGENITER, &obj) == -1) {
			syslog(LOG_ERR, "%s:ioctl(SIOCGENITER): %m",
			    "get_redirect_rule_by_index");
			break;
		}

		if (strcmp(ipn.in_tag.ipt_tag, group_name) != 0)
			continue;

		if (index == n++) {
#if IPFILTER_VERSION >= 5000000
			*proto = ipn.in_pr[0];
			*eport = ntohs(ipn.in_dpmax);
			*iport = ntohs(ipn.in_dpnext);
#else
			*proto = ipn.in_p;
			*eport = ntohs(ipn.in_pmax);
			*iport = ntohs(ipn.in_pnext);
#endif

			if (ifname)
#if IPFILTER_VERSION >= 5000000
				strlcpy(ifname, ipn.in_names + ipn.in_ifnames[0], IFNAMSIZ);
#else
				strlcpy(ifname, ipn.in_ifnames[0], IFNAMSIZ);
#endif
			if (packets != NULL)
				*packets = 0;
			if (bytes != NULL)
				*bytes = 0;
			if ((desc != NULL) && (timestamp != NULL))
				get_redirect_desc(*eport, *proto, desc, desclen, timestamp);
			if ((rhost != NULL) && (rhostlen > 0))
#if IPFILTER_VERSION >= 5000000
				inet_ntop(AF_INET, &ipn.in_nsrc.na_addr[0].in4, rhost, rhostlen);	/* in_nsrcip */
#else
				inet_ntop(AF_INET, &ipn.in_src[0].in4, rhost, rhostlen);
#endif
#if IPFILTER_VERSION >= 5000000
			inet_ntop(AF_INET, &ipn.in_ndst.na_addr[0].in4, iaddr, iaddrlen);	/* in_ndstip */
#else
			inet_ntop(AF_INET, &ipn.in_in[0].in4, iaddr, iaddrlen);
#endif
			r = 0;
		}
	} while (ipn.in_next != NULL);
	return r;
}

static int
real_delete_redirect_rule(const char * ifname, unsigned short eport, int proto)
{
	ipfgeniter_t iter;
	ipfobj_t obj;
	ipnat_t ipn;
	int r;
	UNUSED(ifname);

	memset(&obj, 0, sizeof(obj));
	obj.ipfo_rev = IPFILTER_VERSION;
	obj.ipfo_type = IPFOBJ_GENITER;
	obj.ipfo_size = sizeof(iter);
	obj.ipfo_ptr = &iter;

	iter.igi_type = IPFGENITER_IPNAT;
#if IPFILTER_VERSION > 4011300
	iter.igi_nitems = 1;
#endif
	iter.igi_data = &ipn;

	if (dev < 0) {
		syslog(LOG_ERR, "%s not open", IPNAT_NAME);
		return -1;
	}

	r = -1;
	do {
		if (ioctl(dev, SIOCGENITER, &obj) == -1) {
			syslog(LOG_ERR, "%s:ioctl(SIOCGENITER): %m",
			    "delete_redirect_rule");
			break;
		}
#if IPFILTER_VERSION >= 5000000
		if (eport == ntohs(ipn.in_dpmin) &&
		    eport == ntohs(ipn.in_dpmax) &&
		    strcmp(ipn.in_tag.ipt_tag, group_name) == 0 &&
		    ipn.in_pr[0] == proto)
#else
		if (eport == ntohs(ipn.in_pmin) &&
		    eport == ntohs(ipn.in_pmax) &&
		    strcmp(ipn.in_tag.ipt_tag, group_name) == 0 &&
		    ipn.in_p == proto)
#endif
		{
			obj.ipfo_rev = IPFILTER_VERSION;
			obj.ipfo_size = sizeof(ipn);
			obj.ipfo_ptr = &ipn;
			obj.ipfo_type = IPFOBJ_IPNAT;
			r = ioctl(dev, SIOCRMNAT, &obj);
			if (r == -1)
				syslog(LOG_ERR, "%s:ioctl(SIOCRMNAT): %m",
				    "delete_redirect_rule");
			/* Delete the desc even if the above failed */
			del_redirect_desc(eport, proto);
			break;
		}
	} while (ipn.in_next != NULL);
	return r;
}

/* FIXME: For some reason, the iter isn't reset every other delete,
 * so we attempt 2 deletes. */
int
delete_redirect_rule(const char * ifname, unsigned short eport, int proto)
{
	int r;

	r = real_delete_redirect_rule(ifname, eport, proto);
	if (r == -1)
		r = real_delete_redirect_rule(ifname, eport, proto);
	return r;
}

/* thanks to Seth Mos for this function */
int
add_filter_rule2(const char * ifname, const char * rhost,
    const char * iaddr, unsigned short eport, unsigned short iport,
    int proto, const char * desc)
{
	ipfobj_t obj;
	frentry_t fr;
	fripf_t ipffr;
	int r;
	UNUSED(ifname); UNUSED(desc);
	UNUSED(iport);

	if (dev_ipl < 0) {
		syslog(LOG_ERR, "%s not open", IPL_NAME);
		return -1;
	}

	memset(&obj, 0, sizeof(obj));
	memset(&fr, 0, sizeof(fr));
	memset(&ipffr, 0, sizeof(ipffr));

	fr.fr_flags = FR_PASS|FR_KEEPSTATE|FR_QUICK|FR_INQUE;
	if (GETFLAG(LOGPACKETSMASK))
		fr.fr_flags |= FR_LOG|FR_LOGFIRST;
#if IPFILTER_VERSION >= 5000000
	fr.fr_family = PF_INET;
#else
	fr.fr_v = 4;
#endif

	fr.fr_type = FR_T_IPF;
	fr.fr_dun.fru_ipf = &ipffr;
	fr.fr_dsize = sizeof(ipffr);
	fr.fr_isc = (void *)-1;

	fr.fr_proto = proto;
	fr.fr_mproto = 0xff;
	fr.fr_dcmp = FR_EQUAL;
	fr.fr_dport = eport;
#ifdef USE_IFNAME_IN_RULES
	if (ifname) {
#if IPFILTER_VERSION >= 5000000
		/* XXX check for stack overflow ! */
		fr.fr_ifnames[0] = fr.fr_namelen;
		strlcpy(fr.fr_names + fr.fr_ifnames[0], ifname, IFNAMSIZ);
		fr.fr_namelen += strlen(ifname) + 1;
#else
		strlcpy(fr.fr_ifnames[0], ifname, IFNAMSIZ);
#endif
	}
#endif
#if IPFILTER_VERSION >= 5000000
	/* XXX check for stack overflow ! */
	fr.fr_group = fr.fr_namelen;
	strlcpy(fr.fr_names + fr.fr_group, group_name, FR_GROUPLEN);
	fr.fr_namelen += strlen(group_name) + 1;
#else
	strlcpy(fr.fr_group, group_name, sizeof(fr.fr_group));
#endif

	if (proto == IPPROTO_TCP) {
		fr.fr_tcpf = TH_SYN;
		fr.fr_tcpfm = TH_SYN|TH_ACK|TH_RST|TH_FIN|TH_URG|TH_PUSH;
	}

	if(rhost && rhost[0] != '\0' && rhost[0] != '*')
	{
		inet_pton(AF_INET, rhost, &fr.fr_saddr);
		fr.fr_smask = 0xffffffff;
	}

	inet_pton(AF_INET, iaddr, &fr.fr_daddr);
	fr.fr_dmask = 0xffffffff;

	obj.ipfo_rev = IPFILTER_VERSION;
	obj.ipfo_ptr = &fr;
	obj.ipfo_size = sizeof(fr);

	r = ioctl(dev_ipl, SIOCINAFR, &obj);
	if (r == -1) {
		if (errno == ESRCH)
			syslog(LOG_ERR,
			    "SIOCINAFR(missing 'head %s' rule?):%m",
			    group_name);
		else
			syslog(LOG_ERR, "SIOCINAFR:%m");
	}
	return r;
}

int
delete_filter_rule(const char * ifname, unsigned short eport, int proto)
{
	ipfobj_t wobj, dobj;
	ipfruleiter_t rule;
	u_long darray[1000];
	u_long array[1000];
	friostat_t fio;
	frentry_t *fp;
	int r;
	UNUSED(ifname);

	if (dev_ipl < 0) {
		syslog(LOG_ERR, "%s not open", IPL_NAME);
		return -1;
	}

	wobj.ipfo_rev = IPFILTER_VERSION;
	wobj.ipfo_type = IPFOBJ_IPFSTAT;
	wobj.ipfo_size = sizeof(fio);
	wobj.ipfo_ptr = &fio;

	if (ioctl(dev_ipl, SIOCGETFS, &wobj) == -1) {
		syslog(LOG_ERR, "ioctl(SIOCGETFS): %m");
		return -1;
	}

	wobj.ipfo_rev = IPFILTER_VERSION;
	wobj.ipfo_ptr = &rule;
	wobj.ipfo_size = sizeof(rule);
	wobj.ipfo_type = IPFOBJ_IPFITER;

	fp = (frentry_t *)array;
	fp->fr_dun.fru_data = darray;
	fp->fr_dsize = sizeof(darray);

	rule.iri_inout = 0;
	rule.iri_active = fio.f_active;
#if IPFILTER_VERSION > 4011300
	rule.iri_nrules = 1;
	rule.iri_v = 4;
#endif
	rule.iri_rule = fp;
	strlcpy(rule.iri_group, group_name, sizeof(rule.iri_group));

	dobj.ipfo_rev = IPFILTER_VERSION;
	dobj.ipfo_size = sizeof(*fp);
	dobj.ipfo_type = IPFOBJ_FRENTRY;

	r = -1;
	do {
		memset(array, 0xff, sizeof(array));

		if (ioctl(dev_ipl, SIOCIPFITER, &wobj) == -1) {
			syslog(LOG_ERR, "ioctl(SIOCIPFITER): %m");
			break;
		}

		if (fp->fr_data != NULL)
			fp->fr_data = (char *)fp + sizeof(*fp);
		if ((fp->fr_type & ~FR_T_BUILTIN) == FR_T_IPF &&
		    fp->fr_dport == eport &&
		    fp->fr_proto == proto)
		{
			dobj.ipfo_ptr = fp;

			r = ioctl(dev_ipl, SIOCRMAFR, &dobj);
			if (r == -1)
				syslog(LOG_ERR, "ioctl(SIOCRMAFR): %m");
			break;
		}
	} while (fp->fr_next != NULL);
	return r;
}

unsigned short *
get_portmappings_in_range(unsigned short startport, unsigned short endport,
                          int proto, unsigned int * number)
{
	unsigned short *array, *array2;
	unsigned int capacity;
	unsigned short eport;
	ipfgeniter_t iter;
	ipfobj_t obj;
	ipnat_t ipn;

	*number = 0;
	if (dev < 0) {
		syslog(LOG_ERR, "%s not open", IPNAT_NAME);
		return NULL;
	}
	capacity = 128;
	array = calloc(capacity, sizeof(unsigned short));
	if(!array)
	{
		syslog(LOG_ERR, "get_portmappings_in_range() : calloc error");
		return NULL;
	}

	memset(&obj, 0, sizeof(obj));
	obj.ipfo_rev = IPFILTER_VERSION;
	obj.ipfo_ptr = &iter;
	obj.ipfo_size = sizeof(iter);
	obj.ipfo_type = IPFOBJ_GENITER;

	iter.igi_type = IPFGENITER_IPNAT;
#if IPFILTER_VERSION > 4011300
	iter.igi_nitems = 1;
#endif
	iter.igi_data = &ipn;

	do {
		if (ioctl(dev, SIOCGENITER, &obj) == -1) {
			syslog(LOG_ERR, "%s:ioctl(SIOCGENITER): %m",
			    "get_portmappings_in_range");
			break;
		}

		if (strcmp(ipn.in_tag.ipt_tag, group_name) != 0)
			continue;

#if IPFILTER_VERSION >= 5000000
		eport = ntohs(ipn.in_dpmin);
		if( (eport == ntohs(ipn.in_dpmax))
		  && (ipn.in_pr[0] == proto)
		  && (startport <= eport) && (eport <= endport) )
#else
		eport = ntohs(ipn.in_pmin);
		if( (eport == ntohs(ipn.in_pmax))
		  && (ipn.in_p == proto)
		  && (startport <= eport) && (eport <= endport) )
#endif
		{
			if(*number >= capacity)
			{
				/* need to increase the capacity of the array */
				capacity += 128;
				array2 = realloc(array, sizeof(unsigned short)*capacity);
				if(!array2)
				{
					syslog(LOG_ERR, "get_portmappings_in_range() : realloc(%lu) error", sizeof(unsigned short)*capacity);
					*number = 0;
					free(array);
					return NULL;
				}
				array = array2;
			}
			array[*number] = eport;
			(*number)++;
		}
	} while (ipn.in_next != NULL);
	return array;
}

/* update the port mapping internal port, description and timestamp */
int
update_portmapping(const char * ifname, unsigned short eport, int proto,
                   unsigned short iport, const char * desc,
                   unsigned int timestamp)
{
	UNUSED(ifname); UNUSED(eport); UNUSED(proto);
	UNUSED(iport); UNUSED(desc); UNUSED(timestamp);
	/* TODO: implement update_portmapping() */
	syslog(LOG_ERR, __FILE__ " update_portmapping() is not implemented");
	return -1;
}

/* update the port mapping description and timestamp */
int
update_portmapping_desc_timestamp(const char * ifname,
                   unsigned short eport, int proto,
                   const char * desc, unsigned int timestamp)
{
	UNUSED(ifname);
	del_redirect_desc(eport, proto);
	add_redirect_desc(eport,proto, timestamp, desc);
	return 0;
}

