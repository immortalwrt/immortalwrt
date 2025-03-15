/* $Id: upnphttp.c,v 1.114 2024/01/15 00:13:22 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * Project :  miniupnp
 * Website :  http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * Author :   Thomas Bernard
 * Copyright (c) 2005-2024 Thomas Bernard
 * This software is subject to the conditions detailed in the
 * LICENCE file included in this distribution.
 * */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <ctype.h>
#include <errno.h>
#include "config.h"
#ifdef ENABLE_HTTP_DATE
#include <time.h>
#endif
#include "upnphttp.h"
#include "upnpdescgen.h"
#include "miniupnpdpath.h"
#include "upnpsoap.h"
#include "upnpevents.h"
#include "upnputils.h"
#if defined(RANDOMIZE_URLS) || defined(DYNAMIC_OS_VERSION)
#include "upnpglobalvars.h"
#endif /* RANDOMIZE_URLS */

#ifdef ENABLE_HTTPS
#include <openssl/err.h>
#include <openssl/engine.h>
#include <openssl/conf.h>
static SSL_CTX *ssl_ctx = NULL;

#ifndef HTTPS_CERTFILE
#define HTTPS_CERTFILE "/etc/ssl/certs/ssl-cert-snakeoil.pem"
#endif
#ifndef HTTPS_KEYFILE
#define HTTPS_KEYFILE "/etc/ssl/private/ssl-cert-snakeoil.key"
#endif

static void
syslogsslerr(void)
{
	unsigned long err;
	char buffer[256];
	while((err = ERR_get_error()) != 0) {
		syslog(LOG_ERR, "%s", ERR_error_string(err, buffer));
	}
}

static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx)
{
	syslog(LOG_DEBUG, "verify_callback(%d, %p)", preverify_ok, ctx);
	return preverify_ok;
}

int init_ssl(void)
{
	const SSL_METHOD *method;
	SSL_library_init();
	SSL_load_error_strings();
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	method = TLSv1_server_method();
#else
	method = TLS_server_method();
#endif
	if(method == NULL) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
		syslog(LOG_ERR, "TLSv1_server_method() failed");
#else
		syslog(LOG_ERR, "TLS_server_method() failed");
#endif
		syslogsslerr();
		return -1;
	}
	ssl_ctx = SSL_CTX_new(method);
	if(ssl_ctx == NULL) {
		syslog(LOG_ERR, "SSL_CTX_new() failed");
		syslogsslerr();
		return -1;
	}
	/* set the local certificate */
	if(!SSL_CTX_use_certificate_file(ssl_ctx, HTTPS_CERTFILE, SSL_FILETYPE_PEM)) {
		syslog(LOG_ERR, "SSL_CTX_use_certificate_file(%s) failed", HTTPS_CERTFILE);
		syslogsslerr();
		return -1;
	}
	/* set the private key */
	if(!SSL_CTX_use_PrivateKey_file(ssl_ctx, HTTPS_KEYFILE, SSL_FILETYPE_PEM)) {
		syslog(LOG_ERR, "SSL_CTX_use_PrivateKey_file(%s) failed", HTTPS_KEYFILE);
		syslogsslerr();
		return -1;
	}
	/* verify private key */
	if(!SSL_CTX_check_private_key(ssl_ctx)) {
		syslog(LOG_ERR, "SSL_CTX_check_private_key() failed");
		syslogsslerr();
		return -1;
	}
	/*SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER|SSL_VERIFY_CLIENT_ONCE, verify_callback);*/
	SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, verify_callback);
	/*SSL_CTX_set_verify_depth(depth);*/
	syslog(LOG_INFO, "using %s", SSLeay_version(SSLEAY_VERSION));
	return 0;
}

void free_ssl(void)
{
	/* free context */
	if(ssl_ctx != NULL) {
		SSL_CTX_free(ssl_ctx);
		ssl_ctx = NULL;
	}
#if OPENSSL_VERSION_NUMBER >= 0x10000000L && OPENSSL_VERSION_NUMBER < 0x10100000L
	ERR_remove_thread_state(NULL);
#elif OPENSSL_VERSION_NUMBER < 0x10000000L
	ERR_remove_state(0);
#endif
	ENGINE_cleanup();
	CONF_modules_unload(1);
	ERR_free_strings();
	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
}
#endif /* ENABLE_HTTPS */

struct upnphttp *
New_upnphttp(int s)
{
	struct upnphttp * ret;
	if(s<0)
		return NULL;
	ret = (struct upnphttp *)malloc(sizeof(struct upnphttp));
	if(ret == NULL)
		return NULL;
	memset(ret, 0, sizeof(struct upnphttp));
	ret->socket = s;
	if(!set_non_blocking(s))
		syslog(LOG_WARNING, "New_upnphttp::set_non_blocking(): %m");
	return ret;
}

#ifdef ENABLE_HTTPS
void
InitSSL_upnphttp(struct upnphttp * h)
{
	int r;
	h->ssl = SSL_new(ssl_ctx);
	if(h->ssl == NULL) {
		syslog(LOG_ERR, "SSL_new() failed");
		syslogsslerr();
		abort();
	}
	if(!SSL_set_fd(h->ssl, h->socket)) {
		syslog(LOG_ERR, "SSL_set_fd() failed");
		syslogsslerr();
		abort();
	}
	r = SSL_accept(h->ssl); /* start the handshaking */
	if(r < 0) {
		int err;
		err = SSL_get_error(h->ssl, r);
		syslog(LOG_DEBUG, "SSL_accept() returned %d, SSL_get_error() %d", r, err);
		if(err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE) {
			syslog(LOG_ERR, "SSL_accept() failed");
			syslogsslerr();
			abort();
		}
	}
}
#endif /* ENABLE_HTTPS */

