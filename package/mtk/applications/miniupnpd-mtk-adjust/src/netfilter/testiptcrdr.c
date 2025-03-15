/* $Id: testiptcrdr.c,v 1.21 2016/02/12 12:35:50 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2016 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <syslog.h>

#include "iptcrdr.c"
/*#include "../commonrdr.h"*/

#ifndef PRIu64
#define PRIu64 "llu"
#endif

int
main(int argc, char ** argv)
{
	unsigned short eport, iport;
	const char * iaddr;
	int r;
	int proto = IPPROTO_TCP;
	printf("Usage %s <ext_port> <internal_ip> <internal_port> [TCP/UDP]\n", argv[0]);

	if(argc<4)
		return -1;
	openlog("testiptcrdr", LOG_PERROR|LOG_CONS, LOG_LOCAL0);
	if(init_redirect() < 0)
		return -1;
	eport = (unsigned short)atoi(argv[1]);
	iaddr = argv[2];
	iport = (unsigned short)atoi(argv[3]);
	if(argc >= 4) {
		if(strcasecmp(argv[4], "udp") == 0)
			proto = IPPROTO_UDP;
	}
	printf("trying to redirect port %hu to %s:%hu proto %d\n",
	       eport, iaddr, iport, proto);
	if(addnatrule(proto, eport, iaddr, iport, NULL) < 0)
		return -1;
	r = addmasqueraderule(proto, eport, iaddr, iport, NULL);
	syslog(LOG_DEBUG, "addmasqueraderule() returned %d", r);
	if(add_filter_rule(proto, NULL, iaddr, iport) < 0)
		return -1;
#if 0
	/* TEST */
	if(proto == IPPROTO_UDP) {
		if(addpeernatrule(proto, "8.8.8.8"/*eaddr*/, eport, iaddr, iport, NULL, 0) < 0)
			fprintf(stderr, "addpeenatrule failed\n");
	}
#endif
	/*update_portmapping_desc_timestamp(NULL, eport, proto, "updated desc", time(NULL)+42);*/
	update_portmapping(NULL, eport, proto, iport+1, "updated rule", time(NULL)+42);
	/* test */
	{
		unsigned short p1, p2;
		char addr[16];
		int proto2;
		char desc[256];
		char rhost[256];
		unsigned int timestamp;
		u_int64_t packets, bytes;

		desc[0] = '\0';
		if(get_redirect_rule_by_index(0, "", &p1,
		                              addr, sizeof(addr), &p2,
		                              &proto2, desc, sizeof(desc),
		                              rhost, sizeof(rhost),
		                              &timestamp,
									  &packets, &bytes) < 0)
		{
			printf("rule not found\n");
		}
		else
		{
			printf("redirected port %hu to %s:%hu proto %d   packets=%" PRIu64 " bytes=%" PRIu64 " ts=%u desc='%s'\n",
			       p1, addr, p2, proto2, packets, bytes, timestamp, desc);
		}
	}
	printf("trying to list nat rules :\n");
	list_redirect_rule(NULL);
	printf("deleting\n");
	delete_redirect_and_filter_rules(eport, proto);
	shutdown_redirect();
	return 0;
}

