/* $Id: testgetifstats.c,v 1.5 2012/03/05 20:36:17 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "getifstats.h"

#if defined(__sun)
/* solaris 10 does not define LOG_PERROR */
#define LOG_PERROR 0
#endif

int
main(int argc, char **argv)
{
	int r;
	struct ifdata data;
	if(argc<2)
	{
		fprintf(stderr, "usage : %s <ifname>\n", argv[0]);
		return 1;
	}
	openlog("testgetifstats", LOG_CONS|LOG_PERROR, LOG_USER);
	memset(&data, 0, sizeof(data));
	r = getifstats(argv[1], &data);
	printf("getifstats() returned %d\n", r);
	printf("stats for interface %s :\n", argv[1]);
	printf("bitrate = %lu\n", data.baudrate);
	printf(" input packets : %9lu\t input bytes : %9lu\n",
	       data.ipackets, data.ibytes);
	printf("output packets : %9lu\toutput bytes : %9lu\n",
	       data.opackets, data.obytes);
	return 0;
}

