/* $Id: upnpevents.c,v 1.44 2019/09/24 11:47:06 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2008-2019 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <sys/queue.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "config.h"
#if defined(LIB_UUID)
/* as found on linux */
#include <uuid/uuid.h>
#elif defined(BSD_UUID)
#include <uuid.h>
#endif /* LIB_UUID / BSD_UUID */
#include "upnpevents.h"
#include "miniupnpdpath.h"
#include "upnpglobalvars.h"
#include "upnpdescgen.h"
#include "upnputils.h"

#ifdef ENABLE_EVENTS
/*enum subscriber_service_enum {
 EWanCFG = 1,
 EWanIPC,
 EL3F
};*/

/* stuctures definitions */
struct subscriber {
	LIST_ENTRY(subscriber) entries;
	struct upnp_event_notify * notify;
	time_t timeout;
	uint32_t seq;
	enum subscriber_service_enum service;
	char uuid[42];
	char callback[];
};

struct upnp_event_notify {
	LIST_ENTRY(upnp_event_notify) entries;
    int s;  /* socket */
    enum { ECreated=1,
	       EConnecting,
	       ESending,
	       EWaitingForResponse,
	       EFinished,
	       EError } state;
    struct subscriber * sub;
    char * buffer;
    int buffersize;
	int tosend;
    int sent;
	const char * path;
#ifdef ENABLE_IPV6
	int ipv6;
	char addrstr[48];
#else
	char addrstr[16];
#endif
	char portstr[8];
};

/* prototypes */
static void
upnp_event_create_notify(struct subscriber * sub);

/* Subscriber list */
LIST_HEAD(listhead, subscriber) subscriberlist = { NULL };

/* notify list */
LIST_HEAD(listheadnotif, upnp_event_notify) notifylist = { NULL };

/* create a new subscriber */
static struct subscriber *
newSubscriber(const char * eventurl, const char * callback, int callbacklen)
{
	struct subscriber * tmp;
	if(!eventurl || !callback || !callbacklen)
		return NULL;
	tmp = calloc(1, sizeof(struct subscriber)+callbacklen+1);
	if(!tmp)
		return NULL;
	if(strcmp(eventurl, WANCFG_EVENTURL)==0)
		tmp->service = EWanCFG;
	else if(strcmp(eventurl, WANIPC_EVENTURL)==0)
		tmp->service = EWanIPC;
#ifdef ENABLE_L3F_SERVICE
	else if(strcmp(eventurl, L3F_EVENTURL)==0)
		tmp->service = EL3F;
#endif
#ifdef ENABLE_6FC_SERVICE
	else if(strcmp(eventurl, WANIP6FC_EVENTURL)==0)
		tmp->service = E6FC;
#endif
#ifdef ENABLE_DP_SERVICE
	else if(strcmp(eventurl, DP_EVENTURL)==0)
		tmp->service = EDP;
#endif
	else {
		free(tmp);
		return NULL;
	}
	memcpy(tmp->callback, callback, callbacklen);
	tmp->callback[callbacklen] = '\0';
#if defined(LIB_UUID)
	{
		uuid_t uuid;
		uuid_generate(uuid);
		memcpy(tmp->uuid, "uuid:", 5);
		uuid_unparse(uuid, tmp->uuid + 5);
	}
#elif defined(BSD_UUID)
	{
		uuid_t uuid;
		uint32_t status;
		uuid_create(&uuid, &status);
		if(status != uuid_s_ok) {
			syslog(LOG_ERR, "uuid_create() failed (%u)", status);
		} else {
			char * uuid_str;
			uuid_to_string(&uuid, &uuid_str, &status);
			if(status != uuid_s_ok) {
				syslog(LOG_ERR, "uuid_to_string() failed (%u)", status);
			} else {
				if(strlen(uuid_str) != 36) {
					syslog(LOG_ERR, "uuid_to_string() returned %s", uuid_str);
					status = (uint32_t)-1;
				} else {
					memcpy(tmp->uuid, "uuid:", 5);
					memcpy(tmp->uuid + 5, uuid_str, 36);
					tmp->uuid[sizeof(tmp->uuid)-1] = '\0';
				}
				free(uuid_str);
			}
		}
		if(status != uuid_s_ok) {
			/* make a dummy uuid */
			strncpy(tmp->uuid, uuidvalue_igd, sizeof(tmp->uuid));
			tmp->uuid[sizeof(tmp->uuid)-1] = '\0';
			snprintf(tmp->uuid+sizeof(tmp->uuid)-5, 5, "%04lx", random() & 0xffff);
		}
	}
#else
	/* make a dummy uuid */
	strncpy(tmp->uuid, uuidvalue_igd, sizeof(tmp->uuid));
	tmp->uuid[sizeof(tmp->uuid)-1] = '\0';
	snprintf(tmp->uuid+sizeof(tmp->uuid)-5, 5, "%04lx", random() & 0xffff);
#endif
	return tmp;
}

