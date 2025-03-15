/* $Id: asyncsendto.c,v 1.12 2020/11/11 12:13:26 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2020 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "asyncsendto.h"
#include "upnputils.h"

enum send_state {ESCHEDULED=1, EWAITREADY=2, ESENDNOW=3} state;

/* state diagram for a packet :
 *
 *                     |
 *                     V
 * -> ESCHEDULED -> ESENDNOW -> sent
 *                    ^  |
 *                    |  V
 *                EWAITREADY -> sent
 */
struct scheduled_send {
	LIST_ENTRY(scheduled_send) entries;
	struct timeval ts;
	enum send_state state;
	int sockfd;
	const void * buf;
	size_t len;
	int flags;
	const struct sockaddr *dest_addr;
	socklen_t addrlen;
	const struct sockaddr_in6 *src_addr;
	char data[];
};

static LIST_HEAD(listhead, scheduled_send) send_list = { NULL };

/*
 * ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
 *                const struct sockaddr *dest_addr, socklen_t addrlen);
 */
static ssize_t
send_from_to(int sockfd, const void *buf, size_t len, int flags,
             const struct sockaddr_in6 *src_addr,
             const struct sockaddr *dest_addr, socklen_t addrlen)
{
#ifdef IPV6_PKTINFO
	if(src_addr) {
		struct iovec iov;
		struct in6_pktinfo ipi6;
		uint8_t c[CMSG_SPACE(sizeof(ipi6))];
		struct msghdr msg;
		struct cmsghdr* cmsg;

		iov.iov_base = (void *)buf;
		iov.iov_len = len;
		memset(&msg, 0, sizeof(msg));
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		ipi6.ipi6_addr = src_addr->sin6_addr;
		ipi6.ipi6_ifindex = src_addr->sin6_scope_id;
		msg.msg_control = c;
		msg.msg_controllen = sizeof(c);
		cmsg = CMSG_FIRSTHDR(&msg);
		cmsg->cmsg_level = IPPROTO_IPV6;
		cmsg->cmsg_type = IPV6_PKTINFO;
		cmsg->cmsg_len = CMSG_LEN(sizeof(ipi6));
		memcpy(CMSG_DATA(cmsg), &ipi6, sizeof(ipi6));
		msg.msg_name = (void *)dest_addr;
		msg.msg_namelen = addrlen;
		return sendmsg(sockfd, &msg, flags);
	} else {
#endif /* IPV6_PKTINFO */
		return sendto(sockfd, buf, len, flags, dest_addr, addrlen);
#ifdef IPV6_PKTINFO
	}
#endif /* IPV6_PKTINFO */
}

/* delay = milli seconds */
ssize_t
sendto_schedule2(int sockfd, const void *buf, size_t len, int flags,
                 const struct sockaddr *dest_addr, socklen_t addrlen,
                 const struct sockaddr_in6 *src_addr,
                 unsigned int delay)
{
	enum send_state state;
	ssize_t n;
	size_t alloc_len;
	struct timeval tv;
	struct scheduled_send * elt;

	if(delay == 0) {
		/* first try to send at once */
		n = send_from_to(sockfd, buf, len, flags, src_addr, dest_addr, addrlen);
		if(n >= 0)
			return n;
		else if(errno == EAGAIN || errno == EWOULDBLOCK) {
			/* use select() on this socket */
			state = EWAITREADY;
		} else if(errno == EINTR) {
			state = ESENDNOW;
		} else {
			/* uncatched error */
			return n;
		}
	} else {
		state = ESCHEDULED;
	}

	/* schedule */
	if(upnp_gettimeofday(&tv) < 0) {
		return -1;
	}
	/* allocate enough space for structure + buffers */
	alloc_len = sizeof(struct scheduled_send) + len + addrlen;
	if(src_addr)
		alloc_len += sizeof(struct sockaddr_in6);
	elt = malloc(alloc_len);
	if(elt == NULL) {
		syslog(LOG_ERR, "malloc failed to allocate %u bytes",
		       (unsigned)alloc_len);
		return -1;
	}
	elt->state = state;
	/* time the packet should be sent */
	elt->ts.tv_sec = tv.tv_sec + (delay / 1000);
	elt->ts.tv_usec = tv.tv_usec + (delay % 1000) * 1000;
	if(elt->ts.tv_usec > 1000000) {
		elt->ts.tv_sec++;
		elt->ts.tv_usec -= 1000000;
	}
	elt->sockfd = sockfd;
	elt->flags = flags;
	memcpy(elt->data, dest_addr, addrlen);
	elt->dest_addr = (struct sockaddr *)elt->data;
	elt->addrlen = addrlen;
	if(src_addr) {
		elt->src_addr = (struct sockaddr_in6 *)(elt->data + addrlen);
		memcpy((void *)elt->src_addr, src_addr, sizeof(struct sockaddr_in6));
		elt->buf = (void *)(elt->data + addrlen + sizeof(struct sockaddr_in6));
	} else {
		elt->src_addr = NULL;
		elt->buf = (void *)(elt->data + addrlen);
	}
	elt->len = len;
	memcpy((void *)elt->buf, buf, len);
	/* insert */
	LIST_INSERT_HEAD( &send_list, elt, entries);
	return 0;
}

