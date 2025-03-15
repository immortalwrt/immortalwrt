/* $Id: getconnstatus.c,v 1.6 2013/03/23 10:46:54 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2011-2013 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "getconnstatus.h"
#include "getifaddr.h"

#define STATUS_UNCONFIGURED (0)
#define STATUS_CONNECTING (1)
#define STATUS_CONNECTED (2)
#define STATUS_PENDINGDISCONNECT (3)
#define STATUS_DISCONNECTING (4)
#define STATUS_DISCONNECTED (5)

/**
 * get the connection status
 * return values :
 *  0 - Unconfigured
 *  1 - Connecting
 *  2 - Connected
 *  3 - PendingDisconnect
 *  4 - Disconnecting
 *  5 - Disconnected */
int
get_wan_connection_status(const char * ifname)
{
	char addr[INET_ADDRSTRLEN];
	int r;

	/* we need a better implementation here.
	 * I'm afraid it should be device specific */
	r = getifaddr(ifname, addr, INET_ADDRSTRLEN, NULL, NULL);
	return (r < 0) ? STATUS_DISCONNECTED : STATUS_CONNECTED;
}

/**
 * return the same value as get_wan_connection_status()
 * as a C string */
const char *
get_wan_connection_status_str(const char * ifname)
{
	int status;
	const char * str = NULL;

	status = get_wan_connection_status(ifname);
	switch(status) {
	case 0:
		str = "Unconfigured";
		break;
	case 1:
		str = "Connecting";
		break;
	case 2:
		str = "Connected";
		break;
	case 3:
		str = "PendingDisconnect";
		break;
	case 4:
		str = "Disconnecting";
		break;
	case 5:
		str = "Disconnected";
		break;
	}
	return str;
}

