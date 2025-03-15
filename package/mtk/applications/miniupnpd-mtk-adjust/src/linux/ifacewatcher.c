/* $Id: ifacewatcher.c,v 1.10 2019/10/02 22:02:02 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2020 Thomas Bernard
 *
 * ifacewatcher.c
 *
 * This file implements dynamic serving of new network interfaces
 * which weren't available during daemon start. It also takes care
 * of interfaces which become unavailable.
 *
 * Copyright (c) 2011, Alexey Osipov <simba@lerlan.ru>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    * Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *    * The name of the author may not be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE. */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "config.h"
#include "../macros.h"

#ifdef USE_IFACEWATCHER

#include "../ifacewatcher.h"
#include "../minissdp.h"
#include "../getifaddr.h"
#include "../upnpglobalvars.h"
#include "../natpmp.h"

extern volatile sig_atomic_t should_send_public_address_change_notif;


int
OpenAndConfInterfaceWatchSocket(void)
{
	int s;
	struct sockaddr_nl addr;

	s = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (s == -1)
	{
		syslog(LOG_ERR, "socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE): %m");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;
	/*addr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR;*/

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		syslog(LOG_ERR, "bind(netlink): %m");
		close(s);
		return -1;
	}

	return s;
}

#if 0
/* disabled at the moment */
int
ProcessInterfaceUp(struct ifinfomsg *ifi)
{
	struct lan_iface_s * lan_iface;
	struct lan_iface_s * lan_iface2;
	struct lan_addr_s * lan_addr;
	char ifname[IFNAMSIZ];
	char ifstraddr[16];
	struct in_addr ifaddr;

	/* check if we already have this iface */
	for(lan_iface = lan_ifaces.lh_first; lan_iface != NULL; lan_iface = lan_iface->list.le_next)
		if (lan_iface->iface.index == ifi->ifi_index)
			break;

	if (lan_iface != NULL)
		return 0;

	if (if_indextoname(ifi->ifi_index, ifname) == NULL)
	{
		syslog(LOG_ERR, "if_indextoname(%d, ifname) failed", ifi->ifi_index);
		return -1;
	}

	if (getifaddr(ifname, ifstraddr, 16) < 0)
	{
		syslog(LOG_DEBUG, "getifaddr(%s, ifaddr, 16) failed", ifname);
		return 1;
	}

	if (inet_pton(AF_INET, ifstraddr, &ifaddr) != 1)
	{
		syslog(LOG_ERR, "inet_pton(AF_INET, \"%s\", &ifaddr) failed", ifstraddr);
		return -1;
	}

	/* check if this new interface has address which we need to listen to */
	for(lan_addr = lan_addrs.lh_first; lan_addr != NULL; lan_addr = lan_addr->list.le_next)
	{
		if (lan_addr->addr.s_addr != ifaddr.s_addr)
			continue;

		syslog(LOG_INFO, "Interface up: %s (%s)", ifname, ifstraddr);

		/* adding new lan_iface entry */
		lan_iface = (struct lan_iface_s *) malloc(sizeof(struct lan_iface_s));
		if (lan_iface == NULL)
		{
			syslog(LOG_ERR, "malloc(sizeof(struct lan_iface_s): %m");
			continue;
		}

		lan_iface->lan_addr = lan_addr;
		strncpy(lan_iface->iface.name, ifname, IFNAMSIZ);
		lan_iface->iface.index = ifi->ifi_index;
		lan_iface->iface.addr = ifaddr;
		lan_iface->snotify = -1;
#ifdef ENABLE_NATPMP
		lan_iface->snatpmp = -1;
#endif

		LIST_INSERT_HEAD(&lan_ifaces, lan_iface, list);

		/* adding multicast membership for SSDP */
		if(AddMulticastMembership(sudp, ifaddr.s_addr, ifi->ifi_index) < 0)
			syslog(LOG_WARNING, "Failed to add multicast membership for interface %s (%s)", ifname, ifstraddr);
		else
			syslog(LOG_INFO, "Multicast membership added for %s (%s)", ifname, ifstraddr);

		/* create SSDP notify socket */
		if (OpenAndConfSSDPNotifySocket(lan_iface) < 0)
			syslog(LOG_WARNING, "Failed to open SSDP notify socket for interface %s (%s)", ifname, ifstraddr);

#ifdef ENABLE_NATPMP
		/* create NAT-PMP socket */
		for(lan_iface2 = lan_ifaces.lh_first; lan_iface2 != NULL; lan_iface2 = lan_iface2->list.le_next)
			if (lan_iface2->lan_addr->addr.s_addr == lan_iface->lan_addr->addr.s_addr &&
			    lan_iface2->snatpmp >= 0)
				lan_iface->snatpmp = lan_iface2->snatpmp;

		if (lan_iface->snatpmp < 0)
		{
			lan_iface->snatpmp = OpenAndConfNATPMPSocket(ifaddr.s_addr);
			if (lan_iface->snatpmp < 0)
				syslog(LOG_ERR, "OpenAndConfNATPMPSocket(ifaddr.s_addr) failed for %s (%s)", ifname, ifstraddr);
			else
				syslog(LOG_INFO, "Listening for NAT-PMP connection on %s:%d", ifstraddr, NATPMP_PORT);
		}
#endif
	}

	return 0;
}