/* creates a new subscriber and adds it to the subscriber list
 * also initiate 1st notify
 * TODO : add a check on the number of subscriber in order to
 * prevent memory overflow... */
const char *
upnpevents_addSubscriber(const char * eventurl,
                         const char * callback, int callbacklen,
                         int timeout)
{
	struct subscriber * tmp;
	/*static char uuid[42];*/
	/* "uuid:00000000-0000-0000-0000-000000000000"; 5+36+1=42bytes */
	syslog(LOG_DEBUG, "addSubscriber(%s, %.*s, %d)",
	       eventurl, callbacklen, callback, timeout);
	/*strncpy(uuid, uuidvalue, sizeof(uuid));
	uuid[sizeof(uuid)-1] = '\0';*/
	tmp = newSubscriber(eventurl, callback, callbacklen);
	if(!tmp)
		return NULL;
	if(timeout)
		tmp->timeout = upnp_time() + timeout;
	LIST_INSERT_HEAD(&subscriberlist, tmp, entries);
	upnp_event_create_notify(tmp);
	return tmp->uuid;
}

/* renew a subscription (update the timeout) */
const char *
upnpevents_renewSubscription(const char * sid, int sidlen, int timeout)
{
	struct subscriber * sub;
	for(sub = subscriberlist.lh_first; sub != NULL; sub = sub->entries.le_next) {
		if((sidlen == 41) && (memcmp(sid, sub->uuid, 41) == 0)) {
#ifdef UPNP_STRICT
			/* check if the subscription already timeouted */
			if(sub->timeout && upnp_time() > sub->timeout)
				continue;
#endif
			sub->timeout = (timeout ? upnp_time() + timeout : 0);
			return sub->uuid;
		}
	}
	return NULL;
}

int
upnpevents_removeSubscriber(const char * sid, int sidlen)
{
	struct subscriber * sub;
	if(!sid)
		return -1;
	for(sub = subscriberlist.lh_first; sub != NULL; sub = sub->entries.le_next) {
		if((sidlen == 41) && (memcmp(sid, sub->uuid, 41) == 0)) {
			if(sub->notify) {
				sub->notify->sub = NULL;
			}
			LIST_REMOVE(sub, entries);
			free(sub);
			return 0;
		}
	}
	return -1;
}

/* notifies all subscriber of a number of port mapping change
 * or external ip address change */
void
upnp_event_var_change_notify(enum subscriber_service_enum service)
{
	struct subscriber * sub;
	for(sub = subscriberlist.lh_first; sub != NULL; sub = sub->entries.le_next) {
		if(sub->service == service && sub->notify == NULL)
			upnp_event_create_notify(sub);
	}
}

