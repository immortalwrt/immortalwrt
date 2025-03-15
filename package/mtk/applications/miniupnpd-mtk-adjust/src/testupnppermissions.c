/* $Id: testupnppermissions.c,v 1.3 2009/09/14 15:24:46 nanard Exp $ */
/* (c) 2007-2015 Thomas Bernard
 * MiniUPnP Project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 */
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "upnppermissions.h"

void
print_upnpperm(const struct upnpperm * p)
{
	switch(p->type)
	{
	case UPNPPERM_ALLOW:
		printf("allow ");
		break;
	case UPNPPERM_DENY:
		printf("deny ");
		break;
	default:
		printf("error ! ");
	}
	printf("%hu-%hu ", p->eport_min, p->eport_max);
	printf("%s/", inet_ntoa(p->address));
	printf("%s ", inet_ntoa(p->mask));
	printf("%hu-%hu", p->iport_min, p->iport_max);
	putchar('\n');
}

int main(int argc, char * * argv)
{
	int i, r, ret;
	struct upnpperm p;
	if(argc < 2) {
		fprintf(stderr, "Usage:   %s \"permission line\" [...]\n", argv[0]);
		fprintf(stderr, "Example: %s \"allow 1234 10.10.10.10/32 1234\"\n", argv[0]);
		return 1;
	}
	openlog("testupnppermissions", LOG_PERROR, LOG_USER);
	ret = 0;
	for(i=1; i<argc; i++) {
		printf("%2d '%s'\n", i, argv[i]);
		memset(&p, 0, sizeof(struct upnpperm));
		r = read_permission_line(&p, argv[i]);
		if(r==0) {
			printf("Permission read successfully\n");
			print_upnpperm(&p);
		} else {
			printf("Permission read failed, please check its correctness\n");
			ret++;
		}
		putchar('\n');
	}
	return ret;
}

