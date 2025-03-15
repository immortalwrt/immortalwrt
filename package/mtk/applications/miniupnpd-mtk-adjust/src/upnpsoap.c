/* $Id: upnpsoap.c,v 1.165 2023/06/26 23:15:56 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2024 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

#include "macros.h"
#include "config.h"
#include "upnpglobalvars.h"
#include "upnphttp.h"
#include "upnpsoap.h"
#include "upnpreplyparse.h"
#include "upnpredirect.h"
#include "upnppermissions.h"
#include "upnppinhole.h"
#include "getifaddr.h"
#include "getifstats.h"
#include "getconnstatus.h"
#include "upnpurns.h"
#include "upnputils.h"

/* utility function */
static int is_numeric(const char * s)
{
	while(*s) {
		if(*s < '0' || *s > '9') return 0;
		s++;
	}
	return 1;
}

static void
BuildSendAndCloseSoapResp(struct upnphttp * h,
                          const char * body, int bodylen)
{
	static const char beforebody[] =
		"<?xml version=\"1.0\"?>\r\n"
		"<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
		"s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
		"<s:Body>\r\n";

	static const char afterbody[] =
		"</s:Body>\r\n"
		"</s:Envelope>\r\n";

	int r = BuildHeader_upnphttp(h, 200, "OK",  sizeof(beforebody) - 1
	                             + sizeof(afterbody) - 1 + bodylen );

	if(r >= 0) {
		memcpy(h->res_buf + h->res_buflen, beforebody, sizeof(beforebody) - 1);
		h->res_buflen += sizeof(beforebody) - 1;

		memcpy(h->res_buf + h->res_buflen, body, bodylen);
		h->res_buflen += bodylen;

		memcpy(h->res_buf + h->res_buflen, afterbody, sizeof(afterbody) - 1);
		h->res_buflen += sizeof(afterbody) - 1;
	} else {
		BuildResp2_upnphttp(h, 500, "Internal Server Error", NULL, 0);
	}

	SendRespAndClose_upnphttp(h);
}