/* create and add the notify object to the list */
static void
upnp_event_create_notify(struct subscriber * sub)
{
	struct upnp_event_notify * obj;
	/*struct timeval sock_timeout;*/

	obj = calloc(1, sizeof(struct upnp_event_notify));
	if(!obj) {
		syslog(LOG_ERR, "%s: calloc(): %m", "upnp_event_create_notify");
		return;
	}
	obj->sub = sub;
	obj->state = ECreated;
#ifdef ENABLE_IPV6
	obj->s = socket((obj->sub->callback[7] == '[') ? PF_INET6 : PF_INET,
	                SOCK_STREAM, 0);
#else
	obj->s = socket(PF_INET, SOCK_STREAM, 0);
#endif
	if(obj->s<0) {
		syslog(LOG_ERR, "%s: socket(): %m", "upnp_event_create_notify");
		goto error;
	}
#if 0 /* does not work for non blocking connect() */
	/* set timeout to 3 seconds */
	sock_timeout.tv_sec = 3;
	sock_timeout.tv_usec = 0;
	if(setsockopt(obj->s, SOL_SOCKET, SO_RCVTIMEO, &sock_timeout, sizeof(struct timeval)) < 0) {
		syslog(LOG_WARNING, "%s: setsockopt(SO_RCVTIMEO): %m",
		       "upnp_event_create_notify");
	}
	sock_timeout.tv_sec = 3;
	sock_timeout.tv_usec = 0;
	if(setsockopt(obj->s, SOL_SOCKET, SO_SNDTIMEO, &sock_timeout, sizeof(struct timeval)) < 0) {
		syslog(LOG_WARNING, "%s: setsockopt(SO_SNDTIMEO): %m",
		       "upnp_event_create_notify");
	}
#endif
	/* set socket non blocking */
	if(!set_non_blocking(obj->s)) {
		syslog(LOG_ERR, "%s: set_non_blocking(): %m",
		       "upnp_event_create_notify");
		goto error;
	}
	if(sub)
		sub->notify = obj;
	LIST_INSERT_HEAD(&notifylist, obj, entries);
	return;
error:
	if(obj->s >= 0)
		close(obj->s);
	free(obj);
}

static void
upnp_event_notify_connect(struct upnp_event_notify * obj)
{
	unsigned int i;
	const char * p;
	unsigned short port;
#ifdef ENABLE_IPV6
	struct sockaddr_storage addr;
	socklen_t addrlen;
#else
	struct sockaddr_in addr;
	socklen_t addrlen;
#endif

	if(!obj)
		return;
	memset(&addr, 0, sizeof(addr));
	i = 0;
	if(obj->sub == NULL) {
		obj->state = EError;
		return;
	}
	p = obj->sub->callback;
	p += 7;	/* http:// */
#ifdef ENABLE_IPV6
	if(*p == '[') {	/* ip v6 */
		obj->addrstr[i++] = '[';
		p++;
		obj->ipv6 = 1;
		while(*p != ']' && i < (sizeof(obj->addrstr)-1))
			obj->addrstr[i++] = *(p++);
		if(*p == ']')
			p++;
		if(i < (sizeof(obj->addrstr)-1))
			obj->addrstr[i++] = ']';
	} else {
#endif
		while(*p != '/' && *p != ':' && i < (sizeof(obj->addrstr)-1))
			obj->addrstr[i++] = *(p++);
#ifdef ENABLE_IPV6
	}
#endif
	obj->addrstr[i] = '\0';
	if(*p == ':') {
		obj->portstr[0] = *p;
		i = 1;
		p++;
		port = (unsigned short)atoi(p);
		while(*p != '\0' && *p != '/') {
			if(i<7) obj->portstr[i++] = *p;
			p++;
		}
		obj->portstr[i] = 0;
	} else {
		port = 80;
		obj->portstr[0] = '\0';
	}
	obj->path = p;
#ifdef ENABLE_IPV6
	if(obj->ipv6) {
		char addrstr_tmp[48];
		struct sockaddr_in6 * sa = (struct sockaddr_in6 *)&addr;
		sa->sin6_family = AF_INET6;
		i = (int)strlen(obj->addrstr);
		if(i > 2) {
			i -= 2;
			memcpy(addrstr_tmp, obj->addrstr + 1, i);
			addrstr_tmp[i] = '\0';
			inet_pton(AF_INET6, addrstr_tmp, &(sa->sin6_addr));
		}
		sa->sin6_port = htons(port);
		addrlen = sizeof(struct sockaddr_in6);
	} else {
		struct sockaddr_in * sa = (struct sockaddr_in *)&addr;
		sa->sin_family = AF_INET;
		inet_pton(AF_INET, obj->addrstr, &(sa->sin_addr));
		sa->sin_port = htons(port);
		addrlen = sizeof(struct sockaddr_in);
	}
#else
	addr.sin_family = AF_INET;
	inet_aton(obj->addrstr, &addr.sin_addr);
	addr.sin_port = htons(port);
	addrlen = sizeof(struct sockaddr_in);
#endif
	syslog(LOG_DEBUG, "%s: '%s' %hu '%s'", "upnp_event_notify_connect",
	       obj->addrstr, port, obj->path);
	obj->state = EConnecting;
	if(connect(obj->s, (struct sockaddr *)&addr, addrlen) < 0) {
		if(errno != EINPROGRESS && errno != EWOULDBLOCK) {
			syslog(LOG_ERR, "%s: connect(%d, %s, %u): %m",
			       "upnp_event_notify_connect", obj->s,
			       obj->addrstr, addrlen);
			obj->state = EError;
		}
	}
}

