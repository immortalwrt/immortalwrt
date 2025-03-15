/* $Id: getroute.c,v 1.4 2013/02/06 10:50:04 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2015 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/*#include <linux/in_route.h>*/
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#ifdef USE_LIBNFNETLINK
/* define USE_LIBNFNETLINK in order to use libnfnetlink
 * instead of custom code
 * see https://github.com/miniupnp/miniupnp/issues/110 */
#include <libnfnetlink/libnfnetlink.h>
#endif /* USE_LIBNFNETLINK */

#include "../getroute.h"
#include "../upnputils.h"

int
get_src_for_route_to(const struct sockaddr * dst,
                     void * src, size_t * src_len,
                     int * index)
{
	int fd = -1;
	struct nlmsghdr *h;
	int status;
	struct {
		struct nlmsghdr n;
		struct rtmsg r;
		char buf[1024];
	} req;
	struct sockaddr_nl nladdr;
	struct iovec iov = {
		.iov_base = (void*) &req.n,
	};
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};
	const struct sockaddr_in * dst4;
	const struct sockaddr_in6 * dst6;
#ifndef USE_LIBNFNETLINK
	struct rtattr * rta;
#endif /* USE_LIBNFNETLINK */

	memset(&req, 0, sizeof(req));
	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_type = RTM_GETROUTE;
	req.r.rtm_family = dst->sa_family;
	req.r.rtm_table = 0;
	req.r.rtm_protocol = 0;
	req.r.rtm_scope = 0;
	req.r.rtm_type = 0;
	req.r.rtm_src_len = 0;
	req.r.rtm_dst_len = 0;
	req.r.rtm_tos = 0;

	{
		char dst_str[128];
		sockaddr_to_string(dst, dst_str, sizeof(dst_str));
		syslog(LOG_DEBUG, "get_src_for_route_to (%s)", dst_str);
	}
	/* add address */
#ifndef USE_LIBNFNETLINK
	rta = (struct rtattr *)(((char*)&req) + NLMSG_ALIGN(req.n.nlmsg_len));
	rta->rta_type = RTA_DST;
#endif /* USE_LIBNFNETLINK */
	if(dst->sa_family == AF_INET) {
		dst4 = (const struct sockaddr_in *)dst;
#ifdef USE_LIBNFNETLINK
		nfnl_addattr_l(&req.n, sizeof(req), RTA_DST, &dst4->sin_addr, 4);
#else
		rta->rta_len = RTA_SPACE(sizeof(dst4->sin_addr));
		memcpy(RTA_DATA(rta), &dst4->sin_addr, sizeof(dst4->sin_addr));
#endif /* USE_LIBNFNETLINK */
		req.r.rtm_dst_len = 32;
	} else {
		dst6 = (const struct sockaddr_in6 *)dst;
#ifdef USE_LIBNFNETLINK
		nfnl_addattr_l(&req.n, sizeof(req), RTA_DST, &dst6->sin6_addr, 16);
#else
		rta->rta_len = RTA_SPACE(sizeof(dst6->sin6_addr));
		memcpy(RTA_DATA(rta), &dst6->sin6_addr, sizeof(dst6->sin6_addr));
#endif /* USE_LIBNFNETLINK */
		req.r.rtm_dst_len = 128;
	}
#ifndef USE_LIBNFNETLINK
	req.n.nlmsg_len += rta->rta_len;
#endif /* USE_LIBNFNETLINK */

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (fd < 0) {
		syslog(LOG_ERR, "socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE) : %m");
		return -1;
	}

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	req.n.nlmsg_seq = 1;
	iov.iov_len = req.n.nlmsg_len;

	status = sendmsg(fd, &msg, 0);

	if (status < 0) {
		syslog(LOG_ERR, "sendmsg(rtnetlink) : %m");
		goto error;
	}

	memset(&req, 0, sizeof(req));

	for(;;) {
		iov.iov_len = sizeof(req);
		status = recvmsg(fd, &msg, 0);
		if(status < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			syslog(LOG_ERR, "recvmsg(rtnetlink) %m");
			goto error;
		}
		if(status == 0) {
			syslog(LOG_ERR, "recvmsg(rtnetlink) EOF");
			goto error;
		}
		for (h = (struct nlmsghdr*)&req.n; status >= (int)sizeof(*h); ) {
			int len = h->nlmsg_len;
			int l = len - sizeof(*h);

			if (l<0 || len>status) {
				if (msg.msg_flags & MSG_TRUNC) {
					syslog(LOG_ERR, "Truncated message");
				}
				syslog(LOG_ERR, "malformed message: len=%d", len);
				goto error;
			}

			if(nladdr.nl_pid != 0 || h->nlmsg_seq != 1/*seq*/) {
				syslog(LOG_ERR, "wrong seq = %d\n", h->nlmsg_seq);
				/* Don't forget to skip that message. */
				status -= NLMSG_ALIGN(len);
				h = (struct nlmsghdr*)((char*)h + NLMSG_ALIGN(len));
				continue;
			}

			if(h->nlmsg_type == NLMSG_ERROR) {
				struct nlmsgerr *err = (struct nlmsgerr*)NLMSG_DATA(h);
				syslog(LOG_ERR, "NLMSG_ERROR %d : %s", err->error, strerror(-err->error));
				goto error;
			}
			if(h->nlmsg_type == RTM_NEWROUTE) {
				struct rtattr * rta;
				int len = h->nlmsg_len;
				len -= NLMSG_LENGTH(sizeof(struct rtmsg));
				for(rta = RTM_RTA(NLMSG_DATA((h))); RTA_OK(rta, len); rta = RTA_NEXT(rta,len)) {
					unsigned char * data = RTA_DATA(rta);
					if(rta->rta_type == RTA_PREFSRC) {
						if(src_len && src) {
							if(*src_len < RTA_PAYLOAD(rta)) {
								syslog(LOG_WARNING, "cannot copy src: %u<%lu",
								       (unsigned)*src_len, (unsigned long)RTA_PAYLOAD(rta));
								goto error;
							}
							*src_len = RTA_PAYLOAD(rta);
							memcpy(src, data, RTA_PAYLOAD(rta));
						}
					} else if(rta->rta_type == RTA_OIF) {
						if(index)
							memcpy(index, data, sizeof(int));
					}
				}
				close(fd);
				return 0;
			}
			status -= NLMSG_ALIGN(len);
			h = (struct nlmsghdr*)((char*)h + NLMSG_ALIGN(len));
		}
	}
	syslog(LOG_WARNING, "get_src_for_route_to() : src not found");
error:
	if(fd >= 0)
		close(fd);
	return -1;
}

