/* $Id: testiptpinhole.c,v 1.1 2012/04/26 13:50:48 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2012 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <syslog.h>

#include "../config.h"
#include "iptpinhole.h"
#include "../commonrdr.h"


int main(int argc, char * * argv)
{
	int uid;

	openlog("testiptpinhole", LOG_PERROR|LOG_CONS, LOG_LOCAL0);

	uid = add_pinhole("eth0", NULL, 0, "ff::123", 54321, IPPROTO_TCP);
	return 0;
}

