/* $Id: miniupnpd.c,v 1.264 2024/06/22 18:14:08 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2024 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include "config.h"

/* Experimental support for NFQUEUE interfaces */
#ifdef ENABLE_NFQUEUE
/* apt-get install libnetfilter-queue-dev */
#include <netinet/ip.h>
#include <netinet/udp.h>
#if 0
#include <linux/netfilter_ipv4.h>  /* Defines verdicts (NF_ACCEPT, etc) */
#endif
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <linux/netfilter/nfnetlink_queue.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/file.h>
#include <syslog.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/param.h>
#if defined(sun)
#include <kstat.h>
#elif !defined(__linux__)
/* for BSD's sysctl */
#include <sys/sysctl.h>
#endif
#ifdef HAS_LIBCAP
#include <sys/capability.h>
#endif
#ifdef HAS_LIBCAP_NG
#include <cap-ng.h>
#endif

/* unix sockets */
#ifdef USE_MINIUPNPDCTL
#include <sys/un.h>
#endif
#ifdef ENABLE_HTTPS
#include <openssl/crypto.h>
#endif

#ifdef DYNAMIC_OS_VERSION
#include <sys/utsname.h>
#endif

#ifdef TOMATO
#include <sys/stat.h>
#endif /* TOMATO */
#include "macros.h"
#include "upnpglobalvars.h"
#include "upnphttp.h"
#include "upnpdescgen.h"
#include "miniupnpdpath.h"
#include "getifaddr.h"
#include "upnpsoap.h"
#include "options.h"
#include "minissdp.h"
#include "upnpredirect.h"
#include "upnppinhole.h"
#include "upnpstun.h"
#include "miniupnpdtypes.h"
#include "daemonize.h"
#include "upnpevents.h"
#include "asyncsendto.h"
#ifdef ENABLE_NATPMP
#include "natpmp.h"
#ifdef ENABLE_PCP
#include "pcpserver.h"
#else
#define PCP_MAX_LEN 32
#endif
#endif
#include "commonrdr.h"
#include "upnputils.h"
#ifdef USE_IFACEWATCHER
#include "ifacewatcher.h"
#endif
#ifdef ENABLE_UPNPPINHOLE
#ifdef USE_NETFILTER
void init_iptpinhole(void);
#endif
#endif

#ifdef USE_MINIUPNPDCTL
struct ctlelem {
	int socket;
	LIST_ENTRY(ctlelem) entries;
};
#endif	/* USE_MINIUPNPDCTL */

#ifdef ENABLE_NFQUEUE
/* globals */
static struct nfq_handle *nfqHandle;
static struct sockaddr_in ssdp;

/* prototypes */
static int nfqueue_cb( struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data) ;
int identify_ip_protocol (char *payload);
int get_udp_dst_port (char *payload);
#endif	/* ENABLE_NFQUEUE */

/* variables used by signals */
static volatile sig_atomic_t quitting = 0;
volatile sig_atomic_t should_send_public_address_change_notif = 0;
#if !defined(TOMATO) && defined(ENABLE_LEASEFILE) && defined(LEASEFILE_USE_REMAINING_TIME)
volatile sig_atomic_t should_rewrite_leasefile = 0;
#endif /* !TOMATO && ENABLE_LEASEFILE && LEASEFILE_USE_REMAINING_TIME */

#ifdef TOMATO
#if 1
/* Tomato specific code */
static volatile sig_atomic_t gotusr2 = 0;

static void
sigusr2(int sig)
{
	gotusr2 = 1;
}

static void
tomato_save(const char *fname)
{
	unsigned short eport;
	unsigned short iport;
	unsigned int leaseduration;
	unsigned int timestamp;
	char proto[4];
	char iaddr[32];
	char desc[64];
	char rhost[32];
	int n;
	FILE *f;
	int t;
	char tmp[128];

	strcpy(tmp, "/etc/upnp/saveXXXXXX");
	if ((t = mkstemp(tmp)) != -1)
	{
		if ((f = fdopen(t, "w")) != NULL)
		{
			n = 0;
			while (upnp_get_redirection_infos_by_index(n, &eport, proto, &iport, iaddr, sizeof(iaddr), desc, sizeof(desc), rhost, sizeof(rhost), &leaseduration) == 0)
			{
				timestamp = (leaseduration > 0) ? time(NULL) + leaseduration : 0;
				fprintf(f, "%s %u %s %u [%s] %u\n", proto, eport, iaddr, iport, desc, timestamp);
				++n;
			}
			fclose(f);
			rename(tmp, fname);
		}
		else
		{
			close(t);
		}
		unlink(tmp);
	}
}

static void
tomato_load(void)
{
	FILE *f;
	char s[256];
	unsigned short eport;
	unsigned short iport;
	unsigned int leaseduration;
	unsigned int timestamp;
	time_t current_time;
	char proto[4];
	char iaddr[32];
	char *rhost;
	char *a, *b;

	if ((f = fopen("/etc/upnp/data", "r")) != NULL)
	{
		current_time = time(NULL);
		s[sizeof(s) - 1] = 0;
		while (fgets(s, sizeof(s) - 1, f)) {
			if (sscanf(s, "%3s %hu %31s %hu [%*[^]]] %u", proto, &eport, iaddr, &iport, &timestamp) >= 4)
			{
				if (((a = strchr(s, '[')) != NULL) && ((b = strrchr(a, ']')) != NULL))
				{
					if (timestamp > 0)
					{
						if (timestamp > current_time)
							leaseduration = timestamp - current_time;
						else
							continue;
					}
					else
					{
						leaseduration = 0;	/* default value */
					}
					*b = 0;
					rhost = NULL;
					syslog(LOG_DEBUG, "Read redirection [%s] from file: %s port %hu to %s port %hu, timestamp: %u (%u)",
						a + 1, proto, eport, iaddr, iport, timestamp, leaseduration);
					upnp_redirect(rhost, eport, iaddr, iport, proto, a + 1, leaseduration);
				}
			}
		}
		fclose(f);
	}
#ifdef ENABLE_NATPMP
#if 0
	ScanNATPMPforExpiration();
#endif
#endif /* ENABLE_NATPMP */
	unlink("/etc/upnp/load");
}

static void
tomato_delete(void)
{
	FILE *f;
	char s[128];
	unsigned short eport;
	unsigned short iport;
	unsigned int leaseduration;
	char proto[4];
	char iaddr[32];
	char desc[64];
	char rhost[32];
	int n;

	if ((f = fopen("/etc/upnp/delete", "r")) != NULL)
	{
		s[sizeof(s) - 1] = 0;
		while (fgets(s, sizeof(s) - 1, f))
		{
			if (sscanf(s, "%3s %hu", proto, &eport) == 2)
			{
				if (proto[0] == '*')
				{
					n = upnp_get_portmapping_number_of_entries();
					while (--n >= 0)
					{
						if (upnp_get_redirection_infos_by_index(n, &eport, proto, &iport, iaddr, sizeof(iaddr), desc, sizeof(desc), rhost, sizeof(rhost), &leaseduration) == 0)
						{
							upnp_delete_redirection(eport, proto);
						}
					}
					break;
				}
				else
				{
					upnp_delete_redirection(eport, proto);
				}
			}
		}
		fclose(f);
		unlink("/etc/upnp/delete");
	}
}

static void
tomato_helper(void)
{
	struct stat st;

	if (stat("/etc/upnp/delete", &st) == 0)
	{
		tomato_delete();
	}

	if (stat("/etc/upnp/load", &st) == 0)
	{
		tomato_load();
	}

	if (stat("/etc/upnp/save", &st) == 0)
	{
		tomato_save("/etc/upnp/data");
		unlink("/etc/upnp/save");
	}

	if (stat("/etc/upnp/info", &st) == 0)
	{
		tomato_save("/etc/upnp/data.info");
		unlink("/etc/upnp/info");
	}
}
#endif  /* 1 (tomato) */
#endif	/* TOMATO */

static int gen_current_notify_interval(int notify_interval) {
	/* if possible, remove a random number of seconds between 1 and 64 */
	if (notify_interval > 65)
		return notify_interval - 1 - (random() & 0x3f);
	else
		return notify_interval;
}

/* OpenAndConfHTTPSocket() :
 * setup the socket used to handle incoming HTTP connections. */
static int
#ifdef ENABLE_IPV6
OpenAndConfHTTPSocket(unsigned short * port, int ipv6)
#else
OpenAndConfHTTPSocket(unsigned short * port)
#endif
{
	int s;
	int i = 1;
#ifdef ENABLE_IPV6
	struct sockaddr_in6 listenname6;
	struct sockaddr_in listenname4;
#else
	struct sockaddr_in listenname;
#endif
	socklen_t listenname_len;

	s = socket(
#ifdef ENABLE_IPV6
	           ipv6 ? PF_INET6 : PF_INET,
#else
	           PF_INET,
#endif
	           SOCK_STREAM, 0);
#ifdef ENABLE_IPV6
	if(s < 0 && ipv6 && errno == EAFNOSUPPORT)
	{
		/* the system doesn't support IPV6 */
		syslog(LOG_WARNING, "socket(PF_INET6, ...) failed with EAFNOSUPPORT, disabling IPv6");
		SETFLAG(IPV6DISABLEDMASK);
		ipv6 = 0;
		/* Try again with IPv4 */
		s = socket(PF_INET, SOCK_STREAM, 0);
	}
#endif
	if(s < 0)
	{
		syslog(LOG_ERR, "socket(http): %m");
		return -1;
	}

	if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i)) < 0)
	{
		syslog(LOG_WARNING, "setsockopt(http, SO_REUSEADDR): %m");
	}
#if 0
	/* enable this to force IPV6 only for IPV6 socket.
	 * see http://www.ietf.org/rfc/rfc3493.txt section 5.3 */
	if(setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &i, sizeof(i)) < 0)
	{
		syslog(LOG_WARNING, "setsockopt(http, IPV6_V6ONLY): %m");
	}
#endif

	if(!set_non_blocking(s))
	{
		syslog(LOG_WARNING, "set_non_blocking(http): %m");
	}

#ifdef ENABLE_IPV6
	if(ipv6)
	{
		memset(&listenname6, 0, sizeof(struct sockaddr_in6));
		listenname6.sin6_family = AF_INET6;
		listenname6.sin6_port = htons(*port);
		listenname6.sin6_addr = ipv6_bind_addr;
		listenname_len =  sizeof(struct sockaddr_in6);
	} else {
		memset(&listenname4, 0, sizeof(struct sockaddr_in));
		listenname4.sin_family = AF_INET;
		listenname4.sin_port = htons(*port);
		listenname4.sin_addr.s_addr = htonl(INADDR_ANY);
		listenname_len =  sizeof(struct sockaddr_in);
	}
#else
	memset(&listenname, 0, sizeof(struct sockaddr_in));
	listenname.sin_family = AF_INET;
	listenname.sin_port = htons(*port);
	listenname.sin_addr.s_addr = htonl(INADDR_ANY);
	listenname_len =  sizeof(struct sockaddr_in);
#endif

#if defined(SO_BINDTODEVICE) && !defined(MULTIPLE_EXTERNAL_IP)
	/* One and only one LAN interface */
	if(lan_addrs.lh_first != NULL && lan_addrs.lh_first->list.le_next == NULL
	   && lan_addrs.lh_first->ifname[0] != '\0')
	{
		if(setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE,
		              lan_addrs.lh_first->ifname,
		              strlen(lan_addrs.lh_first->ifname)) < 0)
			syslog(LOG_WARNING, "setsockopt(http, SO_BINDTODEVICE, %s): %m",
			       lan_addrs.lh_first->ifname);
	}
#endif /* defined(SO_BINDTODEVICE) && !defined(MULTIPLE_EXTERNAL_IP) */

#ifdef ENABLE_IPV6
	if(bind(s,
	        ipv6 ? (struct sockaddr *)&listenname6 : (struct sockaddr *)&listenname4,
	        listenname_len) < 0)
#else
	if(bind(s, (struct sockaddr *)&listenname, listenname_len) < 0)
#endif
	{
		syslog(LOG_ERR, "bind(http): %m");
		close(s);
		return -1;
	}

	if(listen(s, 5) < 0)
	{
		syslog(LOG_ERR, "listen(http): %m");
		close(s);
		return -1;
	}

	if(*port == 0) {
#ifdef ENABLE_IPV6
		if(ipv6) {
			struct sockaddr_in6 sockinfo;
			socklen_t len = sizeof(struct sockaddr_in6);
			if (getsockname(s, (struct sockaddr *)&sockinfo, &len) < 0) {
				syslog(LOG_ERR, "getsockname(): %m");
			} else {
				*port = ntohs(sockinfo.sin6_port);
			}
		} else {
#endif /* ENABLE_IPV6 */
			struct sockaddr_in sockinfo;
			socklen_t len = sizeof(struct sockaddr_in);
			if (getsockname(s, (struct sockaddr *)&sockinfo, &len) < 0) {
				syslog(LOG_ERR, "getsockname(): %m");
			} else {
				*port = ntohs(sockinfo.sin_port);
			}
#ifdef ENABLE_IPV6
		}
#endif /* ENABLE_IPV6 */
	}
	return s;
}

