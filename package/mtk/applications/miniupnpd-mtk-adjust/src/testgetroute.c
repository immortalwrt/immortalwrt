/* $Id: testgetroute.c,v 1.7 2018/03/13 10:25:52 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2018 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "getroute.h"
#include "upnputils.h"
#include "upnpglobalvars.h"

#ifndef LOG_PERROR
/* solaris does not define LOG_PERROR */
#define LOG_PERROR 0
#endif

struct lan_addr_list lan_addrs;
int runtime_flags = 0;
time_t startup_time = 0;

int
main(int argc, char ** argv)
{
	struct sockaddr_in dst4;
	struct sockaddr_in6 dst6;
	struct sockaddr * dst;
	void * src;
	size_t src_len;
	int r;
	int index = -1;

	memset(&dst4, 0, sizeof(dst4));
	memset(&dst6, 0, sizeof(dst6));
	dst = NULL;
	if(argc < 2) {
		fprintf(stderr, "usage: %s <ip address>\n", argv[0]);
		fprintf(stderr, "both v4 and v6 IP addresses are supported.\n");
		return 1;
	}
	openlog("testgetroute", LOG_CONS|LOG_PERROR, LOG_USER);
	r = inet_pton (AF_INET, argv[1], &dst4.sin_addr);
	if(r < 0) {
		syslog(LOG_ERR, "inet_pton(AF_INET, %s) : %m", argv[1]);
		closelog();
		return 2;
	}
	if (r == 0) {
		r = inet_pton (AF_INET6, argv[1], &dst6.sin6_addr);
		if(r < 0) {
			syslog(LOG_ERR, "inet_pton(AF_INET6, %s) : %m", argv[1]);
			closelog();
			return 2;
		} else if(r > 0) {
			dst6.sin6_family = AF_INET6;
			dst = (struct sockaddr *)&dst6;
			src = &dst6.sin6_addr;
			src_len = sizeof(dst6.sin6_addr);
		} else {
			/* r == 0 */
			syslog(LOG_ERR, "%s is not a correct IPv4 or IPv6 address", argv[1]);
			closelog();
			return 1;
		}
	} else {
		dst4.sin_family = AF_INET;
		dst = (struct sockaddr *)&dst4;
		src = &dst4.sin_addr;
		src_len = sizeof(dst4.sin_addr);
	}

	if (dst) {
		syslog(LOG_DEBUG, "calling get_src_for_route_to(%p, NULL, NULL, %p)",
		       dst, &index);
		r = get_src_for_route_to (dst, NULL, NULL, &index);
		syslog(LOG_DEBUG, "get_src_for_route_to() returned %d", r);
		if(r >= 0) {
			syslog(LOG_DEBUG, "index=%d", index);
		}
		syslog(LOG_DEBUG, "calling get_src_for_route_to(%p, %p, %p(%u), %p)",
		       dst, src, &src_len, (unsigned)src_len, &index);
		r = get_src_for_route_to (dst, src, &src_len, &index);
		syslog(LOG_DEBUG, "get_src_for_route_to() returned %d", r);
		if(r >= 0) {
			char src_str[128];
			sockaddr_to_string(dst, src_str, sizeof(src_str));
			syslog(LOG_DEBUG, "src=%s", src_str);
			syslog(LOG_DEBUG, "index=%d", index);
		}
	}
	closelog();
	return 0;
}