static void
GetConnectionTypeInfo(struct upnphttp * h, const char * action, const char * ns)
{
#if 0
	static const char resp[] =
		"<u:GetConnectionTypeInfoResponse "
		"xmlns:u=\"" SERVICE_TYPE_WANIPC "\">\r\n"
		"<NewConnectionType>IP_Routed</NewConnectionType>\r\n"
		"<NewPossibleConnectionTypes>IP_Routed</NewPossibleConnectionTypes>\r\n"
		"</u:GetConnectionTypeInfoResponse>\r\n";
#endif
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<NewConnectionType>IP_Routed</NewConnectionType>\r\n"
		"<NewPossibleConnectionTypes>IP_Routed</NewPossibleConnectionTypes>\r\n"
		"</u:%sResponse>\r\n";
	char body[512];
	int bodylen;

	bodylen = snprintf(body, sizeof(body), resp,
	         action, ns, action);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

/* maximum value for a UPNP ui4 type variable */
#define UPNP_UI4_MAX (4294967295ul)

static void
GetTotalBytesSent(struct upnphttp * h, const char * action, const char * ns)
{
	int r;

	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<NewTotalBytesSent>%lu</NewTotalBytesSent>\r\n"
		"</u:%sResponse>\r\n";

	char body[512];
	int bodylen;
	struct ifdata data;

	r = getifstats(ext_if_name, &data);
	bodylen = snprintf(body, sizeof(body), resp,
	         action, ns, /* was "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1" */
#ifdef UPNP_STRICT
             r<0?0:(data.obytes & UPNP_UI4_MAX), action);
#else /* UPNP_STRICT */
             r<0?0:data.obytes, action);
#endif /* UPNP_STRICT */
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void
GetTotalBytesReceived(struct upnphttp * h, const char * action, const char * ns)
{
	int r;

	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<NewTotalBytesReceived>%lu</NewTotalBytesReceived>\r\n"
		"</u:%sResponse>\r\n";

	char body[512];
	int bodylen;
	struct ifdata data;

	r = getifstats(ext_if_name, &data);
	/* TotalBytesReceived
	 * This variable represents the cumulative counter for total number of
	 * bytes received downstream across all connection service instances on
	 * WANDevice. The count rolls over to 0 after it reaching the maximum
	 * value (2^32)-1. */
	bodylen = snprintf(body, sizeof(body), resp,
	         action, ns, /* was "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1" */
#ifdef UPNP_STRICT
	         r<0?0:(data.ibytes & UPNP_UI4_MAX), action);
#else /* UPNP_STRICT */
	         r<0?0:data.ibytes, action);
#endif /* UPNP_STRICT */
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void
GetTotalPacketsSent(struct upnphttp * h, const char * action, const char * ns)
{
	int r;

	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<NewTotalPacketsSent>%lu</NewTotalPacketsSent>\r\n"
		"</u:%sResponse>\r\n";

	char body[512];
	int bodylen;
	struct ifdata data;

	r = getifstats(ext_if_name, &data);
	bodylen = snprintf(body, sizeof(body), resp,
	         action, ns,/*"urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1",*/
#ifdef UPNP_STRICT
	         r<0?0:(data.opackets & UPNP_UI4_MAX), action);
#else /* UPNP_STRICT */
	         r<0?0:data.opackets, action);
#endif /* UPNP_STRICT */
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void
GetTotalPacketsReceived(struct upnphttp * h, const char * action, const char * ns)
{
	int r;

	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<NewTotalPacketsReceived>%lu</NewTotalPacketsReceived>\r\n"
		"</u:%sResponse>\r\n";

	char body[512];
	int bodylen;
	struct ifdata data;

	r = getifstats(ext_if_name, &data);
	bodylen = snprintf(body, sizeof(body), resp,
	         action, ns, /* was "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1" */
#ifdef UPNP_STRICT
	         r<0?0:(data.ipackets & UPNP_UI4_MAX), action);
#else /* UPNP_STRICT */
	         r<0?0:data.ipackets, action);
#endif /* UPNP_STRICT */
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void
GetCommonLinkProperties(struct upnphttp * h, const char * action, const char * ns)
{
	/* WANAccessType : set depending on the hardware :
	 * DSL, POTS (plain old Telephone service), Cable, Ethernet */
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<NewWANAccessType>%s</NewWANAccessType>\r\n"
		"<NewLayer1UpstreamMaxBitRate>%lu</NewLayer1UpstreamMaxBitRate>\r\n"
		"<NewLayer1DownstreamMaxBitRate>%lu</NewLayer1DownstreamMaxBitRate>\r\n"
		"<NewPhysicalLinkStatus>%s</NewPhysicalLinkStatus>\r\n"
		"</u:%sResponse>\r\n";

	char body[2048];
	int bodylen;
	struct ifdata data;
	const char * status = "Up";	/* Up, Down (Required),
	                             * Initializing, Unavailable (Optional) */
	const char * wan_access_type = "Cable"; /* DSL, POTS, Cable, Ethernet */
	char ext_ip_addr[INET_ADDRSTRLEN];

	if((downstream_bitrate == 0) || (upstream_bitrate == 0))
	{
		if(getifstats(ext_if_name, &data) >= 0)
		{
			if(downstream_bitrate == 0) downstream_bitrate = data.baudrate;
			if(upstream_bitrate == 0) upstream_bitrate = data.baudrate;
		}
	}
	if(getifaddr(ext_if_name, ext_ip_addr, INET_ADDRSTRLEN, NULL, NULL) < 0) {
		status = "Down";
	}
	bodylen = snprintf(body, sizeof(body), resp,
	    action, ns, /* was "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1" */
	    wan_access_type,
	    upstream_bitrate, downstream_bitrate,
	    status, action);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void
GetStatusInfo(struct upnphttp * h, const char * action, const char * ns)
{
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<NewConnectionStatus>%s</NewConnectionStatus>\r\n"
		"<NewLastConnectionError>ERROR_NONE</NewLastConnectionError>\r\n"
		"<NewUptime>%ld</NewUptime>\r\n"
		"</u:%sResponse>\r\n";

	char body[512];
	int bodylen;
	time_t uptime;
	const char * status;
	/* ConnectionStatus possible values :
	 * Unconfigured, Connecting, Connected, PendingDisconnect,
	 * Disconnecting, Disconnected */

	status = get_wan_connection_status_str(ext_if_name);
	uptime = upnp_get_uptime();
	bodylen = snprintf(body, sizeof(body), resp,
		action, ns, /*SERVICE_TYPE_WANIPC,*/
		status, (long)uptime, action);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void
GetNATRSIPStatus(struct upnphttp * h, const char * action, const char * ns)
{
#if 0
	static const char resp[] =
		"<u:GetNATRSIPStatusResponse "
		"xmlns:u=\"" SERVICE_TYPE_WANIPC "\">\r\n"
		"<NewRSIPAvailable>0</NewRSIPAvailable>\r\n"
		"<NewNATEnabled>1</NewNATEnabled>\r\n"
		"</u:GetNATRSIPStatusResponse>\r\n";
	UNUSED(action);
#endif
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<NewRSIPAvailable>0</NewRSIPAvailable>\r\n"
		"<NewNATEnabled>1</NewNATEnabled>\r\n"
		"</u:%sResponse>\r\n";
	char body[512];
	int bodylen;
	/* 2.2.9. RSIPAvailable
	 * This variable indicates if Realm-specific IP (RSIP) is available
	 * as a feature on the InternetGatewayDevice. RSIP is being defined
	 * in the NAT working group in the IETF to allow host-NATing using
	 * a standard set of message exchanges. It also allows end-to-end
	 * applications that otherwise break if NAT is introduced
	 * (e.g. IPsec-based VPNs).
	 * A gateway that does not support RSIP should set this variable to 0. */
	bodylen = snprintf(body, sizeof(body), resp,
		action, ns, /*SERVICE_TYPE_WANIPC,*/
		action);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void
GetExternalIPAddress(struct upnphttp * h, const char * action, const char * ns)
{
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<NewExternalIPAddress>%s</NewExternalIPAddress>\r\n"
		"</u:%sResponse>\r\n";

	char body[512];
	int bodylen;
	char ext_ip_addr[INET_ADDRSTRLEN];
	/* Does that method need to work with IPv6 ?
	 * There is usually no NAT with IPv6 */

#ifndef MULTIPLE_EXTERNAL_IP
	if(use_ext_ip_addr)
	{
		strncpy(ext_ip_addr, use_ext_ip_addr, INET_ADDRSTRLEN);
		ext_ip_addr[INET_ADDRSTRLEN - 1] = '\0';
	}
	else
	{
		struct in_addr addr;
		if(getifaddr(ext_if_name, ext_ip_addr, INET_ADDRSTRLEN, &addr, NULL) < 0)
		{
			syslog(LOG_ERR, "Failed to get ip address for interface %s",
				ext_if_name);
			ext_ip_addr[0] = '\0';
		}
	}
#else
	struct lan_addr_s * lan_addr;
	ext_ip_addr[0] = '\0';
	for(lan_addr = lan_addrs.lh_first; lan_addr != NULL; lan_addr = lan_addr->list.le_next)
	{
		if( (h->clientaddr.s_addr & lan_addr->mask.s_addr)
		   == (lan_addr->addr.s_addr & lan_addr->mask.s_addr))
		{
			strncpy(ext_ip_addr, lan_addr->ext_ip_str, INET_ADDRSTRLEN);
			break;
		}
	}
#endif
	/* WANIPConnection:2 Service 2.3.13 :
	 * When the external IP address could not be retrieved by the gateway
	 * (for example, because the interface is down or because there was a
	 * failure in the last connection setup attempt),
	 * then the ExternalIPAddress MUST be equal to the empty string.
	 *
	 * There is no precise requirement on how theses cases must be handled
	 * in IGDv1 specifications, but ExternalIPAddress default value is empty
	 * string. */
	if (strcmp(ext_ip_addr, "0.0.0.0") == 0)
		ext_ip_addr[0] = '\0';
	bodylen = snprintf(body, sizeof(body), resp,
	              action, ns, /*SERVICE_TYPE_WANIPC,*/
				  ext_ip_addr, action);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

/* AddPortMapping method of WANIPConnection Service
 * Ignored argument : NewEnabled */
static void
AddPortMapping(struct upnphttp * h, const char * action, const char * ns)
{
	int r;

	/*static const char resp[] =
		"<u:AddPortMappingResponse "
		"xmlns:u=\"" SERVICE_TYPE_WANIPC "\"/>\r\n";*/
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\"/>\r\n";

	char body[512];
	int bodylen;
	struct NameValueParserData data;
	char * int_ip, * int_port, * ext_port, * protocol, * desc;
	char * leaseduration_str;
	unsigned int leaseduration;
	char * r_host;
	unsigned short iport, eport;

	struct hostent *hp; /* getbyhostname() */
	char ** ptr; /* getbyhostname() */
	struct in_addr result_ip;/*unsigned char result_ip[16];*/ /* inet_pton() */

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	int_ip = GetValueFromNameValueList(&data, "NewInternalClient");
	if (int_ip) {
		/* trim */
		while(int_ip[0] == ' ')
			int_ip++;
	}
#ifdef UPNP_STRICT
	if (!int_ip || int_ip[0] == '\0')
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}
#endif

	/* IGD 2 MUST support both wildcard and specific IP address values
	 * for RemoteHost (only the wildcard value was REQUIRED in release 1.0) */
	r_host = GetValueFromNameValueList(&data, "NewRemoteHost");
#ifndef SUPPORT_REMOTEHOST
#ifdef UPNP_STRICT
	if (r_host && (r_host[0] != '\0') && (0 != strcmp(r_host, "*")))
	{
		ClearNameValueList(&data);
		SoapError(h, 726, "RemoteHostOnlySupportsWildcard");
		return;
	}
#endif
#endif

#ifndef UPNP_STRICT
	/* if <NewInternalClient> arg is empty, use client address
	 * see https://github.com/miniupnp/miniupnp/issues/236 */
	if (!int_ip || int_ip[0] == '\0')
	{
		int_ip = h->clientaddr_str;
		memcpy(&result_ip, &(h->clientaddr), sizeof(struct in_addr));
	}
	else
#endif
	/* if ip not valid assume hostname and convert */
	if (inet_pton(AF_INET, int_ip, &result_ip) <= 0)
	{
		hp = gethostbyname(int_ip);
		if(hp && hp->h_addrtype == AF_INET)
		{
			for(ptr = hp->h_addr_list; ptr && *ptr; ptr++)
		   	{
				int_ip = inet_ntoa(*((struct in_addr *) *ptr));
				result_ip = *((struct in_addr *) *ptr);
				/* TODO : deal with more than one ip per hostname */
				break;
			}
		}
		else
		{
			syslog(LOG_ERR, "Failed to convert hostname '%s' to ip address", int_ip);
			ClearNameValueList(&data);
			SoapError(h, 402, "Invalid Args");
			return;
		}
	}

	/* check if NewInternalAddress is the client address */
	if(GETFLAG(SECUREMODEMASK))
	{
		if(h->clientaddr.s_addr != result_ip.s_addr)
		{
			syslog(LOG_INFO, "Client %s tried to redirect port to %s",
			       inet_ntoa(h->clientaddr), int_ip);
			ClearNameValueList(&data);
#ifdef IGD_V2
			SoapError(h, 606, "Action not authorized");
#else
			SoapError(h, 718, "ConflictInMappingEntry");
#endif
			return;
		}
	}

	int_port = GetValueFromNameValueList(&data, "NewInternalPort");
	ext_port = GetValueFromNameValueList(&data, "NewExternalPort");
	protocol = GetValueFromNameValueList(&data, "NewProtocol");
	desc = GetValueFromNameValueList(&data, "NewPortMappingDescription");
	leaseduration_str = GetValueFromNameValueList(&data, "NewLeaseDuration");

	if (!int_port || !ext_port || !protocol)
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}

	eport = (unsigned short)atoi(ext_port);
	iport = (unsigned short)atoi(int_port);

	if (strcmp(ext_port, "*") == 0 || eport == 0)
	{
		ClearNameValueList(&data);
		SoapError(h, 716, "WildCardNotPermittedInExtPort");
		return;
	}

	leaseduration = leaseduration_str ? atoi(leaseduration_str) : 0;
#ifdef IGD_V2
	/* PortMappingLeaseDuration can be either a value between 1 and
	 * 604800 seconds or the zero value (for infinite lease time).
	 * Note that an infinite lease time can be only set by out-of-band
	 * mechanisms like WWW-administration, remote management or local
	 * management.
	 * If a control point uses the value 0 to indicate an infinite lease
	 * time mapping, it is REQUIRED that gateway uses the maximum value
	 * instead (e.g. 604800 seconds) */
	if(leaseduration == 0 || leaseduration > 604800)
		leaseduration = 604800;
#endif

	syslog(LOG_INFO, "%s: ext port %hu to %s:%hu protocol %s for: %s leaseduration=%u rhost=%s",
	       action, eport, int_ip, iport, protocol, desc, leaseduration,
	       r_host ? r_host : "NULL");

	r = upnp_redirect(r_host, eport, int_ip, iport, protocol, desc, leaseduration);

	ClearNameValueList(&data);

	/* possible error codes for AddPortMapping :
	 * 402 - Invalid Args
	 * 501 - Action Failed
	 * 606 - Action not authorized (added in IGD v2)
	 * 715 - WildCardNotPermittedInSrcIP
	 * 716 - WildCardNotPermittedInExtPort
	 * 718 - ConflictInMappingEntry
	 * 724 - SamePortValuesRequired (deprecated in IGD v2)
     * 725 - OnlyPermanentLeasesSupported
             The NAT implementation only supports permanent lease times on
             port mappings (deprecated in IGD v2)
     * 726 - RemoteHostOnlySupportsWildcard
             RemoteHost must be a wildcard and cannot be a specific IP
             address or DNS name (deprecated in IGD v2)
     * 727 - ExternalPortOnlySupportsWildcard
             ExternalPort must be a wildcard and cannot be a specific port
             value (deprecated in IGD v2)
     * 728 - NoPortMapsAvailable
             There are not enough free ports available to complete the mapping
             (added in IGD v2)
	 * 729 - ConflictWithOtherMechanisms (added in IGD v2)
	 * 732 - WildCardNotPermittedInIntPort (added in IGD v2) */
	switch(r)
	{
	case 0:	/* success */
		bodylen = snprintf(body, sizeof(body), resp,
		                   action, ns/*SERVICE_TYPE_WANIPC*/);
		BuildSendAndCloseSoapResp(h, body, bodylen);
		break;
	case -4:
#ifdef IGD_V2
		SoapError(h, 729, "ConflictWithOtherMechanisms");
		break;
#endif /* IGD_V2 */
	case -3:	/* not permitted */
#ifdef IGD_V2
		SoapError(h, 606, "Action not authorized");
		break;
#endif /* IGD_V2 */
	case -2:	/* already redirected */
		SoapError(h, 718, "ConflictInMappingEntry");
		break;
	default:
		SoapError(h, 501, "Action Failed");
	}
}

/* AddAnyPortMapping was added in WANIPConnection v2 */
static void
AddAnyPortMapping(struct upnphttp * h, const char * action, const char * ns)
{
	int r;
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<NewReservedPort>%hu</NewReservedPort>\r\n"
		"</u:%sResponse>\r\n";

	char body[512];
	int bodylen;

	struct NameValueParserData data;
	const char * int_ip, * int_port, * ext_port, * protocol, * desc;
	const char * r_host;
	unsigned short iport, eport;
	const char * leaseduration_str;
	unsigned int leaseduration;

	struct hostent *hp; /* getbyhostname() */
	char ** ptr; /* getbyhostname() */
	struct in_addr result_ip;/*unsigned char result_ip[16];*/ /* inet_pton() */

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	r_host = GetValueFromNameValueList(&data, "NewRemoteHost");
	ext_port = GetValueFromNameValueList(&data, "NewExternalPort");
	protocol = GetValueFromNameValueList(&data, "NewProtocol");
	int_port = GetValueFromNameValueList(&data, "NewInternalPort");
	int_ip = GetValueFromNameValueList(&data, "NewInternalClient");
	/* NewEnabled */
	desc = GetValueFromNameValueList(&data, "NewPortMappingDescription");
	leaseduration_str = GetValueFromNameValueList(&data, "NewLeaseDuration");

	leaseduration = leaseduration_str ? atoi(leaseduration_str) : 0;
	if(leaseduration == 0)
		leaseduration = 604800;

	if (!int_ip || !ext_port || !int_port || !protocol)
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}

	eport = (0 == strcmp(ext_port, "*")) ? 0 : (unsigned short)atoi(ext_port);
	if (eport == 0) {
		eport = 1024 + ((random() & 0x7ffffffL) % (65536-1024));
	}
	iport = (unsigned short)atoi(int_port);
	if(iport == 0 || (!is_numeric(ext_port) && 0 != strcmp(ext_port, "*"))) {
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}
#ifndef SUPPORT_REMOTEHOST
#ifdef UPNP_STRICT
	if (r_host && (r_host[0] != '\0') && (0 != strcmp(r_host, "*")))
	{
		ClearNameValueList(&data);
		SoapError(h, 726, "RemoteHostOnlySupportsWildcard");
		return;
	}
#endif
#endif

	/* if ip not valid assume hostname and convert */
	if (inet_pton(AF_INET, int_ip, &result_ip) <= 0)
	{
		hp = gethostbyname(int_ip);
		if(hp && hp->h_addrtype == AF_INET)
		{
			for(ptr = hp->h_addr_list; ptr && *ptr; ptr++)
		   	{
				int_ip = inet_ntoa(*((struct in_addr *) *ptr));
				result_ip = *((struct in_addr *) *ptr);
				/* TODO : deal with more than one ip per hostname */
				break;
			}
		}
		else
		{
			syslog(LOG_ERR, "Failed to convert hostname '%s' to ip address", int_ip);
			ClearNameValueList(&data);
			SoapError(h, 402, "Invalid Args");
			return;
		}
	}

	/* check if NewInternalAddress is the client address */
	if(GETFLAG(SECUREMODEMASK))
	{
		if(h->clientaddr.s_addr != result_ip.s_addr)
		{
			syslog(LOG_INFO, "Client %s tried to redirect port to %s",
			       inet_ntoa(h->clientaddr), int_ip);
			ClearNameValueList(&data);
			SoapError(h, 606, "Action not authorized");
			return;
		}
	}

	/* first try the port asked in request, then
	 * try +1, -1, +2, -2, etc. */
	r = upnp_redirect(r_host, eport, int_ip, iport, protocol, desc, leaseduration);
	if (r != 0 && r != -1) {
		unsigned short eport_below, eport_above;
		struct in_addr address;
		uint32_t allowed_eports[65536 / 32];

		if(inet_aton(int_ip, &address) <= 0) {
			syslog(LOG_ERR, "inet_aton(%s) FAILED", int_ip);
		}
		get_permitted_ext_ports(allowed_eports, upnppermlist, num_upnpperm,
		                        address.s_addr, iport);
		eport_above = eport_below = eport;
		for(;;) {
			/* loop invariant
			 * eport is equal to either eport_below or eport_above (or both) */
			if (eport_below <= 1 && eport_above == 65535) {
				/* all possible ports tried */
				r = 1;
				break;
			}
			if (eport_above == 65535 || (eport > eport_below && eport_below > 1)) {
				eport = --eport_below;
			} else {
				eport = ++eport_above;
			}
			if (!(allowed_eports[eport / 32] & ((uint32_t)1U << (eport % 32))))
				continue;	/* not allowed */
			r = upnp_redirect(r_host, eport, int_ip, iport, protocol, desc, leaseduration);
			if (r == 0 || r == -1) {
				/* OK or failure : Stop */
				break;
			}
			/* r : -2 / -4 already redirected or -3 permission check failed :
			 * continue */
		}
	}

	ClearNameValueList(&data);

	switch(r)
	{
	case 1:	/* exhausted possible mappings */
		SoapError(h, 728, "NoPortMapsAvailable");
		break;
	case 0:	/* success */
		bodylen = snprintf(body, sizeof(body), resp,
		              action, ns, /*SERVICE_TYPE_WANIPC,*/
					  eport, action);
		BuildSendAndCloseSoapResp(h, body, bodylen);
		break;
	case -2:	/* already redirected */
		SoapError(h, 718, "ConflictInMappingEntry");
		break;
	case -3:	/* not permitted */
		SoapError(h, 606, "Action not authorized");
		break;
	default:
		SoapError(h, 501, "Action Failed");
	}
}

static void
GetSpecificPortMappingEntry(struct upnphttp * h, const char * action, const char * ns)
{
	int r;

	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<NewInternalPort>%u</NewInternalPort>\r\n"
		"<NewInternalClient>%s</NewInternalClient>\r\n"
		"<NewEnabled>1</NewEnabled>\r\n"
		"<NewPortMappingDescription>%s</NewPortMappingDescription>\r\n"
		"<NewLeaseDuration>%u</NewLeaseDuration>\r\n"
		"</u:%sResponse>\r\n";

	char body[1024];
	int bodylen;
	struct NameValueParserData data;
	const char * r_host, * ext_port, * protocol;
	unsigned short eport, iport;
	char int_ip[32];
	char desc[64];
	unsigned int leaseduration = 0;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	r_host = GetValueFromNameValueList(&data, "NewRemoteHost");
	ext_port = GetValueFromNameValueList(&data, "NewExternalPort");
	protocol = GetValueFromNameValueList(&data, "NewProtocol");

#ifdef UPNP_STRICT
	if(!ext_port || !protocol || !r_host)
#else
	if(!ext_port || !protocol)
#endif
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}
#ifndef SUPPORT_REMOTEHOST
#ifdef UPNP_STRICT
	if (r_host && (r_host[0] != '\0') && (0 != strcmp(r_host, "*")))
	{
		ClearNameValueList(&data);
		SoapError(h, 726, "RemoteHostOnlySupportsWildcard");
		return;
	}
#endif
#endif

	eport = (unsigned short)atoi(ext_port);
	if(eport == 0)
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}

	/* TODO : add r_host as an input parameter ...
	 * We prevent several Port Mapping with same external port
	 * but different remoteHost to be set up, so that is not
	 * a priority. */
	r = upnp_get_redirection_infos(eport, protocol, &iport,
	                               int_ip, sizeof(int_ip),
	                               desc, sizeof(desc),
	                               NULL, 0,
	                               &leaseduration);

	if(r < 0)
	{
		SoapError(h, 714, "NoSuchEntryInArray");
	}
	else
	{
		syslog(LOG_INFO, "%s: rhost='%s' %s %s found => %s:%u desc='%s' duration=%u",
		       action,
		       r_host ? r_host : "NULL", ext_port, protocol, int_ip,
		       (unsigned int)iport, desc, leaseduration);
		bodylen = snprintf(body, sizeof(body), resp,
				action, ns/*SERVICE_TYPE_WANIPC*/,
				(unsigned int)iport, int_ip, desc, leaseduration,
				action);
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}

	ClearNameValueList(&data);
}