static struct upnphttp *
ProcessIncomingHTTP(int shttpl, const char * protocol)
{
	int shttp;
	socklen_t clientnamelen;
#ifdef ENABLE_IPV6
	struct sockaddr_storage clientname;
	clientnamelen = sizeof(struct sockaddr_storage);
#else
	struct sockaddr_in clientname;
	clientnamelen = sizeof(struct sockaddr_in);
#endif
	shttp = accept(shttpl, (struct sockaddr *)&clientname, &clientnamelen);
	if(shttp<0)
	{
		/* ignore EAGAIN, EWOULDBLOCK, EINTR, we just try again later */
		if(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
			syslog(LOG_ERR, "accept(http): %m");
	}
	else
	{
		struct upnphttp * tmp = 0;
		char addr_str[64];

		sockaddr_to_string((struct sockaddr *)&clientname, addr_str, sizeof(addr_str));
#ifdef DEBUG
		syslog(LOG_DEBUG, "%s connection from %s", protocol, addr_str);
#endif /* DEBUG */
		if(get_lan_for_peer((struct sockaddr *)&clientname) == NULL)
		{
			/* The peer is not a LAN ! */
			syslog(LOG_WARNING,
			       "%s peer %s is not from a LAN, closing the connection",
			       protocol, addr_str);
			close(shttp);
		}
		else
		{
			/* Create a new upnphttp object and add it to
			 * the active upnphttp object list */
			tmp = New_upnphttp(shttp);
			if(tmp)
			{
#ifdef ENABLE_IPV6
				if(clientname.ss_family == AF_INET)
				{
					tmp->clientaddr = ((struct sockaddr_in *)&clientname)->sin_addr;
				}
				else if(clientname.ss_family == AF_INET6)
				{
					struct sockaddr_in6 * addr = (struct sockaddr_in6 *)&clientname;
					if(IN6_IS_ADDR_V4MAPPED(&addr->sin6_addr))
					{
						memcpy(&tmp->clientaddr,
						       &addr->sin6_addr.s6_addr[12],
						       4);
					}
					else
					{
						tmp->ipv6 = 1;
						memcpy(&tmp->clientaddr_v6,
						       &addr->sin6_addr,
						       sizeof(struct in6_addr));
					}
				}
#else
				tmp->clientaddr = clientname.sin_addr;
#endif
				memcpy(tmp->clientaddr_str, addr_str, sizeof(tmp->clientaddr_str));
				return tmp;
			}
			else
			{
				syslog(LOG_ERR, "New_upnphttp() failed");
				close(shttp);
			}
		}
	}
	return NULL;
}

#ifdef ENABLE_NFQUEUE

int identify_ip_protocol(char *payload) {
    return payload[9];
}


/*
 * This function returns the destination port of the captured packet UDP
 */
int get_udp_dst_port(char *payload) {
        char *pkt_data_ptr = NULL;
        pkt_data_ptr = payload + sizeof(struct ip);

    /* Cast the UDP Header from the raw packet */
    struct udphdr *udp = (struct udphdr *) pkt_data_ptr;

    /* get the dst port of the packet */
    return(ntohs(udp->dest));

}
static int
OpenAndConfNFqueue(){

        struct nfq_q_handle *myQueue;
        struct nfnl_handle *netlinkHandle;

        int fd = 0, e = 0;

	inet_pton(AF_INET, "239.255.255.250", &(ssdp.sin_addr));

        /* Get a queue connection handle from the module */
        if (!(nfqHandle = nfq_open())) {
		syslog(LOG_ERR, "Error in nfq_open(): %m");
                return -1;
        }

        /* Unbind the handler from processing any IP packets
           Not totally sure why this is done, or if it's necessary... */
        if ((e = nfq_unbind_pf(nfqHandle, AF_INET)) < 0) {
		syslog(LOG_ERR, "Error in nfq_unbind_pf(): %m");
                return -1;
        }

        /* Bind this handler to process IP packets... */
        if (nfq_bind_pf(nfqHandle, AF_INET) < 0) {
		syslog(LOG_ERR, "Error in nfq_bind_pf(): %m");
                return -1;
        }

        /*      Install a callback on queue -Q */
        if (!(myQueue = nfq_create_queue(nfqHandle,  nfqueue, &nfqueue_cb, NULL))) {
		syslog(LOG_ERR, "Error in nfq_create_queue(): %m");
                return -1;
        }

        /*      Turn on packet copy mode */
        if (nfq_set_mode(myQueue, NFQNL_COPY_PACKET, 0xffff) < 0) {
		syslog(LOG_ERR, "Error setting packet copy mode (): %m");
                return -1;
        }

        netlinkHandle = nfq_nfnlh(nfqHandle);
        fd = nfnl_fd(netlinkHandle);

	return fd;

}


static int nfqueue_cb(
                struct nfq_q_handle *qh,
                struct nfgenmsg *nfmsg,
                struct nfq_data *nfa,
                void *data) {

	char	*pkt;
	struct nfqnl_msg_packet_hdr *ph;
	ph = nfq_get_msg_packet_hdr(nfa);

	if ( ph ) {

		int id = 0, size = 0;
		id = ntohl(ph->packet_id);

		size = nfq_get_payload(nfa, &pkt);

    		struct ip *iph = (struct ip *) pkt;

		int id_protocol = identify_ip_protocol(pkt);

		int dport = get_udp_dst_port(pkt);

		int x = sizeof (struct ip) + sizeof (struct udphdr);

		/* packets we are interested in are UDP multicast to 239.255.255.250:1900
		 * and start with a data string M-SEARCH
		 */
		if ( (dport == 1900) && (id_protocol == IPPROTO_UDP)
			&& (ssdp.sin_addr.s_addr == iph->ip_dst.s_addr) ) {

			/* get the index that the packet came in on */
			u_int32_t idx = nfq_get_indev(nfa);
			int i = 0;
			for ( ;i < n_nfqix ; i++) {
				if ( nfqix[i] == idx ) {

					struct udphdr *udp = (struct udphdr *) (pkt + sizeof(struct ip));

					char *dd = pkt + x;

					struct sockaddr_in sendername;
					sendername.sin_family = AF_INET;
					sendername.sin_port = udp->source;
					sendername.sin_addr.s_addr = iph->ip_src.s_addr;

					/* printf("pkt found %s\n",dd);*/
					ProcessSSDPData (sudp, dd, size - x,
					                 &sendername, -1, (unsigned short) 5555);
				}
			}
		}

		nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);

	} else {
		syslog(LOG_ERR,"nfq_get_msg_packet_hdr failed");
		return 1;
		/* from nfqueue source: 0 = ok, >0 = soft error, <0 hard error */
	}

	return 0;
}

static void ProcessNFQUEUE(int fd){
	char buf[4096];

	socklen_t len_r;
	struct sockaddr_in sendername;
	len_r = sizeof(struct sockaddr_in);

        int res = recvfrom(fd, buf, sizeof(buf), 0,
			(struct sockaddr *)&sendername, &len_r);

	nfq_handle_packet(nfqHandle, buf, res);
}
#endif

/* Functions used to communicate with miniupnpdctl */
#ifdef USE_MINIUPNPDCTL
static int
OpenAndConfCtlUnixSocket(const char * path)
{
	struct sockaddr_un localun;
	int s;
	s = socket(AF_UNIX, SOCK_STREAM, 0);
	localun.sun_family = AF_UNIX;
	strncpy(localun.sun_path, path,
	          sizeof(localun.sun_path));
	if(bind(s, (struct sockaddr *)&localun,
	        sizeof(struct sockaddr_un)) < 0)
	{
		syslog(LOG_ERR, "bind(sctl): %m");
		close(s);
		s = -1;
	}
	else if(listen(s, 5) < 0)
	{
		syslog(LOG_ERR, "listen(sctl): %m");
		close(s);
		s = -1;
	}
	return s;
}

static void
write_upnphttp_details(int fd, struct upnphttp * e)
{
	char buffer[256];
	int len;
	write(fd, "HTTP :\n", 7);
	while(e)
	{
		len = snprintf(buffer, sizeof(buffer),
		               "%d %d %s req_buf=%p(%dbytes) res_buf=%p(%dbytes alloc)\n",
		               e->socket, e->state, e->HttpVer,
		               e->req_buf, e->req_buflen,
		               e->res_buf, e->res_buf_alloclen);
		write(fd, buffer, len);
		e = e->entries.le_next;
	}
}

static void
write_ctlsockets_list(int fd, struct ctlelem * e)
{
	char buffer[256];
	int len;
	write(fd, "CTL :\n", 6);
	while(e)
	{
		len = snprintf(buffer, sizeof(buffer),
		               "struct ctlelem: socket=%d\n", e->socket);
		write(fd, buffer, len);
		e = e->entries.le_next;
	}
}

#ifndef DISABLE_CONFIG_FILE
static void
write_option_list(int fd)
{
	char buffer[256];
	int len;
	unsigned int i;
	write(fd, "Options :\n", 10);
	for(i=0; i<num_options; i++)
	{
		len = snprintf(buffer, sizeof(buffer),
		               "opt=%02d %s\n",
		               ary_options[i].id, ary_options[i].value);
		write(fd, buffer, len);
	}
}
#endif

static void
write_command_line(int fd, int argc, char * * argv)
{
	char buffer[256];
	int len;
	int i;
	write(fd, "Command Line :\n", 15);
	for(i=0; i<argc; i++)
	{
		len = snprintf(buffer, sizeof(buffer),
		               "argv[%02d]='%s'\n",
		                i, argv[i]);
		write(fd, buffer, len);
	}
}

#endif

/* Handler for the SIGTERM signal (kill)
 * SIGINT is also handled */
static void
sigterm(int sig)
{
	UNUSED(sig);
	/*int save_errno = errno; */
	/*signal(sig, SIG_IGN);*/	/* Ignore this signal while we are quitting */
	/* Note : isn't it useless ? */

#if 0
	/* calling syslog() is forbidden in signal handler according to
	 * signal(3) */
	syslog(LOG_NOTICE, "received signal %d, good-bye", sig);
#endif

	quitting = 1;
	/*errno = save_errno;*/
}

/* Handler for the SIGUSR1 signal indicating public IP address change. */
static void
sigusr1(int sig)
{
	UNUSED(sig);
#if 0
	/* calling syslog() is forbidden in signal handler according to
	 * signal(3) */
	syslog(LOG_INFO, "received signal %d, public IP address change", sig);
#endif

	should_send_public_address_change_notif = 1;
}

#if !defined(TOMATO) && defined(ENABLE_LEASEFILE) && defined(LEASEFILE_USE_REMAINING_TIME)
/* Handler for the SIGUSR2 signal to request rewrite of lease_file */
static void
sigusr2(int sig)
{
	UNUSED(sig);
	should_rewrite_leasefile = 1;
}
#endif /* !TOMATO && ENABLE_LEASEFILE && LEASEFILE_USE_REMAINING_TIME */

/* record the startup time, for returning uptime */
static void
set_startup_time(void)
{
	startup_time = upnp_time();
#ifdef USE_TIME_AS_BOOTID
	if(upnp_bootid == 1) {
		upnp_bootid = (unsigned int)time(NULL);
		/* from UDA v1.1 :
		 * A convenient mechanism is to set this field value to the time
		 * that the device sends its initial announcement, expressed as
		 * seconds elapsed since midnight January 1, 1970; */
	}
#endif /* USE_TIME_AS_BOOTID */
	if(GETFLAG(SYSUPTIMEMASK))
	{
		/* use system uptime instead of daemon uptime */
#if defined(__linux__)
		unsigned long uptime = 0;
		FILE * f = fopen("/proc/uptime", "r");
		if(f == NULL)
		{
			syslog(LOG_ERR, "fopen(\"/proc/uptime\") : %m");
		}
		else
		{
			if(fscanf(f, "%lu", &uptime) != 1)
			{
				syslog(LOG_ERR, "fscanf(\"/proc/uptime\") : %m");
			}
			else
			{
				syslog(LOG_INFO, "system uptime is %lu seconds", uptime);
			}
			fclose(f);
			startup_time -= uptime;
		}
#elif defined(SOLARIS_KSTATS)
		kstat_ctl_t *kc;
		kc = kstat_open();
		if(kc != NULL)
		{
			kstat_t *ksp;
			ksp = kstat_lookup(kc, "unix", 0, "system_misc");
			if(ksp && (kstat_read(kc, ksp, NULL) != -1))
			{
				void *ptr = kstat_data_lookup(ksp, "boot_time");
				if(ptr)
					memcpy(&startup_time, ptr, sizeof(startup_time));
				else
					syslog(LOG_ERR, "cannot find boot_time kstat");
			}
			else
				syslog(LOG_ERR, "cannot open kstats for unix/0/system_misc: %m");
			kstat_close(kc);
		}
#else
		struct timeval boottime;
		size_t size = sizeof(boottime);
		int name[2] = { CTL_KERN, KERN_BOOTTIME };
		if(sysctl(name, 2, &boottime, &size, NULL, 0) < 0)
		{
			syslog(LOG_ERR, "sysctl(\"kern.boottime\") failed");
		}
		else
		{
			startup_time = boottime.tv_sec;
		}
#endif
	}
}

/* structure containing variables used during "main loop"
 * that are filled during the init */
struct runtime_vars {
	/* LAN IP addresses for SSDP traffic and HTTP */
	/* moved to global vars */
	int port;	/* HTTP Port */
#ifdef ENABLE_HTTPS
	int https_port;	/* HTTPS Port */
#endif
	int notify_interval;	/* seconds between SSDP announces. Should be >= 900s */
	/* unused rules cleaning related variables : */
	int clean_ruleset_threshold;	/* threshold for removing unused rules */
	int clean_ruleset_interval;		/* (minimum) interval between checks. 0=disabled */
};

/* parselanaddr()
 * parse address with mask
 * ex: 192.168.1.1/24 or 192.168.1.1/255.255.255.0
 * When MULTIPLE_EXTERNAL_IP is enabled, the IP address of the
 * external interface associated with the lan subnet follows.
 * ex : 192.168.1.1/24 81.21.41.11
 *
 * Can also use the interface name (ie eth0)
 *
 * return value :
 *    0 : ok
 *   -1 : error */
