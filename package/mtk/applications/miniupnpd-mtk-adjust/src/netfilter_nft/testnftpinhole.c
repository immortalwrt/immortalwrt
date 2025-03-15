/* $Id: testnftpinhole.c,v 1.4 2024/03/19 23:35:54 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2012-2024 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <syslog.h>

#include "config.h"
#include "../miniupnpdtypes.h"
#include "nftpinhole.h"
#include "../commonrdr.h"
#include "../upnputils.h"

struct lan_addr_list lan_addrs;
time_t startup_time = 0;

static void print_infos(unsigned short uid)
{
	char rem_host[64];
	unsigned short rem_port;
	char int_client[64];
	unsigned short int_port;
	int proto;
	char desc[256];
	unsigned int timestamp;
	u_int64_t packets, bytes;
	int ret;
	ret = get_pinhole_info(uid,
	                       rem_host, sizeof(rem_host), &rem_port,
	                       int_client, sizeof(int_client), &int_port,
	                       &proto, desc, sizeof(desc), &timestamp,
	                       &packets, &bytes);
	if (ret < 0) {
		syslog(LOG_WARNING, "get_pinhole_info(%d) returned %d", uid, ret);
	} else {
		syslog(LOG_INFO, "get_pinhole_info(%d) : %s:%hu => %s:%hu %s",
		       uid, rem_host, rem_port, int_client, int_port, proto==IPPROTO_TCP ? "tcp" : "udp");
		syslog(LOG_INFO, " desc \"%s\" ts=%u packets=%llu %llu",
		       desc, timestamp, (long long unsigned)packets, (long long unsigned)bytes);
	}
}

int main(int argc, char * * argv)
{
	int uid, r;
	const char * ifname = "eth0";
	const char * rem_host = "2a00::dead:beaf";
	unsigned short rem_port = 1911;
	const char * int_client = "fe80::1023:4095";
	unsigned short int_port = 34952;
	char desc[1024] = { 0 };
	unsigned int timestamp = 0;

	openlog("testnftpinhole", LOG_PERROR|LOG_CONS, LOG_LOCAL0);

	r = init_redirect();
	if (r < 0) {
		syslog(LOG_ERR, "init_redirect() failed");
		return 1;
	}

	uid = add_pinhole(ifname, rem_host, rem_port, int_client, int_port, IPPROTO_TCP,
	                  "dummy description", upnp_time() + 60 /* timestamp */);
	syslog(LOG_INFO, "add_pinhole(): uid=%d", uid);
	if (uid >= 0) print_infos(uid);

	uid = find_pinhole(ifname, rem_host, rem_port, int_client, int_port, IPPROTO_TCP,
	                   desc, sizeof(desc), &timestamp);
	syslog(LOG_INFO, "find_pinhole(): uid=%d desc=\"%s\" timestamp=%u", uid, desc, timestamp);

	if (uid >= 0) {
		print_infos(uid);

		r = update_pinhole(uid, upnp_time() + 3600);
		syslog(LOG_INFO, "update_pinhole(%d, ...) returned %d", uid, r);
		print_infos(uid);

		r = delete_pinhole(uid);
		syslog(LOG_INFO, "delete_pinhole(%d) returned %d", uid, r);
	}

	shutdown_redirect();

	return 0;
}

