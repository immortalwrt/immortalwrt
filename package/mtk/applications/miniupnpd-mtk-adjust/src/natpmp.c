/* $Id: natpmp.c,v 1.58 2022/10/16 06:03:56 nanard Exp $ */
/* MiniUPnP project
 * (c) 2007-2022 Thomas Bernard
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */
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
#include <sys/uio.h>

#include "macros.h"
#include "rw_unaligned.h"
#include "config.h"
#include "natpmp.h"
#include "upnpglobalvars.h"
#include "getifaddr.h"
#include "upnpredirect.h"
#include "commonrdr.h"
#include "upnputils.h"
#include "portinuse.h"
#include "asyncsendto.h"

#ifdef ENABLE_NATPMP

int OpenAndConfNATPMPSocket(in_addr_t addr)
{
	int snatpmp;
	int i = 1;
	snatpmp = socket(PF_INET, SOCK_DGRAM, 0/*IPPROTO_UDP*/);
	if(snatpmp<0)
	{
		syslog(LOG_ERR, "%s: socket(): %m",
		       "OpenAndConfNATPMPSocket");
		return -1;
	}
	if(setsockopt(snatpmp, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i)) < 0)
	{
		syslog(LOG_WARNING, "%s: setsockopt(SO_REUSEADDR): %m",
		       "OpenAndConfNATPMPSocket");
	}
	if(!set_non_blocking(snatpmp))
	{
		syslog(LOG_WARNING, "%s: set_non_blocking(): %m",
		       "OpenAndConfNATPMPSocket");
	}
	{
		struct sockaddr_in natpmp_addr;
		memset(&natpmp_addr, 0, sizeof(natpmp_addr));
		natpmp_addr.sin_family = AF_INET;
		natpmp_addr.sin_port = htons(NATPMP_PORT);
		/*natpmp_addr.sin_addr.s_addr = INADDR_ANY; */
		natpmp_addr.sin_addr.s_addr = addr;
		if(bind(snatpmp, (struct sockaddr *)&natpmp_addr, sizeof(natpmp_addr)) < 0)
		{
			syslog(LOG_ERR, "%s: bind(): %m",
			       "OpenAndConfNATPMPSocket");
			close(snatpmp);
			return -1;
		}
	}
	return snatpmp;
}

int OpenAndConfNATPMPSockets(int * sockets)
{
	int i;
	struct lan_addr_s * lan_addr;
	for(i = 0, lan_addr = lan_addrs.lh_first;
	    lan_addr != NULL;
	    lan_addr = lan_addr->list.le_next)
	{
		sockets[i] = OpenAndConfNATPMPSocket(lan_addr->addr.s_addr);
		if(sockets[i] < 0)
			goto error;
		i++;
	}
	return 0;
error:
	while(--i >= 0)
	{
		close(sockets[i]);
		sockets[i] = -1;
	}
	return -1;
}

static void FillPublicAddressResponse(unsigned char * resp, in_addr_t senderaddr)
{
#ifndef MULTIPLE_EXTERNAL_IP
	struct in_addr addr;
	char tmp[16];
	UNUSED(senderaddr);

	if(use_ext_ip_addr) {
        inet_pton(AF_INET, use_ext_ip_addr, resp+8);
	} else {
		if(!ext_if_name || ext_if_name[0]=='\0') {
			resp[3] = 3;	/* Network Failure (e.g. NAT box itself
			                 * has not obtained a DHCP lease) */
		} else if(getifaddr(ext_if_name, tmp, INET_ADDRSTRLEN, &addr, NULL) < 0) {
			syslog(LOG_ERR, "Failed to get IP for interface %s", ext_if_name);
			resp[3] = 3;	/* Network Failure (e.g. NAT box itself
			                 * has not obtained a DHCP lease) */
		} else if (0) {
			resp[3] = 3;	/* Network Failure, box has not obtained
			                   public IP address */
		} else {
			inet_pton(AF_INET, tmp, resp+8); /* ok */
		}
	}
#else
	struct lan_addr_s * lan_addr;

	for(lan_addr = lan_addrs.lh_first; lan_addr != NULL; lan_addr = lan_addr->list.le_next) {
		if( (senderaddr & lan_addr->mask.s_addr)
		   == (lan_addr->addr.s_addr & lan_addr->mask.s_addr)) {
			memcpy(resp+8, &lan_addr->ext_ip_addr,
			       sizeof(lan_addr->ext_ip_addr));
			break;
		}
	}
#endif
}

