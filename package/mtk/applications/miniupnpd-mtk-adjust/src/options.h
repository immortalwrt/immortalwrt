/* $Id: options.h,v 1.36 2024/06/22 18:13:33 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * author: Ryan Wagoner
 * (c) 2006-2024 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef OPTIONS_H_INCLUDED
#define OPTIONS_H_INCLUDED

#include "config.h"

#ifndef DISABLE_CONFIG_FILE
/* enum of option available in the miniupnpd.conf */
enum upnpconfigoptions {
	UPNP_INVALID = 0,
	UPNPEXT_IFNAME = 1,		/* ext_ifname */
#ifdef ENABLE_IPV6
	UPNPEXT_IFNAME6,		/* ext_ifname6 */
#endif
	UPNPEXT_IP,				/* ext_ip */
	UPNPEXT_PERFORM_STUN,		/* ext_perform_stun */
	UPNPEXT_STUN_HOST,		/* ext_stun_host */
	UPNPEXT_STUN_PORT,		/* ext_stun_port */
	UPNPLISTENING_IP,		/* listening_ip */
#ifdef ENABLE_IPV6
	UPNPIPV6_LISTENING_IP,		/* listening address for IPv6 */
	UPNPIPV6_DISABLE,		/* ipv6_disable */
#endif /* ENABLE_IPV6 */
	UPNPPORT,				/* "port" / "http_port" */
#ifdef ENABLE_HTTPS
	UPNPHTTPSPORT,			/* "https_port" */
#endif
	UPNPBITRATE_UP,			/* "bitrate_up" */
	UPNPBITRATE_DOWN,		/* "bitrate_down" */
	UPNPPRESENTATIONURL,	/* presentation_url */
#ifdef ENABLE_MANUFACTURER_INFO_CONFIGURATION
	UPNPFRIENDLY_NAME,		/* "friendly_name" */
	UPNPMANUFACTURER_NAME,	/* "manufacturer_name" */
	UPNPMANUFACTURER_URL,	/* "manufacturer_url" */
	UPNPMODEL_NAME,	/* "model_name" */
	UPNPMODEL_DESCRIPTION,	/* "model_description" */
	UPNPMODEL_URL,	/* "model_url" */
#endif
	UPNPNOTIFY_INTERVAL,	/* notify_interval */
	UPNPSYSTEM_UPTIME,		/* "system_uptime" */
	UPNPPACKET_LOG,			/* "packet_log" */
	UPNPUUID,				/* uuid */
	UPNPSERIAL,				/* serial */
	UPNPMODEL_NUMBER,		/* model_number */
	UPNPCLEANTHRESHOLD,		/* clean_ruleset_threshold */
	UPNPCLEANINTERVAL,		/* clean_ruleset_interval */
	UPNPENABLENATPMP,		/* enable_pcp_pmp */
	UPNPPCPMINLIFETIME,		/* minimum lifetime for PCP mapping */
	UPNPPCPMAXLIFETIME,		/* maximum lifetime for PCP mapping */
	UPNPPCPALLOWTHIRDPARTY,		/* allow third-party requests */
#ifdef USE_NETFILTER
	UPNPTABLENAME,
	UPNPNATTABLENAME,
	UPNPFORWARDCHAIN,
	UPNPNATCHAIN,
	UPNPNATPOSTCHAIN,
	UPNPNFFAMILYSPLIT,
#endif
#ifdef USE_PF
	UPNPANCHOR,				/* anchor */
	UPNPQUEUE,				/* queue */
	UPNPTAG,				/* tag */
#endif
#ifdef PF_ENABLE_FILTER_RULES
	UPNPQUICKRULES,			/* quickrules */
#endif
	UPNPSECUREMODE,			/* secure_mode */
#ifdef ENABLE_LEASEFILE
	UPNPLEASEFILE,			/* lease_file */
#ifdef ENABLE_UPNPPINHOLE
	UPNPLEASEFILE6,			/* lease_file v6 */
#endif
#endif
	UPNPMINISSDPDSOCKET,	/* minissdpdsocket */
#ifdef IGD_V2
	UPNPFORCEIGDDESCV1,
#endif
	UPNPENABLE				/* enable_upnp */
};

/* readoptionsfile()
 * parse and store the option file values
 * returns: 0 success, -1 failure */
int
readoptionsfile(const char * fname, int debug_flag);

/* freeoptions()
 * frees memory allocated to option values */
void
freeoptions(void);

struct option
{
	enum upnpconfigoptions id;
	const char * value;
};

extern struct option * ary_options;
extern unsigned int num_options;

#endif /* DISABLE_CONFIG_FILE */

#endif /* OPTIONS_H_INCLUDED */