void
CloseSocket_upnphttp(struct upnphttp * h)
{
	/* SSL_shutdown() ? */
	if(close(h->socket) < 0)
	{
		syslog(LOG_ERR, "CloseSocket_upnphttp: close(%d): %m", h->socket);
	}
	h->socket = -1;
	h->state = EToDelete;
}

void
Delete_upnphttp(struct upnphttp * h)
{
	if(h)
	{
#ifdef ENABLE_HTTPS
		if(h->ssl)
			SSL_free(h->ssl);
#endif
		if(h->socket >= 0)
			CloseSocket_upnphttp(h);
		if(h->req_buf)
			free(h->req_buf);
		if(h->res_buf)
			free(h->res_buf);
		free(h);
	}
}

/* parse HttpHeaders of the REQUEST
 * This function is called after the \r\n\r\n character
 * sequence has been found in h->req_buf */
static void
ParseHttpHeaders(struct upnphttp * h)
{
	char * line;
	char * colon;
	char * p;
	int n;
	if((h->req_buf == NULL) || (h->req_contentoff <= 0))
		return;
	line = h->req_buf;
	while(line < (h->req_buf + h->req_contentoff))
	{
		colon = line;
		while(*colon != ':')
		{
			if(*colon == '\r' || *colon == '\n')
			{
				colon = NULL;	/* no ':' character found on the line */
				break;
			}
			colon++;
		}
		if(colon)
		{
			if(strncasecmp(line, "Content-Length:", 15)==0)
			{
				p = colon;
				while((*p < '0' || *p > '9') && (*p != '\r') && (*p != '\n'))
					p++;
				h->req_contentlen = atoi(p);
				if(h->req_contentlen < 0) {
					syslog(LOG_WARNING, "ParseHttpHeaders() invalid Content-Length %d", h->req_contentlen);
					h->req_contentlen = 0;
				}
				/*printf("*** Content-Lenght = %d ***\n", h->req_contentlen);
				printf("    readbufflen=%d contentoff = %d\n",
					h->req_buflen, h->req_contentoff);*/
			}
			else if(strncasecmp(line, "Host:", 5)==0)
			{
				p = colon;
				n = 0;
				while(*p == ':' || *p == ' ' || *p == '\t')
					p++;
				while(p[n]>' ')
					n++;
				h->req_HostOff = p - h->req_buf;
				h->req_HostLen = n;
			}
			else if(strncasecmp(line, "SOAPAction:", 11)==0)
			{
				p = colon;
				n = 0;
				while(*p == ':' || *p == ' ' || *p == '\t')
					p++;
				while(p[n]>=' ')
					n++;
				if((p[0] == '"' && p[n-1] == '"')
				  || (p[0] == '\'' && p[n-1] == '\''))
				{
					p++; n -= 2;
				}
				h->req_soapActionOff = p - h->req_buf;
				h->req_soapActionLen = n;
			}
			else if(strncasecmp(line, "accept-language:", 16) == 0)
			{
				p = colon;
				n = 0;
				while(*p == ':' || *p == ' ' || *p == '\t')
					p++;
				while(p[n]>=' ')
					n++;
				syslog(LOG_DEBUG, "accept-language HTTP header : '%.*s'", n, p);
				/* keep only the 1st accepted language */
				n = 0;
				while(p[n]>' ' && p[n] != ',')
					n++;
				if(n >= (int)sizeof(h->accept_language))
					n = (int)sizeof(h->accept_language) - 1;
				memcpy(h->accept_language, p, n);
				h->accept_language[n] = '\0';
			}
			else if(strncasecmp(line, "expect:", 7) == 0)
			{
				p = colon;
				n = 0;
				while(*p == ':' || *p == ' ' || *p == '\t')
					p++;
				while(p[n]>=' ')
					n++;
				if(strncasecmp(p, "100-continue", 12) == 0) {
					h->respflags |= FLAG_CONTINUE;
					syslog(LOG_DEBUG, "\"Expect: 100-Continue\" header detected");
				}
			}
			else if(strncasecmp(line, "user-agent:", 11) == 0)
			{
				/* - User-Agent: Microsoft-Windows/10.0 UPnP/1.0
				 * - User-Agent: FDSSDP                           */
				if(strcasestr(line + 11, "microsoft") != NULL || strstr(line + 11, "FDSSDP") != NULL) {
					h->respflags |= FLAG_MS_CLIENT;
				}
			}
#ifdef ENABLE_EVENTS
			else if(strncasecmp(line, "Callback:", 9)==0)
			{
				/* The Callback can contain several urls :
				 * If there is more than one URL, when the service sends
				 * events, it will try these URLs in order until one
				 * succeeds. One or more URLs each enclosed by angle
				 * brackets ("<" and ">") */
				p = colon;
				while(*p != '<' && *p != '\r' )
					p++;
				n = 0;
				while(p[n] != '\r')
					n++;
				while(n > 0 && p[n] != '>')
					n--;
				/* found last > character */
				h->req_CallbackOff = p - h->req_buf;
				h->req_CallbackLen = MAX(0, n + 1);
			}
			else if(strncasecmp(line, "SID:", 4)==0)
			{
				p = colon + 1;
				while((*p == ' ') || (*p == '\t'))
					p++;
				n = 0;
				while(!isspace(p[n]))
					n++;
				h->req_SIDOff = p - h->req_buf;
				h->req_SIDLen = n;
			}
			/* Timeout: Seconds-nnnn */
/* TIMEOUT
Recommended. Requested duration until subscription expires,
either number of seconds or infinite. Recommendation
by a UPnP Forum working committee. Defined by UPnP vendor.
 Consists of the keyword "Second-" followed (without an
intervening space) by either an integer or the keyword "infinite". */
			else if(strncasecmp(line, "Timeout:", 8)==0)
			{
				p = colon + 1;
				while((*p == ' ') || (*p == '\t'))
					p++;
				if(strncasecmp(p, "Second-", 7)==0) {
					h->req_Timeout = atoi(p+7);
				}
			}
#ifdef UPNP_STRICT
			else if(strncasecmp(line, "nt:", 3)==0)
			{
				p = colon + 1;
				while((*p == ' ') || (*p == '\t'))
					p++;
				n = 0;
				while(!isspace(p[n]))
					n++;
				h->req_NTOff = p - h->req_buf;
				h->req_NTLen = n;
			}
#endif /* UPNP_STRICT */
#endif /* ENABLE_EVENTS */
		}
		/* the loop below won't run off the end of the buffer
		 * because the buffer is guaranteed to contain the \r\n\r\n
		 * character sequence */
		while(!(line[0] == '\r' && line[1] == '\n'))
			line++;
		line += 2;
	}
}