static int
parselanaddr(struct lan_addr_s * lan_addr, const char * str, int debug_flag)
{
	const char * p;
	unsigned int n;
	char tmp[16];

	memset(lan_addr, 0, sizeof(struct lan_addr_s));
	p = str;
	while(*p && *p != '/' && !isspace(*p))
		p++;
	n = p - str;
	if(!isdigit(str[0]) && n < (int)sizeof(lan_addr->ifname))
	{
		/* not starting with a digit : suppose it is an interface name */
		memcpy(lan_addr->ifname, str, n);
		lan_addr->ifname[n] = '\0';
		if(getifaddr(lan_addr->ifname, lan_addr->str, sizeof(lan_addr->str),
		             &lan_addr->addr, &lan_addr->mask) < 0) {
#ifdef ENABLE_IPV6
			fprintf(stderr, "interface \"%s\" has no IPv4 address\n", str);
			syslog(LOG_NOTICE, "interface \"%s\" has no IPv4 address\n", str);
			lan_addr->str[0] = '\0';
			lan_addr->addr.s_addr = htonl(0x00000000u);
			lan_addr->mask.s_addr = htonl(0xffffffffu);
#else /* ENABLE_IPV6 */
			goto parselan_error;
#endif /* ENABLE_IPV6 */
		}
		/*printf("%s => %s\n", lan_addr->ifname, lan_addr->str);*/
	}
	else
	{
		if(n>15)
			goto parselan_error;
		memcpy(lan_addr->str, str, n);
		lan_addr->str[n] = '\0';
		if(!inet_aton(lan_addr->str, &lan_addr->addr))
			goto parselan_error;
	}
	if(!addr_is_reserved(&lan_addr->addr)) {
		INIT_PRINT_ERR("Error: LAN address contains public IP address : %s\n", lan_addr->str);
		INIT_PRINT_ERR("Public IP address can be configured via ext_ip= option\n");
		INIT_PRINT_ERR("LAN address should contain private address, e.g. from 192.168. block\n");
		INIT_PRINT_ERR("Listening on public IP address is a security issue\n");
		return -1;
	}
	if(*p == '/')
	{
		const char * q = ++p;
		while(*p && isdigit(*p))
			p++;
		if(*p=='.')
		{
			/* parse mask in /255.255.255.0 format */
			while(*p && (*p=='.' || isdigit(*p)))
				p++;
			n = p - q;
			if(n >= sizeof(tmp))
				goto parselan_error;
			memcpy(tmp, q, n);
			tmp[n] = '\0';
			if(!inet_aton(tmp, &lan_addr->mask))
				goto parselan_error;
		}
		else
		{
			/* it is a /24 format */
			int nbits = atoi(q);
			if(nbits > 32 || nbits < 0)
				goto parselan_error;
			lan_addr->mask.s_addr = htonl(nbits ? (0xffffffffu << (32 - nbits)) : 0);
		}
	}
	else if(lan_addr->mask.s_addr == 0)
	{
		/* by default, networks are /24 */
		lan_addr->mask.s_addr = htonl(0xffffff00u);
	}
#ifdef MULTIPLE_EXTERNAL_IP
	/* skip spaces */
	while(*p && isspace(*p))
		p++;
	if(*p) {
		/* parse the exteral IP address to associate with this subnet */
		n = 0;
		while(p[n] && !isspace(*p))
			n++;
		if(n<=15) {
			memcpy(lan_addr->ext_ip_str, p, n);
			lan_addr->ext_ip_str[n] = '\0';
			if(!inet_aton(lan_addr->ext_ip_str, &lan_addr->ext_ip_addr)) {
				/* error */
				INIT_PRINT_ERR("Error parsing address : %s\n", lan_addr->ext_ip_str);
				return -1;
			}
			if(0) {
				/* error */
				INIT_PRINT_ERR("Error: option ext_ip address contains reserved / private address : %s\n", lan_addr->ext_ip_str);
				return -1;
			}
		}
	}
#else
	while(*p) {
		/* skip spaces */
		while(*p && isspace(*p))
			p++;
		if(*p) {
			unsigned int index;
			n = 0;
			while(p[n] && !isspace(p[n]) && n < sizeof(tmp)) {
				tmp[n] = p[n];
				n++;
			}
			if(n >= sizeof(tmp)) {
				INIT_PRINT_ERR("Cannot parse '%s'\n", p);
				break;
			}
			tmp[n] = '\0';
			index = if_nametoindex(tmp);
			if(index == 0) {
				fprintf(stderr, "Cannot get index for network interface %s\n",
				        tmp);
				syslog(LOG_WARNING, "Cannot get index for network interface %s\n",
				        tmp);
			} else {
				lan_addr->add_indexes |= (1UL << (index - 1));
			}
			p += n;
		}
	}
#endif
	if(lan_addr->ifname[0] != '\0') {
		lan_addr->index = if_nametoindex(lan_addr->ifname);
		if(lan_addr->index == 0) {
			fprintf(stderr, "Cannot get index for network interface %s\n",
			        lan_addr->ifname);
			syslog(LOG_WARNING, "Cannot get index for network interface %s\n",
			        lan_addr->ifname);
		}
	} else {
#ifdef ENABLE_IPV6
		INIT_PRINT_ERR("Error: please specify LAN network interface by name instead of IPv4 address : %s\n", str);
		return -1;
#else
		syslog(LOG_NOTICE, "it is advised to use network interface name instead of %s", str);
#endif
	}
	return 0;
parselan_error:
	INIT_PRINT_ERR("Error parsing address/mask (or interface name) : %s\n", str);
	return -1;
}

static char ext_addr_str[INET_ADDRSTRLEN];

int update_ext_ip_addr_from_stun(int init)
{
	struct in_addr if_addr, ext_addr;
	int restrictive_nat;
	char if_addr_str[INET_ADDRSTRLEN];

	syslog(LOG_INFO, "STUN: Performing with host=%s and port=%u ...", ext_stun_host, (unsigned)ext_stun_port);

	if (getifaddr(ext_if_name, if_addr_str, INET_ADDRSTRLEN, &if_addr, NULL) < 0) {
		syslog(LOG_ERR, "STUN: Cannot get IP address for ext interface %s", ext_if_name);
		return 1;
	}
	if (perform_stun(ext_if_name, if_addr_str, ext_stun_host, ext_stun_port, &ext_addr, &restrictive_nat) != 0) {
		syslog(LOG_ERR, "STUN: Performing STUN failed: %s", strerror(errno));
		return 1;
	}
	if (!inet_ntop(AF_INET, &ext_addr, ext_addr_str, sizeof(ext_addr_str))) {
		syslog(LOG_ERR, "STUN: Function inet_ntop for IP address returned by STUN failed: %s", strerror(errno));
		return 1;
	}

	if ((init || disable_port_forwarding) && !restrictive_nat) {
		if (addr_is_reserved(&if_addr))
			syslog(LOG_INFO, "STUN: ext interface %s with IP address %s is now behind unrestricted full-cone NAT 1:1 with public IP address %s and firewall does not block incoming connections set by miniupnpd", ext_if_name, if_addr_str, ext_addr_str);
		else
			syslog(LOG_INFO, "STUN: ext interface %s has now public IP address %s and firewall does not block incoming connections set by miniupnpd", ext_if_name, if_addr_str);
		syslog(LOG_INFO, "Port forwarding is now enabled");
	} else if ((init || !disable_port_forwarding) && restrictive_nat) {
		if (addr_is_reserved(&if_addr)) {
			syslog(LOG_WARNING, "STUN: ext interface %s with private IP address %s is now behind restrictive or symmetric NAT with public IP address %s which does not support port forwarding", ext_if_name, if_addr_str, ext_addr_str);
			syslog(LOG_WARNING, "NAT on upstream router blocks incoming connections set by miniupnpd");
			syslog(LOG_WARNING, "Turn off NAT on upstream router or change it to full-cone NAT 1:1 type");
		} else {
			syslog(LOG_WARNING, "STUN: ext interface %s has now public IP address %s but firewall filters incoming connections set by miniunnpd", ext_if_name, if_addr_str);
			syslog(LOG_WARNING, "Check configuration of firewall on local machine and also on upstream router");
		}
		syslog(LOG_WARNING, "Port forwarding is now disabled");
	} else {
		syslog(LOG_INFO, "STUN: ... done");
	}

	use_ext_ip_addr = ext_addr_str;
	disable_port_forwarding = restrictive_nat;
	return 0;
}

/* fill uuidvalue_wan and uuidvalue_wcd based on uuidvalue_igd */
void complete_uuidvalues(void)
{
	size_t len;
	len = strlen(uuidvalue_igd);
	memcpy(uuidvalue_wan, uuidvalue_igd, len+1);
	switch(uuidvalue_wan[len-1]) {
	case '9':
		uuidvalue_wan[len-1] = 'a';
		break;
	case 'f':
		uuidvalue_wan[len-1] = '0';
		break;
	default:
		uuidvalue_wan[len-1]++;
	}
	memcpy(uuidvalue_wcd, uuidvalue_wan, len+1);
	switch(uuidvalue_wcd[len-1]) {
	case '9':
		uuidvalue_wcd[len-1] = 'a';
		break;
	case 'f':
		uuidvalue_wcd[len-1] = '0';
		break;
	default:
		uuidvalue_wcd[len-1]++;
	}
}

/* init phase :
 * 1) read configuration file
 * 2) read command line arguments
 * 3) daemonize
 * 4) open syslog
 * 5) check and write pid file
 * 6) set startup time stamp
 * 7) compute presentation URL
 * 8) set signal handlers
 * 9) init random generator (srandom())
 * 10) init redirection engine
 * 11) reload mapping from leasefile */
