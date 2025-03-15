/* $Id: testipfwrdr.c,v 1.7 2011/06/22 21:57:17 nanard Exp $ */
/*
 * MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2009-2011 Jardel Weyrich, Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution
 */

#include <stdio.h>
#include <syslog.h>
#include <time.h>
#include <netinet/in.h>
#include "ipfwrdr.h"

// test program for ipfwrdr.c
static const char * ifname = "lo0";

static void
list_port_mappings(void)
{
	int i;
	unsigned short eport;
	char iaddr[16];
	unsigned short iport;
	int proto;
	char desc[64];
	char rhost[32];
	unsigned int timestamp;
	u_int64_t packets, bytes;

	printf("== Port Mapping List ==\n");
	for(i = 0;; i++) {
		iaddr[0] = '\0';
		desc[0] = '\0';
		eport = iport = 0;
		timestamp = 0;
		packets = bytes = 0;
		proto = -1;
		if(get_redirect_rule_by_index(i, 0/*ifname*/, &eport, iaddr, sizeof(iaddr),
		                              &iport, &proto, desc, sizeof(desc),
		                              rhost, sizeof(rhost),
		                              &timestamp, &packets, &bytes) < 0)
			break;
		printf("%2d - %5hu=>%15s:%5hu %d '%s' '%s' %u %" PRIu64 " %" PRIu64 "\n",
		       i, eport, iaddr, iport, proto, desc, rhost, timestamp,
		       packets, bytes);
	}
	printf("== %d Port Mapping%s ==\n", i, (i > 1)?"s":"");
}

int main(int argc, char * * argv) {
	unsigned int timestamp;
	char desc[64];
	char addr[16];
	char rhost[40];
	unsigned short iport = 0;
	const char * in_rhost = "8.8.8.8";

	desc[0] = '\0';
	addr[0] = '\0';
	openlog("testipfwrdrd", LOG_CONS | LOG_PERROR, LOG_USER);
	if(init_redirect() < 0) {
		fprintf(stderr, "init_redirect() failed.\n");
		return 1;
	}
	list_port_mappings();
	delete_redirect_rule(ifname, 2222, IPPROTO_TCP);
	delete_redirect_rule(ifname, 2223, IPPROTO_TCP);
	add_redirect_rule2(ifname, "", 2223,
	                   "10.1.1.17", 4445, IPPROTO_TCP,
	                   "test miniupnpd", time(NULL) + 60);
	add_redirect_rule2(ifname, in_rhost, 2222,
	                   "10.1.1.16", 4444, IPPROTO_TCP,
	                   "test miniupnpd", time(NULL) + 60);
	get_redirect_rule(ifname, 2222, IPPROTO_TCP, addr, sizeof(addr), &iport,
	                  desc, sizeof(desc), rhost, sizeof(rhost),
	                  &timestamp, NULL, NULL);
	printf("'%s' %s:%hu '%s' %u\n", rhost, addr, iport, desc, timestamp);
	list_port_mappings();
	delete_redirect_rule(ifname, 2222, IPPROTO_TCP);
	delete_redirect_rule(ifname, 2223, IPPROTO_TCP);
	list_port_mappings();
	shutdown_redirect();
	return 0;
}

