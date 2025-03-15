/* $Id: testssdppktgen.c,v 1.2 2021/05/21 22:05:17 nanard Exp $ */
#include <stdio.h>
#include <syslog.h>
#include <time.h>
#include "config.h"
#ifdef DYNAMIC_OS_VERSION
#include <string.h>
#include <sys/utsname.h>
#endif

#include "miniupnpdpath.h"
#include "upnphttp.h"
#include "macros.h"

#define SSDP_PORT 1900

const char uuidvalue_igd[] = "uuid:12345678-0000-0000-0000-000000abcd01";
unsigned upnp_bootid;
unsigned upnp_configid;
#ifdef DYNAMIC_OS_VERSION
char * os_version;
#endif

static int
MakeSSDPPacket(const char * dest_str,
               const char * host, unsigned short http_port,
#ifdef ENABLE_HTTPS
               unsigned short https_port,
#endif
               const char * nt, const char * suffix,
               const char * usn1, const char * usn2, const char * usn3,
               unsigned int lifetime)
{
	char bufr[SSDP_PACKET_MAX_LEN];
	int l;

		l = snprintf(bufr, sizeof(bufr),
		"NOTIFY * HTTP/1.1\r\n"
		"HOST: %s:%d\r\n"
		"CACHE-CONTROL: max-age=%u\r\n"
		"LOCATION: http://%s:%u" ROOTDESC_PATH "\r\n"
#ifdef ENABLE_HTTPS
		"SECURELOCATION.UPNP.ORG: https://%s:%u" ROOTDESC_PATH "\r\n"
#endif
		"SERVER: " MINIUPNPD_SERVER_STRING "\r\n"
		"NT: %s%s\r\n"
		"USN: %s%s%s%s\r\n"
		"NTS: ssdp:alive\r\n"
		"OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n" /* UDA v1.1 */
		"01-NLS: %u\r\n" /* same as BOOTID field. UDA v1.1 */
		"BOOTID.UPNP.ORG: %u\r\n" /* UDA v1.1 */
		"CONFIGID.UPNP.ORG: %u\r\n" /* UDA v1.1 */
		"\r\n",
		dest_str, SSDP_PORT,			/* HOST: */
		lifetime,						/* CACHE-CONTROL: */
		host, (unsigned int)http_port,	/* LOCATION: */
#ifdef ENABLE_HTTPS
		host, (unsigned int)https_port,	/* SECURE-LOCATION: */
#endif
#ifdef DYNAMIC_OS_VERSION
		os_version,
#endif
		nt, suffix,						/* NT: */
		usn1, usn2, usn3, suffix,		/* USN: */
		upnp_bootid,					/* 01-NLS: */
		upnp_bootid,					/* BOOTID.UPNP.ORG: */
		upnp_configid );				/* CONFIGID.UPNP.ORG: */
	if(l<0) {
		syslog(LOG_ERR, "%s: snprintf error", "MakeSSDPPacket()");
		return -1;
	} else if((unsigned int)l >= sizeof(bufr)) {
		syslog(LOG_WARNING, "%s: truncated output (%u>=%u)",
		       "MakeSSDPPacket()", (unsigned)l, (unsigned)sizeof(bufr));
		l = sizeof(bufr) - 1;
		return -1;
	}
	syslog(LOG_DEBUG, "%s", bufr);
	return 0;
}


int main(int argc, char * * argv)
{
	int r;
#ifdef DYNAMIC_OS_VERSION
	struct utsname utsname;
#endif
	UNUSED(argc); UNUSED(argv);

	openlog("testssdppktgen", LOG_CONS|LOG_PERROR, LOG_USER);
#ifdef DYNAMIC_OS_VERSION
	if (uname(&utsname) < 0) {
		syslog(LOG_ERR, "uname(): %m");
		os_version = strdup("unknown");
	} else {
		os_version = strdup(utsname.release);
		syslog(LOG_INFO, "OS_VERSION : %s", os_version);
	}
#endif
	upnp_bootid = (unsigned)time(NULL);
	upnp_configid = 1234567890;
	r = MakeSSDPPacket("123.456.789.123", "222.222.222.222", 12345,
#ifdef ENABLE_HTTPS
	                   54321,
#endif /* ENABLE_HTTPS */
	                   "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:", "1",
	                   uuidvalue_igd, "", "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:",
	                   1234567890);
	if(r < 0) return 1;
#ifdef ENABLE_IPV6
	r = MakeSSDPPacket("[1234:5678:abcd:ef00:1234:5678:abcd:ef00]",
	                   "[1000:2000:3000:4000:5000:6000:7000:8000]", 12345,
#ifdef ENABLE_HTTPS
	                   54321,
#endif /* ENABLE_HTTPS */
	                   "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:", "1",
	                   uuidvalue_igd, "", "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:",
	                   1234567890);
	if(r < 0) return 1;
#endif /* ENABLE_IPV6 */
	return 0;
}