/* very minimalistic 404 error message */
static void
Send404(struct upnphttp * h)
{
	static const char body404[] =
		"<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD>"
		"<BODY><H1>Not Found</H1>The requested URL was not found"
		" on this server.</BODY></HTML>\r\n";

	h->respflags = FLAG_HTML;
	BuildResp2_upnphttp(h, 404, "Not Found",
	                    body404, sizeof(body404) - 1);
	SendRespAndClose_upnphttp(h);
}

static void
Send405(struct upnphttp * h)
{
	static const char body405[] =
		"<HTML><HEAD><TITLE>405 Method Not Allowed</TITLE></HEAD>"
		"<BODY><H1>Method Not Allowed</H1>The HTTP Method "
		"is not allowed on this resource.</BODY></HTML>\r\n";

	h->respflags |= FLAG_HTML;
	BuildResp2_upnphttp(h, 405, "Method Not Allowed",
	                    body405, sizeof(body405) - 1);
	SendRespAndClose_upnphttp(h);
}

/* very minimalistic 501 error message */
static void
Send501(struct upnphttp * h)
{
	static const char body501[] =
		"<HTML><HEAD><TITLE>501 Not Implemented</TITLE></HEAD>"
		"<BODY><H1>Not Implemented</H1>The HTTP Method "
		"is not implemented by this server.</BODY></HTML>\r\n";

	h->respflags = FLAG_HTML;
	BuildResp2_upnphttp(h, 501, "Not Implemented",
	                    body501, sizeof(body501) - 1);
	SendRespAndClose_upnphttp(h);
}

/* findendheaders() find the \r\n\r\n character sequence and
 * return a pointer to it.
 * It returns NULL if not found */
static const char *
findendheaders(const char * s, int len)
{
	while(len-->3)
	{
		if(s[0]=='\r' && s[1]=='\n' && s[2]=='\r' && s[3]=='\n')
			return s;
		s++;
	}
	return NULL;
}

#ifdef HAS_DUMMY_SERVICE
static void
sendDummyDesc(struct upnphttp * h)
{
	static const char xml_desc[] = "<?xml version=\"1.0\"?>\r\n"
		"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">"
		" <specVersion>"
		"    <major>1</major>"
		"    <minor>0</minor>"
		"  </specVersion>"
		"  <actionList />"
		"  <serviceStateTable />"
		"</scpd>\r\n";
	BuildResp_upnphttp(h, xml_desc, sizeof(xml_desc)-1);
	SendRespAndClose_upnphttp(h);
}
#endif

/* Sends the description generated by the parameter */
static void
sendXMLdesc(struct upnphttp * h, char * (f)(int *, int))
{
	char * desc;
	int len;
#ifdef IGD_V2
#ifdef DEBUG
	if(h->respflags & FLAG_MS_CLIENT)
		syslog(LOG_DEBUG, "MS Client, forcing IGD v1");
#endif /* DEBUG */
	desc = f(&len, GETFLAG(FORCEIGDDESCV1MASK) || (h->respflags & FLAG_MS_CLIENT));
#else
	desc = f(&len, 0);
#endif
	if(!desc)
	{
		static const char error500[] = "<HTML><HEAD><TITLE>Error 500</TITLE>"
		   "</HEAD><BODY>Internal Server Error</BODY></HTML>\r\n";
		syslog(LOG_ERR, "Failed to generate XML description");
		h->respflags = FLAG_HTML;
		BuildResp2_upnphttp(h, 500, "Internal Server Error",
		                    error500, sizeof(error500)-1);
	}
	else
	{
		BuildResp_upnphttp(h, desc, len);
	}
	SendRespAndClose_upnphttp(h);
	free(desc);
}

/* ProcessHTTPPOST_upnphttp()
 * executes the SOAP query if it is possible */
static void
ProcessHTTPPOST_upnphttp(struct upnphttp * h)
{
	if((h->req_buflen - h->req_contentoff) >= h->req_contentlen)
	{
		/* the request body is received */
		if(h->req_soapActionOff > 0)
		{
			/* we can process the request */
			syslog(LOG_INFO, "SOAPAction: %.*s",
			       h->req_soapActionLen, h->req_buf + h->req_soapActionOff);
			ExecuteSoapAction(h,
				h->req_buf + h->req_soapActionOff,
				h->req_soapActionLen);
		}
		else
		{
			static const char err400str[] =
				"<html><body>Bad request</body></html>";
			syslog(LOG_INFO, "No SOAPAction in HTTP headers");
			h->respflags = FLAG_HTML;
			BuildResp2_upnphttp(h, 400, "Bad Request",
			                    err400str, sizeof(err400str) - 1);
			SendRespAndClose_upnphttp(h);
		}
	}
	else if(h->respflags & FLAG_CONTINUE)
	{
		/* Sending the 100 Continue response */
		if(!h->res_buf) {
			h->res_buf = malloc(256);
			h->res_buf_alloclen = 256;
		}
		h->res_buflen = snprintf(h->res_buf, h->res_buf_alloclen,
		                         "%s 100 Continue\r\n\r\n", h->HttpVer);
		h->res_sent = 0;
		h->state = ESendingContinue;
		if(SendResp_upnphttp(h))
			h->state = EWaitingForHttpContent;
	}
	else
	{
		/* waiting for remaining data */
		h->state = EWaitingForHttpContent;
	}
}

