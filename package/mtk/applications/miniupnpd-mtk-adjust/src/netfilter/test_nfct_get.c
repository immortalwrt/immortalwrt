#include "nfct_get.c"

int main(int argc, char *argv[])
{
	struct sockaddr_storage src, dst, ext;
	char buff[INET6_ADDRSTRLEN];

	if (argc!=5)
		return 0;

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
