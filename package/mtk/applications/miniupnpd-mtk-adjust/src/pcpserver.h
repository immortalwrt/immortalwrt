/* $Id: pcpserver.h,v 1.3 2014/03/24 10:49:46 nanard Exp $ */
/* MiniUPnP project
 * Website : http://miniupnp.free.fr/
 * Author : Peter Tatrai

Copyright (c) 2013 by Cisco Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * The name of the author may not be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PCPSERVER_H_INCLUDED
#define PCPSERVER_H_INCLUDED

#define PCP_MIN_LEN           24
#define PCP_MAX_LEN           1100

struct sockaddr;

/*
 * receiveraddr is only used for IPV6
 *
 * returns 0 upon success 1 otherwise
 */
int ProcessIncomingPCPPacket(int s, unsigned char *msg_buff, int len,
                             const struct sockaddr *senderaddr,
                             const struct sockaddr_in6 *receiveraddr);

/*
 * returns the socket
 */
int OpenAndConfPCPv6Socket(void);

/*
 * Send Unsolicited ANNOUNCE Message
 */
#ifdef ENABLE_IPV6
void PCPSendUnsolicitedAnnounce(int * sockets, int n_sockets, int socket6);
#else /* IPv4 only */
void PCPSendUnsolicitedAnnounce(int * sockets, int n_sockets);
#endif

/*
 * To be called when Public IP address changed (IPv4)
 */
#ifdef ENABLE_IPV6
void PCPPublicAddressChanged(int * sockets, int n_sockets, int socket6);
#else /* IPV4 Only */
void PCPPublicAddressChanged(int * sockets, int n_sockets);
#endif

#endif /* PCPSERVER_H_INCLUDED */