static void
DeletePortMapping(struct upnphttp * h, const char * action, const char * ns)
{
	int r;

	/*static const char resp[] =
		"<u:DeletePortMappingResponse "
		"xmlns:u=\"" SERVICE_TYPE_WANIPC "\">\r\n"
		"</u:DeletePortMappingResponse>\r\n";*/
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"</u:%sResponse>\r\n";

	char body[512];
	int bodylen;
	struct NameValueParserData data;
	const char * ext_port, * protocol;
	unsigned short eport;
#ifdef UPNP_STRICT
	const char * r_host;
#endif /* UPNP_STRICT */

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	ext_port = GetValueFromNameValueList(&data, "NewExternalPort");
	protocol = GetValueFromNameValueList(&data, "NewProtocol");
#ifdef UPNP_STRICT
	r_host = GetValueFromNameValueList(&data, "NewRemoteHost");
#endif /* UPNP_STRICT */

#ifdef UPNP_STRICT
	if(!ext_port || !protocol || !r_host)
#else
	if(!ext_port || !protocol)
#endif /* UPNP_STRICT */
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}
#ifndef SUPPORT_REMOTEHOST
#ifdef UPNP_STRICT
	if (r_host && (r_host[0] != '\0') && (0 != strcmp(r_host, "*")))
	{
		ClearNameValueList(&data);
		SoapError(h, 726, "RemoteHostOnlySupportsWildcard");
		return;
	}
#endif /* UPNP_STRICT */
#endif /* SUPPORT_REMOTEHOST */

	eport = (unsigned short)atoi(ext_port);
	if(eport == 0)
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}

	syslog(LOG_INFO, "%s: external port: %hu, protocol: %s",
		action, eport, protocol);

	/* if in secure mode, check the IP
	 * Removing a redirection is not a security threat,
	 * just an annoyance for the user using it. So this is not
	 * a priority. */
	if(GETFLAG(SECUREMODEMASK))
	{
		char int_ip[32];
		struct in_addr int_ip_addr;
		unsigned short iport;
		unsigned int leaseduration = 0;
		r = upnp_get_redirection_infos(eport, protocol, &iport,
		                               int_ip, sizeof(int_ip),
		                               NULL, 0, NULL, 0,
		                               &leaseduration);
		if(r >= 0)
		{
			if(inet_pton(AF_INET, int_ip, &int_ip_addr) > 0)
			{
				if(h->clientaddr.s_addr != int_ip_addr.s_addr)
				{
#ifdef IGD_V2
					SoapError(h, 606, "Action not authorized");
#else
					SoapError(h, 714, "NoSuchEntryInArray");
#endif
					ClearNameValueList(&data);
					return;
				}
			}
		}
	}

	r = upnp_delete_redirection(eport, protocol);

	if(r < 0)
	{
		SoapError(h, 714, "NoSuchEntryInArray");
	}
	else
	{
		bodylen = snprintf(body, sizeof(body), resp,
		                   action, ns, action);
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}

	ClearNameValueList(&data);
}