#ifdef ENABLE_EVENTS
/**
 * checkCallbackURL()
 * check that url is on originating IP
 * extract first correct URL
 * returns 0 if the callback header value is not valid
 * 1 if it is valid.
 */
static int
checkCallbackURL(struct upnphttp * h)
{
	char addrstr[48];
	int ipv6;
	const char * p;
	unsigned int i;

start_again:
	if(h->req_CallbackOff <= 0 || h->req_CallbackLen < 10)
		return 0;
	if(memcmp(h->req_buf + h->req_CallbackOff, "<http://", 8) != 0) {
		p = h->req_buf + h->req_CallbackOff + 1;
		goto invalid;
	}
	/* extract host from url to addrstr[] */
	i = 0;
	p = h->req_buf + h->req_CallbackOff + 8;
	if(*p == '[') {
		p++;
		ipv6 = 1;
		while(*p != ']' && *p != '>' && i < (sizeof(addrstr)-1)
		      && p < (h->req_buf + h->req_CallbackOff + h->req_CallbackLen))
			addrstr[i++] = *(p++);
	} else {
		ipv6 = 0;
		while(*p != '/' && *p != ':' && *p != '>' && i < (sizeof(addrstr)-1)
		      && p < (h->req_buf + h->req_CallbackOff + h->req_CallbackLen))
			addrstr[i++] = *(p++);
	}
	addrstr[i] = '\0';
	/* check addrstr */
	if(ipv6) {
#ifdef ENABLE_IPV6
		struct in6_addr addr;
		if(inet_pton(AF_INET6, addrstr, &addr) <= 0)
			goto invalid;
		if(!h->ipv6
		  || (0!=memcmp(&addr, &(h->clientaddr_v6), sizeof(struct in6_addr))))
			goto invalid;
#else
		goto invalid;
#endif
	} else {
		struct in_addr addr;
		if(inet_pton(AF_INET, addrstr, &addr) <= 0)
			goto invalid;
#ifdef ENABLE_IPV6
		if(h->ipv6) {
			if(!IN6_IS_ADDR_V4MAPPED(&(h->clientaddr_v6)))
				goto invalid;
			if(0!=memcmp(&addr, ((const char *)&(h->clientaddr_v6) + 12), 4))
				goto invalid;
		} else {
			if(0!=memcmp(&addr, &(h->clientaddr), sizeof(struct in_addr)))
				goto invalid;
		}
#else
		if(0!=memcmp(&addr, &(h->clientaddr), sizeof(struct in_addr)))
			goto invalid;
#endif
	}
	/* select only the good callback url */
	while(p < h->req_buf + h->req_CallbackOff + h->req_CallbackLen && *p != '>')
		p++;
	h->req_CallbackOff++;	/* skip initial '<' */
	h->req_CallbackLen = (int)(p - h->req_buf - h->req_CallbackOff);
	return 1;
invalid:
	while(p < h->req_buf + h->req_CallbackOff + h->req_CallbackLen && *p != '>')
		p++;
	if(*p != '>') return 0;
	while(p < h->req_buf + h->req_CallbackOff + h->req_CallbackLen && *p != '<')
		p++;
	if(*p != '<') return 0;
	h->req_CallbackLen -= (int)(p - h->req_buf - h->req_CallbackOff);
	h->req_CallbackOff = (int)(p - h->req_buf);
	goto start_again;
}