/*
 * Receives NATPMP and PCP packets and stores them in msg_buff.
 * The sender information is stored in senderaddr.
 * Returns number of bytes recevied, even if number is negative.
 */
int ReceiveNATPMPOrPCPPacket(int s, struct sockaddr * senderaddr,
                             socklen_t * senderaddrlen,
                             struct sockaddr_in6 * receiveraddr,
                             unsigned char * msg_buff, size_t msg_buff_size)
{
#ifdef IPV6_PKTINFO
	struct iovec iov;
	uint8_t c[1000];
	struct msghdr msg;
	int n;
	struct cmsghdr *h;

	iov.iov_base = msg_buff;
	iov.iov_len = msg_buff_size;
	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_name = senderaddr;
	msg.msg_namelen = *senderaddrlen;
	msg.msg_control = c;
	msg.msg_controllen = sizeof(c);

	n = recvmsg(s, &msg, 0);
	if(n < 0) {
		/* EAGAIN, EWOULDBLOCK and EINTR : silently ignore (retry next time)
		 * other errors : log to LOG_ERR */
		if(errno != EAGAIN &&
		   errno != EWOULDBLOCK &&
		   errno != EINTR) {
			syslog(LOG_ERR, "recvmsg(natpmp): %m");
		}
		return n;
	}

	if(receiveraddr) {
		memset(receiveraddr, 0, sizeof(struct sockaddr_in6));
	}
	if ((msg.msg_flags & MSG_TRUNC) || (msg.msg_flags & MSG_CTRUNC)) {
		syslog(LOG_WARNING, "%s: truncated message",
		       "ReceiveNATPMPOrPCPPacket");
	}
	for(h = CMSG_FIRSTHDR(&msg); h;
	    h = CMSG_NXTHDR(&msg, h)) {
		if(h->cmsg_level == IPPROTO_IPV6 && h->cmsg_type == IPV6_PKTINFO) {
			char tmp[INET6_ADDRSTRLEN];
			struct in6_pktinfo *ipi6 = (struct in6_pktinfo *)CMSG_DATA(h);
			syslog(LOG_DEBUG, "%s: packet destination: %s scope_id=%u",
			       "ReceiveNATPMPOrPCPPacket",
			       inet_ntop(AF_INET6, &ipi6->ipi6_addr, tmp, sizeof(tmp)),
			       ipi6->ipi6_ifindex);
			if(receiveraddr) {
				receiveraddr->sin6_addr = ipi6->ipi6_addr;
				receiveraddr->sin6_scope_id = ipi6->ipi6_ifindex;
				receiveraddr->sin6_family = AF_INET6;
				receiveraddr->sin6_port = htons(NATPMP_PORT);
			}
		}
	}
#else /* IPV6_PKTINFO */
	int n;

	n = recvfrom(s, msg_buff, msg_buff_size, 0,
	             senderaddr, senderaddrlen);

	if(n<0) {
		/* EAGAIN, EWOULDBLOCK and EINTR : silently ignore (retry next time)
		 * other errors : log to LOG_ERR */
		if(errno != EAGAIN &&
		   errno != EWOULDBLOCK &&
		   errno != EINTR) {
			syslog(LOG_ERR, "recvfrom(natpmp): %m");
		}
		return n;
	}
#endif /* IPV6_PKTINFO */

	return n;
}

/** read the request from the socket, process it and then send the
 * response back.
 */