/* DeletePortMappingRange was added in IGD spec v2 */
static void
DeletePortMappingRange(struct upnphttp * h, const char * action, const char * ns)
{
	int r = -1;
	/*static const char resp[] =
		"<u:DeletePortMappingRangeResponse "
		"xmlns:u=\"" SERVICE_TYPE_WANIPC "\">\r\n"
		"</u:DeletePortMappingRangeResponse>\r\n";*/
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"</u:%sResponse>\r\n";
	char body[512];
	int bodylen;
	struct NameValueParserData data;
	const char * protocol;
	const char * startport_s, * endport_s;
	unsigned short startport, endport;
	/*int manage;*/
	unsigned short * port_list;
	unsigned int i, number = 0;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	startport_s = GetValueFromNameValueList(&data, "NewStartPort");
	endport_s = GetValueFromNameValueList(&data, "NewEndPort");
	protocol = GetValueFromNameValueList(&data, "NewProtocol");
	/*manage = atoi(GetValueFromNameValueList(&data, "NewManage"));*/
	if(startport_s == NULL || endport_s == NULL || protocol == NULL ||
	   !is_numeric(startport_s) || !is_numeric(endport_s)) {
		SoapError(h, 402, "Invalid Args");
		ClearNameValueList(&data);
		return;
	}
	startport = (unsigned short)atoi(startport_s);
	endport = (unsigned short)atoi(endport_s);

	/* possible errors :
	   606 - Action not authorized
	   730 - PortMappingNotFound
	   733 - InconsistentParameter
	 */
	if(startport > endport)
	{
		SoapError(h, 733, "InconsistentParameter");
		ClearNameValueList(&data);
		return;
	}

	syslog(LOG_INFO, "%s: deleting external ports: %hu-%hu, protocol: %s",
	       action, startport, endport, protocol);

	port_list = upnp_get_portmappings_in_range(startport, endport,
	                                           protocol, &number);
	if(number == 0)
	{
		SoapError(h, 730, "PortMappingNotFound");
		ClearNameValueList(&data);
		free(port_list);
		return;
	}

	for(i = 0; i < number; i++)
	{
		r = upnp_delete_redirection(port_list[i], protocol);
		syslog(LOG_INFO, "%s: deleting external port: %hu, protocol: %s: %s",
		       action, port_list[i], protocol, r < 0 ? "failed" : "ok");
	}
	free(port_list);
	bodylen = snprintf(body, sizeof(body), resp,
	                   action, ns, action);
	BuildSendAndCloseSoapResp(h, body, bodylen);

	ClearNameValueList(&data);
}

static void
GetGenericPortMappingEntry(struct upnphttp * h, const char * action, const char * ns)
{
	int r;

	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<NewRemoteHost>%s</NewRemoteHost>\r\n"
		"<NewExternalPort>%u</NewExternalPort>\r\n"
		"<NewProtocol>%s</NewProtocol>\r\n"
		"<NewInternalPort>%u</NewInternalPort>\r\n"
		"<NewInternalClient>%s</NewInternalClient>\r\n"
		"<NewEnabled>1</NewEnabled>\r\n"
		"<NewPortMappingDescription>%s</NewPortMappingDescription>\r\n"
		"<NewLeaseDuration>%u</NewLeaseDuration>\r\n"
		"</u:%sResponse>\r\n";

	long int index = 0;
	unsigned short eport, iport;
	const char * m_index;
	char * endptr;
	char protocol[8], iaddr[32];
	char desc[64];
	char rhost[40];
	unsigned int leaseduration = 0;
	struct NameValueParserData data;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	m_index = GetValueFromNameValueList(&data, "NewPortMappingIndex");

	if(!m_index)
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}
	errno = 0;	/* To distinguish success/failure after call */
	index = strtol(m_index, &endptr, 10);
	if((errno == ERANGE && (index == LONG_MAX || index == LONG_MIN))
	   || (errno != 0 && index == 0) || (m_index == endptr))
	{
		/* should condition (*endptr != '\0') be also an error ? */
		if(m_index == endptr)
			syslog(LOG_WARNING, "%s: no digits were found in <%s>",
			       "GetGenericPortMappingEntry", "NewPortMappingIndex");
		else
			syslog(LOG_WARNING, "%s: strtol('%s'): %m",
			       "GetGenericPortMappingEntry", m_index);
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}

	syslog(LOG_INFO, "%s: index=%d", action, (int)index);

	rhost[0] = '\0';
	r = upnp_get_redirection_infos_by_index((int)index, &eport, protocol, &iport,
                                            iaddr, sizeof(iaddr),
	                                        desc, sizeof(desc),
	                                        rhost, sizeof(rhost),
	                                        &leaseduration);

	if(r < 0)
	{
		SoapError(h, 713, "SpecifiedArrayIndexInvalid");
	}
	else
	{
		int bodylen;
		char body[2048];
		bodylen = snprintf(body, sizeof(body), resp,
			action, ns, /*SERVICE_TYPE_WANIPC,*/ rhost,
			(unsigned int)eport, protocol, (unsigned int)iport, iaddr, desc,
		    leaseduration, action);
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}

	ClearNameValueList(&data);
}