int
ProcessInterfaceDown(struct ifinfomsg *ifi)
{
	struct lan_iface_s * lan_iface;
	struct lan_iface_s * lan_iface2;

	/* check if we have this iface */
	for(lan_iface = lan_ifaces.lh_first; lan_iface != NULL; lan_iface = lan_iface->list.le_next)
		if (lan_iface->iface.index == ifi->ifi_index)
			break;

	if (lan_iface == NULL)
		return 0;

	/* one of our interfaces is going down, clean up */
	syslog(LOG_INFO, "Interface down: %s", lan_iface->iface.name);

	/* remove multicast membership for SSDP */
	if(DropMulticastMembership(sudp, lan_iface->lan_addr->addr.s_addr, lan_iface->iface.index) < 0)
		syslog(LOG_WARNING, "Failed to drop multicast membership for interface %s (%s)", lan_iface->iface.name, lan_iface->lan_addr->str);
	else
		syslog(LOG_INFO, "Multicast membership dropped for %s (%s)", lan_iface->iface.name, lan_iface->lan_addr->str);

	/* closing SSDP notify socket */
	close(lan_iface->snotify);

#ifdef ENABLE_NATPMP
	/* closing NAT-PMP socket if it's not used anymore */
	for(lan_iface2 = lan_ifaces.lh_first; lan_iface2 != NULL; lan_iface2 = lan_iface2->list.le_next)
		if (lan_iface2 != lan_iface && lan_iface2->snatpmp == lan_iface->snatpmp)
			break;

	if (lan_iface2 == NULL)
		close(lan_iface->snatpmp);
#endif

	/* del corresponding lan_iface entry */
	LIST_REMOVE(lan_iface, list);
	free(lan_iface);

	return 0;
}
#endif

void
ProcessInterfaceWatchNotify(int s)
{
	char buffer[4096];
	struct iovec iov;
	struct msghdr hdr;
	struct nlmsghdr *nlhdr;
#if 0
/* disabled at the moment */
	struct ifinfomsg *ifi;
#endif
	struct ifaddrmsg *ifa;
	int len;

	struct rtattr *rth;
	int rtl;

	unsigned int ext_if_name_index = 0;

	iov.iov_base = buffer;
	iov.iov_len = sizeof(buffer);

	memset(&hdr, 0, sizeof(hdr));
	hdr.msg_iov = &iov;
	hdr.msg_iovlen = 1;

	len = recvmsg(s, &hdr, 0);
	if (len < 0)
	{
		syslog(LOG_ERR, "recvmsg(s, &hdr, 0): %m");
		return;
	}

	if(ext_if_name) {
		ext_if_name_index = if_nametoindex(ext_if_name);
	}

	for (nlhdr = (struct nlmsghdr *) buffer;
	     NLMSG_OK (nlhdr, (unsigned int)len);
	     nlhdr = NLMSG_NEXT (nlhdr, len))
	{
		int is_del = 0;
		char address[48];
		char ifname[IFNAMSIZ];
		address[0] = '\0';
		ifname[0] = '\0';
		if (nlhdr->nlmsg_type == NLMSG_DONE)
			break;
		switch(nlhdr->nlmsg_type) {
		case RTM_DELLINK:
			is_del = 1;
			FALL_THROUGH;
		case RTM_NEWLINK:
#if 0
/* disabled at the moment */
			ifi = (struct ifinfomsg *) NLMSG_DATA(nlhdr);
			if(is_del) {
				if(ProcessInterfaceDown(ifi) < 0)
					syslog(LOG_ERR, "ProcessInterfaceDown(ifi) failed");
			} else {
				if(ProcessInterfaceUp(ifi) < 0)
					syslog(LOG_ERR, "ProcessInterfaceUp(ifi) failed");
			}
#endif
			break;
		case RTM_DELADDR:
			is_del = 1;
			FALL_THROUGH;
		case RTM_NEWADDR:
			/* see /usr/include/linux/netlink.h
			 * and /usr/include/linux/rtnetlink.h */
			ifa = (struct ifaddrmsg *) NLMSG_DATA(nlhdr);
			syslog(LOG_DEBUG, "%s %s index=%d fam=%d", "ProcessInterfaceWatchNotify",
			       is_del ? "RTM_DELADDR" : "RTM_NEWADDR",
			       ifa->ifa_index, ifa->ifa_family);
			for(rth = IFA_RTA(ifa), rtl = IFA_PAYLOAD(nlhdr);
			    rtl && RTA_OK(rth, rtl);
			    rth = RTA_NEXT(rth, rtl)) {
				char tmp[128];
				memset(tmp, 0, sizeof(tmp));
				switch(rth->rta_type) {
				case IFA_ADDRESS:
				case IFA_LOCAL:
				case IFA_BROADCAST:
				case IFA_ANYCAST:
					inet_ntop(ifa->ifa_family, RTA_DATA(rth), tmp, sizeof(tmp));
					if(rth->rta_type == IFA_ADDRESS)
						strncpy(address, tmp, sizeof(address));
					break;
				case IFA_LABEL:
					strncpy(tmp, RTA_DATA(rth), sizeof(tmp));
					strncpy(ifname, tmp, sizeof(ifname));
					break;
				case IFA_CACHEINFO:
					{
						struct ifa_cacheinfo *cache_info;
						cache_info = RTA_DATA(rth);
						snprintf(tmp, sizeof(tmp), "valid=%u preferred=%u",
						         cache_info->ifa_valid, cache_info->ifa_prefered);
					}
					break;
				default:
					strncpy(tmp, "*unknown*", sizeof(tmp));
				}
				syslog(LOG_DEBUG, " - %u - %s type=%d",
				       ifa->ifa_index, tmp,
				       rth->rta_type);
			}
			if(ifa->ifa_index == ext_if_name_index) {
				should_send_public_address_change_notif = 1;
			}
			break;
		default:
			syslog(LOG_DEBUG, "%s type %d ignored",
			       "ProcessInterfaceWatchNotify", nlhdr->nlmsg_type);
		}
	}

}

#endif
