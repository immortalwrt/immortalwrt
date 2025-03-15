/* $Id: natpmp.h,v 1.12 2014/03/24 10:49:46 nanard Exp $ */
/* MiniUPnP project
 * author : Thomas Bernard
 * website : http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 */
#ifndef NATPMP_H_INCLUDED
#define NATPMP_H_INCLUDED

/* The NAT-PMP specification which can be found at the url :
 * http://files.dns-sd.org/draft-cheshire-nat-pmp.txt
 * draft version 3 of April 2008
 * define 5351 as listening port for the gateway,
 * and the 224.0.0.1 port 5350 as the local link
 * multicast address for address change announces.
 * Previous versions of the specification defined 5351
 * as the port for address change announces. */
#define NATPMP_PORT (5351)
#define NATPMP_NOTIF_PORT	(5350)
#define NATPMP_NOTIF_ADDR	("224.0.0.1")

int OpenAndConfNATPMPSockets(int * sockets);

/* receiveraddr is only used with IPV6 sockets */
int ReceiveNATPMPOrPCPPacket(int s, struct sockaddr * senderaddr,
                             socklen_t * senderaddrlen,
                             struct sockaddr_in6 * receiveraddr,
                             unsigned char * msg_buff, size_t msg_buff_size);

void ProcessIncomingNATPMPPacket(int s, unsigned char * msg_buff, int len,
                                 struct sockaddr_in * senderaddr);

void SendNATPMPPublicAddressChangeNotification(int * sockets, int n_sockets);

#endif

