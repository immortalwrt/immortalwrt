/* $Id: testminissdp.c,v 1.3 2018/01/15 16:46:48 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2017-2018 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include "config.h"
#include "minissdp.h"
#include "upnpglobalvars.h"

void test(const char * buffer, size_t n)
{
	int s = 0;
	struct sockaddr_in dummy_sender;

	memset(&dummy_sender, 0, sizeof(dummy_sender));
	dummy_sender.sin_family = AF_INET;

	ProcessSSDPData(s, buffer, n,
	                (struct sockaddr *)&dummy_sender, 0,
#ifdef ENABLE_HTTPS
	                80, 443
#else
	                80
#endif
	                );

}

int main(int argc, char * * argv)
{
	FILE * f = NULL;
	char buffer[1500];
	size_t n = 0;
	struct lan_addr_s * lan_addr;

	if((argc > 1) && ((strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0))) {
		fprintf(stderr, "Usage:\t%s [file]\nIf no file is specified, the program is reading the standard input.\n", argv[0]);
		return 1;
	}
	openlog("testminissdp", LOG_CONS|LOG_PERROR, LOG_USER);

	/* populate lan_addrs */
	LIST_INIT(&lan_addrs);
	lan_addr = (struct lan_addr_s *) malloc(sizeof(struct lan_addr_s));
	memset(lan_addr, 0, sizeof(struct lan_addr_s));
	LIST_INSERT_HEAD(&lan_addrs, lan_addr, list);

	if(argc > 1) {
		f = fopen(argv[1], "rb");
		if(f == NULL) {
			syslog(LOG_ERR, "error opening file %s", argv[1]);
			return 1;
		}
	}
	n = fread(buffer, 1, sizeof(buffer), f ? f : stdin);
	if(n <= 0) {
		syslog(LOG_ERR, "error reading");
		return 1;
	}

	test(buffer, n);

	if(f) fclose(f);
	/* free memory */
	while(lan_addrs.lh_first != NULL)
	{
		lan_addr = lan_addrs.lh_first;
		LIST_REMOVE(lan_addrs.lh_first, list);
		free(lan_addr);
	}

	return 0;
}