static void
ProcessHTTPSubscribe_upnphttp(struct upnphttp * h, const char * path)
{
	const char * sid;
	syslog(LOG_DEBUG, "ProcessHTTPSubscribe %s", path);
	syslog(LOG_DEBUG, "Callback '%.*s' Timeout=%d",
	       h->req_CallbackLen, h->req_buf + h->req_CallbackOff,
	       h->req_Timeout);
	syslog(LOG_DEBUG, "SID '%.*s'", h->req_SIDLen, h->req_buf + h->req_SIDOff);
#if defined(UPNP_STRICT) && (UPNP_VERSION_MAJOR > 1) || (UPNP_VERSION_MINOR > 0)
	if(h->req_Timeout < 1800) {
		/* Second-infinite is forbidden with UDA v1.1 and later :
		 * (UDA 1.1 : 4.1.1 Subscription)
		 * UPnP 1.1 control points MUST NOT subscribe using keyword infinite,
		 * UPnP 1.1 devices MUST NOT set actual subscription durations to
		 * "infinite". The presence of infinite in a request MUST be silently
		 * ignored by a UPnP 1.1 device (the presence of infinite is handled
		 * by the device as if the TIMEOUT header field in a request was not
		 * present) . The keyword infinite MUST NOT be returned by a UPnP 1.1
		 * device.
		 * Also the device must return a value of minimum 1800 seconds in the
		 * response, according to UDA 1.1 (4.1.2 SUBSCRIBE with NT and CALLBACK):
		 * TIMEOUT
		 *   REQUIRED. Field value contains actual duration until subscription
		 *   expires. Keyword "Second-" followed by an integer (no space).
		 *   SHOULD be greater than or equal to 1800 seconds (30 minutes).*/
		h->req_Timeout = 1800;	/* default to 30 minutes */
	}
#endif /* UPNP_STRICT */
	if((h->req_CallbackOff <= 0) && (h->req_SIDOff <= 0)) {
		/* Missing or invalid CALLBACK : 412 Precondition Failed.
		 * If CALLBACK header is missing or does not contain a valid HTTP URL,
		 * the publisher must respond with HTTP error 412 Precondition Failed*/
		BuildResp2_upnphttp(h, 412, "Precondition Failed", 0, 0);
		SendRespAndClose_upnphttp(h);
	} else {
	/* - add to the subscriber list
	 * - respond HTTP/x.x 200 OK
	 * - Send the initial event message */
/* Server:, SID:; Timeout: Second-(xx|infinite) */
	/* Check that the callback URL is on the same IP as
	 * the request, and not on the internet, nor on ourself (DOS attack ?) */
		if(h->req_CallbackOff > 0) {
#ifdef UPNP_STRICT
			/* SID: and Callback: are incompatible */
			if(h->req_SIDOff > 0) {
				syslog(LOG_WARNING, "Both Callback: and SID: in SUBSCRIBE");
				BuildResp2_upnphttp(h, 400, "Incompatible header fields", 0, 0);
			/* "NT: upnp:event" header is mandatory */
			} else if(h->req_NTOff <= 0 || h->req_NTLen != 10 ||
			   0 != memcmp("upnp:event", h->req_buf + h->req_NTOff, 10)) {
				syslog(LOG_WARNING, "Invalid NT in SUBSCRIBE %.*s",
				       h->req_NTLen, h->req_buf + h->req_NTOff);
				BuildResp2_upnphttp(h, 412, "Precondition Failed", 0, 0);
			} else
#endif
			if(checkCallbackURL(h)) {
				sid = upnpevents_addSubscriber(path, h->req_buf + h->req_CallbackOff,
				                               h->req_CallbackLen, h->req_Timeout);
				h->respflags = FLAG_TIMEOUT;
				if(sid) {
					syslog(LOG_DEBUG, "generated sid=%s", sid);
					h->respflags |= FLAG_SID;
					h->res_SID = sid;
				}
				BuildResp_upnphttp(h, 0, 0);
			} else {
				syslog(LOG_WARNING, "Invalid Callback in SUBSCRIBE %.*s",
				       h->req_CallbackLen, h->req_buf + h->req_CallbackOff);
				BuildResp2_upnphttp(h, 412, "Precondition Failed", 0, 0);
			}
		} else {
			/* subscription renew */
			/* Invalid SID
412 Precondition Failed. If a SID does not correspond to a known,
un-expired subscription, the publisher must respond
with HTTP error 412 Precondition Failed. */
#ifdef UPNP_STRICT
			/* SID: and NT: headers are incompatibles */
			if(h->req_NTOff > 0) {
				syslog(LOG_WARNING, "Both NT: and SID: in SUBSCRIBE");
				BuildResp2_upnphttp(h, 400, "Incompatible header fields", 0, 0);
			} else {
#endif /* UPNP_STRICT */
				sid = upnpevents_renewSubscription(h->req_buf + h->req_SIDOff,
				                                   h->req_SIDLen, h->req_Timeout);
				if(!sid) {
					BuildResp2_upnphttp(h, 412, "Precondition Failed", 0, 0);
				} else {
					h->respflags = FLAG_TIMEOUT | FLAG_SID;
					h->res_SID = sid;
					BuildResp_upnphttp(h, 0, 0);
				}
#ifdef UPNP_STRICT
			}
#endif /* UPNP_STRICT */
		}
		SendRespAndClose_upnphttp(h);
	}
}

static void
ProcessHTTPUnSubscribe_upnphttp(struct upnphttp * h, const char * path)
{
	syslog(LOG_DEBUG, "ProcessHTTPUnSubscribe %s", path);
	syslog(LOG_DEBUG, "SID '%.*s'", h->req_SIDLen, h->req_buf + h->req_SIDOff);
	/* Remove from the list */
#ifdef UPNP_STRICT
	if(h->req_SIDOff <= 0 || h->req_SIDLen == 0) {
		/* SID: header missing or empty */
		BuildResp2_upnphttp(h, 412, "Precondition Failed", 0, 0);
	} else if(h->req_CallbackOff > 0 || h->req_NTOff > 0) {
		/* no NT: or Callback: header must be present */
		BuildResp2_upnphttp(h, 400, "Incompatible header fields", 0, 0);
	} else
#endif
	if(upnpevents_removeSubscriber(h->req_buf + h->req_SIDOff, h->req_SIDLen) < 0) {
		BuildResp2_upnphttp(h, 412, "Precondition Failed", 0, 0);
	} else {
		BuildResp_upnphttp(h, 0, 0);
	}
	SendRespAndClose_upnphttp(h);
}
#endif

/* Parse and process Http Query
 * called once all the HTTP headers have been received,
 * so it is guaranteed that h->req_buf contains the \r\n\r\n
 * character sequence */