void ProcessIncomingNATPMPPacket(int s, unsigned char *msg_buff, int len,
		struct sockaddr_in *senderaddr)
{
	unsigned char *req=msg_buff;	/* request udp packet */
	unsigned char resp[32];	/* response udp packet */
	int resplen;
	int n = len;
	char senderaddrstr[16];

	if(!inet_ntop(AF_INET, &senderaddr->sin_addr,
			senderaddrstr, sizeof(senderaddrstr))) {
		syslog(LOG_ERR, "inet_ntop(natpmp): %m");
	}

	syslog(LOG_INFO, "NAT-PMP request received from %s:%hu %dbytes",
	       senderaddrstr, ntohs(senderaddr->sin_port), n);

	if(n<2 || ((((req[1]-1)&~1)==0) && n<12)) {
		syslog(LOG_WARNING, "discarding NAT-PMP request (too short) %dBytes",
		       n);
		return;
	}
	if(req[1] & 128) {
		/* discarding NAT-PMP responses silently */
		return;
	}
	memset(resp, 0, sizeof(resp));
	resplen = 8;
	resp[1] = 128 + req[1];	/* response OPCODE is request OPCODE + 128 */
	/* setting response TIME STAMP :
	 * time elapsed since its port mapping table was initialized on
	 * startup or reset for any other reason */
	if(epoch_origin == 0) {
		epoch_origin = startup_time;
	}
	WRITENU32(resp+4, upnp_time() - epoch_origin);
	if(req[0] > 0) {
		/* invalid version */
		syslog(LOG_WARNING, "unsupported NAT-PMP version : %u",
		       (unsigned)req[0]);
		resp[3] = 1;	/* unsupported version */
	} else switch(req[1]) {
	case 0:	/* Public address request */
		syslog(LOG_INFO, "NAT-PMP public address request");
		FillPublicAddressResponse(resp, senderaddr->sin_addr.s_addr);
		resplen = 12;
		break;
	case 1:	/* UDP port mapping request */
	case 2:	/* TCP port mapping request */
		{
			unsigned short iport;	/* private port */
			unsigned short eport;	/* public port */
			uint32_t lifetime; 		/* lifetime=0 => remove port mapping */
			int r;
			int proto;
			char iaddr_old[16];
			unsigned short iport_old;
			unsigned int timestamp;

			iport = READNU16(req+4);
			eport = READNU16(req+6);
			lifetime = READNU32(req+8);
			proto = (req[1]==1)?IPPROTO_UDP:IPPROTO_TCP;
			syslog(LOG_INFO, "NAT-PMP port mapping request : "
			                 "%hu->%s:%hu %s lifetime=%us",
			                 eport, senderaddrstr, iport,
			                 (req[1]==1)?"udp":"tcp", lifetime);
			/* TODO: accept port mapping if iport ok but eport not ok
			 * (and set eport correctly) */
			if(lifetime == 0) {
				/* remove the mapping */
				/* RFC6886 :
				 * A client MAY also send an explicit packet to request deletion of a
				 * mapping that is no longer needed. A client requests explicit
				 * deletion of a mapping by sending a message to the NAT gateway
				 * requesting the mapping, with the Requested Lifetime in Seconds set to
				 * zero. The Suggested External Port MUST be set to zero by the client
				 * on sending, and MUST be ignored by the gateway on reception. */
				int index = 0;
				unsigned short eport2, iport2;
				char iaddr2[16];
				int proto2;
				char desc[64];
				eport = 0; /* to indicate correct removing of port mapping */
				while(get_redirect_rule_by_index(index, 0,
				          &eport2, iaddr2, sizeof(iaddr2),
						  &iport2, &proto2,
						  desc, sizeof(desc),
				          0, 0, &timestamp, 0, 0) >= 0) {
					syslog(LOG_DEBUG, "%d %d %hu->'%s':%hu '%s'",
					       index, proto2, eport2, iaddr2, iport2, desc);
					if(0 == strcmp(iaddr2, senderaddrstr)
					  && 0 == memcmp(desc, "NAT-PMP", 7)) {
						/* (iport == 0) => remove all the mappings for this client */
						if((iport == 0) || ((iport == iport2) && (proto == proto2))) {
							r = _upnp_delete_redir(eport2, proto2);
							if(r < 0) {
								syslog(LOG_ERR, "Failed to remove NAT-PMP mapping eport %hu, protocol %s",
								       eport2, (proto2==IPPROTO_TCP)?"TCP":"UDP");
								resp[3] = 2;	/* Not Authorized/Refused */
								break;
							} else {
								syslog(LOG_INFO, "NAT-PMP %s port %hu mapping removed",
								       proto2==IPPROTO_TCP?"TCP":"UDP", eport2);
								index--;
							}
						}
					}
					index++;
				}
			} else if(iport==0) {
				resp[3] = 2;	/* Not Authorized/Refused */
			} else { /* iport > 0 && lifetime > 0 */
				unsigned short eport_first = 0;
				int any_eport_allowed = 0;
				char desc[64];
				if(eport==0)	/* if no suggested external port, use same a internal port */
					eport = iport;
				while(resp[3] == 0) {
					if(eport_first == 0) { /* first time in loop */
						eport_first = eport;
					} else if(eport == eport_first) { /* no eport available */
						if(any_eport_allowed == 0) { /* all eports rejected by permissions */
							syslog(LOG_ERR, "No allowed eport for NAT-PMP %hu %s->%s:%hu",
							       eport, (proto==IPPROTO_TCP)?"tcp":"udp", senderaddrstr, iport);
							resp[3] = 2;	/* Not Authorized/Refused */
						} else { /* at least one eport allowed (but none available) */
							syslog(LOG_ERR, "Failed to find available eport for NAT-PMP %hu %s->%s:%hu",
							       eport, (proto==IPPROTO_TCP)?"tcp":"udp", senderaddrstr, iport);
							resp[3] = 4;	/* Out of resources */
						}
						break;
					}
					if(!check_upnp_rule_against_permissions(upnppermlist, num_upnpperm, eport, senderaddr->sin_addr, iport, "NAT-PMP")) {
						eport++;
						if(eport == 0) eport++; /* skip port zero */
						continue;
					}
					any_eport_allowed = 1;	/* at lease one eport is allowed */
#ifdef CHECK_PORTINUSE
					if (port_in_use(ext_if_name, eport, proto, senderaddrstr, iport) > 0) {
						syslog(LOG_INFO, "port %hu protocol %s already in use",
						       eport, (proto==IPPROTO_TCP)?"tcp":"udp");
						eport++;
						if(eport == 0) eport++; /* skip port zero */
						continue;
					}
#endif
					r = get_redirect_rule(ext_if_name, eport, proto,
					                      iaddr_old, sizeof(iaddr_old),
					                      &iport_old, 0, 0, 0, 0,
					                      &timestamp, 0, 0);
					if(r==0) {
						if(strcmp(senderaddrstr, iaddr_old)==0
						    && iport==iport_old) {
							/* redirection already existing */
							syslog(LOG_INFO, "port %hu %s already redirected to %s:%hu, replacing",
							       eport, (proto==IPPROTO_TCP)?"tcp":"udp", iaddr_old, iport_old);
							/* remove and then add again */
							if(_upnp_delete_redir(eport, proto) < 0) {
								syslog(LOG_ERR, "failed to remove port mapping");
								break;
							}
						} else {
							eport++;
							if(eport == 0) eport++; /* skip port zero */
							continue;
						}
					}
					/* do the redirection */
					timestamp = upnp_time() + lifetime;
					snprintf(desc, sizeof(desc), "NAT-PMP %hu %s",
					         eport, (proto==IPPROTO_TCP)?"tcp":"udp");
					/* TODO : check return code */
					if(upnp_redirect_internal(NULL, eport, senderaddrstr,
					                          iport, proto, desc,
					                          timestamp) < 0) {
						syslog(LOG_ERR, "Failed to add NAT-PMP %hu %s->%s:%hu '%s'",
						       eport, (proto==IPPROTO_TCP)?"tcp":"udp", senderaddrstr, iport, desc);
						resp[3] = 3;  /* Failure */
					}
					break;
				}
			}
			WRITENU16(resp+8, iport);	/* private port */
			WRITENU16(resp+10, eport);	/* public port */
			WRITENU32(resp+12, lifetime);	/* Port Mapping lifetime */
		}
		resplen = 16;
		break;
	default:
		resp[3] = 5;	/* Unsupported OPCODE */
	}
	n = sendto_or_schedule(s, resp, resplen, 0,
	           (struct sockaddr *)senderaddr, sizeof(*senderaddr));
	if(n<0) {
		syslog(LOG_ERR, "sendto(natpmp): %m");
	} else if(n<resplen) {
		syslog(LOG_ERR, "sendto(natpmp): sent only %d bytes out of %d",
		       n, resplen);
	}
}