static void upnp_event_prepare(struct upnp_event_notify * obj)
{
	static const char notifymsg[] =
		"NOTIFY %s HTTP/1.1\r\n"
		"Host: %s%s\r\n"
#if (UPNP_VERSION_MAJOR == 1) && (UPNP_VERSION_MINOR == 0)
		"Content-Type: text/xml\r\n"	/* UDA v1.0 */
#else
		"Content-Type: text/xml; charset=\"utf-8\"\r\n"	/* UDA v1.1 or later */
#endif
		"Content-Length: %d\r\n"
		"NT: upnp:event\r\n"
		"NTS: upnp:propchange\r\n"
		"SID: %s\r\n"
		"SEQ: %u\r\n"
		"Connection: close\r\n"
		"Cache-Control: no-cache\r\n"
		"\r\n"
		"%.*s\r\n";
	char * xml;
	int l;
	if(obj->sub == NULL) {
		obj->state = EError;
		return;
	}
	switch(obj->sub->service) {
	case EWanCFG:
		xml = getVarsWANCfg(&l);
		break;
	case EWanIPC:
		xml = getVarsWANIPCn(&l);
		break;
#ifdef ENABLE_L3F_SERVICE
	case EL3F:
		xml = getVarsL3F(&l);
		break;
#endif
#ifdef ENABLE_6FC_SERVICE
	case E6FC:
		xml = getVars6FC(&l);
		break;
#endif
#ifdef ENABLE_DP_SERVICE
	case EDP:
		xml = getVarsDP(&l);
		break;
#endif
	default:
		xml = NULL;
		l = 0;
	}
	obj->buffersize = 1024;
	for (;;) {
		obj->buffer = malloc(obj->buffersize);
		if(!obj->buffer) {
			syslog(LOG_ERR, "%s: malloc returned NULL", "upnp_event_prepare");
			if(xml) {
				free(xml);
			}
			obj->state = EError;
			return;
		}
		obj->tosend = snprintf(obj->buffer, obj->buffersize, notifymsg,
		                       (obj->path[0] != '\0') ? obj->path : "/",
		                       obj->addrstr, obj->portstr, l+2,
		                       obj->sub->uuid, obj->sub->seq,
		                       l, xml);
		if (obj->tosend < 0) {
			syslog(LOG_ERR, "%s: snprintf() failed", "upnp_event_prepare");
			if(xml) {
				free(xml);
			}
			obj->state = EError;
			return;
		} else if (obj->tosend < obj->buffersize) {
			break; /* the buffer was large enough */
		}
		/* Try again with a buffer big enough */
		free(obj->buffer);
		obj->buffersize = obj->tosend + 1;	/* reserve space for the final 0 */
	}
	if(xml) {
		free(xml);
		xml = NULL;
	}
	obj->state = ESending;
}