static void
ProcessHttpQuery_upnphttp(struct upnphttp * h)
{
	static const struct {
		const char * path;
		char * (* f)(int *, int);
	} path_desc[] = {
		{ ROOTDESC_PATH, genRootDesc},
		{ WANIPC_PATH, genWANIPCn},
		{ WANCFG_PATH, genWANCfg},
#ifdef HAS_DUMMY_SERVICE
		{ DUMMY_PATH, NULL},
#endif
#ifdef ENABLE_L3F_SERVICE
		{ L3F_PATH, genL3F},
#endif
#ifdef ENABLE_6FC_SERVICE
		{ WANIP6FC_PATH, gen6FC},
#endif
#ifdef ENABLE_DP_SERVICE
		{ DP_PATH, genDP},
#endif
		{ NULL, NULL}
	};
	char HttpCommand[16];
	char HttpUrl[128];
	char * HttpVer;
	char * p;
	int i;

	p = h->req_buf;
	if(!p)
		return;
	/* note : checking (*p != '\r') is enough to avoid running off the
	 * end of the buffer, because h->req_buf is guaranteed to contain
	 * the \r\n\r\n character sequence */
	for(i = 0; i<15 && *p != ' ' && *p != '\r'; i++)
		HttpCommand[i] = *(p++);
	HttpCommand[i] = '\0';
	while(*p==' ')
		p++;
	for(i = 0; i<127 && *p != ' ' && *p != '\r'; i++)
		HttpUrl[i] = *(p++);
	HttpUrl[i] = '\0';
	while(*p==' ')
		p++;
	HttpVer = h->HttpVer;
	for(i = 0; i<15 && *p != '\r'; i++)
		HttpVer[i] = *(p++);
	HttpVer[i] = '\0';
	syslog(LOG_INFO, "HTTP REQUEST from %s : %s %s (%s)",
	       h->clientaddr_str, HttpCommand, HttpUrl, HttpVer);
	ParseHttpHeaders(h);
	if(h->req_HostOff > 0 && h->req_HostLen > 0) {
		syslog(LOG_DEBUG, "Host: %.*s", h->req_HostLen, h->req_buf + h->req_HostOff);
		p = h->req_buf + h->req_HostOff;
		if(*p == '[') {
			/* IPv6 */
			p++;
			while(p < h->req_buf + h->req_HostOff + h->req_HostLen) {
				if(*p == ']') break;
				/* TODO check *p in [0-9a-f:.] */
				p++;
			}
			if(*p != ']') {
				syslog(LOG_NOTICE, "DNS rebinding attack suspected (Host: %.*s)", h->req_HostLen, h->req_buf + h->req_HostOff);
				Send404(h);/* 403 */
				return;
			}
			p++;
			/* TODO : Check port */
		} else {
			for(i = 0; i < h->req_HostLen; i++) {
				if(*p != ':' && *p != '.' && (*p > '9' || *p < '0')) {
					syslog(LOG_NOTICE, "DNS rebinding attack suspected (Host: %.*s)", h->req_HostLen, h->req_buf + h->req_HostOff);
					Send404(h);/* 403 */
					return;
				}
				p++;
			}
		}
	}
#ifdef RANDOMIZE_URLS
	/* first check if the URL begins with the randomized string */
	if(HttpUrl[0] != '/' || memcmp(HttpUrl+1, random_url, strlen(random_url)) != 0)
	{
		Send404(h);
		return;
	}
	/* remove "random" from the start of the URL */
	p = HttpUrl + strlen(random_url) + 1;
	memmove(HttpUrl, p, strlen(p) + 1);
#endif /* RANDOMIZE_URLS */
	if(strcmp("POST", HttpCommand) == 0)
	{
		h->req_command = EPost;
		ProcessHTTPPOST_upnphttp(h);
	}
	else if(strcmp("GET", HttpCommand) == 0)
	{
		h->req_command = EGet;
		for(i=0; path_desc[i].path; i++) {
			if(strcasecmp(path_desc[i].path, HttpUrl) == 0) {
				if(path_desc[i].f)
					sendXMLdesc(h, path_desc[i].f);
				else
#ifdef HAS_DUMMY_SERVICE
					sendDummyDesc(h);
#else
					continue;
#endif
				return;
			}
		}
		if(0 == memcmp(HttpUrl, "/ctl/", 5)) {
			/* 405 Method Not Allowed
			 * Allow: POST */
			h->respflags = FLAG_ALLOW_POST;
			Send405(h);
			return;
		}
#ifdef ENABLE_EVENTS
		if(0 == memcmp(HttpUrl, "/evt/", 5)) {
			/* 405 Method Not Allowed
			 * Allow: SUBSCRIBE, UNSUBSCRIBE */
			h->respflags = FLAG_ALLOW_SUB_UNSUB;
			Send405(h);
			return;
		}
#endif
		syslog(LOG_NOTICE, "%s not found, responding ERROR 404", HttpUrl);
		Send404(h);
	}
#ifdef ENABLE_EVENTS
	else if(strcmp("SUBSCRIBE", HttpCommand) == 0)
	{
		h->req_command = ESubscribe;
		ProcessHTTPSubscribe_upnphttp(h, HttpUrl);
	}
	else if(strcmp("UNSUBSCRIBE", HttpCommand) == 0)
	{
		h->req_command = EUnSubscribe;
		ProcessHTTPUnSubscribe_upnphttp(h, HttpUrl);
	}
#else
	else if(strcmp("SUBSCRIBE", HttpCommand) == 0)
	{
		syslog(LOG_NOTICE, "SUBSCRIBE not implemented. ENABLE_EVENTS compile option disabled");
		Send501(h);
	}
#endif
	else
	{
		syslog(LOG_NOTICE, "Unsupported HTTP Command %s", HttpCommand);
		Send501(h);
	}
}


