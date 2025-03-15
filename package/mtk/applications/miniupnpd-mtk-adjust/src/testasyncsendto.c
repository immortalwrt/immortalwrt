/* $Id: testasyncsendto.c,v 1.6 2019/09/24 11:48:47 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2019 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>

#include "miniupnpdtypes.h"
#include "upnputils.h"
#include "asyncsendto.h"

struct lan_addr_list lan_addrs;
int runtime_flags = 0;
time_t startup_time = 0;

#define DEST_IP "239.255.255.250"
#define DEST_PORT 1900
/*
ssize_t
sendto_schedule(int sockfd, const void *buf, size_t len, int flags,
                const struct sockaddr *dest_addr, socklen_t addrlen,
                unsigned int delay)
*/

int test(void)
{
	int s;
	ssize_t n;
	int i;
	struct sockaddr_in addr;
	struct sockaddr_in dest_addr;
	struct timeval next_send;
	if( (s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		syslog(LOG_ERR, "socket(): %m");
		return 1;
	}
	set_non_blocking(s);
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		syslog(LOG_ERR, "bind(): %m");
		close(s);
		return 1;
	}
	memset(&dest_addr, 0, sizeof(struct sockaddr_in));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = inet_addr(DEST_IP);
	dest_addr.sin_port =  htons(DEST_PORT);
	n = sendto_or_schedule(s, "1234", 4, 0,
	                       (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	syslog(LOG_DEBUG, "sendto_or_schedule : %d", (int)n);
	n = sendto_schedule(s, "1234", 4, 0,
	                    (struct sockaddr *)&dest_addr, sizeof(dest_addr),
	                    4400);
	syslog(LOG_DEBUG, "sendto_schedule : %d", (int)n);
	n = sendto_schedule(s, "1234", 4, 0,
	                    (struct sockaddr *)&dest_addr, sizeof(dest_addr),
	                    3000);
	syslog(LOG_DEBUG, "sendto_schedule : %d", (int)n);
	while ((i = get_next_scheduled_send(&next_send)) > 0) {
		fd_set writefds;
		int max_fd;
		struct timeval timeout;
		struct timeval now;
		syslog(LOG_DEBUG, "get_next_scheduled_send : %d next_send=%lld.%06ld",
		       i, (long long)next_send.tv_sec, (long)next_send.tv_usec);
		FD_ZERO(&writefds);
		max_fd = 0;
		gettimeofday(&now, NULL);
		i = get_sendto_fds(&writefds, &max_fd, &now);
		if(now.tv_sec > next_send.tv_sec ||
		   (now.tv_sec == next_send.tv_sec && now.tv_usec >= next_send.tv_usec)) {
			if(i > 0) {
				/* don't wait */
				timeout.tv_sec = 0;
			} else {
				/* wait 10sec :) */
				timeout.tv_sec = 10;
			}
			timeout.tv_usec = 0;
		} else {
			/* ... */
			timeout.tv_sec = (next_send.tv_sec - now.tv_sec);
			timeout.tv_usec = (next_send.tv_usec - now.tv_usec);
			if(timeout.tv_usec < 0) {
				timeout.tv_usec += 1000000;
				timeout.tv_sec--;
			}
		}
		syslog(LOG_DEBUG, "get_sendto_fds() returned %d", i);
		syslog(LOG_DEBUG, "select(%d, NULL, xx, NULL, %lld.%06ld)",
		       max_fd, (long long)timeout.tv_sec, (long)timeout.tv_usec);
		i = select(max_fd, NULL, &writefds, NULL, &timeout);
		if(i < 0) {
			syslog(LOG_ERR, "select: %m");
			if(errno != EINTR)
				break;
		} else if(try_sendto(&writefds) < 0) {
			syslog(LOG_ERR, "try_sendto: %m");
			break;
		}
	}
	close(s);
	return 0;
}

int main(int argc, char * * argv)
{
	int r;
	(void)argc;
	(void)argv;
	openlog("testasyncsendto", LOG_CONS|LOG_PERROR, LOG_USER);
	r = test();
	closelog();
	return r;
}