/* GetListOfPortMappings was added in the IGD v2 specification */
static void
GetListOfPortMappings(struct upnphttp * h, const char * action, const char * ns)
{
	static const char resp_start[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<NewPortListing><![CDATA[";
	static const char resp_end[] =
		"]]></NewPortListing>\r\n"
		"</u:%sResponse>\r\n";

	static const char list_start[] =
		"<p:PortMappingList xmlns:p=\"urn:schemas-upnp-org:gw:WANIPConnection\""
		" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
		" xsi:schemaLocation=\"urn:schemas-upnp-org:gw:WANIPConnection"
		" http://www.upnp.org/schemas/gw/WANIPConnection-v2.xsd\">\r\n";
	static const char list_end[] =
		"</p:PortMappingList>\r\n";

	static const char entry[] =
		"<p:PortMappingEntry>\r\n"
		"<p:NewRemoteHost>%s</p:NewRemoteHost>\r\n"
		"<p:NewExternalPort>%hu</p:NewExternalPort>\r\n"
		"<p:NewProtocol>%s</p:NewProtocol>\r\n"
		"<p:NewInternalPort>%hu</p:NewInternalPort>\r\n"
		"<p:NewInternalClient>%s</p:NewInternalClient>\r\n"
		"<p:NewEnabled>1</p:NewEnabled>\r\n"
		"<p:NewDescription>%s</p:NewDescription>\r\n"
		"<p:NewLeaseTime>%u</p:NewLeaseTime>\r\n"
		"</p:PortMappingEntry>\r\n";

	char * body;
	size_t bodyalloc;
	int bodylen;

	int r = -1;
	unsigned short iport;
	char int_ip[32];
	char desc[64];
	char rhost[64];
	unsigned int leaseduration = 0;

	struct NameValueParserData data;
	const char * startport_s, * endport_s;
	unsigned short startport, endport;
	const char * protocol;
	/*int manage;*/
	const char * number_s;
	int number;
	unsigned short * port_list;
	unsigned int i, list_size = 0;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	startport_s = GetValueFromNameValueList(&data, "NewStartPort");
	endport_s = GetValueFromNameValueList(&data, "NewEndPort");
	protocol = GetValueFromNameValueList(&data, "NewProtocol");
	/*manage_s = GetValueFromNameValueList(&data, "NewManage");*/
	number_s = GetValueFromNameValueList(&data, "NewNumberOfPorts");
	if(startport_s == NULL || endport_s == NULL || protocol == NULL ||
	   number_s == NULL || !is_numeric(number_s) ||
	   !is_numeric(startport_s) || !is_numeric(endport_s)) {
		SoapError(h, 402, "Invalid Args");
		ClearNameValueList(&data);
		return;
	}

	startport = (unsigned short)atoi(startport_s);
	endport = (unsigned short)atoi(endport_s);
	/*manage = atoi(manage_s);*/
	number = atoi(number_s);
	if(number == 0) number = 1000;	/* return up to 1000 mappings by default */

	if(startport > endport)
	{
		SoapError(h, 733, "InconsistentParameter");
		ClearNameValueList(&data);
		return;
	}
/*
build the PortMappingList xml document :

<p:PortMappingList xmlns:p="urn:schemas-upnp-org:gw:WANIPConnection"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
xsi:schemaLocation="urn:schemas-upnp-org:gw:WANIPConnection
http://www.upnp.org/schemas/gw/WANIPConnection-v2.xsd">
<p:PortMappingEntry>
<p:NewRemoteHost>202.233.2.1</p:NewRemoteHost>
<p:NewExternalPort>2345</p:NewExternalPort>
<p:NewProtocol>TCP</p:NewProtocol>
<p:NewInternalPort>2345</p:NewInternalPort>
<p:NewInternalClient>192.168.1.137</p:NewInternalClient>
<p:NewEnabled>1</p:NewEnabled>
<p:NewDescription>dooom</p:NewDescription>
<p:NewLeaseTime>345</p:NewLeaseTime>
</p:PortMappingEntry>
</p:PortMappingList>
*/
	bodyalloc = 4096;
	body = malloc(bodyalloc);
	if(!body)
	{
		ClearNameValueList(&data);
		SoapError(h, 501, "Action Failed");
		return;
	}
	bodylen = snprintf(body, bodyalloc, resp_start,
	              action, ns/*SERVICE_TYPE_WANIPC*/);
	if(bodylen < 0)
	{
		SoapError(h, 501, "Action Failed");
		free(body);
		return;
	}
	memcpy(body+bodylen, list_start, sizeof(list_start));
	bodylen += (sizeof(list_start) - 1);

	port_list = upnp_get_portmappings_in_range(startport, endport,
	                                           protocol, &list_size);
	/* loop through port mappings */
	for(i = 0; number > 0 && i < list_size; i++)
	{
		/* have a margin of 1024 bytes to store the new entry */
		if((unsigned int)bodylen + 1024 > bodyalloc)
		{
			char * body_sav = body;
			bodyalloc += 4096;
			body = realloc(body, bodyalloc);
			if(!body)
			{
				syslog(LOG_CRIT, "realloc(%p, %u) FAILED", body_sav, (unsigned)bodyalloc);
				ClearNameValueList(&data);
				SoapError(h, 501, "Action Failed");
				free(body_sav);
				free(port_list);
				return;
			}
		}
		rhost[0] = '\0';
		r = upnp_get_redirection_infos(port_list[i], protocol, &iport,
		                               int_ip, sizeof(int_ip),
		                               desc, sizeof(desc),
		                               rhost, sizeof(rhost),
		                               &leaseduration);
		if(r == 0)
		{
			bodylen += snprintf(body+bodylen, bodyalloc-bodylen, entry,
			                    rhost, port_list[i], protocol,
			                    iport, int_ip, desc, leaseduration);
			number--;
		}
	}
	free(port_list);
	port_list = NULL;

	if((bodylen + sizeof(list_end) + 1024) > bodyalloc)
	{
		char * body_sav = body;
		bodyalloc += (sizeof(list_end) + 1024);
		body = realloc(body, bodyalloc);
		if(!body)
		{
			syslog(LOG_CRIT, "realloc(%p, %u) FAILED", body_sav, (unsigned)bodyalloc);
			ClearNameValueList(&data);
			SoapError(h, 501, "Action Failed");
			free(body_sav);
			return;
		}
	}
	memcpy(body+bodylen, list_end, sizeof(list_end));
	bodylen += (sizeof(list_end) - 1);
	bodylen += snprintf(body+bodylen, bodyalloc-bodylen, resp_end,
	                    action);
	BuildSendAndCloseSoapResp(h, body, bodylen);
	free(body);

	ClearNameValueList(&data);
}

#ifdef ENABLE_L3F_SERVICE
static void
SetDefaultConnectionService(struct upnphttp * h, const char * action, const char * ns)
{
	/*static const char resp[] =
		"<u:SetDefaultConnectionServiceResponse "
		"xmlns:u=\"urn:schemas-upnp-org:service:Layer3Forwarding:1\">\r\n"
		"</u:SetDefaultConnectionServiceResponse>\r\n";*/
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"</u:%sResponse>\r\n";
	char body[512];
	int bodylen;
	struct NameValueParserData data;
	char * p;
	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	p = GetValueFromNameValueList(&data, "NewDefaultConnectionService");
	if(p) {
		/* 720 InvalidDeviceUUID
		 * 721 InvalidServiceID
		 * 723 InvalidConnServiceSelection */
#ifdef UPNP_STRICT
		char * service;
		service = strchr(p, ',');
		if(0 != memcmp(uuidvalue_wcd, p, sizeof("uuid:00000000-0000-0000-0000-000000000000") - 1)) {
			SoapError(h, 720, "InvalidDeviceUUID");
		} else if(service == NULL || 0 != strcmp(service+1, SERVICE_ID_WANIPC)) {
			SoapError(h, 721, "InvalidServiceID");
		} else
#endif
		{
			syslog(LOG_INFO, "%s(%s) : Ignored", action, p);
			bodylen = snprintf(body, sizeof(body), resp,
			                   action, ns, action);
			BuildSendAndCloseSoapResp(h, body, bodylen);
		}
	} else {
		/* missing argument */
		SoapError(h, 402, "Invalid Args");
	}
	ClearNameValueList(&data);
}

static void
GetDefaultConnectionService(struct upnphttp * h, const char * action, const char * ns)
{
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
#ifdef IGD_V2
		"<NewDefaultConnectionService>%s:WANConnectionDevice:2,"
#else
		"<NewDefaultConnectionService>%s:WANConnectionDevice:1,"
#endif
		SERVICE_ID_WANIPC "</NewDefaultConnectionService>\r\n"
		"</u:%sResponse>\r\n";
	/* example from UPnP_IGD_Layer3Forwarding 1.0.pdf :
	 * uuid:44f5824f-c57d-418c-a131-f22b34e14111:WANConnectionDevice:1,
	 * urn:upnp-org:serviceId:WANPPPConn1 */
	char body[1024];
	int bodylen;

	/* namespace : urn:schemas-upnp-org:service:Layer3Forwarding:1 */
	bodylen = snprintf(body, sizeof(body), resp,
	                   action, ns, uuidvalue_wcd, action);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}
#endif

/* Added for compliance with WANIPConnection v2 */
static void
SetConnectionType(struct upnphttp * h, const char * action, const char * ns)
{
#ifdef UPNP_STRICT
	const char * connection_type;
#endif /* UPNP_STRICT */
	struct NameValueParserData data;
	UNUSED(action);
	UNUSED(ns);

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
#ifdef UPNP_STRICT
	connection_type = GetValueFromNameValueList(&data, "NewConnectionType");
	if(!connection_type) {
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}
#endif /* UPNP_STRICT */
	/* Unconfigured, IP_Routed, IP_Bridged */
	ClearNameValueList(&data);
	/* always return a ReadOnly error */
	SoapError(h, 731, "ReadOnly");
}

/* Added for compliance with WANIPConnection v2 */
static void
RequestConnection(struct upnphttp * h, const char * action, const char * ns)
{
	UNUSED(action);
	UNUSED(ns);
	SoapError(h, 606, "Action not authorized");
}

/* Added for compliance with WANIPConnection v2 */
static void
ForceTermination(struct upnphttp * h, const char * action, const char * ns)
{
	UNUSED(action);
	UNUSED(ns);
	SoapError(h, 606, "Action not authorized");
}

/*
If a control point calls QueryStateVariable on a state variable that is not
buffered in memory within (or otherwise available from) the service,
the service must return a SOAP fault with an errorCode of 404 Invalid Var.

QueryStateVariable remains useful as a limited test tool but may not be
part of some future versions of UPnP.
*/
static void
QueryStateVariable(struct upnphttp * h, const char * action, const char * ns)
{
	static const char resp[] =
        "<u:%sResponse "
        "xmlns:u=\"%s\">\r\n"
		"<return>%s</return>\r\n"
        "</u:%sResponse>\r\n";

	char body[512];
	int bodylen;
	struct NameValueParserData data;
	const char * var_name;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	/*var_name = GetValueFromNameValueList(&data, "QueryStateVariable"); */
	/*var_name = GetValueFromNameValueListIgnoreNS(&data, "varName");*/
	var_name = GetValueFromNameValueList(&data, "varName");

	/*syslog(LOG_INFO, "QueryStateVariable(%.40s)", var_name); */

	if(!var_name)
	{
		SoapError(h, 402, "Invalid Args");
	}
	else if(strcmp(var_name, "ConnectionStatus") == 0)
	{
		const char * status;

		status = get_wan_connection_status_str(ext_if_name);
		bodylen = snprintf(body, sizeof(body), resp,
                           action, ns,/*"urn:schemas-upnp-org:control-1-0",*/
		                   status, action);
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}
#if 0
	/* not useful */
	else if(strcmp(var_name, "ConnectionType") == 0)
	{
		bodylen = snprintf(body, sizeof(body), resp, "IP_Routed");
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}
	else if(strcmp(var_name, "LastConnectionError") == 0)
	{
		bodylen = snprintf(body, sizeof(body), resp, "ERROR_NONE");
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}
#endif
	else if(strcmp(var_name, "PortMappingNumberOfEntries") == 0)
	{
		char strn[10];
		snprintf(strn, sizeof(strn), "%i",
		         upnp_get_portmapping_number_of_entries());
		bodylen = snprintf(body, sizeof(body), resp,
                           action, ns,/*"urn:schemas-upnp-org:control-1-0",*/
		                   strn, action);
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}
	else
	{
		syslog(LOG_NOTICE, "%s: Unknown: %s", action, var_name?var_name:"");
		SoapError(h, 404, "Invalid Var");
	}

	ClearNameValueList(&data);
}

#ifdef ENABLE_6FC_SERVICE
#ifndef ENABLE_IPV6
#error "ENABLE_6FC_SERVICE needs ENABLE_IPV6"
#endif
/* WANIPv6FirewallControl actions */
static void
GetFirewallStatus(struct upnphttp * h, const char * action, const char * ns)
{
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<FirewallEnabled>%d</FirewallEnabled>\r\n"
		"<InboundPinholeAllowed>%d</InboundPinholeAllowed>\r\n"
		"</u:%sResponse>\r\n";

	char body[512];
	int bodylen;

	bodylen = snprintf(body, sizeof(body), resp,
		action, ns, /*"urn:schemas-upnp-org:service:WANIPv6FirewallControl:1",*/
	    GETFLAG(IPV6FCFWDISABLEDMASK) ? 0 : 1,
	    GETFLAG(IPV6FCINBOUNDDISALLOWEDMASK) ? 0 : 1,
	    action);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static int
CheckStatus(struct upnphttp * h)
{
	if (GETFLAG(IPV6FCFWDISABLEDMASK))
	{
		SoapError(h, 702, "FirewallDisabled");
		return 0;
	}
	else if(GETFLAG(IPV6FCINBOUNDDISALLOWEDMASK))
	{
		SoapError(h, 703, "InboundPinholeNotAllowed");
		return 0;
	}
	else
		return 1;
}

#if 0
static int connecthostport(const char * host, unsigned short port, char * result)
{
	int s, n;
	char hostname[INET6_ADDRSTRLEN];
	char port_str[8], ifname[8], tmp[4];
	struct addrinfo *ai, *p;
	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	/* hints.ai_flags = AI_ADDRCONFIG; */
#ifdef AI_NUMERICSERV
	hints.ai_flags = AI_NUMERICSERV;
#endif
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC; /* AF_INET, AF_INET6 or AF_UNSPEC */
	/* hints.ai_protocol = IPPROTO_TCP; */
	snprintf(port_str, sizeof(port_str), "%hu", port);
	strcpy(hostname, host);
	if(!strncmp(host, "fe80", 4))
	{
		printf("Using an linklocal address\n");
		strcpy(ifname, "%");
		snprintf(tmp, sizeof(tmp), "%d", linklocal_index);
		strcat(ifname, tmp);
		strcat(hostname, ifname);
		printf("host: %s\n", hostname);
	}
	n = getaddrinfo(hostname, port_str, &hints, &ai);
	if(n != 0)
	{
		fprintf(stderr, "getaddrinfo() error : %s\n", gai_strerror(n));
		return -1;
	}
	s = -1;
	for(p = ai; p; p = p->ai_next)
	{
#ifdef DEBUG
		char tmp_host[256];
		char tmp_service[256];
		printf("ai_family=%d ai_socktype=%d ai_protocol=%d ai_addrlen=%d\n ",
		       p->ai_family, p->ai_socktype, p->ai_protocol, p->ai_addrlen);
		getnameinfo(p->ai_addr, p->ai_addrlen, tmp_host, sizeof(tmp_host),
		            tmp_service, sizeof(tmp_service),
		            NI_NUMERICHOST | NI_NUMERICSERV);
		printf(" host=%s service=%s\n", tmp_host, tmp_service);
#endif
		inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)p->ai_addr)->sin6_addr), result, INET6_ADDRSTRLEN);
		return 0;
	}
	freeaddrinfo(ai);
}
#endif

