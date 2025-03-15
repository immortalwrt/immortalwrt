/*
 * MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2009-2012 Jardel Weyrich
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution
 */

#include "ipfwaux.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

int ipfw_exec(int optname, void * optval, uintptr_t optlen) {
	static int sock = -1;
	int result;

	switch (optname) {
		case IP_FW_INIT:
			if (sock == -1)
				sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
			if (sock < 0) {
				syslog(LOG_ERR, "socket(SOCK_RAW): %m");
				return -1;
			}
			break;
		case IP_FW_TERM:
			if (sock != -1)
				close(sock);
			sock = -1;
			break;
		case IP_FW_ADD:
		case IP_FW_DEL:
			result = setsockopt(sock, IPPROTO_IP, optname, optval, optlen);
			if (result == -1) {
				syslog(LOG_ERR, "setsockopt(): %m");
				return -1;
			}
			break;
		case IP_FW_GET:
			result = getsockopt(sock, IPPROTO_IP, optname, optval, (socklen_t *)optlen);
			if (result == -1) {
				syslog(LOG_ERR, "getsockopt(): %m");
				return -1;
			}
			break;
		default:
			syslog(LOG_ERR, "unhandled option");
			return -1;
	}

	return 0;
}

void ipfw_free_ruleset(struct ip_fw ** rules) {
	if (rules == NULL || *rules == NULL)
		return;
	free(*rules);
	*rules = NULL;
}

int ipfw_fetch_ruleset(struct ip_fw ** rules, int * total_fetched, int count) {
	int fetched;
	socklen_t size;

	if (rules == NULL || *total_fetched < 0 || count < 1)
		return -1;

	size = sizeof(struct ip_fw) * (*total_fetched + count);
	*rules = (struct ip_fw *)realloc(*rules, size);
	if (*rules == NULL) {
		syslog(LOG_ERR, "realloc(): %m");
		return -1;
	}

	(*rules)->version = IP_FW_CURRENT_API_VERSION;
	if (ipfw_exec(IP_FW_GET, *rules, (uintptr_t)&size) < 0)
		return -1;
	fetched = *total_fetched;
	*total_fetched = size / sizeof(struct ip_fw);

	return *total_fetched - fetched;
}

int ipfw_validate_protocol(int value) {
	switch (value) {
		case IPPROTO_TCP:
		case IPPROTO_UDP:
			break;
		default:
			syslog(LOG_ERR, "invalid protocol");
			return -1;
	}
	return 0;
}

int ipfw_validate_ifname(const char * const value) {
	int len = strlen(value);
	if (len < 2 || len > FW_IFNLEN) {
		syslog(LOG_ERR, "invalid interface name");
		return -1;
	}
	return 0;
}

