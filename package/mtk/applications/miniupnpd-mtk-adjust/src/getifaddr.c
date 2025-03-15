/* $Id: getifaddr.c,v 1.28 2022/02/19 18:58:25 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2024 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#if defined(sun)
#include <sys/sockio.h>
#endif

#include "config.h"
#include "getifaddr.h"
#if defined(USE_GETIFADDRS) || defined(ENABLE_IPV6) || defined(ENABLE_PCP)
#include <ifaddrs.h>
#endif

int
getifaddr(const char * ifname, char * buf, int len,
          struct in_addr * addr, struct in_addr * mask)
{
#ifndef USE_GETIFADDRS
	/* use ioctl SIOCGIFADDR. Works only for ip v4 */
	/* SIOCGIFADDR struct ifreq *  */
	int s;
	struct ifreq ifr;
	int ifrlen;
	struct sockaddr_in * ifaddr;
	ifrlen = sizeof(ifr);

	if(!ifname || ifname[0]=='\0')
		return -1;
	s = socket(PF_INET, SOCK_DGRAM, 0);
	if(s < 0)
	{
		syslog(LOG_ERR, "socket(PF_INET, SOCK_DGRAM): %m");
		return -1;
	}
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
	ifr.ifr_name[IFNAMSIZ-1] = '\0';
	if(ioctl(s, SIOCGIFFLAGS, &ifr, &ifrlen) < 0)
	{
		syslog(LOG_DEBUG, "ioctl(s, SIOCGIFFLAGS, ...): %m");
		close(s);
		return -1;
	}
	if ((ifr.ifr_flags & IFF_UP) == 0)
	{
		syslog(LOG_DEBUG, "network interface %s is down", ifname);
		close(s);
		return -1;
	}
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
	ifr.ifr_name[IFNAMSIZ-1] = '\0';
	if(ioctl(s, SIOCGIFADDR, &ifr, &ifrlen) < 0)
	{
		syslog(LOG_ERR, "ioctl(s, SIOCGIFADDR, ...): %m");
		close(s);
		return -1;
	}
	ifaddr = (struct sockaddr_in *)&ifr.ifr_addr;
	if(addr) *addr = ifaddr->sin_addr;
	if(buf)
	{
		if(!inet_ntop(AF_INET, &ifaddr->sin_addr, buf, len))
		{
			syslog(LOG_ERR, "inet_ntop(): %m");
			close(s);
			return -1;
		}
	}
	if(mask)
	{
		strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
		ifr.ifr_name[IFNAMSIZ-1] = '\0';
		if(ioctl(s, SIOCGIFNETMASK, &ifr, &ifrlen) < 0)
		{
			syslog(LOG_ERR, "ioctl(s, SIOCGIFNETMASK, ...): %m");
			close(s);
			return -1;
		}
#ifdef ifr_netmask
		*mask = ((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr;
#else
		*mask = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
#endif
	}
	close(s);
#else /* ifndef USE_GETIFADDRS */
	/* Works for all address families (both ip v4 and ip v6) */
	struct ifaddrs * ifap;
	struct ifaddrs * ife;
	struct ifaddrs * candidate = NULL;

	if(!ifname || ifname[0]=='\0')
		return -1;
	if(getifaddrs(&ifap)<0)
	{
		syslog(LOG_ERR, "getifaddrs: %m");
		return -1;
	}
	for(ife = ifap; ife; ife = ife->ifa_next)
	{
		/* skip other interfaces if one was specified */
		if(ifname && (0 != strcmp(ifname, ife->ifa_name)))
			continue;
		if(ife->ifa_addr == NULL)
			continue;
		switch(ife->ifa_addr->sa_family)
		{
		case AF_INET:
			/* only consider the address if it is the 1st candidate or
			   if it is not privante AND the current candidate is a private address.
			   So we return a private address only if there is no public address
			   on this interface */
			if(!candidate ||
			   (addr_is_reserved(&((struct sockaddr_in *)candidate->ifa_addr)->sin_addr) &&
			    !addr_is_reserved(&((struct sockaddr_in *)ife->ifa_addr)->sin_addr)))
				candidate = ife;
			break;
/*
		case AF_INET6:
			inet_ntop(ife->ifa_addr->sa_family,
			          &((struct sockaddr_in6 *)ife->ifa_addr)->sin6_addr,
			          buf, len);
*/
		}
	}
	if(candidate)
	{
		if(buf)
		{
			inet_ntop(candidate->ifa_addr->sa_family,
			          &((struct sockaddr_in *)candidate->ifa_addr)->sin_addr,
			          buf, len);
		}
		if(addr) *addr = ((struct sockaddr_in *)candidate->ifa_addr)->sin_addr;
		if(mask) *mask = ((struct sockaddr_in *)candidate->ifa_netmask)->sin_addr;
	}
	else
	{
		syslog(LOG_WARNING, "no AF_INET address found for %s", ifname);
		freeifaddrs(ifap);
		return -1;
	}
	freeifaddrs(ifap);
#endif
	return 0;
}

#ifdef ENABLE_PCP

int getifaddr_in6(const char * ifname, int af, struct in6_addr * addr)
{
#if defined(ENABLE_IPV6) || defined(USE_GETIFADDRS)
	struct ifaddrs * ifap;
	struct ifaddrs * ife;
#ifdef ENABLE_IPV6
	const struct sockaddr_in6 * tmpaddr;
#endif /* ENABLE_IPV6 */
	int found = 0;

	if(!ifname || ifname[0]=='\0')
		return -1;
	if(getifaddrs(&ifap)<0)
	{
		syslog(LOG_ERR, "getifaddrs: %m");
		return -1;
	}
	for(ife = ifap; ife && !found; ife = ife->ifa_next)
	{
		/* skip other interfaces if one was specified */
		if(ifname && (0 != strcmp(ifname, ife->ifa_name)))
			continue;
		if(ife->ifa_addr == NULL)
			continue;
		if (ife->ifa_addr->sa_family != af)
			continue;
		switch(ife->ifa_addr->sa_family)
		{
		case AF_INET:
			/* IPv4-mapped IPv6 address ::ffff:1.2.3.4 */
			memset(addr->s6_addr, 0, 10);
			addr->s6_addr[10] = 0xff;
			addr->s6_addr[11] = 0xff;
			memcpy(addr->s6_addr + 12,
			       &(((struct sockaddr_in *)ife->ifa_addr)->sin_addr.s_addr),
			       4);
			found = 1;
			break;

#ifdef ENABLE_IPV6
		case AF_INET6:
			tmpaddr = (const struct sockaddr_in6 *)ife->ifa_addr;
			if(!IN6_IS_ADDR_LOOPBACK(&tmpaddr->sin6_addr)
			   && !IN6_IS_ADDR_LINKLOCAL(&tmpaddr->sin6_addr))
			{
				memcpy(addr->s6_addr,
				       &tmpaddr->sin6_addr,
				       16);
				found = 1;
			}
			break;
#endif /* ENABLE_IPV6 */
		}
	}
	freeifaddrs(ifap);
	return (found ? 0 : -1);
#else /* defined(ENABLE_IPV6) || defined(USE_GETIFADDRS) */
	/* IPv4 only */
	struct in_addr addr4;
	if(af != AF_INET)
		return -1;
	if(getifaddr(ifname, NULL, 0, &addr4, NULL) < 0)
		return -1;
	/* IPv4-mapped IPv6 address ::ffff:1.2.3.4 */
	memset(addr->s6_addr, 0, 10);
	addr->s6_addr[10] = 0xff;
	addr->s6_addr[11] = 0xff;
	memcpy(addr->s6_addr + 12, &addr4.s_addr, 4);
	return 0;
#endif
}
#endif /* ENABLE_PCP */

#ifdef ENABLE_IPV6
int
find_ipv6_addr(const char * ifname,
               char * dst, int n)
{
	struct ifaddrs * ifap;
	struct ifaddrs * ife;
	const struct sockaddr_in6 * addr;
	char buf[64];
	int r = 0;

	if(!dst)
		return -1;

	if(getifaddrs(&ifap)<0)
	{
		syslog(LOG_ERR, "getifaddrs: %m");
		return -1;
	}
	for(ife = ifap; ife; ife = ife->ifa_next)
	{
		/* skip other interfaces if one was specified */
		if(ifname && (0 != strcmp(ifname, ife->ifa_name)))
			continue;
		if(ife->ifa_addr == NULL)
			continue;
		if(ife->ifa_addr->sa_family == AF_INET6)
		{
			addr = (const struct sockaddr_in6 *)ife->ifa_addr;
			if(!IN6_IS_ADDR_LOOPBACK(&addr->sin6_addr)
			   && !IN6_IS_ADDR_LINKLOCAL(&addr->sin6_addr)
			   /* RFC4193 "Unique Local IPv6 Unicast Addresses" only if no
			    * other address found */
			   && (r == 0 || (addr->sin6_addr.s6_addr[0] & 0xfe) != 0xfc))
			{
				inet_ntop(ife->ifa_addr->sa_family,
				          &addr->sin6_addr,
				          buf, sizeof(buf));
				/* add brackets */
				snprintf(dst, n, "[%s]", buf);
				r = 1;
			}
		}
	}
	freeifaddrs(ifap);
	return r;
}
#endif

/* List of IP address blocks which are private / reserved and therefore not suitable for public external IP addresses */
/* If interface has IP address from one of this block, then it is either behind NAT or port forwarding is impossible */
#define IP(a, b, c, d) (((a) << 24) + ((b) << 16) + ((c) << 8) + (d))
#define MSK(m) (32-(m))
static const struct { uint32_t address; uint32_t rmask; } reserved[] = {
	{ IP(  0,   0,   0, 0), MSK( 8) }, /* RFC1122 "This host on this network" */
	{ IP( 10,   0,   0, 0), MSK( 8) }, /* RFC1918 Private-Use */
	{ IP(100,  64,   0, 0), MSK(10) }, /* RFC6598 Shared Address Space */
	{ IP(127,   0,   0, 0), MSK( 8) }, /* RFC1122 Loopback */
	{ IP(169, 254,   0, 0), MSK(16) }, /* RFC3927 Link-Local */
	{ IP(172,  16,   0, 0), MSK(12) }, /* RFC1918 Private-Use */
	{ IP(192,   0,   0, 0), MSK(24) }, /* RFC6890 IETF Protocol Assignments */
	{ IP(192,   0,   2, 0), MSK(24) }, /* RFC5737 Documentation (TEST-NET-1) */
	{ IP(192,  31, 196, 0), MSK(24) }, /* RFC7535 AS112-v4 */
	{ IP(192,  52, 193, 0), MSK(24) }, /* RFC7450 AMT */
	{ IP(192,  88,  99, 0), MSK(24) }, /* RFC7526 6to4 Relay Anycast */
	{ IP(192, 168,   0, 0), MSK(16) }, /* RFC1918 Private-Use */
	{ IP(192, 175,  48, 0), MSK(24) }, /* RFC7534 Direct Delegation AS112 Service */
	{ IP(198,  18,   0, 0), MSK(15) }, /* RFC2544 Benchmarking */
	{ IP(198,  51, 100, 0), MSK(24) }, /* RFC5737 Documentation (TEST-NET-2) */
	{ IP(203,   0, 113, 0), MSK(24) }, /* RFC5737 Documentation (TEST-NET-3) */
	{ IP(224,   0,   0, 0), MSK( 4) }, /* RFC1112 Multicast */
	{ IP(240,   0,   0, 0), MSK( 4) }, /* RFC1112 Reserved for Future Use + RFC919 Limited Broadcast */
};
#undef IP
#undef MSK

int
addr_is_reserved(struct in_addr * addr)
{
	uint32_t address = ntohl(addr->s_addr);
	size_t i;

	for (i = 0; i < sizeof(reserved)/sizeof(reserved[0]); ++i) {
		if ((address >> reserved[i].rmask) == (reserved[i].address >> reserved[i].rmask))
			return 1;
	}

	return 0;
}
