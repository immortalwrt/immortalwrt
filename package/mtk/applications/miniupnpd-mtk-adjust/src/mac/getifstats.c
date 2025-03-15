/* $Id: getifstats.c,v 1.9 2020/05/10 22:25:45 nanard Exp $ */
/*
 * MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2009 Jardel Weyrich
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution
 */

#include <syslog.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_types.h>
#include <net/route.h>
#include <nlist.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "../getifstats.h"

int getifstats(const char * ifname, struct ifdata * data) {
	int mib[] = { CTL_NET, PF_ROUTE, 0, AF_INET, NET_RT_IFLIST, if_nametoindex(ifname) };
	const size_t mib_len = sizeof(mib) / sizeof(mib[0]);
	size_t needed;
	char *buf, *end, *p;
	struct if_msghdr *ifm;
	struct if_data ifdata;
#ifdef ENABLE_GETIFSTATS_CACHING
	static time_t cache_timestamp = 0;
	static struct ifdata cache_data;
	time_t current_time;
#endif

	if (data == NULL || ifname == NULL || ifname[0] == '\0')
		return -1; /* error */

	data->baudrate = 4200000;
	data->opackets = 0;
	data->ipackets = 0;
	data->obytes = 0;
	data->ibytes = 0;

#ifdef ENABLE_GETIFSTATS_CACHING
	current_time = upnp_time();
	if (current_time == ((time_t)-1)) {
		syslog(LOG_ERR, "getifstats() : time() error : %m");
	} else {
		if (current_time < cache_timestamp + GETIFSTATS_CACHING_DURATION) {
			memcpy(data, &cache_data, sizeof(struct ifdata));
			return 0;
		}
	}
#endif

	if (sysctl(mib, mib_len, NULL, &needed, NULL, 0) == -1) {
		syslog(LOG_ERR, "sysctl(): %m");
		return -1;
	}
	buf = (char *) malloc(needed);
	if (buf == NULL)
		return -1; /* error */
	if (sysctl(mib, mib_len, buf, &needed, NULL, 0) == -1) {
		syslog(LOG_ERR, "sysctl(): %m");
		free(buf);
		return -1; /* error */
	} else {
		for (end = buf + needed, p = buf; p < end; p += ifm->ifm_msglen) {
			ifm = (struct if_msghdr *) p;
			if (ifm->ifm_type == RTM_IFINFO && ifm->ifm_data.ifi_type == IFT_ETHER) {
				ifdata = ifm->ifm_data;
				data->opackets = ifdata.ifi_opackets;
				data->ipackets = ifdata.ifi_ipackets;
				data->obytes = ifdata.ifi_obytes;
				data->ibytes = ifdata.ifi_ibytes;
				data->baudrate = ifdata.ifi_baudrate;
				free(buf);
#ifdef ENABLE_GETIFSTATS_CACHING
				if (current_time!=((time_t)-1)) {
					cache_timestamp = current_time;
					memcpy(&cache_data, data, sizeof(struct ifdata));
				}
#endif
				return 0; /* found, ok */
			}
		}
	}
	free(buf);
	return -1; /* not found or error */
}

