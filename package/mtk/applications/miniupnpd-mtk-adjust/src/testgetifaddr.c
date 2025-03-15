/* $Id: testgetifaddr.c,v 1.7 2013/04/27 15:38:57 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2014 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */
#include <stdio.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "config.h"
#include "getifaddr.h"

int runtime_flags = 0;

#if defined(__sun)
/* solaris 10 does not define LOG_PERROR */
#define LOG_PERROR 0
#endif

int main(int argc, char * * argv) {
	char str_addr[64];
	struct in_addr addr;
	struct in_addr mask;
#ifdef ENABLE_IPV6
	int r;
	char str_addr6[64];
#endif
	if(argc < 2) {
		fprintf(stderr, "Usage:\t%s interface_name\n", argv[0]);
		return 1;
	}

	openlog("testgetifaddr", LOG_CONS|LOG_PERROR, LOG_USER);
	if(getifaddr(argv[1], str_addr, sizeof(str_addr), &addr, &mask) < 0) {
		fprintf(stderr, "Cannot get address for interface %s.\n", argv[1]);
		return 1;
	}
	printf("Interface %s has IP address %s.\n", argv[1], str_addr);
	printf("addr=%s ", inet_ntoa(addr));
	printf("mask=%s\n", inet_ntoa(mask));
#ifdef ENABLE_IPV6
	r = find_ipv6_addr(argv[1], str_addr6, sizeof(str_addr6));
	if(r < 0) {
		fprintf(stderr, "find_ipv6_addr() failed\n");
		return 1;
	} else if(r == 0) {
		printf("Interface %s has no IPv6 address.\n", argv[1]);
	} else {
		printf("Interface %s has IPv6 address %s.\n", argv[1], str_addr6);
	}
#endif
	return 0;
}
