/* $Id: portinuse.c,v 1.12 2020/11/04 21:29:50 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * (c) 2007-2020 Thomas Bernard
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#if defined(__DragonFly__) || defined(__FreeBSD__)
#include <err.h>
#include <stdlib.h>
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#if defined(__OpenBSD__)
#include <sys/queue.h>
#include <sys/select.h>
#include <kvm.h>
#include <fcntl.h>
#include <nlist.h>
#include <limits.h>
#include <net/route.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#endif

#if defined(__DragonFly__) || defined(__FreeBSD__)
#include <sys/socketvar.h>
#include <sys/sysctl.h>
/* sys/socketvar.h must be included above the following headers */
#include <netinet/in_pcb.h>
#include <netinet/tcp_var.h>
#endif

#include "macros.h"
#include "config.h"
#include "upnpglobalvars.h"
#include "getifaddr.h"
#include "portinuse.h"

#if defined(USE_NETFILTER)
#include "netfilter/iptcrdr.h"
#endif

#ifdef CHECK_PORTINUSE

#if defined(USE_NETFILTER)
/* Hardcoded for now.  Ideally would come from .conf file */
#	ifdef TOMATO
		const char *chains_to_check[] = { "WANPREROUTING" , 0 };
#	else
		const char *chains_to_check[] = { "PREROUTING" , 0 };
#	endif
#endif

int
port_in_use(const char *if_name,
            unsigned eport, int proto,
            const char *iaddr, unsigned iport)
{
	int found = 0;
	char ip_addr_str[INET_ADDRSTRLEN];
	struct in_addr ip_addr;
#ifdef __linux__
	/* linux code */
	char line[256];
	FILE *f;
	const char * tcpfile = "/proc/net/tcp";
	const char * udpfile = "/proc/net/udp";
#endif

	if(getifaddr(if_name, ip_addr_str, INET_ADDRSTRLEN, &ip_addr, NULL) < 0) {
		ip_addr.s_addr = 0;
		ip_addr_str[0] = '\0';
	}

	syslog(LOG_DEBUG, "Check protocol %s for port %u on ext_if %s %s, %08X",
	    (proto==IPPROTO_TCP)?"tcp":"udp", eport, if_name,
	    ip_addr_str, (unsigned)ip_addr.s_addr);

	/* Phase 1 : check for local sockets (would be listed by netstat) */
#if defined(__linux__)
	f = fopen((proto==IPPROTO_TCP)?tcpfile:udpfile, "r");
	if (!f) {
		syslog(LOG_ERR, "cannot open %s", (proto==IPPROTO_TCP)?tcpfile:udpfile);
		return -1;
	}

	while (fgets(line, 255, f)) {
		char eaddr[68];
		unsigned tmp_port;
		if (sscanf(line, "%*d: %64[0-9A-Fa-f]:%x %*x:%*x %*x %*x:%*x "
				"%*x:%*x %*x %*d %*d %*llu",
				eaddr, &tmp_port) == 2
		) {
			/* TODO add IPV6 support if enabled
			 * Presently assumes IPV4 */
#ifdef DEBUG
			syslog(LOG_DEBUG, "port_in_use check port %u and address %s", tmp_port, eaddr);
#endif
			if (tmp_port == eport) {
				char tmp_addr[4];
				struct in_addr *tmp_ip_addr = (struct in_addr *)tmp_addr;
				if (sscanf(eaddr,"%2hhx%2hhx%2hhx%2hhx",
					&tmp_addr[3],&tmp_addr[2],&tmp_addr[1],&tmp_addr[0]) == 4)
				{
					if (tmp_ip_addr->s_addr == 0 || tmp_ip_addr->s_addr == ip_addr.s_addr)
					{
						found++;
						break;  /* don't care how many, just that we found at least one */
					}
				}
			}
		}
	}
	fclose(f);

#elif defined(__OpenBSD__)
static struct nlist list[] = {
#if 0
        {"_tcpstat", 0, 0, 0, 0},
        {"_udpstat", 0, 0, 0, 0},
        {"_tcbinfo", 0, 0, 0, 0},
        {"_udbinfo", 0, 0, 0, 0},
#endif
		{"_tcbtable", 0, 0, 0, 0},
		{"_udbtable", 0, 0, 0, 0},
        {NULL,0, 0, 0, 0}
};
	char errstr[_POSIX2_LINE_MAX];
	kvm_t *kd;
	ssize_t n;
	struct inpcbtable table;
	struct inpcb *next;
	struct inpcb inpcb;
	kd = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, errstr);
	if(!kd) {
		syslog(LOG_ERR, "%s: kvm_openfiles(): %s",
		       "portinuse()", errstr);
		return -1;
	}
	if(kvm_nlist(kd, list) < 0) {
		syslog(LOG_ERR, "%s: kvm_nlist(): %s",
		       "portinuse()", kvm_geterr(kd));
		kvm_close(kd);
		return -1;
	}
	n = kvm_read(kd, list[(proto==IPPROTO_TCP)?0:1].n_value, &table, sizeof(table));
	if(n < 0) {
		syslog(LOG_ERR, "%s: kvm_read(): %s",
		       "portinuse()", kvm_geterr(kd));
		kvm_close(kd);
		return -1;
	}
	/* inpt_queue was CIRCLEQ_HEAD, it is TAILQ_HEAD since OpenBSD 5.5 */