/* try to send at once, and queue the packet if needed */
ssize_t
sendto_or_schedule(int sockfd, const void *buf, size_t len, int flags,
                   const struct sockaddr *dest_addr, socklen_t addrlen)
{
	return sendto_schedule2(sockfd, buf, len, flags, dest_addr, addrlen, NULL, 0);
}

ssize_t
sendto_or_schedule2(int sockfd, const void *buf, size_t len, int flags,
                   const struct sockaddr *dest_addr, socklen_t addrlen,
                   const struct sockaddr_in6 *src_addr)
{
	return sendto_schedule2(sockfd, buf, len, flags, dest_addr, addrlen, src_addr, 0);
}

/* get_next_scheduled_send() return number of scheduled send in list */
int get_next_scheduled_send(struct timeval * next_send)
{
	int n = 0;
	struct scheduled_send * elt;
	if(next_send == NULL)
		return -1;
	for(elt = send_list.lh_first; elt != NULL; elt = elt->entries.le_next) {
		if(n == 0 || (elt->ts.tv_sec < next_send->tv_sec) ||
		   (elt->ts.tv_sec == next_send->tv_sec && elt->ts.tv_usec < next_send->tv_usec)) {
			next_send->tv_sec = elt->ts.tv_sec;
			next_send->tv_usec = elt->ts.tv_usec;
		}
		n++;
	}
	return n;
}

/* update writefds for select() call
 * return the number of packets to try to send at once */
int get_sendto_fds(fd_set * writefds, int * max_fd, const struct timeval * now)
{
	int n = 0;
	struct scheduled_send * elt;
	for(elt = send_list.lh_first; elt != NULL; elt = elt->entries.le_next) {
		if(elt->state == EWAITREADY) {
			/* last sendto() call returned EAGAIN/EWOULDBLOCK */
			FD_SET(elt->sockfd, writefds);
			if(elt->sockfd > *max_fd)
				*max_fd = elt->sockfd;
			n++;
		} else if((elt->ts.tv_sec < now->tv_sec) ||
		          (elt->ts.tv_sec == now->tv_sec && elt->ts.tv_usec <= now->tv_usec)) {
			/* we waited long enough, now send ! */
			elt->state = ESENDNOW;
			n++;
		}
	}
	return n;
}