static int
init(int argc, char * * argv, struct runtime_vars * v)
{
	int i;
#ifndef NO_BACKGROUND_NO_PIDFILE
	int pid;
#endif
	int debug_flag = 0;
	int verbosity_level = 0;	/* for determining setlogmask() */
	int openlog_option;
	struct in_addr addr;
	struct sigaction sa;
	/*const char * logfilename = 0;*/
	const char * presurl = 0;
#ifndef DISABLE_CONFIG_FILE
	int options_flag = 0;
	const char * optionsfile = DEFAULT_CONFIG;
#endif /* DISABLE_CONFIG_FILE */
	struct lan_addr_s * lan_addr;
	struct lan_addr_s * lan_addr2;

	/* only print usage if -h is used */
	for(i=1; i<argc; i++)
	{
		if(0 == strcmp(argv[i], "-h") || 0 == strcmp(argv[i], "--help"))
			goto print_usage;
		if(0 == strcmp(argv[i], "-d"))
			debug_flag = 1;
	}

	openlog_option = LOG_PID|LOG_CONS;
	if(debug_flag)
	{
		openlog_option |= LOG_PERROR;	/* also log on stderr */
	}

	openlog("miniupnpd", openlog_option, LOG_MINIUPNPD);

#ifndef DISABLE_CONFIG_FILE
	/* first check if "-f" option is used */
	for(i=2; i<argc; i++)
	{
		if(0 == strcmp(argv[i-1], "-f"))
		{
			optionsfile = argv[i];
			options_flag = 1;
			break;
		}
	}
#endif /* DISABLE_CONFIG_FILE */

	/* set initial values */
	SETFLAG(ENABLEUPNPMASK | SECUREMODEMASK);	/* UPnP and secure mode */
#ifdef ENABLE_IPV6
	ipv6_bind_addr = in6addr_any;
#endif /* ENABLE_IPV6 */

	LIST_INIT(&lan_addrs);
	v->port = -1;
#ifdef ENABLE_HTTPS
	v->https_port = -1;
#endif
	v->notify_interval = 900;	/* seconds between SSDP announces */
	v->clean_ruleset_threshold = 20;
	v->clean_ruleset_interval = 0;	/* interval between ruleset check. 0=disabled */
#ifndef DISABLE_CONFIG_FILE
	/* read options file first since
	 * command line arguments have final say */
	if(readoptionsfile(optionsfile, debug_flag) < 0)
	{
		/* only error if file exists or using -f */
		if(access(optionsfile, F_OK) == 0 || options_flag)
		{
			INIT_PRINT_ERR("Error reading configuration file %s\n", optionsfile);
			return 1;
		}
	}
	else
	{
		for(i=0; i<(int)num_options; i++)
		{
			switch(ary_options[i].id)
			{
			case UPNPEXT_IFNAME:
				ext_if_name = ary_options[i].value;
				break;
#ifdef ENABLE_IPV6
			case UPNPEXT_IFNAME6:
				ext_if_name6 = ary_options[i].value;
				break;
#endif
			case UPNPEXT_IP:
				use_ext_ip_addr = ary_options[i].value;
				break;
			case UPNPEXT_PERFORM_STUN:
				if(strcmp(ary_options[i].value, "yes") == 0)
					SETFLAG(PERFORMSTUNMASK);
				break;
			case UPNPEXT_STUN_HOST:
				ext_stun_host = ary_options[i].value;
				break;
			case UPNPEXT_STUN_PORT:
				ext_stun_port = atoi(ary_options[i].value);
				break;
			case UPNPLISTENING_IP:
				lan_addr = (struct lan_addr_s *) malloc(sizeof(struct lan_addr_s));
				if (lan_addr == NULL)
				{
					INIT_PRINT_ERR("malloc(sizeof(struct lan_addr_s)): %m");
					return 1;
				}
				if(parselanaddr(lan_addr, ary_options[i].value, debug_flag) != 0)
				{
					INIT_PRINT_ERR("can't parse \"%s\" as a valid "
#ifndef ENABLE_IPV6
					        "LAN address or "
#endif
					        "interface name\n", ary_options[i].value);
					free(lan_addr);
					return 1;
				}
				LIST_INSERT_HEAD(&lan_addrs, lan_addr, list);
				break;
#ifdef ENABLE_IPV6
			case UPNPIPV6_LISTENING_IP:
				if (inet_pton(AF_INET6, ary_options[i].value, &ipv6_bind_addr) < 1)
				{
					INIT_PRINT_ERR("can't parse \"%s\" as valid IPv6 listening address", ary_options[i].value);
					return 1;
				}
				break;
			case UPNPIPV6_DISABLE:
				if(strcmp(ary_options[i].value, "yes") == 0)
					SETFLAG(IPV6DISABLEDMASK);
				break;
#endif /* ENABLE_IPV6 */
			case UPNPPORT:
				v->port = atoi(ary_options[i].value);
				break;
#ifdef ENABLE_HTTPS
			case UPNPHTTPSPORT:
				v->https_port = atoi(ary_options[i].value);
				break;
#endif
			case UPNPBITRATE_UP:
				upstream_bitrate = strtoul(ary_options[i].value, 0, 0);
				break;
			case UPNPBITRATE_DOWN:
				downstream_bitrate = strtoul(ary_options[i].value, 0, 0);
				break;
			case UPNPPRESENTATIONURL:
				presurl = ary_options[i].value;
				break;
#ifdef ENABLE_MANUFACTURER_INFO_CONFIGURATION
			case UPNPFRIENDLY_NAME:
				strncpy(friendly_name, ary_options[i].value, FRIENDLY_NAME_MAX_LEN);
				friendly_name[FRIENDLY_NAME_MAX_LEN-1] = '\0';
				break;
			case UPNPMANUFACTURER_NAME:
				strncpy(manufacturer_name, ary_options[i].value, MANUFACTURER_NAME_MAX_LEN);
				manufacturer_name[MANUFACTURER_NAME_MAX_LEN-1] = '\0';
				break;
			case UPNPMANUFACTURER_URL:
				strncpy(manufacturer_url, ary_options[i].value, MANUFACTURER_URL_MAX_LEN);
				manufacturer_url[MANUFACTURER_URL_MAX_LEN-1] = '\0';
				break;
			case UPNPMODEL_NAME:
				strncpy(model_name, ary_options[i].value, MODEL_NAME_MAX_LEN);
				model_name[MODEL_NAME_MAX_LEN-1] = '\0';
				break;
			case UPNPMODEL_DESCRIPTION:
				strncpy(model_description, ary_options[i].value, MODEL_DESCRIPTION_MAX_LEN);
				model_description[MODEL_DESCRIPTION_MAX_LEN-1] = '\0';
				break;
			case UPNPMODEL_URL:
				strncpy(model_url, ary_options[i].value, MODEL_URL_MAX_LEN);
				model_url[MODEL_URL_MAX_LEN-1] = '\0';
				break;
#endif	/* ENABLE_MANUFACTURER_INFO_CONFIGURATION */
#ifdef USE_NETFILTER
			case UPNPTABLENAME:
				set_rdr_name(RDR_TABLE_NAME, ary_options[i].value);
				break;
			case UPNPNATTABLENAME:
				set_rdr_name(RDR_NAT_TABLE_NAME, ary_options[i].value);
				break;
			case UPNPFORWARDCHAIN:
				set_rdr_name(RDR_FORWARD_CHAIN_NAME, ary_options[i].value);
				break;
			case UPNPNATCHAIN:
				set_rdr_name(RDR_NAT_PREROUTING_CHAIN_NAME, ary_options[i].value);
				break;
			case UPNPNATPOSTCHAIN:
				set_rdr_name(RDR_NAT_POSTROUTING_CHAIN_NAME, ary_options[i].value);
				break;
			case UPNPNFFAMILYSPLIT:
				set_rdr_name(RDR_FAMILY_SPLIT, ary_options[i].value);
				break;
#endif    /* USE_NETFILTER */
			case UPNPNOTIFY_INTERVAL:
				v->notify_interval = atoi(ary_options[i].value);
				break;
			case UPNPSYSTEM_UPTIME:
				if(strcmp(ary_options[i].value, "yes") == 0)
					SETFLAG(SYSUPTIMEMASK);	/*sysuptime = 1;*/
				break;
#if defined(USE_PF) || defined(USE_IPF)
			case UPNPPACKET_LOG:
				if(strcmp(ary_options[i].value, "yes") == 0)
					SETFLAG(LOGPACKETSMASK);	/*logpackets = 1;*/
				break;
#endif	/* defined(USE_PF) || defined(USE_IPF) */
			case UPNPUUID:
				strncpy(uuidvalue_igd+5, ary_options[i].value,
				        strlen(uuidvalue_igd+5) + 1);
				complete_uuidvalues();
				break;
			case UPNPSERIAL:
				strncpy(serialnumber, ary_options[i].value, SERIALNUMBER_MAX_LEN);
				serialnumber[SERIALNUMBER_MAX_LEN-1] = '\0';
				break;
			case UPNPMODEL_NUMBER:
				strncpy(modelnumber, ary_options[i].value, MODELNUMBER_MAX_LEN);
				modelnumber[MODELNUMBER_MAX_LEN-1] = '\0';
				break;
			case UPNPCLEANTHRESHOLD:
				v->clean_ruleset_threshold = atoi(ary_options[i].value);
				break;
			case UPNPCLEANINTERVAL:
				v->clean_ruleset_interval = atoi(ary_options[i].value);
				break;
#ifdef USE_PF
			case UPNPANCHOR:
				anchor_name = ary_options[i].value;
				break;
			case UPNPQUEUE:
				queue = ary_options[i].value;
				break;
			case UPNPTAG:
				tag = ary_options[i].value;
				break;
#endif	/* USE_PF */
#ifdef ENABLE_NATPMP
			/* enable both NAT-PMP and PCP (if enabled) */
			case UPNPENABLENATPMP:
				if(strcmp(ary_options[i].value, "yes") == 0)
					SETFLAG(ENABLENATPMPMASK);
				else
					if(atoi(ary_options[i].value))
						SETFLAG(ENABLENATPMPMASK);
				break;
#endif	/* ENABLE_NATPMP */
#ifdef ENABLE_PCP
			case UPNPPCPMINLIFETIME:
					min_lifetime = atoi(ary_options[i].value);
					/* RFC6887 15. the minimum value SHOULD be 120 seconds */
					if (min_lifetime < 120 ) {
						min_lifetime = 120;
					}
				break;
			case UPNPPCPMAXLIFETIME:
					max_lifetime = atoi(ary_options[i].value);
					/* maximum is 24 hours */
					if (max_lifetime > 86400 ) {
						max_lifetime = 86400;
					}
				break;
			case UPNPPCPALLOWTHIRDPARTY:
				if(strcmp(ary_options[i].value, "yes") == 0)
					SETFLAG(PCP_ALLOWTHIRDPARTYMASK);
				break;
#endif	/* ENABLE_PCP */
#ifdef PF_ENABLE_FILTER_RULES
			case UPNPQUICKRULES:
				if(strcmp(ary_options[i].value, "no") == 0)
					SETFLAG(PFNOQUICKRULESMASK);
				break;
#endif	/* PF_ENABLE_FILTER_RULES */
			case UPNPENABLE:
				if(strcmp(ary_options[i].value, "yes") != 0)
					CLEARFLAG(ENABLEUPNPMASK);
				break;
			case UPNPSECUREMODE:
				if (strcmp(ary_options[i].value, "no") == 0)
					CLEARFLAG(SECUREMODEMASK);
				break;
#ifdef ENABLE_LEASEFILE
			case UPNPLEASEFILE:
				lease_file = ary_options[i].value;
				break;
#ifdef ENABLE_UPNPPINHOLE
			case UPNPLEASEFILE6:
				lease_file6 = ary_options[i].value;
				break;
#endif	/* ENABLE_UPNPPINHOLE */
#endif	/* ENABLE_LEASEFILE */
			case UPNPMINISSDPDSOCKET:
				minissdpdsocketpath = ary_options[i].value;
				break;
#ifdef IGD_V2
			case UPNPFORCEIGDDESCV1:
				if (strcmp(ary_options[i].value, "yes") == 0)
					SETFLAG(FORCEIGDDESCV1MASK);
				else if (strcmp(ary_options[i].value, "no") != 0 ) {
					INIT_PRINT_ERR("force_igd_desc_v1 can only be yes or no\n");
					return 1;
				}
				break;
#endif
			default:
				INIT_PRINT_ERR("Unknown option in file %s\n",
				        optionsfile);
			}
		}
#ifdef ENABLE_PCP
		/* if lifetimes are inverse */
		if (min_lifetime >= max_lifetime) {
			INIT_PRINT_ERR("Minimum lifetime (%lu) is greater than or equal to maximum lifetime (%lu).\n", min_lifetime, max_lifetime);
			INIT_PRINT_ERR("Check your configuration file.\n");
			return 1;
		}
#endif	/* ENABLE_PCP */
		if (GETFLAG(PERFORMSTUNMASK) && !ext_stun_host) {
			INIT_PRINT_ERR("You must specify ext_stun_host= when ext_perform_stun=yes\n");
			return 1;
		}
	}
#endif /* DISABLE_CONFIG_FILE */

	/* command line arguments processing */
	for(i=1; i<argc; i++)
	{
		if(argv[i][0]!='-')
		{
			INIT_PRINT_ERR("Unknown option: %s\n", argv[i]);
			goto print_usage;
		}
		else switch(argv[i][1])
		{
		case 'v':
			{
				int j;
				for (j = 1; argv[i][j] != '\0'; j++)
					verbosity_level++;
			}
			break;
#ifdef ENABLE_IPV6
		case '4':
			SETFLAG(IPV6DISABLEDMASK);
			break;
#endif
#ifdef IGD_V2
		case '1':
			SETFLAG(FORCEIGDDESCV1MASK);
			break;
#endif
		case 'b':
			if(i+1 < argc) {
				upnp_bootid = (unsigned int)strtoul(argv[++i], NULL, 10);
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			break;
		case 'o':
			if(i+1 < argc) {
				i++;
				if (0 == strncasecmp(argv[i], "STUN:", 5)) {
					char *ptr;
					SETFLAG(PERFORMSTUNMASK);
					ext_stun_host = argv[i] + 5;
					ptr = strchr(ext_stun_host, ':');
					if (ptr) {
						ext_stun_port = atoi(ptr+1);
						*ptr = 0;
					}
				} else
					use_ext_ip_addr = argv[i];
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			break;
		case 't':
			if(i+1 < argc) {
				v->notify_interval = atoi(argv[++i]);
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			break;
		case 'r':
			if(i+1 < argc) {
				v->clean_ruleset_interval = atoi(argv[++i]);
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			break;
		case 'u':
			if(i+1 < argc) {
				strncpy(uuidvalue_igd+5, argv[++i], strlen(uuidvalue_igd+5) + 1);
				complete_uuidvalues();
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			break;
#ifdef ENABLE_MANUFACTURER_INFO_CONFIGURATION
		case 'z':
			if(i+1 < argc) {
				strncpy(friendly_name, argv[++i], FRIENDLY_NAME_MAX_LEN);
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			friendly_name[FRIENDLY_NAME_MAX_LEN-1] = '\0';
			break;
#endif	/* ENABLE_MANUFACTURER_INFO_CONFIGURATION */
		case 's':
			if(i+1 < argc) {
				strncpy(serialnumber, argv[++i], SERIALNUMBER_MAX_LEN);
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			serialnumber[SERIALNUMBER_MAX_LEN-1] = '\0';
			break;
		case 'm':
			if(i+1 < argc) {
				strncpy(modelnumber, argv[++i], MODELNUMBER_MAX_LEN);
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			modelnumber[MODELNUMBER_MAX_LEN-1] = '\0';
			break;
#ifdef ENABLE_NATPMP
		case 'N':
			/* enable both NAT-PMP and PCP (if enabled) */
			SETFLAG(ENABLENATPMPMASK);
			break;
#endif	/* ENABLE_NATPMP */
		case 'U':
			/*sysuptime = 1;*/
			SETFLAG(SYSUPTIMEMASK);
			break;
		/*case 'l':
			logfilename = argv[++i];
			break;*/
#if defined(USE_PF) || defined(USE_IPF)
		case 'L':
			/*logpackets = 1;*/
			SETFLAG(LOGPACKETSMASK);
			break;
#endif	/* defined(USE_PF) || defined(USE_IPF) */
		case 'S':
			/* -S0 to disable secure mode, for backward compatibility
			 * -S is ignored */
			if (argv[i][2] == '0') {
				CLEARFLAG(SECUREMODEMASK);
			} else if (argv[i][2] != '\0') {
				INIT_PRINT_ERR("Uses -S0 to disable secure mode.\n");
				goto print_usage;
			}
			break;
		case 'i':
			if(i+1 < argc) {
				ext_if_name = argv[++i];
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			break;
#ifdef ENABLE_IPV6
		case 'I':
			if(i+1 < argc) {
				ext_if_name6 = argv[++i];
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			break;
#endif
#ifdef USE_PF
		case 'q':
			if(i+1 < argc) {
				queue = argv[++i];
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			break;
		case 'T':
			if(i+1 < argc) {
				tag = argv[++i];
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			break;
#endif	/* USE_PF */
		case 'p':
			if(i+1 < argc) {
				v->port = atoi(argv[++i]);
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			break;
#ifdef ENABLE_HTTPS
		case 'H':
			if(i+1 < argc) {
				v->https_port = atoi(argv[++i]);
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			break;
#endif	/* ENABLE_HTTPS */
#ifdef ENABLE_NFQUEUE
		case 'Q':
			if(i+1<argc) {
				nfqueue = atoi(argv[++i]);
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			break;
		case 'n':
			if (i+1 < argc) {
				i++;
				if(n_nfqix < MAX_LAN_ADDR) {
					nfqix[n_nfqix++] = if_nametoindex(argv[i]);
				} else {
					INIT_PRINT_ERR( "Too many nfq interfaces. Ignoring %s\n", argv[i]);
				}
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			break;
#endif	/* ENABLE_NFQUEUE */
#ifndef NO_BACKGROUND_NO_PIDFILE
		case 'P':
			if(i+1 < argc)
				pidfilename = argv[++i];
			else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			break;
#endif
		case 'd':	/* discarding */
			break;
		case 'w':
			if(i+1 < argc)
				presurl = argv[++i];
			else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			break;
		case 'B':
			if(i+2<argc) {
				downstream_bitrate = strtoul(argv[++i], 0, 0);
				upstream_bitrate = strtoul(argv[++i], 0, 0);
			} else {
				INIT_PRINT_ERR("Option -%c takes two argument.\n", argv[i][1]);
				goto print_usage;
			}
			break;
		case 'a':
#ifndef MULTIPLE_EXTERNAL_IP
			if(i+1 < argc)
			{
				i++;
				lan_addr = (struct lan_addr_s *) malloc(sizeof(struct lan_addr_s));
				if (lan_addr == NULL)
				{
					INIT_PRINT_ERR("malloc(sizeof(struct lan_addr_s)): %m");
					return 1;
				}
				if(parselanaddr(lan_addr, argv[i], debug_flag) != 0)
				{
					INIT_PRINT_ERR("can't parse \"%s\" as a valid "
#ifndef ENABLE_IPV6
					        "LAN address or "
#endif	/* #ifndef ENABLE_IPV6 */
					        "interface name\n", argv[i]);
					free(lan_addr);
					return 1;
				}
				/* check if we already have this address */
				for(lan_addr2 = lan_addrs.lh_first; lan_addr2 != NULL; lan_addr2 = lan_addr2->list.le_next)
				{
					if (0 == strncmp(lan_addr2->str, lan_addr->str, 15))
						break;
				}
				if (lan_addr2 == NULL)
					LIST_INSERT_HEAD(&lan_addrs, lan_addr, list);
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
#else	/* #ifndef MULTIPLE_EXTERNAL_IP */
			if(i+2 < argc)
			{
				char *val = calloc((strlen(argv[i+1]) + strlen(argv[i+2]) + 2), sizeof(char));
				if (val == NULL)
				{
					INIT_PRINT_ERR("memory allocation error for listen address storage\n");
					return 1;
				}
				sprintf(val, "%s %s", argv[i+1], argv[i+2]);

				lan_addr = (struct lan_addr_s *) malloc(sizeof(struct lan_addr_s));
				if (lan_addr == NULL)
				{
					INIT_PRINT_ERR("malloc(sizeof(struct lan_addr_s)): %m");
					free(val);
					return 1;
				}
				if(parselanaddr(lan_addr, val, debug_flag) != 0)
				{
					INIT_PRINT_ERR("can't parse \"%s\" as a valid LAN address or interface name\n", val);
					free(lan_addr);
					free(val);
					return 1;
				}
				/* check if we already have this address */
				for(lan_addr2 = lan_addrs.lh_first; lan_addr2 != NULL; lan_addr2 = lan_addr2->list.le_next)
				{
					if (0 == strncmp(lan_addr2->str, lan_addr->str, 15))
						break;
				}
				if (lan_addr2 == NULL)
					LIST_INSERT_HEAD(&lan_addrs, lan_addr, list);

				free(val);
				i+=2;
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
#endif 	/* #ifndef MULTIPLE_EXTERNAL_IP */
			break;
		case 'A':
			if(i+1 < argc) {
				void * tmp;
				tmp = realloc(upnppermlist, sizeof(struct upnpperm) * (num_upnpperm+1));
				if(tmp == NULL) {
					INIT_PRINT_ERR("memory allocation error for permission\n");
					return 1;
				} else {
					upnppermlist = tmp;
					if(read_permission_line(upnppermlist + num_upnpperm, argv[++i]) >= 0) {
						num_upnpperm++;
					} else {
						INIT_PRINT_ERR("Permission rule parsing error :\n%s\n", argv[i]);
						return 1;
					}
				}
			} else {
				INIT_PRINT_ERR("Option -%c takes one argument.\n", argv[i][1]);
				goto print_usage;
			}
			break;
		case 'f':
			i++;	/* discarding, the config file is already read */
			break;
		default:
			INIT_PRINT_ERR("Unknown option: %s\n", argv[i]);
			goto print_usage;
		}
	}
	if(!ext_if_name || !lan_addrs.lh_first) {
		/* bad configuration */
		if(!ext_if_name)
		    INIT_PRINT_ERR("Error: Option -i missing and ext_ifname is not set in config file\n");
		if (!lan_addrs.lh_first)
		    INIT_PRINT_ERR("Error: Option -a missing and listening_ip is not set in config file\n");
		goto print_usage;
	}

	/* IPv6 ifname is defaulted to same as IPv4 */
#ifdef ENABLE_IPV6
	if(!ext_if_name6)
		ext_if_name6 = ext_if_name;
#endif

	if (use_ext_ip_addr && GETFLAG(PERFORMSTUNMASK)) {
		INIT_PRINT_ERR("Error: options ext_ip= and ext_perform_stun=yes cannot be specified together\n");
		return 1;
	}

	if (use_ext_ip_addr) {
		if (inet_pton(AF_INET, use_ext_ip_addr, &addr) != 1) {
			INIT_PRINT_ERR("Error: option ext_ip contains invalid address %s\n", use_ext_ip_addr);
			return 1;
		}
		if (0) {
			INIT_PRINT_ERR("Error: option ext_ip contains reserved / private address %s, not public routable\n", use_ext_ip_addr);
			return 1;
		}
	}

#ifndef NO_BACKGROUND_NO_PIDFILE
	if(debug_flag)
	{
		pid = getpid();
	}
	else
	{
#ifdef USE_DAEMON
		if(daemon(0, 0)<0) {
			perror("daemon()");
		}
		pid = getpid();
#else
		pid = daemonize();
#endif
	}
#endif

	if(!debug_flag)
	{
		switch (verbosity_level)
		{
		case 0:
			/* speed things up and ignore LOG_INFO and LOG_DEBUG */
			setlogmask(LOG_UPTO(LOG_NOTICE));
			break;
		case 1:
			/* ignore LOG_DEBUG */
			setlogmask(LOG_UPTO(LOG_INFO));
			break;
		case 2:
			setlogmask(LOG_UPTO(LOG_DEBUG));
		}
	}

#ifndef NO_BACKGROUND_NO_PIDFILE
	if(checkforrunning(pidfilename) < 0)
	{
		syslog(LOG_ERR, "MiniUPnPd is already running. EXITING");
		return 1;
	}
#endif

#ifdef TOMATO
	syslog(LOG_NOTICE, "version " MINIUPNPD_VERSION " started");
#endif /* TOMATO */

	set_startup_time();

	/* presentation url */
	if(presurl)
	{
		strncpy(presentationurl, presurl, PRESENTATIONURL_MAX_LEN);
		presentationurl[PRESENTATIONURL_MAX_LEN-1] = '\0';
	}
	else
	{
		snprintf(presentationurl, PRESENTATIONURL_MAX_LEN,
		         "http://%s/", lan_addrs.lh_first->str);
		         /*"http://%s:%d/", lan_addrs.lh_first->str, 80);*/
	}

	/* set signal handler */
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = sigterm;

	if(sigaction(SIGTERM, &sa, NULL) < 0)
	{
		syslog(LOG_ERR, "Failed to set %s handler. EXITING", "SIGTERM");
		return 1;
	}
	if(sigaction(SIGINT, &sa, NULL) < 0)
	{
		syslog(LOG_ERR, "Failed to set %s handler. EXITING", "SIGINT");
		return 1;
	}
#ifdef TOMATO
	sa.sa_handler = sigusr2;
	sigaction(SIGUSR2, &sa, NULL);
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
#else	/* TOMATO */
	sa.sa_handler = SIG_IGN;
	if(sigaction(SIGPIPE, &sa, NULL) < 0)
#endif	/* TOMATO */
	{
		syslog(LOG_ERR, "Failed to ignore SIGPIPE signals");
	}
	sa.sa_handler = sigusr1;
	if(sigaction(SIGUSR1, &sa, NULL) < 0)
	{
		syslog(LOG_NOTICE, "Failed to set %s handler", "SIGUSR1");
	}
#if !defined(TOMATO) && defined(ENABLE_LEASEFILE) && defined(LEASEFILE_USE_REMAINING_TIME)
	sa.sa_handler = sigusr2;
	if(sigaction(SIGUSR2, &sa, NULL) < 0)
	{
		syslog(LOG_NOTICE, "Failed to set %s handler", "SIGUSR2");
	}
#endif /* !TOMATO && ENABLE_LEASEFILE && LEASEFILE_USE_REMAINING_TIME */

	/* initialize random number generator */
	srandom((unsigned int)time(NULL));
#ifdef RANDOMIZE_URLS
	snprintf(random_url, RANDOM_URL_MAX_LEN, "%08lx", random());
#endif /* RANDOMIZE_URLS */

	/* initialize redirection engine (and pinholes) */
	if(init_redirect() < 0)
	{
		syslog(LOG_ERR, "Failed to init redirection engine. EXITING");
		return 1;
	}
#ifdef ENABLE_UPNPPINHOLE
#ifdef USE_NETFILTER
	init_iptpinhole();
#endif
#endif

#ifndef NO_BACKGROUND_NO_PIDFILE
	if(writepidfile(pidfilename, pid) < 0)
		pidfilename = NULL;
#endif

#ifdef ENABLE_LEASEFILE
	/*remove(lease_file);*/
	syslog(LOG_INFO, "Reloading rules from lease file");
	reload_from_lease_file();
#ifdef ENABLE_UPNPPINHOLE
	reload_from_lease_file6();
#endif
#endif

#ifdef TOMATO
	tomato_load();
#endif /* TOMATO */

	return 0;
print_usage:
	fprintf(stderr, "Usage:\n\t"
	        "%s --version\n\t"
	        "%s --help\n\t"
	        "%s "
#ifndef DISABLE_CONFIG_FILE
			"[-f config_file] "
#endif
			"[-i ext_ifname] "
#ifdef ENABLE_IPV6
			"[-I ext_ifname6] [-4] "
#endif
			"[-o ext_ip]\n"
#ifndef MULTIPLE_EXTERNAL_IP
			"\t\t[-a listening_ip]"
#else
			"\t\t[-a listening_ip ext_ip]"
#endif
#ifdef ENABLE_HTTPS
			" [-H https_port]"
#endif
			" [-p port] [-d] [-v]"
#if defined(USE_PF) || defined(USE_IPF)
			" [-L]"
#endif
			" [-U] [-S0]"
#ifdef ENABLE_NATPMP
			" [-N]"
#endif
			"\n"
			/*"[-l logfile] " not functionnal */
			"\t\t[-u uuid] [-s serial] [-m model_number] \n"
			"\t\t[-t notify_interval] "
#ifndef NO_BACKGROUND_NO_PIDFILE
			"[-P pid_filename] "
#endif
#ifdef ENABLE_MANUFACTURER_INFO_CONFIGURATION
			"[-z fiendly_name]"
#endif
			"\n\t\t[-B down up] [-w url] [-r clean_ruleset_interval]\n"
#ifdef USE_PF
                        "\t\t[-q queue] [-T tag]\n"
#endif
#ifdef ENABLE_NFQUEUE
                        "\t\t[-Q queue] [-n name]\n"
#endif
			"\t\t[-A \"permission rule\"] [-b BOOTID]"
#ifdef IGD_V2
			" [-1]"
#endif
			"\n"
	        "\nNotes:\n\tThere can be one or several listening_ips.\n"
	        "\tNotify interval is in seconds. Default is 900 seconds.\n"
#ifndef NO_BACKGROUND_NO_PIDFILE
			"\tDefault pid file is '%s'.\n"
#endif
			"\tDefault config file is '%s'.\n"
			"\tWith -d miniupnpd will run as a standard program.\n"
			"\t-o argument is either an IPv4 address or \"STUN:host[:port]\".\n"
#ifdef ENABLE_IPV6
			"\t-4 disable IPv6\n"
#endif
#if defined(USE_PF) || defined(USE_IPF)
			"\t-L sets packet log in pf and ipf on.\n"
#endif
			"\t-S0 disable \"secure\" mode so clients can add mappings to other ips\n"
			"\t-U causes miniupnpd to report system uptime instead "
			"of daemon uptime.\n"
#ifdef ENABLE_NATPMP
#ifdef ENABLE_PCP
			"\t-N enables NAT-PMP and PCP functionality.\n"
#else
			"\t-N enables NAT-PMP functionality.\n"
#endif
#endif
			"\t-B sets bitrates reported by daemon in bits per second.\n"
			"\t-w sets the presentation url. Default is http address on port 80\n"
#ifdef USE_PF
			"\t-q sets the ALTQ queue in pf.\n"
			"\t-T sets the tag name in pf.\n"
#endif
#ifdef ENABLE_NFQUEUE
			"\t-Q sets the queue number that is used by NFQUEUE.\n"
			"\t-n sets the name of the interface(s) that packets will arrive on.\n"
#endif
			"\t-A use following syntax for permission rules :\n"
			"\t  (allow|deny) (external port range) ip/mask (internal port range)\n"
			"\texamples :\n"
			"\t  \"allow 1024-65535 192.168.1.0/24 1024-65535\"\n"
			"\t  \"deny 0-65535 0.0.0.0/0 0-65535\"\n"
			"\t-b sets the value of BOOTID.UPNP.ORG SSDP header\n"
#ifdef IGD_V2
			"\t-1 force reporting IGDv1 in rootDesc *use with care*\n"
#endif
			"\t-v enables LOG_INFO messages, -vv LOG_DEBUG as well (default with -d)\n"
			"\t-h / --help prints this help and quits.\n"
	        "", argv[0], argv[0], argv[0],
#ifndef NO_BACKGROUND_NO_PIDFILE
			pidfilename,
#endif
			DEFAULT_CONFIG);
	return 1;
}

/* === main === */
/* process HTTP or SSDP requests */
int
main(int argc, char * * argv)
{
	int i;
	int shttpl = -1;	/* socket for HTTP */
#if defined(V6SOCKETS_ARE_V6ONLY) && defined(ENABLE_IPV6)
	int shttpl_v4 = -1;	/* socket for HTTP (ipv4 only) */
#endif
#ifdef ENABLE_HTTPS
	int shttpsl = -1;	/* socket for HTTPS */
#if defined(V6SOCKETS_ARE_V6ONLY) && defined(ENABLE_IPV6)
	int shttpsl_v4 = -1;	/* socket for HTTPS (ipv4 only) */
#endif
#endif /* ENABLE_HTTPS */
	int sudp = -1;		/* IP v4 socket for receiving SSDP */
#ifdef ENABLE_IPV6
	int sudpv6 = -1;	/* IP v6 socket for receiving SSDP */
#endif
#ifdef ENABLE_NATPMP
	int * snatpmp = NULL;	/* also used for PCP */
#endif
#if defined(ENABLE_IPV6) && defined(ENABLE_PCP)
	int spcp_v6 = -1;
#endif
#ifdef ENABLE_NFQUEUE
	int nfqh = -1;
#endif
#ifdef USE_IFACEWATCHER
	int sifacewatcher = -1;
#endif

	int * snotify = NULL;
	int addr_count;
	LIST_HEAD(httplisthead, upnphttp) upnphttphead;
	struct upnphttp * e = 0;
	struct upnphttp * next;
	fd_set readset;	/* for select() */
	fd_set writeset;
	struct timeval timeout, timeofday, lasttimeofday = {0, 0};
	int current_notify_interval;	/* with random variation */
	int max_fd = -1;
#ifdef USE_MINIUPNPDCTL
	int sctl = -1;
	LIST_HEAD(ctlstructhead, ctlelem) ctllisthead;
	struct ctlelem * ectl;
	struct ctlelem * ectlnext;
#endif
	struct runtime_vars v;
	/* variables used for the unused-rule cleanup process */
	struct rule_state * rule_list = 0;
	struct timeval checktime = {0, 0};
	struct lan_addr_s * lan_addr;
#ifdef ENABLE_UPNPPINHOLE
	unsigned int next_pinhole_ts;
#endif

	for(i = 0; i < argc; i++) {
		if(strcmp(argv[i], "version") == 0 || strcmp(argv[i], "--version") == 0) {
			puts("miniupnpd " MINIUPNPD_VERSION
#ifdef MINIUPNPD_GIT_REF
			     " " MINIUPNPD_GIT_REF
#endif
			     " " __DATE__ );
#ifdef USE_PF
			puts("using pf backend");
#endif
#ifdef USE_IPF
			puts("using ipf backend");
#endif
#ifdef USE_IPFW
			puts("using ipfw backend");
#endif
#ifdef USE_IPTABLES
			puts("using netfilter(iptables) backend");
#endif
#ifdef USE_NFTABLES
			puts("using netfilter(nftables) backend");
#endif
#ifdef ENABLE_HTTPS
#ifdef OPENSSL_VERSION
			puts(OpenSSL_version(OPENSSL_VERSION));
#else
			puts(SSLeay_version(SSLEAY_VERSION));
#endif
#endif
			puts("build options:"
#ifdef USE_MINIUPNPDCTL
				" miniupnpdctl"
#endif
#ifdef ENABLE_IPV6
				" ipv6"
#endif
#ifdef UPNP_STRICT
				" strict"
#endif
#ifdef ENABLE_NATPMP
				" NAT-PMP"
#endif
#ifdef ENABLE_PCP
				" PCP"
#ifdef PCP_PEER
				" PCP-PEER"
#endif
#ifdef PCP_FLOWP
				" PCP-FLOWP"
#endif
#ifdef PCP_SADSCP
				" PCP-SADSCP"
#endif
#endif /* ENABLE_PCP */
#ifdef ENABLE_LEASEFILE
				" leasefile"
#endif
#ifdef CHECK_PORTINUSE
				" check_portinuse"
#endif
#ifdef IGD_V2
				" igdv2"
#endif
			);
			return 0;
		}
	}
	memset(&v, 0, sizeof(v));
	if(init(argc, argv, &v) != 0)
		return 1;
	current_notify_interval = gen_current_notify_interval(v.notify_interval);
#ifdef ENABLE_HTTPS
	if(init_ssl() < 0)
		return 1;
#endif /* ENABLE_HTTPS */
	/* count lan addrs */
	addr_count = 0;
	for(lan_addr = lan_addrs.lh_first; lan_addr != NULL; lan_addr = lan_addr->list.le_next)
		addr_count++;
	if(addr_count > 0) {
#ifndef ENABLE_IPV6
		snotify = calloc(addr_count, sizeof(int));
#else
		/* one for IPv4, one for IPv6 */
		snotify = calloc(addr_count * 2, sizeof(int));
#endif
	}
#ifdef ENABLE_NATPMP
	if(addr_count > 0) {
		snatpmp = malloc(addr_count * sizeof(int));
		for(i = 0; i < addr_count; i++)
			snatpmp[i] = -1;
	}
#endif

	LIST_INIT(&upnphttphead);
#ifdef USE_MINIUPNPDCTL
	LIST_INIT(&ctllisthead);
#endif

	if(
#ifdef ENABLE_NATPMP
	   !GETFLAG(ENABLENATPMPMASK) && !GETFLAG(ENABLEUPNPMASK)
#else
	   !GETFLAG(ENABLEUPNPMASK)
#endif
	   ) {
		syslog(LOG_ERR, "Why did you run me anyway?");
		return 0;
	}

	syslog(LOG_INFO, "version " MINIUPNPD_VERSION " starting%s%sext if %s BOOTID=%u",
#ifdef ENABLE_NATPMP
#ifdef ENABLE_PCP
	       GETFLAG(ENABLENATPMPMASK) ? " NAT-PMP/PCP " : " ",
#else
	       GETFLAG(ENABLENATPMPMASK) ? " NAT-PMP " : " ",
#endif
#else
	       " ",
#endif
	       GETFLAG(ENABLEUPNPMASK) ? "UPnP-IGD " : "",
	       ext_if_name, upnp_bootid);
#ifdef ENABLE_IPV6
	if (ext_if_name6 != ext_if_name) {
		syslog(LOG_INFO, "specific IPv6 ext if %s", ext_if_name6);
	}
#endif

	if(GETFLAG(PERFORMSTUNMASK))
	{
		if (update_ext_ip_addr_from_stun(1) != 0) {
			syslog(LOG_ERR, "Performing STUN failed. EXITING");
			return 1;
		}
	}
	else if (!use_ext_ip_addr)
	{
		char if_addr[INET_ADDRSTRLEN];
		struct in_addr addr;
		if (getifaddr(ext_if_name, if_addr, INET_ADDRSTRLEN, &addr, NULL) < 0) {
			syslog(LOG_WARNING, "Cannot get IP address for ext interface %s. Network is down", ext_if_name);
			disable_port_forwarding = 1;
		} else if (0) {
			syslog(LOG_INFO, "Reserved / private IP address %s on ext interface %s: Port forwarding is impossible", if_addr, ext_if_name);
			syslog(LOG_INFO, "You are probably behind NAT, enable option ext_perform_stun=yes to detect public IP address");
			syslog(LOG_INFO, "Or use ext_ip= / -o option to declare public IP address");
			syslog(LOG_INFO, "Public IP address is required by UPnP/PCP/PMP protocols and clients do not work without it");
			disable_port_forwarding = 1;
		}
	}

#ifdef DYNAMIC_OS_VERSION
	{
		struct utsname utsname;
		if (uname(&utsname) < 0) {
			syslog(LOG_ERR, "uname(): %m");
			os_version = strdup("unknown");
		} else {
			os_version = strdup(utsname.release);
		}
	}
#endif /* DYNAMIC_OS_VERSION */

	if(GETFLAG(ENABLEUPNPMASK))
	{
		unsigned short listen_port;
		listen_port = (v.port > 0) ? v.port : 0;
		/* open socket for HTTP connections. Listen on the 1st LAN address */
#ifdef ENABLE_IPV6
		shttpl = OpenAndConfHTTPSocket(&listen_port, !GETFLAG(IPV6DISABLEDMASK));
#else /* ENABLE_IPV6 */
		shttpl = OpenAndConfHTTPSocket(&listen_port);
#endif /* ENABLE_IPV6 */
		if(shttpl < 0)
		{
			syslog(LOG_ERR, "Failed to open socket for HTTP. EXITING");
			return 1;
		}
		v.port = listen_port;
		syslog(LOG_NOTICE, "HTTP listening on port %d", v.port);
#if defined(V6SOCKETS_ARE_V6ONLY) && defined(ENABLE_IPV6)
		if(!GETFLAG(IPV6DISABLEDMASK))
		{
			shttpl_v4 =  OpenAndConfHTTPSocket(&listen_port, 0);
			if(shttpl_v4 < 0)
			{
				syslog(LOG_ERR, "Failed to open socket for HTTP on port %d (IPv4). EXITING", v.port);
				return 1;
			}
		}
#endif /* V6SOCKETS_ARE_V6ONLY */
#ifdef ENABLE_HTTPS
		/* https */
		listen_port = (v.https_port > 0) ? v.https_port : 0;
#ifdef ENABLE_IPV6
		shttpsl = OpenAndConfHTTPSocket(&listen_port, !GETFLAG(IPV6DISABLEDMASK));
#else /* ENABLE_IPV6 */
		shttpsl = OpenAndConfHTTPSocket(&listen_port);
#endif /* ENABLE_IPV6 */
		if(shttpsl < 0)
		{
			syslog(LOG_ERR, "Failed to open socket for HTTPS. EXITING");
			return 1;
		}
		v.https_port = listen_port;
		syslog(LOG_NOTICE, "HTTPS listening on port %d", v.https_port);
#if defined(V6SOCKETS_ARE_V6ONLY) && defined(ENABLE_IPV6)
		shttpsl_v4 =  OpenAndConfHTTPSocket(&listen_port, 0);
		if(shttpsl_v4 < 0)
		{
			syslog(LOG_ERR, "Failed to open socket for HTTPS on port %d (IPv4). EXITING", v.https_port);
			return 1;
		}
#endif /* V6SOCKETS_ARE_V6ONLY */
#endif /* ENABLE_HTTPS */
#ifdef ENABLE_IPV6
		if(!GETFLAG(IPV6DISABLEDMASK)) {
			if(find_ipv6_addr(lan_addrs.lh_first ? lan_addrs.lh_first->ifname : NULL,
			                  ipv6_addr_for_http_with_brackets, sizeof(ipv6_addr_for_http_with_brackets)) > 0) {
				syslog(LOG_NOTICE, "HTTP IPv6 address given to control points : %s",
				       ipv6_addr_for_http_with_brackets);
			} else {
				memcpy(ipv6_addr_for_http_with_brackets, "[::1]", 6);
				syslog(LOG_WARNING, "no HTTP IPv6 address, disabling IPv6");
				SETFLAG(IPV6DISABLEDMASK);
			}
		}
#endif	/* ENABLE_IPV6 */

		/* open socket for SSDP connections */
		sudp = OpenAndConfSSDPReceiveSocket(0);
		if(sudp < 0)
		{
			syslog(LOG_NOTICE, "Failed to open socket for receiving SSDP. Trying to use MiniSSDPd");
			if(SubmitServicesToMiniSSDPD(lan_addrs.lh_first->str, v.port) < 0) {
				syslog(LOG_ERR, "Failed to connect to MiniSSDPd. EXITING");
				return 1;
			}
		}
#ifdef ENABLE_IPV6
		if(!GETFLAG(IPV6DISABLEDMASK))
		{
			sudpv6 = OpenAndConfSSDPReceiveSocket(1);
			if(sudpv6 < 0)
			{
				syslog(LOG_WARNING, "Failed to open socket for receiving SSDP (IP v6).");
			}
		}
#endif

		/* open socket for sending notifications */
		if(OpenAndConfSSDPNotifySockets(snotify) < 0)
		{
			syslog(LOG_ERR, "Failed to open sockets for sending SSDP notify "
		                "messages. EXITING");
			return 1;
		}

#if defined(UPNP_STRICT) && defined(IGD_V2)
		/* WANIPConnection:2 Service p9 :
		 * Upon startup, UPnP IGD DCP MUST broadcast an ssdp:byebye before
		 * sending the initial ssdp:alive onto the local network. Sending an
		 * ssdp:byebye as part of the normal start up process for a UPnP
		 * device ensures that UPnP control points with information about the
		 * previous device instance will safely discard state information
		 * about the previous device instance before communicating with the
		 * new device instance. */
		if (GETFLAG(ENABLEUPNPMASK))
		{
#ifndef ENABLE_IPV6
			if(SendSSDPGoodbye(snotify, addr_count) < 0)
#else
			if(SendSSDPGoodbye(snotify, addr_count * 2) < 0)
#endif
			{
				syslog(LOG_WARNING, "Failed to broadcast good-bye notifications");
			}
		}
#endif /* UPNP_STRICT */

#ifdef USE_IFACEWATCHER
		/* open socket for kernel notifications about new network interfaces */
		if (sudp >= 0)
		{
			sifacewatcher = OpenAndConfInterfaceWatchSocket();
			if (sifacewatcher < 0)
			{
				syslog(LOG_ERR, "Failed to open socket for receiving network interface notifications");
			}
		}
#endif
	}

#ifdef ENABLE_NATPMP
	/* open socket for NAT PMP traffic */
	if(GETFLAG(ENABLENATPMPMASK))
	{
		if(OpenAndConfNATPMPSockets(snatpmp) < 0)
#ifdef ENABLE_PCP
		{
			syslog(LOG_ERR, "Failed to open sockets for NAT-PMP/PCP.");
		} else {
			syslog(LOG_NOTICE, "Listening for NAT-PMP/PCP traffic on port %u",
			       NATPMP_PORT);
		}
#else
		{
			syslog(LOG_ERR, "Failed to open sockets for NAT PMP.");
		} else {
			syslog(LOG_NOTICE, "Listening for NAT-PMP traffic on port %u",
			       NATPMP_PORT);
		}
#endif
	}
#endif

#if defined(ENABLE_IPV6) && defined(ENABLE_PCP)
	if(!GETFLAG(IPV6DISABLEDMASK)) {
		spcp_v6 = OpenAndConfPCPv6Socket();
	}
#endif

	/* for miniupnpdctl */
#ifdef USE_MINIUPNPDCTL
	sctl = OpenAndConfCtlUnixSocket("/var/run/miniupnpd.ctl");
#endif

#ifdef ENABLE_NFQUEUE
	if ( nfqueue != -1 && n_nfqix > 0) {
		nfqh = OpenAndConfNFqueue();
		if(nfqh < 0) {
			syslog(LOG_ERR, "Failed to open fd for NFQUEUE.");
			return 1;
		} else {
			syslog(LOG_NOTICE, "Opened NFQUEUE %d",nfqueue);
		}
	}
#endif

#ifdef TOMATO
	tomato_helper();
#endif

#ifdef ENABLE_PCP
	if(GETFLAG(ENABLENATPMPMASK))
	{
		/* Send PCP startup announcements */
#ifdef ENABLE_IPV6
		PCPSendUnsolicitedAnnounce(snatpmp, addr_count, spcp_v6);
#else /* IPv4 only */
		PCPSendUnsolicitedAnnounce(snatpmp, addr_count);
#endif
	}
#endif

	/* drop privileges */
#ifdef HAS_PLEDGE
	/* mcast ? unix ? */
	if (pledge("stdio inet pf", NULL) < 0) {
		syslog(LOG_ERR, "pledge(): %m");
		return 1;
	}
#endif /* HAS_PLEDGE */
#ifdef HAS_LIBCAP
	{
		cap_t caps = cap_get_proc();
		if (caps == NULL) {
			syslog(LOG_ERR, "cap_get_proc(): %m");
		} else {
			static const cap_value_t cap_list[3] = { CAP_NET_BROADCAST, CAP_NET_ADMIN, CAP_NET_RAW };
			char * txt_caps = cap_to_text(caps, NULL);
			if (txt_caps == NULL) {
				syslog(LOG_ERR, "cap_to_text(): %m");
			} else {
				syslog(LOG_DEBUG, "capabilities %s", txt_caps);
				if (cap_free(txt_caps) < 0) {
					syslog(LOG_ERR, "cap_free(): %m");
				}
			}
			if (cap_clear(caps) < 0) {
				syslog(LOG_ERR, "cap_clear(): %m");
			}
			if (cap_set_flag(caps, CAP_PERMITTED, sizeof(cap_list)/sizeof(cap_list[0]), cap_list, CAP_SET) < 0) {
				syslog(LOG_ERR, "cap_set_flag(): %m");
			}
			if (cap_set_flag(caps, CAP_EFFECTIVE, sizeof(cap_list)/sizeof(cap_list[0]), cap_list, CAP_SET) < 0) {
				syslog(LOG_ERR, "cap_set_flag(): %m");
			}
			txt_caps = cap_to_text(caps, NULL);
			if (txt_caps == NULL) {
				syslog(LOG_ERR, "cap_to_text(): %m");
			} else {
				syslog(LOG_DEBUG, "capabilities %s", txt_caps);
				if (cap_free(txt_caps) < 0) {
					syslog(LOG_ERR, "cap_free(): %m");
				}
			}
			if (cap_set_proc(caps) < 0) {
				syslog(LOG_ERR, "cap_set_proc(): %m");
			}
			if (cap_free(caps) < 0) {
				syslog(LOG_ERR, "cap_free(): %m");
			}
		}
	}
#endif /* HAS_LIBCAP */
#ifdef HAS_LIBCAP_NG
	capng_setpid(getpid());
	capng_clear(CAPNG_SELECT_BOTH);
	if (capng_updatev(CAPNG_ADD, CAPNG_EFFECTIVE|CAPNG_PERMITTED, CAP_NET_BROADCAST, CAP_NET_ADMIN, CAP_NET_RAW, -1) < 0) {
		syslog(LOG_ERR, "capng_updatev() failed");
	} else {
		if (capng_apply(CAPNG_SELECT_BOTH) < 0) {
			syslog(LOG_ERR, "capng_apply() failed");
		}
	}
#endif /* HAS_LIBCAP_NG */

	/* main loop */
	while(!quitting)
	{
#ifdef USE_TIME_AS_BOOTID
		/* Correct startup_time if it was set with a RTC close to 0 */
		if((upnp_bootid<60*60*24) && (time(NULL)>60*60*24))
		{
			upnp_bootid = time(NULL);
		}
#endif
#if !defined(TOMATO) && defined(ENABLE_LEASEFILE) && defined(LEASEFILE_USE_REMAINING_TIME)
		if(should_rewrite_leasefile)
		{
			lease_file_rewrite();
			should_rewrite_leasefile = 0;
		}
#endif /* !TOMATO && ENABLE_LEASEFILE && LEASEFILE_USE_REMAINING_TIME */
		/* send public address change notifications if needed */
		if(should_send_public_address_change_notif)
		{
			syslog(LOG_INFO, "should send external iface address change notification(s)");
			if(GETFLAG(PERFORMSTUNMASK))
			{
				if (update_ext_ip_addr_from_stun(0) != 0) {
					/* if stun succeed it updates disable_port_forwarding;
					 * if stun failed (non-zero return value) then port forwarding would not work, so disable it */
					disable_port_forwarding = 1;
				}
			}
			else if (!use_ext_ip_addr)
			{
				char if_addr[INET_ADDRSTRLEN];
				struct in_addr addr;
				if (getifaddr(ext_if_name, if_addr, INET_ADDRSTRLEN, &addr, NULL) < 0) {
					syslog(LOG_WARNING, "Cannot get IP address for ext interface %s. Network is down", ext_if_name);
					disable_port_forwarding = 1;
				} else {
					int reserved = 0;
					if (!disable_port_forwarding && reserved) {
						syslog(LOG_INFO, "Reserved / private IP address %s on ext interface %s: Port forwarding is impossible", if_addr, ext_if_name);
						syslog(LOG_INFO, "You are probably behind NAT, enable option ext_perform_stun=yes to detect public IP address");
						syslog(LOG_INFO, "Or use ext_ip= / -o option to declare public IP address");
						syslog(LOG_INFO, "Public IP address is required by UPnP/PCP/PMP protocols and clients do not work without it");
						disable_port_forwarding = 1;
					} else if (disable_port_forwarding && !reserved) {
						syslog(LOG_INFO, "Public IP address %s on ext interface %s: Port forwarding is enabled", if_addr, ext_if_name);
						disable_port_forwarding = 0;
					}
				}
			}
#ifdef ENABLE_NATPMP
			if(GETFLAG(ENABLENATPMPMASK))
				SendNATPMPPublicAddressChangeNotification(snatpmp, addr_count);
#endif
#ifdef ENABLE_EVENTS
			if(GETFLAG(ENABLEUPNPMASK))
			{
				upnp_event_var_change_notify(EWanIPC);
			}
#endif
#ifdef ENABLE_PCP
			if(GETFLAG(ENABLENATPMPMASK))
			{
#ifdef ENABLE_IPV6
				PCPPublicAddressChanged(snatpmp, addr_count, spcp_v6);
#else /* IPv4 only */
				PCPPublicAddressChanged(snatpmp, addr_count);
#endif
			}
#endif
			should_send_public_address_change_notif = 0;
		}
		/* Check if we need to send SSDP NOTIFY messages and do it if
		 * needed */
		if(upnp_gettimeofday(&timeofday) < 0)
		{
			syslog(LOG_ERR, "gettimeofday(): %m");
			timeout.tv_sec = current_notify_interval;
			timeout.tv_usec = 0;
		}
		else
		{
			/* the comparaison is not very precise but who cares ? */
			if(timeofday.tv_sec >= (lasttimeofday.tv_sec + current_notify_interval))
			{
				if (GETFLAG(ENABLEUPNPMASK))
					SendSSDPNotifies2(snotify,
				                  (unsigned short)v.port,
#ifdef ENABLE_HTTPS
					              (unsigned short)v.https_port,
#endif
				                  v.notify_interval << 1);
				current_notify_interval = gen_current_notify_interval(v.notify_interval);
				memcpy(&lasttimeofday, &timeofday, sizeof(struct timeval));
				timeout.tv_sec = current_notify_interval;
				timeout.tv_usec = 0;
			}
			else
			{
				timeout.tv_sec = lasttimeofday.tv_sec + current_notify_interval
				                 - timeofday.tv_sec;
				if(timeofday.tv_usec > lasttimeofday.tv_usec)
				{
					timeout.tv_usec = 1000000 + lasttimeofday.tv_usec
					                  - timeofday.tv_usec;
					timeout.tv_sec--;
				}
				else
				{
					timeout.tv_usec = lasttimeofday.tv_usec - timeofday.tv_usec;
				}
			}
		}
		/* remove unused rules */
		if( v.clean_ruleset_interval
		  && (timeofday.tv_sec >= checktime.tv_sec + v.clean_ruleset_interval))
		{
			if(rule_list)
			{
				remove_unused_rules(rule_list);
				rule_list = NULL;
			}
			else
			{
				rule_list = get_upnp_rules_state_list(v.clean_ruleset_threshold);
			}
			memcpy(&checktime, &timeofday, sizeof(struct timeval));
		}
		/* Remove expired port mappings, based on UPnP IGD LeaseDuration
		 * or NAT-PMP lifetime) */
		if(nextruletoclean_timestamp
		  && ((unsigned int)timeofday.tv_sec >= nextruletoclean_timestamp))
		{
			syslog(LOG_DEBUG, "cleaning expired Port Mappings");
			get_upnp_rules_state_list(0);
		}
		if(nextruletoclean_timestamp
		  && ((unsigned int)timeout.tv_sec >= (nextruletoclean_timestamp - timeofday.tv_sec)))
		{
			timeout.tv_sec = nextruletoclean_timestamp - timeofday.tv_sec;
			timeout.tv_usec = 0;
			syslog(LOG_DEBUG, "setting timeout to %u sec",
			       (unsigned)timeout.tv_sec);
		}
#ifdef ENABLE_UPNPPINHOLE
		/* Clean up expired IPv6 PinHoles */
		next_pinhole_ts = 0;
		upnp_clean_expired_pinholes(&next_pinhole_ts);
		if(next_pinhole_ts &&
		   timeout.tv_sec >= (int)(next_pinhole_ts - timeofday.tv_sec)) {
			timeout.tv_sec = next_pinhole_ts - timeofday.tv_sec;
			timeout.tv_usec = 0;
		}
#endif /* ENABLE_UPNPPINHOLE */

		/* select open sockets (SSDP, HTTP listen, and all HTTP soap sockets) */
		FD_ZERO(&readset);
		FD_ZERO(&writeset);

		if (sudp >= 0)
		{
			FD_SET(sudp, &readset);
			max_fd = MAX( max_fd, sudp);
#ifdef USE_IFACEWATCHER
			if (sifacewatcher >= 0)
			{
				FD_SET(sifacewatcher, &readset);
				max_fd = MAX(max_fd, sifacewatcher);
			}
#endif
		}
		if (shttpl >= 0)
		{
			FD_SET(shttpl, &readset);
			max_fd = MAX( max_fd, shttpl);
		}
#if defined(V6SOCKETS_ARE_V6ONLY) && defined(ENABLE_IPV6)
		if (shttpl_v4 >= 0)
		{
			FD_SET(shttpl_v4, &readset);
			max_fd = MAX( max_fd, shttpl_v4);
		}
#endif
#ifdef ENABLE_HTTPS
		if (shttpsl >= 0)
		{
			FD_SET(shttpsl, &readset);
			max_fd = MAX( max_fd, shttpsl);
		}
#if defined(V6SOCKETS_ARE_V6ONLY) && defined(ENABLE_IPV6)
		if (shttpsl_v4 >= 0)
		{
			FD_SET(shttpsl_v4, &readset);
			max_fd = MAX( max_fd, shttpsl_v4);
		}
#endif
#endif /* ENABLE_HTTPS */
#ifdef ENABLE_IPV6
		if (sudpv6 >= 0)
		{
			FD_SET(sudpv6, &readset);
			max_fd = MAX( max_fd, sudpv6);
		}
#endif

#ifdef ENABLE_NFQUEUE
		if (nfqh >= 0)
		{
			FD_SET(nfqh, &readset);
			max_fd = MAX( max_fd, nfqh);
		}
#endif

		i = 0;	/* active HTTP connections count */
		for(e = upnphttphead.lh_first; e != NULL; e = e->entries.le_next)
		{
			if(e->socket >= 0)
			{
				if(e->state <= EWaitingForHttpContent)
					FD_SET(e->socket, &readset);
				else if(e->state == ESendingAndClosing)
					FD_SET(e->socket, &writeset);
				else
					continue;
				max_fd = MAX(max_fd, e->socket);
				i++;
			}
		}
		/* for debug */
#ifdef DEBUG
		if(i > 1)
		{
			syslog(LOG_DEBUG, "%d active incoming HTTP connections", i);
		}
#endif
#ifdef ENABLE_NATPMP
		for(i=0; i<addr_count; i++) {
			if(snatpmp[i] >= 0) {
				FD_SET(snatpmp[i], &readset);
				max_fd = MAX( max_fd, snatpmp[i]);
			}
		}
#endif
#if defined(ENABLE_IPV6) && defined(ENABLE_PCP)
		if(spcp_v6 >= 0) {
			FD_SET(spcp_v6, &readset);
			max_fd = MAX(max_fd, spcp_v6);
		}
#endif
#ifdef USE_MINIUPNPDCTL
		if(sctl >= 0) {
			FD_SET(sctl, &readset);
			max_fd = MAX( max_fd, sctl);
		}

		for(ectl = ctllisthead.lh_first; ectl; ectl = ectl->entries.le_next)
		{
			if(ectl->socket >= 0) {
				FD_SET(ectl->socket, &readset);
				max_fd = MAX( max_fd, ectl->socket);
			}
		}
#endif

#ifdef ENABLE_EVENTS
		upnpevents_selectfds(&readset, &writeset, &max_fd);
#endif

		/* queued "sendto" */
		{
			struct timeval next_send;
			i = get_next_scheduled_send(&next_send);
			if(i > 0) {
#ifdef DEBUG
				syslog(LOG_DEBUG, "%d queued sendto", i);
#endif
				i = get_sendto_fds(&writeset, &max_fd, &timeofday);
				if(timeofday.tv_sec > next_send.tv_sec ||
				   (timeofday.tv_sec == next_send.tv_sec && timeofday.tv_usec >= next_send.tv_usec)) {
					if(i > 0) {
						timeout.tv_sec = 0;
						timeout.tv_usec = 0;
					}
				} else {
					struct timeval tmp_timeout;
					tmp_timeout.tv_sec = (next_send.tv_sec - timeofday.tv_sec);
					tmp_timeout.tv_usec = (next_send.tv_usec - timeofday.tv_usec);
					if(tmp_timeout.tv_usec < 0) {
						tmp_timeout.tv_usec += 1000000;
						tmp_timeout.tv_sec--;
					}
					if(timeout.tv_sec > tmp_timeout.tv_sec
					   || (timeout.tv_sec == tmp_timeout.tv_sec && timeout.tv_usec > tmp_timeout.tv_usec)) {
						timeout.tv_sec = tmp_timeout.tv_sec;
						timeout.tv_usec = tmp_timeout.tv_usec;
					}
				}
			}
		}

		if(select(max_fd+1, &readset, &writeset, 0, &timeout) < 0)
		{
			if(quitting) goto shutdown;
#ifdef TOMATO
			if (gotusr2)
			{
				gotusr2 = 0;
				tomato_helper();
				continue;
			}
#endif	/* TOMATO */
			if(errno == EINTR) continue; /* interrupted by a signal, start again */
			syslog(LOG_ERR, "select(all): %m");
			syslog(LOG_ERR, "Failed to select open sockets. EXITING");
			return 1;	/* very serious cause of error */
		}
		i = try_sendto(&writeset);
		if(i < 0) {
			syslog(LOG_ERR, "try_sendto failed to send %d packets", -i);
		}
#ifdef USE_MINIUPNPDCTL
		for(ectl = ctllisthead.lh_first; ectl;)
		{
			ectlnext =  ectl->entries.le_next;
			if((ectl->socket >= 0) && FD_ISSET(ectl->socket, &readset))
			{
				char buf[256];
				int l;
				l = read(ectl->socket, buf, sizeof(buf));
				if(l > 0)
				{
					/*write(ectl->socket, buf, l);*/
					write_command_line(ectl->socket, argc, argv);
#ifndef DISABLE_CONFIG_FILE
					write_option_list(ectl->socket);
#endif
					write_permlist(ectl->socket, upnppermlist, num_upnpperm);
					write_upnphttp_details(ectl->socket, upnphttphead.lh_first);
					write_ctlsockets_list(ectl->socket, ctllisthead.lh_first);
					write_ruleset_details(ectl->socket);
#ifdef ENABLE_EVENTS
					write_events_details(ectl->socket);
#endif
					/* close the socket */
					close(ectl->socket);
					ectl->socket = -1;
				}
				else
				{
					close(ectl->socket);
					ectl->socket = -1;
				}
			}
			if(ectl->socket < 0)
			{
				LIST_REMOVE(ectl, entries);
				free(ectl);
			}
			ectl = ectlnext;
		}
		if((sctl >= 0) && FD_ISSET(sctl, &readset))
		{
			int s;
			struct sockaddr_un clientname;
			struct ctlelem * tmp;
			socklen_t clientnamelen = sizeof(struct sockaddr_un);
			/*syslog(LOG_DEBUG, "sctl!");*/
			s = accept(sctl, (struct sockaddr *)&clientname,
			           &clientnamelen);
			syslog(LOG_DEBUG, "sctl! : '%s'", clientname.sun_path);
			tmp = malloc(sizeof(struct ctlelem));
			if (tmp == NULL)
			{
				syslog(LOG_ERR, "Unable to allocate memory for ctlelem in main()");
				close(s);
			}
			else
			{
				tmp->socket = s;
				LIST_INSERT_HEAD(&ctllisthead, tmp, entries);
			}
		}
#endif
#ifdef ENABLE_EVENTS
		upnpevents_processfds(&readset, &writeset);
#endif
#ifdef ENABLE_NATPMP
		/* process NAT-PMP packets */
		for(i=0; i<addr_count; i++)
		{
			if((snatpmp[i] >= 0) && FD_ISSET(snatpmp[i], &readset))
			{
				unsigned char msg_buff[PCP_MAX_LEN];
				struct sockaddr_in senderaddr;
				socklen_t senderaddrlen;
				int len;
				memset(msg_buff, 0, PCP_MAX_LEN);
				senderaddrlen = sizeof(senderaddr);
				len = ReceiveNATPMPOrPCPPacket(snatpmp[i],
				                               (struct sockaddr *)&senderaddr,
				                               &senderaddrlen,
				                               NULL,
				                               msg_buff, sizeof(msg_buff));
				if (len < 1)
					continue;
#ifdef ENABLE_PCP
				if (msg_buff[0]==0) {  /* version equals to 0 -> means NAT-PMP */
					/* Check if the packet is coming from a LAN to enforce RFC6886 :
					 * The NAT gateway MUST NOT accept mapping requests destined to the NAT
					 * gateway's external IP address or received on its external network
					 * interface.  Only packets received on the internal interface(s) with a
					 * destination address matching the internal address(es) of the NAT
					 * gateway should be allowed. */
					/* TODO : move to ProcessIncomingNATPMPPacket() ? */
					lan_addr = get_lan_for_peer((struct sockaddr *)&senderaddr);
					if(lan_addr == NULL) {
						char sender_str[64];
						sockaddr_to_string((struct sockaddr *)&senderaddr, sender_str, sizeof(sender_str));
						syslog(LOG_WARNING, "NAT-PMP packet sender %s not from a LAN, ignoring",
						       sender_str);
						continue;
					}
					ProcessIncomingNATPMPPacket(snatpmp[i], msg_buff, len,
					                            &senderaddr);
				} else { /* everything else can be PCP */
					ProcessIncomingPCPPacket(snatpmp[i], msg_buff, len,
					                         (struct sockaddr *)&senderaddr, NULL);
				}

#else
				/* Check if the packet is coming from a LAN to enforce RFC6886 :
				 * The NAT gateway MUST NOT accept mapping requests destined to the NAT
				 * gateway's external IP address or received on its external network
				 * interface.  Only packets received on the internal interface(s) with a
				 * destination address matching the internal address(es) of the NAT
				 * gateway should be allowed. */
				/* TODO : move to ProcessIncomingNATPMPPacket() ? */
				lan_addr = get_lan_for_peer((struct sockaddr *)&senderaddr);
				if(lan_addr == NULL) {
					char sender_str[64];
					sockaddr_to_string((struct sockaddr *)&senderaddr, sender_str, sizeof(sender_str));
					syslog(LOG_WARNING, "NAT-PMP packet sender %s not from a LAN, ignoring",
					       sender_str);
					continue;
				}
				ProcessIncomingNATPMPPacket(snatpmp[i], msg_buff, len, &senderaddr);
#endif
			}
		}
#endif
#if defined(ENABLE_IPV6) && defined(ENABLE_PCP)
		/* in IPv6, only PCP is supported, not NAT-PMP */
		if(spcp_v6 >= 0 && FD_ISSET(spcp_v6, &readset))
		{
			unsigned char msg_buff[PCP_MAX_LEN];
			struct sockaddr_in6 senderaddr;
			socklen_t senderaddrlen;
			struct sockaddr_in6 receiveraddr;
			int len;
			memset(msg_buff, 0, PCP_MAX_LEN);
			senderaddrlen = sizeof(senderaddr);
			len = ReceiveNATPMPOrPCPPacket(spcp_v6,
			                               (struct sockaddr *)&senderaddr,
			                               &senderaddrlen,
			                               &receiveraddr,
			                               msg_buff, sizeof(msg_buff));
			if(len >= 1)
				ProcessIncomingPCPPacket(spcp_v6, msg_buff, len,
				                         (struct sockaddr *)&senderaddr,
				                         &receiveraddr);
		}
#endif
		/* process SSDP packets */
		if(sudp >= 0 && FD_ISSET(sudp, &readset))
		{
			/*syslog(LOG_INFO, "Received UDP Packet");*/
#ifdef ENABLE_HTTPS
			ProcessSSDPRequest(sudp, (unsigned short)v.port, (unsigned short)v.https_port);
#else
			ProcessSSDPRequest(sudp, (unsigned short)v.port);
#endif
		}
#ifdef ENABLE_IPV6
		if(sudpv6 >= 0 && FD_ISSET(sudpv6, &readset))
		{
			syslog(LOG_INFO, "Received UDP Packet (IPv6)");
#ifdef ENABLE_HTTPS
			ProcessSSDPRequest(sudpv6, (unsigned short)v.port, (unsigned short)v.https_port);
#else
			ProcessSSDPRequest(sudpv6, (unsigned short)v.port);
#endif
		}
#endif
#ifdef USE_IFACEWATCHER
		/* process kernel notifications */
		if (sifacewatcher >= 0 && FD_ISSET(sifacewatcher, &readset))
			ProcessInterfaceWatchNotify(sifacewatcher);
#endif

		/* process active HTTP connections */
		/* LIST_FOREACH macro is not available under linux */
		for(e = upnphttphead.lh_first; e != NULL; e = e->entries.le_next)
		{
			if(e->socket >= 0)
			{
				if(FD_ISSET(e->socket, &readset) ||
				   FD_ISSET(e->socket, &writeset))
				{
					Process_upnphttp(e);
				}
			}
		}
		/* process incoming HTTP connections */
		if(shttpl >= 0 && FD_ISSET(shttpl, &readset))
		{
			struct upnphttp * tmp;
			tmp = ProcessIncomingHTTP(shttpl, "HTTP");
			if(tmp)
			{
				LIST_INSERT_HEAD(&upnphttphead, tmp, entries);
			}
		}
#if defined(V6SOCKETS_ARE_V6ONLY) && defined(ENABLE_IPV6)
		if(shttpl_v4 >= 0 && FD_ISSET(shttpl_v4, &readset))
		{
			struct upnphttp * tmp;
			tmp = ProcessIncomingHTTP(shttpl_v4, "HTTP");
			if(tmp)
			{
				LIST_INSERT_HEAD(&upnphttphead, tmp, entries);
			}
		}
#endif
#ifdef ENABLE_HTTPS
		if(shttpsl >= 0 && FD_ISSET(shttpsl, &readset))
		{
			struct upnphttp * tmp;
			tmp = ProcessIncomingHTTP(shttpsl, "HTTPS");
			if(tmp)
			{
				InitSSL_upnphttp(tmp);
				LIST_INSERT_HEAD(&upnphttphead, tmp, entries);
			}
		}
#if defined(V6SOCKETS_ARE_V6ONLY) && defined(ENABLE_IPV6)
		if(shttpsl_v4 >= 0 && FD_ISSET(shttpsl_v4, &readset))
		{
			struct upnphttp * tmp;
			tmp = ProcessIncomingHTTP(shttpsl_v4, "HTTPS");
			if(tmp)
			{
				InitSSL_upnphttp(tmp);
				LIST_INSERT_HEAD(&upnphttphead, tmp, entries);
			}
		}
#endif
#endif /* ENABLE_HTTPS */
#ifdef ENABLE_NFQUEUE
		/* process NFQ packets */
		if(nfqh >= 0 && FD_ISSET(nfqh, &readset))
		{
			/* syslog(LOG_INFO, "Received NFQUEUE Packet");*/
			ProcessNFQUEUE(nfqh);
		}
#endif
		/* delete finished HTTP connections */
		for(e = upnphttphead.lh_first; e != NULL; )
		{
			next = e->entries.le_next;
			if(e->state >= EToDelete)
			{
				LIST_REMOVE(e, entries);
				Delete_upnphttp(e);
			}
			e = next;
		}

	}	/* end of main loop */

shutdown:
	syslog(LOG_NOTICE, "shutting down MiniUPnPd");
	/* send good-bye */
	if (GETFLAG(ENABLEUPNPMASK))
	{
#ifndef ENABLE_IPV6
		if(SendSSDPGoodbye(snotify, addr_count) < 0)
#else
		if(SendSSDPGoodbye(snotify, addr_count * 2) < 0)
#endif
		{
			syslog(LOG_ERR, "Failed to broadcast good-bye notifications");
		}
	}
	/* try to send pending packets */
	finalize_sendto();

#ifdef TOMATO
	tomato_save("/etc/upnp/data");
#endif	/* TOMATO */
#if defined(ENABLE_LEASEFILE) && defined(LEASEFILE_USE_REMAINING_TIME)
	lease_file_rewrite();
#endif /* ENABLE_LEASEFILE && LEASEFILE_USE_REMAINING_TIME */
	/* close out open sockets */
	while(upnphttphead.lh_first != NULL)
	{
		e = upnphttphead.lh_first;
		LIST_REMOVE(e, entries);
		Delete_upnphttp(e);
	}

	if (sudp >= 0) close(sudp);
	if (shttpl >= 0) close(shttpl);
#if defined(V6SOCKETS_ARE_V6ONLY) && defined(ENABLE_IPV6)
	if (shttpl_v4 >= 0) close(shttpl_v4);
#endif
#ifdef ENABLE_IPV6
	if (sudpv6 >= 0) close(sudpv6);
#endif
#ifdef USE_IFACEWATCHER
	if(sifacewatcher >= 0) close(sifacewatcher);
#endif
#ifdef ENABLE_NATPMP
	for(i=0; i<addr_count; i++) {
		if(snatpmp[i]>=0)
		{
			close(snatpmp[i]);
			snatpmp[i] = -1;
		}
	}
#endif
#if defined(ENABLE_IPV6) && defined(ENABLE_PCP)
	if(spcp_v6 >= 0)
	{
		close(spcp_v6);
		spcp_v6 = -1;
	}
#endif
#ifdef USE_MINIUPNPDCTL
	if(sctl>=0)
	{
		close(sctl);
		sctl = -1;
		if(unlink("/var/run/miniupnpd.ctl") < 0)
		{
			syslog(LOG_ERR, "unlink() %m");
		}
	}
#endif

	if (GETFLAG(ENABLEUPNPMASK))
	{
#ifndef ENABLE_IPV6
		for(i = 0; i < addr_count; i++)
#else
		for(i = 0; i < addr_count * 2; i++)
#endif
			close(snotify[i]);
	}

	/* remove pidfile */
#ifndef NO_BACKGROUND_NO_PIDFILE
	if(pidfilename && (unlink(pidfilename) < 0))
	{
		syslog(LOG_ERR, "Failed to remove pidfile %s: %m", pidfilename);
	}
#endif

	/* delete lists */
	while(lan_addrs.lh_first != NULL)
	{
		lan_addr = lan_addrs.lh_first;
		LIST_REMOVE(lan_addrs.lh_first, list);
		free(lan_addr);
	}

#ifdef ENABLE_HTTPS
	free_ssl();
#endif
#ifdef ENABLE_NATPMP
	free(snatpmp);
#endif
	free(snotify);

	shutdown_redirect();

#ifndef DISABLE_CONFIG_FILE
	/* in some case shutdown_redirect() may need the option values */
	freeoptions();
#endif
#ifdef DYNAMIC_OS_VERSION
	free(os_version);
#endif
	closelog();
	return 0;
}