/* SendNATPMPPublicAddressChangeNotification()
 * should be called when the public IP address changed */
void SendNATPMPPublicAddressChangeNotification(int * sockets, int n_sockets)
{
	struct sockaddr_in sockname;
	unsigned char notif[12];
	int j, n;

	notif[0] = 0;	/* vers */
	notif[1] = 128;	/* OP code */
	notif[2] = 0;	/* result code */
	notif[3] = 0;	/* result code */
	/* seconds since "start of epoch" :
	 * time elapsed since the port mapping table was initialized on
	 * startup or reset for any other reason */
	if(epoch_origin == 0) {
		epoch_origin = startup_time;
	}
	WRITENU32(notif+4, upnp_time() - epoch_origin);
#ifndef MULTIPLE_EXTERNAL_IP
	FillPublicAddressResponse(notif, 0);
	if(notif[3])
	{
		syslog(LOG_WARNING, "%s: cannot get public IP address, stopping",
		       "SendNATPMPPublicAddressChangeNotification");
		return;
	}
#endif
	memset(&sockname, 0, sizeof(struct sockaddr_in));
    sockname.sin_family = AF_INET;
    sockname.sin_addr.s_addr = inet_addr(NATPMP_NOTIF_ADDR);

	for(j=0; j<n_sockets; j++)
	{
		if(sockets[j] < 0)
			continue;
#ifdef MULTIPLE_EXTERNAL_IP
		{
			struct lan_addr_s * lan_addr = lan_addrs.lh_first;
			int i;
			for(i=0; i<j; i++)
				lan_addr = lan_addr->list.le_next;
			FillPublicAddressResponse(notif, lan_addr->addr.s_addr);
		}
#endif
		/* Port to use in 2006 version of the NAT-PMP specification */
    	sockname.sin_port = htons(NATPMP_PORT);
		n = sendto_or_schedule(sockets[j], notif, 12, 0,
		           (struct sockaddr *)&sockname, sizeof(struct sockaddr_in));
		if(n < 0)
		{
			syslog(LOG_ERR, "%s: sendto(s_udp=%d, port=%d): %m",
			       "SendNATPMPPublicAddressChangeNotification", sockets[j], NATPMP_PORT);
			return;
		}
		/* Port to use in 2008 version of the NAT-PMP specification */
    	sockname.sin_port = htons(NATPMP_NOTIF_PORT);
		n = sendto_or_schedule(sockets[j], notif, 12, 0,
		           (struct sockaddr *)&sockname, sizeof(struct sockaddr_in));
		if(n < 0)
		{
			syslog(LOG_ERR, "%s: sendto(s_udp=%d, port=%d): %m",
			       "SendNATPMPPublicAddressChangeNotification", sockets[j], NATPMP_NOTIF_PORT);
			return;
		}
	}
}

#endif /* ENABLE_NATPMP */
