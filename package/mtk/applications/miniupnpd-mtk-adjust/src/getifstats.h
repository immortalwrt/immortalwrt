/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2008 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef GETIFSTATS_H_INCLUDED
#define GETIFSTATS_H_INCLUDED

struct ifdata {
	unsigned long opackets;
	unsigned long ipackets;
	unsigned long obytes;
	unsigned long ibytes;
	unsigned long baudrate;
};

/* getifstats()
 * Fill the ifdata structure with statistics for network interface ifname.
 * Return 0 in case of success, -1 for bad arguments or any error */
int
getifstats(const char * ifname, struct ifdata * data);

#endif

