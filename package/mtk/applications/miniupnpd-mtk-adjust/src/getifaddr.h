/* $Id: getifaddr.h,v 1.11 2018/07/06 11:47:29 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2018 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef GETIFADDR_H_INCLUDED
#define GETIFADDR_H_INCLUDED

struct in_addr;
struct in6_addr;

/* getifaddr()
 * take a network interface name and write the
 * ip v4 address as text in the buffer
 * returns: 0 success, -1 failure */
int
getifaddr(const char * ifname, char * buf, int len,
          struct in_addr * addr, struct in_addr * mask);

int
getifaddr_in6(const char * ifname, int af, struct in6_addr* addr);

/* find a non link local IP v6 address for the interface.
 * if ifname is NULL, look for all interfaces */
int
find_ipv6_addr(const char * ifname,
               char * dst, int n);

/* check if address is in private / reserved block (e.g. local area network) */
int
addr_is_reserved(struct in_addr * addr);

#endif