#ifdef INPT_QUEUE_IS_CIRCLEQ
	next = CIRCLEQ_FIRST(&table.inpt_queue);
#else
	next = TAILQ_FIRST(&table.inpt_queue);
#endif
	while(next != NULL) {
		if(((u_long)next & 3) != 0) break;
		n = kvm_read(kd, (u_long)next, &inpcb, sizeof(inpcb));
		if(n < 0) {
			syslog(LOG_ERR, "kvm_read(): %s", kvm_geterr(kd));
			break;
		}
#ifdef INPT_QUEUE_IS_CIRCLEQ
		next = CIRCLEQ_NEXT(&inpcb, inp_queue);
#else
		next = TAILQ_NEXT(&inpcb, inp_queue);
#endif
		/* skip IPv6 sockets */
		if((inpcb.inp_flags & INP_IPV6) != 0)
			continue;
#ifdef DEBUG
		syslog(LOG_DEBUG, "%08lx:%hu %08lx:%hu",
		       (u_long)inpcb.inp_laddr.s_addr, ntohs(inpcb.inp_lport),
		       (u_long)inpcb.inp_faddr.s_addr, ntohs(inpcb.inp_fport));
#endif
		if(eport == (unsigned)ntohs(inpcb.inp_lport)) {
			if(inpcb.inp_laddr.s_addr == INADDR_ANY || inpcb.inp_laddr.s_addr == ip_addr.s_addr) {
				found++;
				break;  /* don't care how many, just that we found at least one */
			}
		}
	}
	kvm_close(kd);

#elif defined(__DragonFly__)
	const char *varname;
	struct xinpcb *xip;
	struct xtcpcb *xtp;
	struct inpcb *inp;
	void *buf = NULL;
	void *so_begin, *so_end;

	size_t len;

	switch (proto) {
	case IPPROTO_TCP:
		varname = "net.inet.tcp.pcblist";
		break;
	case IPPROTO_UDP:
		varname = "net.inet.udp.pcblist";
		break;
	default:
		syslog(LOG_ERR, "port_in_use() unknown proto=%d", proto);
		return -1;
	}

	if (sysctlbyname(varname, NULL, &len, NULL, 0) < 0) {
		syslog(LOG_ERR, "sysctlbyname(%s, NULL, ...): %m", varname);
		return -1;
	}
	buf = malloc(len);
	if (buf == NULL) {
		syslog(LOG_ERR, "malloc(%u) failed", (unsigned)len);
		return -1;
	}
	if (sysctlbyname(varname, buf, &len, NULL, 0) < 0) {
		syslog(LOG_ERR, "sysctlbyname(%s, buf, ...): %m", varname);
		free(buf);
		return -1;
	}

	so_begin = buf;
	so_end = (uint8_t *)buf + len;
	for (so_begin = buf, so_end = (uint8_t *)so_begin + len;
	     (uint8_t *)so_begin + sizeof(size_t) < (uint8_t *)so_end &&
	     (uint8_t *)so_begin + *(size_t *)so_begin <= (uint8_t *)so_end;
	     so_begin = (uint8_t *)so_begin + *(size_t *)so_begin) {
		switch (proto) {
		case IPPROTO_TCP:
			xtp = (struct xtcpcb *)so_begin;
			if (xtp->xt_len != sizeof *xtp) {
				syslog(LOG_WARNING, "struct xtcpcb size mismatch; %ld vs %ld",
				       (long)xtp->xt_len, sizeof *xtp);
				free(buf);
				return -1;
			}
			inp = &xtp->xt_inp;
			break;
		case IPPROTO_UDP:
			xip = (struct xinpcb *)so_begin;
			if (xip->xi_len != sizeof *xip) {
				syslog(LOG_WARNING, "struct xinpcb size mismatch : %ld vs %ld",
				       (long)xip->xi_len, sizeof *xip);
				free(buf);
				return -1;
			}
			inp = &xip->xi_inp;
			break;
		default:
			abort();
		}
		/* no support for IPv6 */
		if (INP_ISIPV6(inp) != 0)
			continue;
		syslog(LOG_DEBUG, "%08lx:%hu %08lx:%hu <=> %u %08lx:%u",
		       (u_long)inp->inp_laddr.s_addr, ntohs(inp->inp_lport),
		       (u_long)inp->inp_faddr.s_addr, ntohs(inp->inp_fport),
		       eport, (u_long)ip_addr.s_addr, iport
		);
		if (eport == (unsigned)ntohs(inp->inp_lport)) {
			if (inp->inp_laddr.s_addr == INADDR_ANY || inp->inp_laddr.s_addr == ip_addr.s_addr) {
				found++;
				break;  /* don't care how many, just that we found at least one */
			}
		}
	}
	if (buf) {
		free(buf);
		buf = NULL;
	}
