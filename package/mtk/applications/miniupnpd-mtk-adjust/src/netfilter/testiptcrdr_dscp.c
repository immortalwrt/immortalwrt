/* $Id: testiptcrdr.c,v 1.18 2012/04/24 22:41:53 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2012 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <syslog.h>

#include "iptcrdr.h"
#include "../commonrdr.h"
#include "iptcrdr.c"

#ifndef PRIu64
#define PRIu64 "llu"
#endif

int
main(int argc, char ** argv)
{
	unsigned char dscp;
	unsigned short iport, rport;
	const char * iaddr, *rhost;
	printf("Usage %s <dscp> <internal_ip> <internal_port> <peer_ip> <peer_port>\n", argv[0]);

	if(argc<6)
		return -1;
	openlog("testiptcrdr_peer", LOG_PERROR|LOG_CONS, LOG_LOCAL0);
	dscp = (unsigned short)atoi(argv[1]);
	iaddr = argv[2];
	iport = (unsigned short)atoi(argv[3]);
	rhost = argv[4];
	rport = (unsigned short)atoi(argv[5]);
#if 1
	if(addpeerdscprule(IPPROTO_TCP, dscp, iaddr, iport, rhost, rport) < 0)
		return -1;
#endif
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
			printf("redirected port %hu to %s:%hu proto %d   packets=%" PRIu64 " bytes=%" PRIu64 "\n",
			       p1, addr, p2, proto2, packets, bytes);
		}
	}
	printf("trying to list nat rules :\n");
	list_redirect_rule(argv[1]);
	printf("deleting\n");
//	delete_redirect_and_filter_rules(eport, IPPROTO_TCP);
	return 0;
}

