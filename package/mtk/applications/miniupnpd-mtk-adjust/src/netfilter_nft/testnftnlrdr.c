/* $Id: testnftnlrdr.c,v 1.2 2019/06/30 19:49:18 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2023 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <syslog.h>
/* for PRIu64 */
#include <inttypes.h>

#include <linux/netfilter/nf_tables.h>
#include <libnftnl/table.h>
#include <libnftnl/chain.h>
#include <libnftnl/rule.h>
#include <libnftnl/expr.h>

#include "nftnlrdr.h"
#include "nftnlrdr_misc.h"
#include "../commonrdr.h"

#ifndef PRIu64
#define PRIu64 "llu"
#endif

static int
add_filter_rule(int proto, const char * rhost,
		const char * iaddr, unsigned short iport)
{
	return add_filter_rule2(NULL/* ifname */, rhost, iaddr, 0/* eport */, iport, proto, NULL/* desc */);
}

static int
addnatrule(int proto, unsigned short eport,
	   const char * iaddr, unsigned short iport,
	   const char * rhost)
{
	return add_redirect_rule2(NULL/* ifname */, rhost, eport, iaddr, iport, proto, NULL/* desc */, 0/* timestamp */);
}

int
main(int argc, char ** argv)
{
	unsigned short eport, iport;
	const char * iaddr;

	if(argc<4) {
		printf("Usage %s <ext_port> <internal_ip> <internal_port>\n", argv[0]);
		return -1;
	}
	openlog("testnftnlrdr", LOG_PERROR|LOG_CONS, LOG_LOCAL0);
	if (init_redirect() < 0) {
		fprintf(stderr, "init_redirect() FAILED\n");
		return -1;
	}
	eport = (unsigned short)atoi(argv[1]);
	iaddr = argv[2];
	iport = (unsigned short)atoi(argv[3]);
	printf("trying to redirect port %hu to %s:%hu\n", eport, iaddr, iport);
	if(addnatrule(IPPROTO_TCP, eport, iaddr, iport, NULL) < 0) {
		printf("addnatrule() failed!\n");
		return -1;
	}
	if(add_filter_rule(IPPROTO_TCP, NULL, iaddr, iport) < 0) {
		printf("add_filter_rule() failed!\n");
		return -1;
	}
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
		printf("test0\n");
		if(get_redirect_rule_by_index(0/* index */, NULL/* ifname */, &p1/* eport */,
		                              addr, sizeof(addr), &p2/* iport */,
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
			printf("timestamp=%u desc=\"%s\"\n", timestamp, desc);
		}
		printf("test\n");
	}
	printf("trying to list nat rules :\n");
	print_redirect_rules(argv[1]);
	printf("deleting\n");
	delete_redirect_and_filter_rules(eport, IPPROTO_TCP);
	shutdown_redirect();
	return 0;
}