#elif defined(__FreeBSD__)
	const char *varname;
	struct xinpgen *xig, *exig;
	struct xinpcb *xip;
	struct xtcpcb *xtp;
	struct in_conninfo *inc;
	void *buf = NULL;
	size_t len;

	switch (proto) {
	case IPPROTO_TCP:
		varname = "net.inet.tcp.pcblist";
		break;
	case IPPROTO_UDP:
		varname = "net.inet.udp.pcblist";
		break;
	default:
		syslog(LOG_ERR, "port_in_use() unknown proto=%d", proto);
		return -1;
	}

	if (sysctlbyname(varname, NULL, &len, NULL, 0) < 0) {
		syslog(LOG_ERR, "sysctlbyname(%s, NULL, ...): %m", varname);
		return -1;
	}
	buf = malloc(len);
	if (buf == NULL) {
		syslog(LOG_ERR, "malloc(%u) failed", (unsigned)len);
		return -1;
	}
	if (sysctlbyname(varname, buf, &len, NULL, 0) < 0) {
		syslog(LOG_ERR, "sysctlbyname(%s, buf, ...): %m", varname);
		free(buf);
		return -1;
	}

	xig = (struct xinpgen *)buf;
	exig = (struct xinpgen *)(void *)((char *)buf + len - sizeof *exig);
	if (xig->xig_len != sizeof *xig) {
		syslog(LOG_WARNING, "struct xinpgen size mismatch; %ld vs %ld",
		       (long)xig->xig_len, sizeof *xig);
		free(buf);
		return -1;
	}
	if (exig->xig_len != sizeof *exig) {
		syslog(LOG_WARNING, "struct xinpgen size mismatch; %ld vs %ld",
		       (long)exig->xig_len, sizeof *exig);
		free(buf);
		return -1;
	}

	while (1) {
		xig = (struct xinpgen *)(void *)((char *)xig + xig->xig_len);
		if (xig >= exig)
			break;
		switch (proto) {
		case IPPROTO_TCP:
			xtp = (struct xtcpcb *)xig;
			if (xtp->xt_len != sizeof *xtp) {
				syslog(LOG_WARNING, "struct xtcpcb size mismatch; %ld vs %ld",
				       (long)xtp->xt_len, sizeof *xtp);
				free(buf);
				return -1;
			}
			xip = &xtp->xt_inp;
			inc = &xip->inp_inc;
			break;
		case IPPROTO_UDP:
			xip = (struct xinpcb *)xig;
			if (xip->xi_len != sizeof *xip) {
				syslog(LOG_WARNING, "struct xinpcb size mismatch : %ld vs %ld",
				       (long)xip->xi_len, sizeof *xip);
				free(buf);
				return -1;
			}
			inc = &xip->inp_inc;
			break;
		default:
			abort();
		}
		/* no support for IPv6 */
		if ((xip->inp_vflag & INP_IPV6) != 0)
			continue;
		syslog(LOG_DEBUG, "%08lx:%hu %08lx:%hu <=> %u %08lx:%u",
		       (u_long)inc->inc_laddr.s_addr, ntohs(inc->inc_lport),
		       (u_long)inc->inc_faddr.s_addr, ntohs(inc->inc_fport),
		       eport, (u_long)ip_addr.s_addr, iport
		);
		if (eport == (unsigned)ntohs(inc->inc_lport)) {
			if (inc->inc_laddr.s_addr == INADDR_ANY || inc->inc_laddr.s_addr == ip_addr.s_addr) {
				found++;
				break;  /* don't care how many, just that we found at least one */
			}
		}
	}
	if (buf) {
		free(buf);
		buf = NULL;
	}
/* #elif __NetBSD__ */
#else
/* TODO : NetBSD / Darwin (OS X) / Solaris code */
#error "No port_in_use() implementation available for this OS"
#endif

	/* Phase 2 : check existing mappings
	 * TODO : implement for pf/ipfw/etc. */
#if defined(USE_NETFILTER)
	if (!found) {
		char iaddr_old[16];
		unsigned short iport_old;
		int i;
		for (i = 0; chains_to_check[i]; i++) {
			if (get_nat_redirect_rule(chains_to_check[i], if_name, eport, proto,
					iaddr_old, sizeof(iaddr_old), &iport_old,
					0, 0, 0, 0, 0, 0, 0) == 0)
			{
				syslog(LOG_DEBUG, "port_in_use check port %d on nat chain %s redirected to %s port %d", eport,
						chains_to_check[i], iaddr_old, iport_old);
				if (!(strcmp(iaddr, iaddr_old)==0 && iport==iport_old)) {
					/* only "in use" if redirected to somewhere else */
					found++;
					break;  /* don't care how many, just that we found at least one */
				}
			}
		}
	}
#else /* USE_NETFILTER */
	UNUSED(iport); UNUSED(iaddr);
#endif /* USE_NETFILTER */
	return found;
}
#endif /* CHECK_PORTINUSE */
