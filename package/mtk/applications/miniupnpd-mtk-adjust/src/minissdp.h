/* $Id: minissdp.h,v 1.12 2014/04/09 07:20:59 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2017 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */
#ifndef MINISSDP_H_INCLUDED
#define MINISSDP_H_INCLUDED

#include "miniupnpdtypes.h"

int
OpenAndConfSSDPReceiveSocket(int ipv6);

int
OpenAndConfSSDPNotifySockets(int * sockets);

#ifdef ENABLE_HTTPS
void
SendSSDPNotifies2(int * sockets,
                  unsigned short http_port,
                  unsigned short https_port,
                  unsigned int lifetime);
#else
void
SendSSDPNotifies2(int * sockets,
                  unsigned short http_port,
                  unsigned int lifetime);
#endif

void
#ifdef ENABLE_HTTPS
ProcessSSDPRequest(int s,
                   unsigned short http_port, unsigned short https_port);
#else
ProcessSSDPRequest(int s, unsigned short http_port);
#endif

#ifdef ENABLE_HTTPS
void
ProcessSSDPData(int s, const char *bufr, int n,
                const struct sockaddr * sendername, int source_if,
                unsigned short http_port, unsigned short https_port);
#else
void
ProcessSSDPData(int s, const char *bufr, int n,
                const struct sockaddr * sendername, int source_if,
                unsigned short http_port);
#endif

int
SendSSDPGoodbye(int * sockets, int n);

int
SubmitServicesToMiniSSDPD(const char * host, unsigned short port);

#endif

