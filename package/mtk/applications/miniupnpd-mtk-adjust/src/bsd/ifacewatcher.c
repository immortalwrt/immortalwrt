/* $Id: ifacewatcher.c,v 1.9 2020/05/10 22:23:56 nanard Exp $ */
/* Project MiniUPnP
 * web : http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2011-2020 Thomas BERNARD
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/route.h>
#include <syslog.h>
#include <signal.h>

#define	SALIGN	(sizeof(long) - 1)
#define	SA_RLEN(sa)	(SA_LEN(sa) ? ((SA_LEN(sa) + SALIGN) & ~SALIGN) : (SALIGN + 1))

#include "config.h"

#include "../upnputils.h"
#include "../upnpglobalvars.h"

extern volatile sig_atomic_t should_send_public_address_change_notif;

int
OpenAndConfInterfaceWatchSocket(void)
{
	int s;

	/*s = socket(PF_ROUTE, SOCK_RAW, AF_INET);*/
	s = socket(PF_ROUTE, SOCK_RAW, AF_UNSPEC);
/* The family parameter may be AF_UNSPEC which will provide routing informa-
 * tion for all address families, or can be restricted to a specific address
 * family by specifying which one is desired.  There can be more than one
 * routing socket open per system. */
	if(s < 0) {
		syslog(LOG_ERR, "OpenAndConfInterfaceWatchSocket socket: %m");
	}
	return s;
}

void
ProcessInterfaceWatchNotify(int s)
{
	char buf[4096];
	ssize_t len;
	char tmp[64];
	struct rt_msghdr * rtm;
	struct if_msghdr * ifm;
	struct ifa_msghdr * ifam;
#ifdef RTM_IFANNOUNCE
	struct if_announcemsghdr * ifanm;
#endif
	char * p;
	struct sockaddr * sa;
	unsigned int ext_if_name_index = 0;

	len = recv(s, buf, sizeof(buf), 0);
	if(len < 0) {
		syslog(LOG_ERR, "ProcessInterfaceWatchNotify recv: %m");
		return;
	}
	if(ext_if_name) {
		ext_if_name_index = if_nametoindex(ext_if_name);
	}
	rtm = (struct rt_msghdr *)buf;
	syslog(LOG_DEBUG, "%u rt_msg : msglen=%d version=%d type=%d", (unsigned)len,
	       rtm->rtm_msglen, rtm->rtm_version, rtm->rtm_type);
	switch(rtm->rtm_type) {
	case RTM_IFINFO:	/* iface going up/down etc. */
		ifm = (struct if_msghdr *)buf;
		syslog(LOG_DEBUG, " RTM_IFINFO: addrs=%x flags=%x index=%hu",
		       ifm->ifm_addrs, ifm->ifm_flags, ifm->ifm_index);
		break;
	case RTM_ADD:	/* Add Route */
		syslog(LOG_DEBUG, " RTM_ADD");
		break;
	case RTM_DELETE:	/* Delete Route */
		syslog(LOG_DEBUG, " RTM_DELETE");
		break;
	case RTM_CHANGE:	/* Change Metrics or flags */
		syslog(LOG_DEBUG, " RTM_CHANGE");
		break;
	case RTM_GET:	/* Report Metrics */
		syslog(LOG_DEBUG, " RTM_GET");
		break;
#ifdef RTM_IFANNOUNCE
	case RTM_IFANNOUNCE:	/* iface arrival/departure */
		ifanm = (struct if_announcemsghdr *)buf;
		syslog(LOG_DEBUG, " RTM_IFANNOUNCE: index=%hu what=%hu ifname=%s",
		       ifanm->ifan_index, ifanm->ifan_what, ifanm->ifan_name);
		break;
#endif
#ifdef RTM_IEEE80211
	case RTM_IEEE80211:	/* IEEE80211 wireless event */
		syslog(LOG_DEBUG, " RTM_IEEE80211");
		break;
#endif
	case RTM_NEWADDR:	/* address being added to iface */
		ifam = (struct ifa_msghdr *)buf;
		syslog(LOG_DEBUG, " RTM_NEWADDR: addrs=%x flags=%x index=%hu",
		       ifam->ifam_addrs, ifam->ifam_flags, ifam->ifam_index);
		p = buf + sizeof(struct ifa_msghdr);
		while(p < buf + len) {
			sa = (struct sockaddr *)p;
			sockaddr_to_string(sa, tmp, sizeof(tmp));
			syslog(LOG_DEBUG, "  %s", tmp);
			p += SA_RLEN(sa);
		}
		if(ifam->ifam_index == ext_if_name_index) {
			should_send_public_address_change_notif = 1;
		}
		break;
	case RTM_DELADDR:	/* address being removed from iface */
		ifam = (struct ifa_msghdr *)buf;
		if(ifam->ifam_index == ext_if_name_index) {
			should_send_public_address_change_notif = 1;
		}
		break;
	default:
		syslog(LOG_DEBUG, "unprocessed RTM message type=%d", rtm->rtm_type);
	}
}