void
Process_upnphttp(struct upnphttp * h)
{
	char * h_tmp;
	char buf[2048];
	int n;

	if(!h)
		return;
	switch(h->state)
	{
	case EWaitingForHttpRequest:
#ifdef ENABLE_HTTPS
		if(h->ssl) {
			n = SSL_read(h->ssl, buf, sizeof(buf));
		} else {
			n = recv(h->socket, buf, sizeof(buf), 0);
		}
#else
		n = recv(h->socket, buf, sizeof(buf), 0);
#endif
		if(n<0)
		{
#ifdef ENABLE_HTTPS
			if(h->ssl) {
				int err;
				err = SSL_get_error(h->ssl, n);
				if(err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE)
				{
					syslog(LOG_ERR, "SSL_read() failed");
					syslogsslerr();
					h->state = EToDelete;
				}
			} else {
#endif
			if(errno != EAGAIN &&
			   errno != EWOULDBLOCK &&
			   errno != EINTR)
			{
				syslog(LOG_ERR, "recv (state0): %m");
				h->state = EToDelete;
			}
			/* if errno is EAGAIN, EWOULDBLOCK or EINTR, try again later */
#ifdef ENABLE_HTTPS
			}
#endif
		}
		else if(n==0)
		{
#ifdef ENABLE_IPV6
			if (h->ipv6)
			{
				char clientaddr_str[INET6_ADDRSTRLEN];
				if(inet_ntop(AF_INET6, &(h->clientaddr_v6), clientaddr_str, INET6_ADDRSTRLEN) == NULL)
					strncpy(clientaddr_str, "*inet_ntop error*", sizeof(clientaddr_str));
				syslog(LOG_WARNING, "HTTP Connection from %s closed unexpectedly", clientaddr_str);
			}
			else
#endif
			{
				syslog(LOG_WARNING, "HTTP Connection from %s closed unexpectedly", inet_ntoa(h->clientaddr));
			}
			h->state = EToDelete;
		}
		else
		{
			const char * endheaders;
			/* if 1st arg of realloc() is null,
			 * realloc behaves the same as malloc() */
			h_tmp = (char *)realloc(h->req_buf, n + h->req_buflen + 1);
			if (h_tmp == NULL)
			{
				syslog(LOG_WARNING, "Unable to allocate new memory for h->req_buf)");
				h->state = EToDelete;
			}
			else
			{
				h->req_buf = h_tmp;
				memcpy(h->req_buf + h->req_buflen, buf, n);
				h->req_buflen += n;
				h->req_buf[h->req_buflen] = '\0';
			}
			/* search for the string "\r\n\r\n" */
			endheaders = findendheaders(h->req_buf, h->req_buflen);
			if(endheaders)
			{
				/* at this point, the request buffer (h->req_buf)
				 * is guaranteed to contain the \r\n\r\n character sequence */
				h->req_contentoff = endheaders - h->req_buf + 4;
				ProcessHttpQuery_upnphttp(h);
			}
		}
		break;
	case EWaitingForHttpContent:
#ifdef ENABLE_HTTPS
		if(h->ssl) {
			n = SSL_read(h->ssl, buf, sizeof(buf));
		} else {
			n = recv(h->socket, buf, sizeof(buf), 0);
		}
#else
		n = recv(h->socket, buf, sizeof(buf), 0);
#endif
		if(n<0)
		{
#ifdef ENABLE_HTTPS
			if(h->ssl) {
				int err;
				err = SSL_get_error(h->ssl, n);
				if(err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE)
				{
					syslog(LOG_ERR, "SSL_read() failed");
					syslogsslerr();
					h->state = EToDelete;
				}
			} else {
#endif
			if(errno != EAGAIN &&
			   errno != EWOULDBLOCK &&
			   errno != EINTR)
			{
				syslog(LOG_ERR, "recv (state1): %m");
				h->state = EToDelete;
			}
			/* if errno is EAGAIN, EWOULDBLOCK or EINTR, try again later */
#ifdef ENABLE_HTTPS
			}
#endif
		}
		else if(n==0)
		{
#ifdef ENABLE_IPV6
			if (h->ipv6)
			{
				char clientaddr_str[INET6_ADDRSTRLEN];
				if(inet_ntop(AF_INET6, &(h->clientaddr_v6), clientaddr_str, INET6_ADDRSTRLEN) == NULL)
					strncpy(clientaddr_str, "*inet_ntop error*", sizeof(clientaddr_str));
				syslog(LOG_WARNING, "HTTP Connection from %s closed unexpectedly", clientaddr_str);
			}
			else
#endif
			{
				syslog(LOG_WARNING, "HTTP Connection from %s closed unexpectedly", inet_ntoa(h->clientaddr));
			}
			h->state = EToDelete;
		}
		else
		{
			void * tmp = realloc(h->req_buf, n + h->req_buflen);
			if(!tmp)
			{
				syslog(LOG_ERR, "memory allocation error %m");
				h->state = EToDelete;
			}
			else
			{
				h->req_buf = tmp;
				memcpy(h->req_buf + h->req_buflen, buf, n);
				h->req_buflen += n;
				if((h->req_buflen - h->req_contentoff) >= h->req_contentlen)
				{
					ProcessHTTPPOST_upnphttp(h);
				}
			}
		}
		break;
	case ESendingContinue:
		if(SendResp_upnphttp(h))
			h->state = EWaitingForHttpContent;
		break;
	case ESendingAndClosing:
		SendRespAndClose_upnphttp(h);
		break;
	default:
		syslog(LOG_WARNING, "Unexpected state: %d", h->state);
	}
}

static const char httpresphead[] =
	"%s %d %s\r\n"
	"Content-Type: %s\r\n"
	"Connection: close\r\n"
	"Content-Length: %d\r\n"
	"Server: " MINIUPNPD_SERVER_STRING "\r\n"
	"Ext:\r\n"
	;	/*"\r\n";*/
/*
		"<?xml version=\"1.0\"?>\n"
		"<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
		"s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
		"<s:Body>"

		"</s:Body>"
		"</s:Envelope>";
*/
/* with response code and response message
 * also allocate enough memory */