/* Check the security policy rights
 * Pinhole InternalClient address must correspond to the action sender
 * returns 1 if it passes.
 * call SoapError() and returns 0 if it fails
 * Side effect : if int_ip is a hostname, convert it to litteral ipv6 */
static int
PinholeVerification(struct upnphttp * h, char * int_ip, unsigned short int_port)
{
	char clientaddr_str[INET6_ADDRSTRLEN];
	struct in6_addr result_ip;

	/* if ip not valid assume hostname and convert */
	if (inet_pton(AF_INET6, int_ip, &result_ip) <= 0)
	{
		int r;
		struct addrinfo hints, *ai, *p;

		syslog(LOG_INFO, "%s: InternalClient %s is not an IPv6, assume hostname and convert",
		       "PinholeVerification", int_ip);

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = 0; /* we may indicate which protocol IPPROTO_UDP / IPPROTO_TCP / IPPROTO_SCTP / IPPROTO_UDPLITE */

		r = getaddrinfo(int_ip, NULL, &hints, &ai);
		if (r == 0)
		{
			int found = 0;
			for(p = ai; p; p = p->ai_next)
			{
				if(p->ai_family == AF_INET6)
				{
					if (!found)
					{
						result_ip = ((struct sockaddr_in6 *)p->ai_addr)->sin6_addr;
						if (inet_ntop(AF_INET6, &result_ip, int_ip, sizeof(struct in6_addr)) == NULL)
							syslog(LOG_WARNING, "%s: inet_ntop(): %m", "PinholeVerification");
						syslog(LOG_INFO, "%s: InternalClient resolved as %s",
						       "PinholeVerification", int_ip);
						found = 1;
					}
					else
					{
						char tmp[48];
						sockaddr_to_string(p->ai_addr, tmp, sizeof(tmp));
						syslog(LOG_INFO, "%s: additionnal IPv6: %s",
						       "PinholeVerification", tmp);
					}
				}
			}
			freeaddrinfo(ai);
			if (!found)
			{
				syslog(LOG_NOTICE, "%s: No IPv6 address for hostname '%s'",
				       "PinholeVerification", int_ip);
				SoapError(h, 402, "Invalid Args");
				return -1;
			}
		}
		else
		{
			syslog(LOG_WARNING, "%s: Failed to convert hostname '%s' to IP address : %s",
			       "PinholeVerification", int_ip, gai_strerror(r));
			SoapError(h, 402, "Invalid Args");
			return -1;
		}
	}

	if(inet_ntop(AF_INET6, &(h->clientaddr_v6), clientaddr_str, INET6_ADDRSTRLEN) == NULL)
	{
		syslog(LOG_ERR, "inet_ntop: %m");
		strncpy(clientaddr_str, "*ERROR*", sizeof(clientaddr_str));
	}

	if(memcmp(&h->clientaddr_v6, &result_ip, sizeof(struct in6_addr)) != 0)
	{
		syslog(LOG_INFO, "%s: Client %s tried to access pinhole for internal %s and is not authorized",
		       "PinholeVerification", clientaddr_str, int_ip);
		SoapError(h, 606, "Action not authorized");
		return 0;
	}
#ifdef DEBUG
	else
	{
		syslog(LOG_DEBUG, "%s: sender %s == InternalClient %s",
		       "PinholeVerification", clientaddr_str, int_ip);
	}
#endif

	/* Pinhole InternalPort must be greater than or equal to 1024 */
	if (int_port < 1024)
	{
		syslog(LOG_INFO, "Client %s tried to access pinhole with port < 1024 and is not authorized to do it",
		       clientaddr_str);
		SoapError(h, 606, "Action not authorized");
		return 0;
	}
	return 1;
}

static void
AddPinhole(struct upnphttp * h, const char * action, const char * ns)
{
	int r;
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<UniqueID>%d</UniqueID>\r\n"
		"</u:%sResponse>\r\n";
	char body[512];
	int bodylen;
	struct NameValueParserData data;
	char * rem_host, * rem_port, * int_ip, * int_port, * protocol, * leaseTime;
	int uid = 0;
	unsigned short iport, rport;
	int ltime;
	long proto;
	char rem_ip[INET6_ADDRSTRLEN];

	if(CheckStatus(h)==0)
		return;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	rem_host = GetValueFromNameValueList(&data, "RemoteHost");
	rem_port = GetValueFromNameValueList(&data, "RemotePort");
	int_ip = GetValueFromNameValueList(&data, "InternalClient");
	int_port = GetValueFromNameValueList(&data, "InternalPort");
	protocol = GetValueFromNameValueList(&data, "Protocol");
	leaseTime = GetValueFromNameValueList(&data, "LeaseTime");

#ifdef UPNP_STRICT
	if (rem_port == NULL || rem_port[0] == '\0' || int_port == NULL || int_port[0] == '\0' )
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}
#endif
	rport = (unsigned short)(rem_port ? atoi(rem_port) : 0);
	iport = (unsigned short)(int_port ? atoi(int_port) : 0);
	ltime = leaseTime ? atoi(leaseTime) : -1;
	errno = 0;
	proto = protocol ? strtol(protocol, NULL, 0) : -1;
	if(errno != 0 || proto > 65535 || proto < 0)
	{
		SoapError(h, 402, "Invalid Args");
		goto clear_and_exit;
	}
	if(iport == 0)
	{
		SoapError(h, 706, "InternalPortWilcardingNotAllowed");
		goto clear_and_exit;
	}

	/* In particular, [IGD2] RECOMMENDS that unauthenticated and
	 * unauthorized control points are only allowed to invoke
	 * this action with:
	 * - InternalPort value greater than or equal to 1024,
	 * - InternalClient value equals to the control point's IP address.
	 * It is REQUIRED that InternalClient cannot be one of IPv6
	 * addresses used by the gateway. */
	if(!int_ip || int_ip[0] == '\0' || 0 == strcmp(int_ip, "*"))
	{
		SoapError(h, 708, "WildCardNotPermittedInSrcIP");
		goto clear_and_exit;
	}
	/* I guess it is useless to convert int_ip to literal ipv6 address */
	if(rem_host)
	{
		/* trim */
		while(isspace(rem_host[0]))
			rem_host++;
	}
	/* rem_host should be converted to literal ipv6 : */
	if(rem_host && (rem_host[0] != '\0') && (rem_host[0] != '*'))
	{
		struct addrinfo *ai, *p;
		struct addrinfo hints;
		int err;
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_INET6;
		/*hints.ai_flags = */
		/* hints.ai_protocol = proto; */
		err = getaddrinfo(rem_host, rem_port, &hints, &ai);
		if(err == 0)
		{
			/* take the 1st IPv6 address */
			for(p = ai; p; p = p->ai_next)
			{
				if(p->ai_family == AF_INET6)
				{
					inet_ntop(AF_INET6,
					          &(((struct sockaddr_in6 *)p->ai_addr)->sin6_addr),
					          rem_ip, sizeof(rem_ip));
					syslog(LOG_INFO, "resolved '%s' to '%s'", rem_host, rem_ip);
					rem_host = rem_ip;
					break;
				}
			}
			freeaddrinfo(ai);
		}
		else
		{
			syslog(LOG_WARNING, "AddPinhole : getaddrinfo(%s) : %s",
			       rem_host, gai_strerror(err));
#if 0
			SoapError(h, 402, "Invalid Args");
			goto clear_and_exit;
#endif
		}
	}

	if(proto == 65535)
	{
		SoapError(h, 707, "ProtocolWilcardingNotAllowed");
		goto clear_and_exit;
	}
	if(proto != IPPROTO_UDP && proto != IPPROTO_TCP
#ifdef IPPROTO_UDPITE
	   && atoi(protocol) != IPPROTO_UDPLITE
#endif
	  )
	{
		SoapError(h, 705, "ProtocolNotSupported");
		goto clear_and_exit;
	}
	if(ltime < 1 || ltime > 86400)
	{
		syslog(LOG_WARNING, "%s: LeaseTime=%d not supported, (ip=%s)",
		       action, ltime, int_ip);
		SoapError(h, 402, "Invalid Args");
		goto clear_and_exit;
	}

	if(PinholeVerification(h, int_ip, iport) <= 0)
		goto clear_and_exit;

	syslog(LOG_INFO, "%s: (inbound) from [%s]:%hu to [%s]:%hu with proto %ld during %d sec",
	       action, rem_host?rem_host:"any",
	       rport, int_ip, iport,
	       proto, ltime);

	/* In cases where the RemoteHost, RemotePort, InternalPort,
	 * InternalClient and Protocol are the same than an existing pinhole,
	 * but LeaseTime is different, the device MUST extend the existing
	 * pinhole's lease time and return the UniqueID of the existing pinhole. */
	r = upnp_add_inboundpinhole(rem_host, rport, int_ip, iport, proto, "IGD2 pinhole", ltime, &uid);

	switch(r)
	{
		case 1:	        /* success */
			bodylen = snprintf(body, sizeof(body),
			                   resp, action,
			                   ns/*"urn:schemas-upnp-org:service:WANIPv6FirewallControl:1"*/,
			                   uid, action);
			BuildSendAndCloseSoapResp(h, body, bodylen);
			break;
		case -1: 	/* not permitted */
			SoapError(h, 701, "PinholeSpaceExhausted");
			break;
		default:
			SoapError(h, 501, "Action Failed");
			break;
	}
	/* 606 Action not authorized
	 * 701 PinholeSpaceExhausted
	 * 702 FirewallDisabled
	 * 703 InboundPinholeNotAllowed
	 * 705 ProtocolNotSupported
	 * 706 InternalPortWildcardingNotAllowed
	 * 707 ProtocolWildcardingNotAllowed
	 * 708 WildCardNotPermittedInSrcIP */
