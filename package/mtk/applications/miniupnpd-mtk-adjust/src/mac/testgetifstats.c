/* $Id: testgetifstats.c,v 1.3 2011/02/20 23:43:41 nanard Exp $ */
/*
 * MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2009 Jardel Weyrich
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution
 */

#include <stdio.h>
#include "../getifstats.h"

int main(int argc, char * * argv) {
	int r;
	struct ifdata data;
	printf("usage: %s if_name\n", argv[0]);
	if (argc < 2)
		return -1;
	r = getifstats(argv[1], &data);
	if (r < 0)
		printf("getifstats() failed\n");
	else {
		printf("ipackets = %10lu   opackets = %10lu\n", data.ipackets, data.opackets);
		printf("ibytes   = %10lu   obytes   = %10lu\n", data.ibytes, data.obytes);
		printf("baudrate = %10lu\n", data.baudrate);
	}
	return 0;
}

