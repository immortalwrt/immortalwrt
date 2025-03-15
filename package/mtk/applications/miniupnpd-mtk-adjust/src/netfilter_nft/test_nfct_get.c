/* $Id: test_nfct_get.c,v 1.2 2019/06/30 19:49:18 nanard Exp $ */
#include <stdio.h>
#include <syslog.h>
#include "nfct_get.c"

int main(int argc, char *argv[])
{
	struct sockaddr_storage src, dst, ext;
	char buff[INET6_ADDRSTRLEN];

	if (argc!=5) {
		fprintf(stderr, "Usage: %s SRC_IP SRC_PORT DST_IP DST_PORT\n", argv[0]);
		return 1;
	}

	openlog("test_nfct_get", LOG_PERROR|LOG_CONS, LOG_LOCAL0);

	if (1 != inet_pton(AF_INET, argv[1],
				&((struct sockaddr_in*)&src)->sin_addr)) {
		if (1 != inet_pton(AF_INET6, argv[1],
					&((struct sockaddr_in6*) &src)->sin6_addr)) {
			perror("bad input param");
		} else {
			((struct sockaddr_in6*)(&src))->sin6_port = htons(atoi(argv[2]));
			src.ss_family = AF_INET6;
		}
	} else {
		((struct sockaddr_in*)(&src))->sin_port = htons(atoi(argv[2]));
		src.ss_family = AF_INET;
	}

	if (1 != inet_pton(AF_INET, argv[3],
				&((struct sockaddr_in*)&dst)->sin_addr)) {
		if (1 != inet_pton(AF_INET6, argv[3],
					&((struct sockaddr_in6*) &dst)->sin6_addr)) {
			perror("bad input param");
		} else {
			((struct sockaddr_in6*)(&dst))->sin6_port = htons(atoi(argv[4]));
			dst.ss_family = AF_INET6;
		}
	} else {
		((struct sockaddr_in*)(&dst))->sin_port = htons(atoi(argv[4]));
		dst.ss_family = AF_INET;
	}

	if (get_nat_ext_addr((struct sockaddr*)&src, (struct sockaddr*)&dst,
			IPPROTO_TCP, &ext)) {
		printf("Ext address %s:%d\n",
			inet_ntop(src.ss_family,
				&((struct sockaddr_in*)&ext)->sin_addr,
				buff, sizeof(buff)),
			ntohs(((struct sockaddr_in*)(&ext))->sin_port));
	} else {
		printf("no entry\n");
	}
	return 0;
}
