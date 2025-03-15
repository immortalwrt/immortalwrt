/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * author: Gleb Smirnoff <glebius@FreeBSD.org>
 * (c) 2006 Ryan Wagoner
 * (c) 2014 Gleb Smirnoff
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <errno.h>
#include <ifaddrs.h>
#include <string.h>
#include <syslog.h>

#ifdef ENABLE_GETIFSTATS_CACHING
#include <time.h>
#endif

#include "../getifstats.h"
#include "config.h"

int
getifstats(const char *ifname, struct ifdata *data)
{
	static struct ifaddrs *ifap, *ifa;
#ifdef ENABLE_GETIFSTATS_CACHING
	static time_t cache_timestamp;
	time_t current_time;
#endif
	if(!data)
		return -1;
	data->baudrate = 4200000;
	data->opackets = 0;
	data->ipackets = 0;
	data->obytes = 0;
	data->ibytes = 0;
	if(!ifname || ifname[0]=='\0')
		return -1;

#ifdef ENABLE_GETIFSTATS_CACHING
	current_time = upnp_time();
	if (ifap != NULL &&
	    current_time < cache_timestamp + GETIFSTATS_CACHING_DURATION)
		goto copy;
#endif

	if (ifap != NULL) {
		freeifaddrs(ifap);
		ifap = NULL;
	}

	if (getifaddrs(&ifap) != 0) {
		syslog (LOG_ERR, "getifstats() : getifaddrs(): %s",
		    strerror(errno));
		return (-1);
	}

	for (ifa = ifap; ifa; ifa = ifa->ifa_next)
		if (ifa->ifa_addr->sa_family == AF_LINK &&
		    strcmp(ifa->ifa_name, ifname) == 0) {
#ifdef ENABLE_GETIFSTATS_CACHING
			cache_timestamp = current_time;
copy:
#endif
#define	IFA_STAT(s)	(((struct if_data *)ifa->ifa_data)->ifi_ ## s)   
			data->opackets = IFA_STAT(opackets);
			data->ipackets = IFA_STAT(ipackets);
			data->obytes = IFA_STAT(obytes);
			data->ibytes = IFA_STAT(ibytes);
			data->baudrate = IFA_STAT(baudrate);
			return (0);
		}

	return (-1);
}
