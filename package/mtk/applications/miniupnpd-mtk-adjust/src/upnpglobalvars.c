/* $Id: upnpglobalvars.c,v 1.47 2021/05/21 22:03:38 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2021 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <sys/types.h>
#include <netinet/in.h>

#include "config.h"
#include "upnpglobalvars.h"
#include "upnpdescstrings.h"

/* network interface for internet */
const char * ext_if_name = 0;

#ifdef ENABLE_IPV6
/* network interface for internet - IPv6 */
const char * ext_if_name6 = 0;
#endif

/* stun host/port configuration */
const char * ext_stun_host = 0;
uint16_t ext_stun_port = 0;

/* file to store leases */
#ifdef ENABLE_LEASEFILE
const char* lease_file = 0;
#ifdef ENABLE_UPNPPINHOLE
const char* lease_file6 = 0;
#endif
#endif

/* forced ip address to use for this interface
 * when NULL, getifaddr() is used */
const char * use_ext_ip_addr = 0;

/* disallow all port forwarding requests when
 * we are behind restrictive nat */
int disable_port_forwarding = 0;

unsigned long downstream_bitrate = 0;
unsigned long upstream_bitrate = 0;

/* startup time */
time_t startup_time = 0;

#if defined(ENABLE_NATPMP) || defined(ENABLE_PCP)
/* origin for "epoch time" sent into NATPMP and PCP responses */
time_t epoch_origin = 0;
#endif /*  defined(ENABLE_NATPMP) || defined(ENABLE_PCP) */

#ifdef ENABLE_PCP
/* for PCP */
unsigned long int min_lifetime = 120;
unsigned long int max_lifetime = 86400;
#endif

int runtime_flags = 0;

const char * pidfilename = "/var/run/miniupnpd.pid";

char uuidvalue_igd[] = "uuid:00000000-0000-0000-0000-000000000000";
char uuidvalue_wan[] = "uuid:00000000-0000-0000-0000-000000000000";
char uuidvalue_wcd[] = "uuid:00000000-0000-0000-0000-000000000000";
char serialnumber[SERIALNUMBER_MAX_LEN] = "00000000";

char modelnumber[MODELNUMBER_MAX_LEN] = "1";

/* presentation url :
 * http://nnn.nnn.nnn.nnn:ppppp/  => max 30 bytes including terminating 0 */
char presentationurl[PRESENTATIONURL_MAX_LEN];

#ifdef ENABLE_MANUFACTURER_INFO_CONFIGURATION
/* friendly name for root devices in XML description */
char friendly_name[FRIENDLY_NAME_MAX_LEN] = OS_NAME " router";

/* manufacturer name for root devices in XML description */
char manufacturer_name[MANUFACTURER_NAME_MAX_LEN] = ROOTDEV_MANUFACTURER;

/* manufacturer url for root devices in XML description */
char manufacturer_url[MANUFACTURER_URL_MAX_LEN] = ROOTDEV_MANUFACTURERURL;

/* model name for root devices in XML description */
char model_name[MODEL_NAME_MAX_LEN] = ROOTDEV_MODELNAME;

/* model description for root devices in XML description */
char model_description[MODEL_DESCRIPTION_MAX_LEN] = ROOTDEV_MODELDESCRIPTION;

/* model url for root devices in XML description */
char model_url[MODEL_URL_MAX_LEN] = ROOTDEV_MODELURL;
#endif

/* UPnP permission rules : */
struct upnpperm * upnppermlist = 0;
unsigned int num_upnpperm = 0;

#ifdef PCP_SADSCP
struct dscp_values* dscp_values_list = 0;
unsigned int num_dscp_values = 0;
#endif /*PCP_SADSCP*/

/* For automatic removal of expired rules (with LeaseDuration) */
unsigned int nextruletoclean_timestamp = 0;

#ifdef USE_PF
/* "rdr-anchor miniupnpd" or/and "anchor miniupnpd" in pf.conf */
const char * anchor_name = "miniupnpd";
const char * queue = 0;
const char * tag = 0;
#endif

#ifdef ENABLE_NFQUEUE
int nfqueue = -1;
int n_nfqix = 0;
unsigned nfqix[MAX_LAN_ADDR];
#endif /* ENABLE_NFQUEUE */

struct lan_addr_list lan_addrs;

#ifdef ENABLE_IPV6
/* ipv6 address used for HTTP */
char ipv6_addr_for_http_with_brackets[64];

/* address used to bind local services */
struct in6_addr ipv6_bind_addr;
#endif

/* Path of the Unix socket used to communicate with MiniSSDPd */
const char * minissdpdsocketpath = "/var/run/minissdpd.sock";

/* BOOTID.UPNP.ORG and CONFIGID.UPNP.ORG */
/* See UPnP Device Architecture v1.1 section 1.2 Advertisement :
 * The field value of the BOOTID.UPNP.ORG header field MUST be increased
 * each time a device (re)joins the network and sends an initial announce
 * (a "reboot" in UPnP terms), or adds a UPnP-enabled interface.
 * Unless the device explicitly announces a change in the BOOTID.UPNP.ORG
 * field value using an SSDP message, as long as the device remains
 * continuously available in the network, the same BOOTID.UPNP.ORG field
 * value MUST be used in all repeat announcements, search responses,
 * update messages and eventually bye-bye messages. */
unsigned int upnp_bootid = 1;      /* BOOTID.UPNP.ORG */
/* The field value of the CONFIGID.UPNP.ORG header field identifies the
 * current set of device and service descriptions; control points can
 * parse this header field to detect whether they need to send new
 * description query messages. */
/* UPnP 1.1 devices MAY freely assign configid numbers from 0 to
 * 16777215 (2^24-1). Higher numbers are reserved for future use, and
 * can be assigned by the Technical Committee. The configuration of a
 * root device consists of the following information: the DDD of the
 * root device and all its embedded devices, and the SCPDs of all the
 * contained services. If any part of the configuration changes, the
 * CONFIGID.UPNP.ORG field value MUST be changed.
 * DDD = Device Description Document
 * SCPD = Service Control Protocol Description */
unsigned int upnp_configid = 1337; /* CONFIGID.UPNP.ORG */

#ifdef RANDOMIZE_URLS
char random_url[RANDOM_URL_MAX_LEN] = "random";
#endif /* RANDOMIZE_URLS */

#ifdef DYNAMIC_OS_VERSION
char * os_version = NULL;
#endif /* DYNAMIC_OS_VERSION */