static void upnp_event_send(struct upnp_event_notify * obj)
{
	int i;

	syslog(LOG_DEBUG, "%s: sending event notify message to %s%s",
	       "upnp_event_send", obj->addrstr, obj->portstr);
	syslog(LOG_DEBUG, "%s: msg: %s",
	       "upnp_event_send", obj->buffer + obj->sent);
	i = send(obj->s, obj->buffer + obj->sent, obj->tosend - obj->sent, 0);
	if(i<0) {
		if(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
			syslog(LOG_NOTICE, "%s: send(%s%s): %m", "upnp_event_send",
			       obj->addrstr, obj->portstr);
			obj->state = EError;
			return;
		} else {
			/* EAGAIN or EWOULDBLOCK or EINTR : no data sent */
			i = 0;
		}
	}
	if(i != (obj->tosend - obj->sent))
		syslog(LOG_NOTICE, "%s: %d bytes send out of %d",
		       "upnp_event_send", i, obj->tosend - obj->sent);
	obj->sent += i;
	if(obj->sent == obj->tosend)
		obj->state = EWaitingForResponse;
}

static void upnp_event_recv(struct upnp_event_notify * obj)
{
	int n;
	n = recv(obj->s, obj->buffer, obj->buffersize, 0);
	if(n<0) {
		if(errno != EAGAIN &&
		   errno != EWOULDBLOCK &&
		   errno != EINTR) {
			syslog(LOG_ERR, "%s: recv(): %m", "upnp_event_recv");
			obj->state = EError;
		}
		return;
	}
	syslog(LOG_DEBUG, "%s: (%dbytes) %.*s", "upnp_event_recv",
	       n, n, obj->buffer);
	/* TODO : do something with the data recevied ?
	 * right now, n (number of bytes received) is ignored
	 * We may need to recv() more bytes. */
	obj->state = EFinished;
	if(obj->sub)
		obj->sub->seq++;
}

