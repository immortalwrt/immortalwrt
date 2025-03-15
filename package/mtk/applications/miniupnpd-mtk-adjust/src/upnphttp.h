/* $Id: upnphttp.h,v 1.42 2015/12/16 10:21:49 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2021 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef UPNPHTTP_H_INCLUDED
#define UPNPHTTP_H_INCLUDED

#include <netinet/in.h>
#include <sys/queue.h>

#include "config.h"

#ifdef ENABLE_HTTPS
#include <openssl/ssl.h>
#endif /* ENABLE_HTTPS */

#define UPNP_VERSION_STRING "UPnP/" UPNP_VERSION_MAJOR_STR "." UPNP_VERSION_MINOR_STR

/* server: HTTP header returned in all HTTP responses : */
#define MINIUPNPD_SERVER_STRING	OS_VERSION " " UPNP_VERSION_STRING " MiniUPnPd/" MINIUPNPD_VERSION

/*
 states :
  0 - waiting for data to read
  1 - waiting for HTTP Post Content.
  ...
  >= 100 - to be deleted
*/
enum httpStates {
	EWaitingForHttpRequest = 0,
	EWaitingForHttpContent,
	ESendingContinue,
	ESendingAndClosing,
	EToDelete = 100
};

enum httpCommands {
	EUnknown = 0,
	EGet,
	EPost,
	ESubscribe,
	EUnSubscribe
};

struct upnphttp {
	int socket;
	struct in_addr clientaddr;	/* client address */
#ifdef ENABLE_IPV6
	int ipv6;
	struct in6_addr clientaddr_v6;
#endif /* ENABLE_IPV6 */
#ifdef ENABLE_HTTPS
	SSL * ssl;
#endif /* ENABLE_HTTPS */
	char clientaddr_str[64];	/* used for syslog() output */
	enum httpStates state;
	char HttpVer[16];
	/* request */
	char * req_buf;
	char accept_language[8];
	int req_buflen;
	int req_contentlen;
	int req_contentoff;     /* header length */
	enum httpCommands req_command;
	int req_soapActionOff;
	int req_soapActionLen;
	int req_HostOff;	/* Host: header */
	int req_HostLen;
#ifdef ENABLE_EVENTS
	int req_CallbackOff;	/* For SUBSCRIBE */
	int req_CallbackLen;
	int req_Timeout;
	int req_SIDOff;		/* For UNSUBSCRIBE */
	int req_SIDLen;
	const char * res_SID;
#ifdef UPNP_STRICT
	int req_NTOff;
	int req_NTLen;
#endif
#endif
	int respflags;				/* see FLAG_* constants below */
	/* response */
	char * res_buf;
	int res_buflen;
	int res_sent;
	int res_buf_alloclen;
	LIST_ENTRY(upnphttp) entries;
};

/* Include the "Timeout:" header in response */
#define FLAG_TIMEOUT	0x01
/* Include the "SID:" header in response */
#define FLAG_SID		0x02

/* If set, the POST request included a "Expect: 100-continue" header */
#define FLAG_CONTINUE	0x40

/* If set, the Content-Type is set to text/xml, otherwise it is text/xml */
#define FLAG_HTML		0x80

/* If set, the corresponding Allow: header is set */
#define FLAG_ALLOW_POST			0x100
#define FLAG_ALLOW_SUB_UNSUB	0x200

/* If set, the User-Agent: contains "microsoft" */
#define FLAG_MS_CLIENT	0x400

#ifdef ENABLE_HTTPS
int init_ssl(void);
void free_ssl(void);
#endif /* ENABLE_HTTPS */

/* New_upnphttp() */
struct upnphttp *
New_upnphttp(int);

#ifdef ENABLE_HTTPS
void
InitSSL_upnphttp(struct upnphttp *);
#endif /* ENABLE_HTTPS */

/* CloseSocket_upnphttp() */
void
CloseSocket_upnphttp(struct upnphttp *);

/* Delete_upnphttp() */
void
Delete_upnphttp(struct upnphttp *);

/* Process_upnphttp() */
void
Process_upnphttp(struct upnphttp *);

/* BuildHeader_upnphttp()
 * build the header for the HTTP Response
 * also allocate the buffer for body data
 * return -1 on error */
int
BuildHeader_upnphttp(struct upnphttp * h, int respcode,
                     const char * respmsg,
                     int bodylen);

/* BuildResp_upnphttp()
 * fill the res_buf buffer with the complete
 * HTTP 200 OK response from the body passed as argument */
void
BuildResp_upnphttp(struct upnphttp *, const char *, int);

/* BuildResp2_upnphttp()
 * same but with given response code/message */
void
BuildResp2_upnphttp(struct upnphttp * h, int respcode,
                    const char * respmsg,
                    const char * body, int bodylen);

int
SendResp_upnphttp(struct upnphttp *);

/* SendRespAndClose_upnphttp() */
void
SendRespAndClose_upnphttp(struct upnphttp *);

#endif

