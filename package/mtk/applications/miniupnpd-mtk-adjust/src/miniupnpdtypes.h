/* $Id: miniupnpdtypes.h,v 1.4 2012/04/06 15:27:21 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2020 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */
#ifndef MINIUPNPDTYPES_H_INCLUDED
#define MINIUPNPDTYPES_H_INCLUDED

#include "config.h"
#include <netinet/in.h>
#include <net/if.h>
#include <sys/queue.h>

/* structure and list for storing lan addresses
 * with ascii representation and mask */
struct lan_addr_s {
	char ifname[IFNAMSIZ];	/* example: eth0 */
	unsigned int index;		/* use if_nametoindex() */
	char str[16];	/* example: 192.168.0.1 */
	struct in_addr addr, mask;	/* ip/mask */
#ifdef MULTIPLE_EXTERNAL_IP
	char ext_ip_str[16];
	struct in_addr ext_ip_addr;
#else
	unsigned long add_indexes;	/* mask of indexes of associated interfaces */
#endif
	LIST_ENTRY(lan_addr_s) list;
};
LIST_HEAD(lan_addr_list, lan_addr_s);

#endif