clear_and_exit:
	ClearNameValueList(&data);
}

static void
UpdatePinhole(struct upnphttp * h, const char * action, const char * ns)
{
#if 0
	static const char resp[] =
		"<u:UpdatePinholeResponse "
		"xmlns:u=\"urn:schemas-upnp-org:service:WANIPv6FirewallControl:1\">\r\n"
		"</u:UpdatePinholeResponse>\r\n";
#endif
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"</u:%sResponse>\r\n";
	char body[512];
	int bodylen;
	struct NameValueParserData data;
	const char * uid_str, * leaseTime;
	char iaddr[INET6_ADDRSTRLEN];
	unsigned short iport;
	int ltime;
	int uid;
	int n;

	if(CheckStatus(h)==0)
		return;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	uid_str = GetValueFromNameValueList(&data, "UniqueID");
	leaseTime = GetValueFromNameValueList(&data, "NewLeaseTime");
	uid = uid_str ? atoi(uid_str) : -1;
	ltime = leaseTime ? atoi(leaseTime) : -1;
	ClearNameValueList(&data);

	if(uid < 0 || uid > 65535 || ltime <= 0 || ltime > 86400)
	{
		SoapError(h, 402, "Invalid Args");
		return;
	}

	/* Check that client is not updating an pinhole
	 * it doesn't have access to, because of its public access */
	n = upnp_get_pinhole_info(uid, NULL, 0, NULL,
	                          iaddr, sizeof(iaddr), &iport,
	                          NULL, /* proto */
	                          NULL, 0, /* desc, desclen */
	                          NULL, NULL);
	if (n >= 0)
	{
		if(PinholeVerification(h, iaddr, iport) <= 0)
			return;
	}
	else if(n == -2)
	{
		SoapError(h, 704, "NoSuchEntry");
		return;
	}
	else
	{
		SoapError(h, 501, "Action Failed");
		return;
	}

	syslog(LOG_INFO, "%s: (inbound) updating lease duration to %d for pinhole with ID: %d",
	       action, ltime, uid);

	n = upnp_update_inboundpinhole(uid, ltime);
	if(n == -1)
		SoapError(h, 704, "NoSuchEntry");
	else if(n < 0)
		SoapError(h, 501, "Action Failed");
	else {
		bodylen = snprintf(body, sizeof(body), resp,
		                   action, ns, action);
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}
}

static void
GetOutboundPinholeTimeout(struct upnphttp * h, const char * action, const char * ns)
{
	int r;

	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<OutboundPinholeTimeout>%d</OutboundPinholeTimeout>\r\n"
		"</u:%sResponse>\r\n";

	char body[512];
	int bodylen;
	struct NameValueParserData data;
	char * int_ip, * int_port, * rem_host, * rem_port, * protocol;
	int opt=0;
	/*int proto=0;*/
	unsigned short iport, rport;

	if (GETFLAG(IPV6FCFWDISABLEDMASK))
	{
		SoapError(h, 702, "FirewallDisabled");
		return;
	}

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	int_ip = GetValueFromNameValueList(&data, "InternalClient");
	int_port = GetValueFromNameValueList(&data, "InternalPort");
	rem_host = GetValueFromNameValueList(&data, "RemoteHost");
	rem_port = GetValueFromNameValueList(&data, "RemotePort");
	protocol = GetValueFromNameValueList(&data, "Protocol");

	if (!int_port || !rem_port || !protocol)
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}

	rport = (unsigned short)atoi(rem_port);
	iport = (unsigned short)atoi(int_port);
	/*proto = atoi(protocol);*/

	syslog(LOG_INFO, "%s: retrieving timeout for outbound pinhole from [%s]:%hu to [%s]:%hu protocol %s", action, int_ip, iport,rem_host, rport, protocol);

	/* TODO */
	r = -1;/*upnp_check_outbound_pinhole(proto, &opt);*/

	switch(r)
	{
		case 1:	/* success */
			bodylen = snprintf(body, sizeof(body), resp,
			                   action, ns/*"urn:schemas-upnp-org:service:WANIPv6FirewallControl:1"*/,
			                   opt, action);
			BuildSendAndCloseSoapResp(h, body, bodylen);
			break;
		case -5:	/* Protocol not supported */
			SoapError(h, 705, "ProtocolNotSupported");
			break;
		default:
			SoapError(h, 501, "Action Failed");
	}
	ClearNameValueList(&data);
}

static void
DeletePinhole(struct upnphttp * h, const char * action, const char * ns)
{
	int n;
#if 0
	static const char resp[] =
		"<u:DeletePinholeResponse "
		"xmlns:u=\"urn:schemas-upnp-org:service:WANIPv6FirewallControl:1\">\r\n"
		"</u:DeletePinholeResponse>\r\n";
#endif
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"</u:%sResponse>\r\n";
	char body[512];
	int bodylen;

	struct NameValueParserData data;
	const char * uid_str;
	char iaddr[INET6_ADDRSTRLEN];
	int proto;
	unsigned short iport;
	unsigned int leasetime;
	int uid;

	if(CheckStatus(h)==0)
		return;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	uid_str = GetValueFromNameValueList(&data, "UniqueID");
	uid = uid_str ? atoi(uid_str) : -1;
	ClearNameValueList(&data);

	if(uid < 0 || uid > 65535)
	{
		SoapError(h, 402, "Invalid Args");
		return;
	}

	/* Check that client is not deleting an pinhole
	 * it doesn't have access to, because of its public access */
	n = upnp_get_pinhole_info(uid, NULL, 0, NULL,
	                          iaddr, sizeof(iaddr), &iport,
	                          &proto,
	                          NULL, 0, /* desc, desclen */
	                          &leasetime, NULL);
	if (n >= 0)
	{
		if(PinholeVerification(h, iaddr, iport) <= 0)
			return;
	}
	else if(n == -2)
	{
		SoapError(h, 704, "NoSuchEntry");
		return;
	}
	else
	{
		SoapError(h, 501, "Action Failed");
		return;
	}

	n = upnp_delete_inboundpinhole(uid);
	if(n < 0)
	{
		syslog(LOG_INFO, "%s: (inbound) failed to remove pinhole with ID: %d",
	           action, uid);
		SoapError(h, 501, "Action Failed");
		return;
	}
	syslog(LOG_INFO, "%s: (inbound) pinhole with ID %d successfully removed",
	       action, uid);
	bodylen = snprintf(body, sizeof(body), resp,
	                   action, ns, action);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void
CheckPinholeWorking(struct upnphttp * h, const char * action, const char * ns)
{
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<IsWorking>%d</IsWorking>\r\n"
		"</u:%sResponse>\r\n";
	char body[512];
	int bodylen;
	int r;
	struct NameValueParserData data;
	const char * uid_str;
	int uid;
	char iaddr[INET6_ADDRSTRLEN];
	unsigned short iport;
	unsigned int packets;

	if(CheckStatus(h)==0)
		return;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	uid_str = GetValueFromNameValueList(&data, "UniqueID");
	uid = uid_str ? atoi(uid_str) : -1;
	ClearNameValueList(&data);

	if(uid < 0 || uid > 65535)
	{
		SoapError(h, 402, "Invalid Args");
		return;
	}

	/* Check that client is not checking a pinhole
	 * it doesn't have access to, because of its public access */
	r = upnp_get_pinhole_info(uid,
	                          NULL, 0, NULL,
	                          iaddr, sizeof(iaddr), &iport,
	                          NULL, /* proto */
	                          NULL, 0, /* desc, desclen */
	                          NULL, &packets);
	if (r >= 0)
	{
		if(PinholeVerification(h, iaddr, iport) <= 0)
			return ;
		if(packets == 0)
		{
			SoapError(h, 709, "NoTrafficReceived");
			return;
		}
		bodylen = snprintf(body, sizeof(body), resp,
						action, ns/*"urn:schemas-upnp-org:service:WANIPv6FirewallControl:1"*/,
						1, action);
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}
	else if(r == -2)
		SoapError(h, 704, "NoSuchEntry");
	else
		SoapError(h, 501, "Action Failed");
}

static void
GetPinholePackets(struct upnphttp * h, const char * action, const char * ns)
{
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<PinholePackets>%u</PinholePackets>\r\n"
		"</u:%sResponse>\r\n";
	char body[512];
	int bodylen;
	struct NameValueParserData data;
	const char * uid_str;
	int n;
	char iaddr[INET6_ADDRSTRLEN];
	unsigned short iport;
	unsigned int packets = 0;
	int uid;
	int proto;
	unsigned int leasetime;

	if(CheckStatus(h)==0)
		return;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	uid_str = GetValueFromNameValueList(&data, "UniqueID");
	uid = uid_str ? atoi(uid_str) : -1;
	ClearNameValueList(&data);

	if(uid < 0 || uid > 65535)
	{
		SoapError(h, 402, "Invalid Args");
		return;
	}

	/* Check that client is not getting infos of a pinhole
	 * it doesn't have access to, because of its public access */
	n = upnp_get_pinhole_info(uid, NULL, 0, NULL,
	                          iaddr, sizeof(iaddr), &iport,
	                          &proto,
	                          NULL, 0, /* desc, desclen */
	                          &leasetime, &packets);
	if (n >= 0)
	{
		if(PinholeVerification(h, iaddr, iport)<=0)
			return ;
	}
#if 0
	else if(r == -4 || r == -1)
	{
		SoapError(h, 704, "NoSuchEntry");
	}
#endif

	bodylen = snprintf(body, sizeof(body), resp,
			action, ns/*"urn:schemas-upnp-org:service:WANIPv6FirewallControl:1"*/,
			packets, action);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}
#endif

#ifdef ENABLE_DP_SERVICE
static void
SendSetupMessage(struct upnphttp * h, const char * action, const char * ns)
{
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<OutMessage>%s</OutMessage>\r\n"
		"</u:%sResponse>\r\n";
	char body[1024];
	int bodylen;
	struct NameValueParserData data;
	const char * ProtocolType;	/* string */
	const char * InMessage;		/* base64 */
	const char * OutMessage = "";	/* base64 */

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	ProtocolType = GetValueFromNameValueList(&data, "ProtocolType");	/* string */
	InMessage = GetValueFromNameValueList(&data, "InMessage");	/* base64 */

	if(ProtocolType == NULL || InMessage == NULL)
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}
	/*if(strcmp(ProtocolType, "DeviceProtection:1") != 0)*/
	if(strcmp(ProtocolType, "WPS") != 0)
	{
		ClearNameValueList(&data);
		SoapError(h, 600, "Argument Value Invalid"); /* 703 ? */
		return;
	}
	/* TODO : put here code for WPS */

	bodylen = snprintf(body, sizeof(body), resp,
	                   action, ns/*"urn:schemas-upnp-org:service:DeviceProtection:1"*/,
	                   OutMessage, action);
	BuildSendAndCloseSoapResp(h, body, bodylen);
	ClearNameValueList(&data);
}