int
BuildHeader_upnphttp(struct upnphttp * h, int respcode,
                     const char * respmsg,
                     int bodylen)
{
	int templen = sizeof(httpresphead) + 256 + bodylen;
	if(!h->res_buf ||
	   h->res_buf_alloclen < templen) {
		if(h->res_buf)
			free(h->res_buf);
		h->res_buf = (char *)malloc(templen);
		if(!h->res_buf) {
			syslog(LOG_ERR, "malloc error in BuildHeader_upnphttp()");
			return -1;
		}
		h->res_buf_alloclen = templen;
	}
	h->res_sent = 0;
	h->res_buflen = snprintf(h->res_buf, h->res_buf_alloclen,
	                         httpresphead, h->HttpVer,	/* HTTP/x.x */
	                         respcode, respmsg,
	                         (h->respflags&FLAG_HTML)?"text/html":"text/xml; charset=\"utf-8\"",	/* Content-Type: */
	                         bodylen		/* Content-Length: */
#ifdef DYNAMIC_OS_VERSION
	                         , os_version	/* Server: */
#endif
	                        );
	/* Content-Type MUST be 'text/xml; charset="utf-8"' according to UDA v1.1 */
	/* Content-Type MUST be 'text/xml' according to UDA v1.0 */
	/* Additional headers */
#ifdef ENABLE_HTTP_DATE
	{
		char http_date[64];
		time_t t;
		struct tm tm;
		time(&t);
		gmtime_r(&t, &tm);
		/* %a and %b depend on locale */
		strftime(http_date, sizeof(http_date),
		         "%a, %d %b %Y %H:%M:%S GMT", &tm);
		h->res_buflen += snprintf(h->res_buf + h->res_buflen,
		                          h->res_buf_alloclen - h->res_buflen,
		                          "Date: %s\r\n", http_date);
	}
#endif
#ifdef ENABLE_EVENTS
	if(h->respflags & FLAG_TIMEOUT) {
		h->res_buflen += snprintf(h->res_buf + h->res_buflen,
		                          h->res_buf_alloclen - h->res_buflen,
		                          "Timeout: Second-");
		if(h->req_Timeout) {
			h->res_buflen += snprintf(h->res_buf + h->res_buflen,
			                          h->res_buf_alloclen - h->res_buflen,
			                          "%d\r\n", h->req_Timeout);
		} else {
			h->res_buflen += snprintf(h->res_buf + h->res_buflen,
			                          h->res_buf_alloclen - h->res_buflen,
			                          "infinite\r\n");
		}
	}
	if(h->respflags & FLAG_SID) {
		h->res_buflen += snprintf(h->res_buf + h->res_buflen,
		                          h->res_buf_alloclen - h->res_buflen,
		                          "SID: %s\r\n", h->res_SID);
	}
#endif
	if(h->respflags & FLAG_ALLOW_POST) {
		h->res_buflen += snprintf(h->res_buf + h->res_buflen,
		                          h->res_buf_alloclen - h->res_buflen,
		                          "Allow: %s\r\n", "POST");
	} else if(h->respflags & FLAG_ALLOW_SUB_UNSUB) {
		h->res_buflen += snprintf(h->res_buf + h->res_buflen,
		                          h->res_buf_alloclen - h->res_buflen,
		                          "Allow: %s\r\n", "SUBSCRIBE, UNSUBSCRIBE");
	}
	if(h->accept_language[0] != '\0') {
		/* defaulting to "en" */
		h->res_buflen += snprintf(h->res_buf + h->res_buflen,
		                          h->res_buf_alloclen - h->res_buflen,
		                          "Content-Language: %s\r\n",
		                          h->accept_language[0] == '*' ? "en" : h->accept_language);
	}
	h->res_buf[h->res_buflen++] = '\r';
	h->res_buf[h->res_buflen++] = '\n';
	if(h->res_buf_alloclen < (h->res_buflen + bodylen))
	{
		char * tmp;
		tmp = (char *)realloc(h->res_buf, (h->res_buflen + bodylen));
		if(tmp)
		{
			h->res_buf = tmp;
			h->res_buf_alloclen = h->res_buflen + bodylen;
		}
		else
		{
			syslog(LOG_ERR, "realloc error in BuildHeader_upnphttp()");
			return -1;
		}
	}
	return 0;
}

void
BuildResp2_upnphttp(struct upnphttp * h, int respcode,
                    const char * respmsg,
                    const char * body, int bodylen)
{
	int r = BuildHeader_upnphttp(h, respcode, respmsg, bodylen);
	if(body && (r >= 0)) {
		memcpy(h->res_buf + h->res_buflen, body, bodylen);
		h->res_buflen += bodylen;
	}
}

/* responding 200 OK ! */
void
BuildResp_upnphttp(struct upnphttp * h,
                   const char * body, int bodylen)
{
	BuildResp2_upnphttp(h, 200, "OK", body, bodylen);
}

int
SendResp_upnphttp(struct upnphttp * h)
{
	ssize_t n;

	while (h->res_sent < h->res_buflen)
	{
#ifdef ENABLE_HTTPS
		if(h->ssl) {
			n = SSL_write(h->ssl, h->res_buf + h->res_sent,
			         h->res_buflen - h->res_sent);
		} else {
			n = send(h->socket, h->res_buf + h->res_sent,
			         h->res_buflen - h->res_sent, 0);
		}
#else
		n = send(h->socket, h->res_buf + h->res_sent,
		         h->res_buflen - h->res_sent, 0);
#endif
		if(n<0)
		{
#ifdef ENABLE_HTTPS
			if(h->ssl) {
				int err;
				err = SSL_get_error(h->ssl, n);
				if(err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
					/* try again later */
					return 0;
				}
				syslog(LOG_ERR, "SSL_write() failed");
				syslogsslerr();
				break;
			} else {
#endif
			if(errno == EINTR)
				continue;	/* try again immediately */
			if(errno == EAGAIN || errno == EWOULDBLOCK)
			{
				/* try again later */
				return 0;
			}
			syslog(LOG_ERR, "send(res_buf): %m");
			break; /* avoid infinite loop */
#ifdef ENABLE_HTTPS
			}
#endif
		}
		else if(n == 0)
		{
			syslog(LOG_ERR, "send(res_buf): %d bytes sent (out of %d)",
							h->res_sent, h->res_buflen);
			break;
		}
		else
		{
			h->res_sent += n;
		}
	}
	return 1;	/* finished */
}

void
SendRespAndClose_upnphttp(struct upnphttp * h)
{
	if (SendResp_upnphttp(h))
		CloseSocket_upnphttp(h);
	else
		h->state = ESendingAndClosing;
}