/* executed sendto() when needed */
int try_sendto(fd_set * writefds)
{
	int ret = 0;
	ssize_t n;
	struct scheduled_send * elt;
	struct scheduled_send * next;
	for(elt = send_list.lh_first; elt != NULL; elt = next) {
		next = elt->entries.le_next;
		if((elt->state == ESENDNOW) ||
		   (elt->state == EWAITREADY && FD_ISSET(elt->sockfd, writefds))) {
#ifdef DEBUG
			syslog(LOG_DEBUG, "%s: %d bytes on socket %d",
			       "try_sendto", (int)elt->len, elt->sockfd);
#endif
			n = send_from_to(elt->sockfd, elt->buf, elt->len, elt->flags,
			                 elt->src_addr, elt->dest_addr, elt->addrlen);
			/*n = sendto(elt->sockfd, elt->buf, elt->len, elt->flags,
			           elt->dest_addr, elt->addrlen);*/
			if(n < 0) {
				if(errno == EINTR) {
					/* retry at once */
					elt->state = ESENDNOW;
					continue;
				} else if(errno == EAGAIN || errno == EWOULDBLOCK) {
					/* retry once the socket is ready for writing */
					elt->state = EWAITREADY;
					continue;
				} else {
					char addr_str[64];
					/* uncatched error */
					if(sockaddr_to_string(elt->dest_addr, addr_str, sizeof(addr_str)) <= 0)
						addr_str[0] = '\0';
					syslog(LOG_ERR, "%s(sock=%d, len=%u, dest=%s): sendto: %m",
					       "try_sendto", elt->sockfd, (unsigned)elt->len,
					       addr_str);
					ret--;
				}
			} else if((int)n != (int)elt->len) {
				syslog(LOG_WARNING, "%s: %d bytes sent out of %d",
				       "try_sendto", (int)n, (int)elt->len);
			}
			/* remove from the list */
			LIST_REMOVE(elt, entries);
			free(elt);
		}
	}
	return ret;
}

/* maximum execution time for finalize_sendto() in milliseconds */
#define FINALIZE_SENDTO_DELAY	(500)

/* empty the list */
void finalize_sendto(void)
{
	ssize_t n;
	struct scheduled_send * elt;
	struct scheduled_send * next;
	fd_set writefds;
	struct timeval deadline;
	struct timeval now;
	struct timeval timeout;
	int max_fd;

	if(upnp_gettimeofday(&deadline) < 0) {
		syslog(LOG_ERR, "gettimeofday: %m");
		return;
	}
	deadline.tv_usec += FINALIZE_SENDTO_DELAY*1000;
	if(deadline.tv_usec > 1000000) {
		deadline.tv_sec++;
		deadline.tv_usec -= 1000000;
	}
	while(send_list.lh_first) {
		FD_ZERO(&writefds);
		max_fd = -1;
		for(elt = send_list.lh_first; elt != NULL; elt = next) {
			next = elt->entries.le_next;
			syslog(LOG_DEBUG, "finalize_sendto(): %d bytes on socket %d",
			       (int)elt->len, elt->sockfd);
			n = send_from_to(elt->sockfd, elt->buf, elt->len, elt->flags,
			                 elt->src_addr, elt->dest_addr, elt->addrlen);
			/*n = sendto(elt->sockfd, elt->buf, elt->len, elt->flags,
			           elt->dest_addr, elt->addrlen);*/
			if(n < 0) {
				if(errno==EAGAIN || errno==EWOULDBLOCK) {
					FD_SET(elt->sockfd, &writefds);
					if(elt->sockfd > max_fd)
						max_fd = elt->sockfd;
					continue;
				}
				syslog(LOG_WARNING, "finalize_sendto(): socket=%d sendto: %m", elt->sockfd);
			}
			/* remove from the list */
			LIST_REMOVE(elt, entries);
			free(elt);
		}
		/* check deadline */
		if(upnp_gettimeofday(&now) < 0) {
			syslog(LOG_ERR, "gettimeofday: %m");
			return;
		}
		if(now.tv_sec > deadline.tv_sec ||
		   (now.tv_sec == deadline.tv_sec && now.tv_usec > deadline.tv_usec)) {
			/* deadline ! */
			while((elt = send_list.lh_first) != NULL) {
				LIST_REMOVE(elt, entries);
				free(elt);
			}
			return;
		}
		/* compute timeout value */
		timeout.tv_sec = deadline.tv_sec - now.tv_sec;
		timeout.tv_usec = deadline.tv_usec - now.tv_usec;
		if(timeout.tv_usec < 0) {
			timeout.tv_sec--;
			timeout.tv_usec += 1000000;
		}
		if(max_fd >= 0) {
			if(select(max_fd + 1, NULL, &writefds, NULL, &timeout) < 0) {
				syslog(LOG_ERR, "select: %m");
				return;
			}
		}
	}
}