static void
GetSupportedProtocols(struct upnphttp * h, const char * action, const char * ns)
{
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<ProtocolList><![CDATA[%s]]></ProtocolList>\r\n"
		"</u:%sResponse>\r\n";
	char body[1024];
	int bodylen;
	const char * ProtocolList =
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<SupportedProtocols xmlns=\"urn:schemas-upnp-org:gw:DeviceProtection\""
		" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
		" xsi:schemaLocation=\"urn:schemas-upnp-org:gw:DeviceProtection"
		" http://www.upnp.org/schemas/gw/DeviceProtection-v1.xsd\">\r\n"
		"<Introduction><Name>WPS</Name></Introduction>\r\n"
		"<Login><Name>PKCS5</Name></Login>\r\n"
		"</SupportedProtocols>\r\n";

	bodylen = snprintf(body, sizeof(body), resp,
	                   action, ns/*"urn:schemas-upnp-org:service:DeviceProtection:1"*/,
	                   ProtocolList, action);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void
GetAssignedRoles(struct upnphttp * h, const char * action, const char * ns)
{
	static const char resp[] =
		"<u:%sResponse "
		"xmlns:u=\"%s\">\r\n"
		"<RoleList>%s</RoleList>\r\n"
		"</u:%sResponse>\r\n";
	char body[1024];
	int bodylen;
	const char * RoleList = "Public"; /* list of roles separated by spaces */

#ifdef ENABLE_HTTPS
	if(h->ssl != NULL) {
		/* we should get the Roles of the session (based on client certificate) */
		X509 * peercert;
		peercert = SSL_get_peer_certificate(h->ssl);
		if(peercert != NULL) {
			RoleList = "Admin Basic";
			X509_free(peercert);
		}
	}
#endif

	bodylen = snprintf(body, sizeof(body), resp,
	                   action, ns/*"urn:schemas-upnp-org:service:DeviceProtection:1"*/,
	                   RoleList, action);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}
#endif

/* Windows XP as client send the following requests :
 * GetConnectionTypeInfo
 * GetNATRSIPStatus
 * ? GetTotalBytesSent - WANCommonInterfaceConfig
 * ? GetTotalBytesReceived - idem
 * ? GetTotalPacketsSent - idem
 * ? GetTotalPacketsReceived - idem
 * GetCommonLinkProperties - idem
 * GetStatusInfo - WANIPConnection
 * GetExternalIPAddress
 * QueryStateVariable / ConnectionStatus!
 */
static const struct
{
	const char * methodName;
	void (*methodImpl)(struct upnphttp *, const char *, const char *);
}
soapMethods[] =
{
	/* WANCommonInterfaceConfig */
	{ "QueryStateVariable", QueryStateVariable},
	{ "GetTotalBytesSent", GetTotalBytesSent},
	{ "GetTotalBytesReceived", GetTotalBytesReceived},
	{ "GetTotalPacketsSent", GetTotalPacketsSent},
	{ "GetTotalPacketsReceived", GetTotalPacketsReceived},
	{ "GetCommonLinkProperties", GetCommonLinkProperties},
	{ "GetStatusInfo", GetStatusInfo},
	/* WANIPConnection */
	{ "GetConnectionTypeInfo", GetConnectionTypeInfo },
	{ "GetNATRSIPStatus", GetNATRSIPStatus},
	{ "GetExternalIPAddress", GetExternalIPAddress},
	{ "AddPortMapping", AddPortMapping},
	{ "DeletePortMapping", DeletePortMapping},
	{ "GetGenericPortMappingEntry", GetGenericPortMappingEntry},
	{ "GetSpecificPortMappingEntry", GetSpecificPortMappingEntry},
/* Required in WANIPConnection:2 */
	{ "SetConnectionType", SetConnectionType},
	{ "RequestConnection", RequestConnection},
	{ "ForceTermination", ForceTermination},
	{ "AddAnyPortMapping", AddAnyPortMapping},
	{ "DeletePortMappingRange", DeletePortMappingRange},
	{ "GetListOfPortMappings", GetListOfPortMappings},
#ifdef ENABLE_L3F_SERVICE
	/* Layer3Forwarding */
	{ "SetDefaultConnectionService", SetDefaultConnectionService},
	{ "GetDefaultConnectionService", GetDefaultConnectionService},
#endif
#ifdef ENABLE_6FC_SERVICE
	/* WANIPv6FirewallControl */
	{ "GetFirewallStatus", GetFirewallStatus},	/* Required */
	{ "AddPinhole", AddPinhole},				/* Required */
	{ "UpdatePinhole", UpdatePinhole},			/* Required */
	{ "GetOutboundPinholeTimeout", GetOutboundPinholeTimeout},	/* Optional */
	{ "DeletePinhole", DeletePinhole},			/* Required */
	{ "CheckPinholeWorking", CheckPinholeWorking},	/* Optional */
	{ "GetPinholePackets", GetPinholePackets},	/* Required */
#endif
#ifdef ENABLE_DP_SERVICE
	/* DeviceProtection */
	{ "SendSetupMessage", SendSetupMessage},	/* Required */
	{ "GetSupportedProtocols", GetSupportedProtocols},	/* Required */
	{ "GetAssignedRoles", GetAssignedRoles},	/* Required */
#endif
	{ 0, 0 }
};

void
ExecuteSoapAction(struct upnphttp * h, const char * action, int n)
{
	char * p;
	int i, len, methodlen;
	char namespace[256];

	/* SoapAction example :
	 * urn:schemas-upnp-org:service:WANIPConnection:1#GetStatusInfo */
	p = memchr(action, '#', n);
	if(p) {
		for(i = 0; i < ((int)sizeof(namespace) - 1) && (action + i) < p; i++)
			namespace[i] = action[i];
		namespace[i] = '\0';
		p++;
		methodlen = n - (int)(p - action);
		if(p[methodlen-1] == '"') {
			methodlen--;	/* remove the ending " */
		}
		/*syslog(LOG_DEBUG, "SoapMethod: %.*s %d %d %p %p %d",
		       methodlen, p, methodlen, n, action, p, (int)(p - action));*/
		for(i = 0; soapMethods[i].methodName; i++) {
			len = strlen(soapMethods[i].methodName);
			if((len == methodlen) && memcmp(p, soapMethods[i].methodName, len) == 0) {
#ifdef DEBUG
				syslog(LOG_DEBUG, "Remote Call of SoapMethod '%s' %s",
				       soapMethods[i].methodName, namespace);
#endif /* DEBUG */
				soapMethods[i].methodImpl(h, soapMethods[i].methodName, namespace);
				return;
			}
		}
		syslog(LOG_NOTICE, "SoapMethod: Unknown: %.*s %s", methodlen, p, namespace);
	} else {
		syslog(LOG_NOTICE, "cannot parse SoapAction");
	}

	SoapError(h, 401, "Invalid Action");
}

/* Standard Errors:
 *
 * errorCode errorDescription Description
 * --------	---------------- -----------
 * 401 		Invalid Action 	No action by that name at this service.
 * 402 		Invalid Args 	Could be any of the following: not enough in args,
 * 							too many in args, no in arg by that name,
 * 							one or more in args are of the wrong data type.
 * 403 		Out of Sync 	Out of synchronization.
 * 501 		Action Failed 	May be returned in current state of service
 * 							prevents invoking that action.
 * 600-699 	TBD 			Common action errors. Defined by UPnP Forum
 * 							Technical Committee.
 * 700-799 	TBD 			Action-specific errors for standard actions.
 * 							Defined by UPnP Forum working committee.
 * 800-899 	TBD 			Action-specific errors for non-standard actions.
 * 							Defined by UPnP vendor.
*/
void
SoapError(struct upnphttp * h, int errCode, const char * errDesc)
{
	static const char resp[] =
		"<s:Envelope "
		"xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
		"s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
		"<s:Body>\r\n"
		"<s:Fault>\r\n"
		"<faultcode>s:Client</faultcode>\r\n"
		"<faultstring>UPnPError</faultstring>\r\n"
		"<detail>\r\n"
		"<UPnPError xmlns=\"urn:schemas-upnp-org:control-1-0\">\r\n"
		"<errorCode>%d</errorCode>\r\n"
		"<errorDescription>%s</errorDescription>\r\n"
		"</UPnPError>\r\n"
		"</detail>\r\n"
		"</s:Fault>\r\n"
		"</s:Body>\r\n"
		"</s:Envelope>\r\n";

	char body[2048];
	int bodylen;

	syslog(LOG_INFO, "Returning UPnPError %d: %s", errCode, errDesc);
	bodylen = snprintf(body, sizeof(body), resp, errCode, errDesc);
	BuildResp2_upnphttp(h, 500, "Internal Server Error", body, bodylen);
	SendRespAndClose_upnphttp(h);
}