static void
upnp_event_process_notify(struct upnp_event_notify * obj)
{
	int err;
	socklen_t len;
	switch(obj->state) {
	case EConnecting:
		/* now connected or failed to connect */
		len = sizeof(err);
		if(getsockopt(obj->s, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
			syslog(LOG_ERR, "%s: getsockopt: %m", "upnp_event_process_notify");
			obj->state = EError;
			break;
		}
		if(err != 0) {
			errno = err;
			syslog(LOG_WARNING, "%s: connect(%s%s): %m",
			       "upnp_event_process_notify",
			       obj->addrstr, obj->portstr);
			obj->state = EError;
			break;
		}
		upnp_event_prepare(obj);
		if(obj->state == ESending)
			upnp_event_send(obj);
		break;
	case ESending:
		upnp_event_send(obj);
		break;
	case EWaitingForResponse:
		upnp_event_recv(obj);
		break;
	case EFinished:
		close(obj->s);
		obj->s = -1;
		break;
	default:
		syslog(LOG_ERR, "%s: unknown state", "upnp_event_process_notify");
	}
}

void upnpevents_selectfds(fd_set *readset, fd_set *writeset, int * max_fd)
{
	struct upnp_event_notify * obj;
	for(obj = notifylist.lh_first; obj != NULL; obj = obj->entries.le_next) {
		syslog(LOG_DEBUG, "upnpevents_selectfds: %p %d %d",
		       obj, obj->state, obj->s);
		if(obj->s >= 0) {
			switch(obj->state) {
			case ECreated:
				upnp_event_notify_connect(obj);
				if(obj->state != EConnecting)
					break;
#if defined(__GNUC__) && (__GNUC__ >= 7)
				__attribute__ ((fallthrough));
#endif
			case EConnecting:
			case ESending:
				FD_SET(obj->s, writeset);
				if(obj->s > *max_fd)
					*max_fd = obj->s;
				break;
			case EWaitingForResponse:
				FD_SET(obj->s, readset);
				if(obj->s > *max_fd)
					*max_fd = obj->s;
				break;
			default:
				;
			}
		}
	}
}

void upnpevents_processfds(fd_set *readset, fd_set *writeset)
{
	struct upnp_event_notify * obj;
	struct upnp_event_notify * next;
	struct subscriber * sub;
	struct subscriber * subnext;
	time_t curtime;
	for(obj = notifylist.lh_first; obj != NULL; obj = obj->entries.le_next) {
		syslog(LOG_DEBUG, "%s: %p %d %d %d %d",
		       "upnpevents_processfds", obj, obj->state, obj->s,
		       FD_ISSET(obj->s, readset), FD_ISSET(obj->s, writeset));
		if(obj->s >= 0) {
			if(FD_ISSET(obj->s, readset) || FD_ISSET(obj->s, writeset))
				upnp_event_process_notify(obj);
		}
	}
	obj = notifylist.lh_first;
	while(obj != NULL) {
		next = obj->entries.le_next;
		if(obj->state == EError || obj->state == EFinished) {
			if(obj->s >= 0) {
				close(obj->s);
			}
			if(obj->sub)
				obj->sub->notify = NULL;
			/* remove also the subscriber from the list if there was an error */
			if(obj->state == EError && obj->sub) {
				syslog(LOG_ERR, "%s: %p, remove subscriber %s after an ERROR cb: %s",
				       "upnpevents_processfds", obj, obj->sub->uuid, obj->sub->callback);
				LIST_REMOVE(obj->sub, entries);
				free(obj->sub);
			}
			if(obj->buffer) {
				free(obj->buffer);
			}
			LIST_REMOVE(obj, entries);
			free(obj);
		}
		obj = next;
	}
	/* remove timeouted subscribers */
	curtime = upnp_time();
	for(sub = subscriberlist.lh_first; sub != NULL; ) {
		subnext = sub->entries.le_next;
		if(sub->timeout && curtime > sub->timeout && sub->notify == NULL) {
			syslog(LOG_INFO, "subscriber timeouted : %u > %u SID=%s",
			       (unsigned)curtime, (unsigned)sub->timeout, sub->uuid);
			LIST_REMOVE(sub, entries);
			free(sub);
		}
		sub = subnext;
	}
}

#ifdef USE_MINIUPNPDCTL
void write_events_details(int s) {
	int n;
	char buff[80];
	struct upnp_event_notify * obj;
	struct subscriber * sub;
	write(s, "Events details :\n", 17);
	for(obj = notifylist.lh_first; obj != NULL; obj = obj->entries.le_next) {
		n = snprintf(buff, sizeof(buff), " %p sub=%p state=%d s=%d\n",
		             obj, obj->sub, obj->state, obj->s);
		write(s, buff, n);
	}
	write(s, "Subscribers :\n", 14);
	for(sub = subscriberlist.lh_first; sub != NULL; sub = sub->entries.le_next) {
		n = snprintf(buff, sizeof(buff), " %p timeout=%d seq=%u service=%d\n",
		             sub, (int)sub->timeout, sub->seq, sub->service);
		write(s, buff, n);
		n = snprintf(buff, sizeof(buff), "   notify=%p %s\n",
		             sub->notify, sub->uuid);
		write(s, buff, n);
		n = snprintf(buff, sizeof(buff), "   %s\n",
		             sub->callback);
		write(s, buff, n);
	}
}
#endif

#endif

