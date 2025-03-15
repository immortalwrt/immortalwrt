/* $Id: upnppinhole.c,v 1.17 2024/04/28 23:32:21 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2024 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#include "macros.h"
#include "config.h"
#include "upnpredirect.h"
#include "upnpglobalvars.h"
#include "upnpevents.h"
#include "upnputils.h"
#include "upnppinhole.h"
#ifdef __APPLE__
/* XXX - Apple version of PF API seems to differ from what
 * pf/pfpinhole.c expects so don't use that at least.. */
#ifdef USE_PF
#undef USE_PF
#endif /* USE_PF */
#endif /* __APPLE__ */
#if defined(USE_NETFILTER)
#include "netfilter/iptpinhole.h"
#endif
#if defined(USE_PF)
#include "pf/pfpinhole.h"
#endif
#if defined(USE_IPF)
#endif
#if defined(USE_IPFW)
#endif
#ifdef ENABLE_LEASEFILE
#include <sys/stat.h>
#endif

#ifdef ENABLE_UPNPPINHOLE

#if 0
int
upnp_check_outbound_pinhole(int proto, int * timeout)
{
	int s, tmptimeout, tmptime_out;
	switch(proto)
	{
		case IPPROTO_UDP:
			s = retrieve_timeout("udp_timeout", timeout);
			return s;
			break;
		case IPPROTO_UDPLITE:
			s = retrieve_timeout("udp_timeout_stream", timeout);
			return s;
			break;
		case IPPROTO_TCP:
			s = retrieve_timeout("tcp_timeout_established", timeout);
			return s;
			break;
		case 65535:
			s = retrieve_timeout("udp_timeout", timeout);
			s = retrieve_timeout("udp_timeout_stream", &tmptimeout);
			s = retrieve_timeout("tcp_timeout_established", &tmptime_out);
			if(tmptimeout<tmptime_out)
			{
				if(tmptimeout<*timeout)
					*timeout = tmptimeout;
			}
			else
			{
				if(tmptime_out<*timeout)
					*timeout = tmptimeout;
			}
			return s;
			break;
		default:
			return -5;
			break;
	}
	return 0;
}
#endif

#ifdef ENABLE_LEASEFILE
/* proto_atoi()
 * convert the string "UDP" or "TCP" to IPPROTO_UDP and IPPROTO_UDP */
static int
proto_atoi(const char * protocol)
{
	int proto = IPPROTO_TCP;
	if(strcasecmp(protocol, "UDP") == 0)
		proto = IPPROTO_UDP;
#ifdef IPPROTO_UDPLITE
	else if(strcasecmp(protocol, "UDPLITE") == 0)
		proto = IPPROTO_UDPLITE;
#endif /* IPPROTO_UDPLITE */
	return proto;
}

/* proto_itoa()
 * convert IPPROTO_UDP, IPPROTO_UDP, etc. to "UDP", "TCP" */
static const char *
proto_itoa(int proto)
{
	const char * protocol;
	switch(proto) {
	case IPPROTO_UDP:
		protocol = "UDP";
		break;
	case IPPROTO_TCP:
		protocol = "TCP";
		break;
#ifdef IPPROTO_UDPLITE
	case IPPROTO_UDPLITE:
		protocol = "UDPLITE";
		break;
#endif /* IPPROTO_UDPLITE */
	default:
		protocol = "*UNKNOWN*";
	}
	return protocol;
}

static int
lease_file6_add(const char * rem_client,
			   unsigned short rem_port,
               const char * int_client,
               unsigned short int_port,
               int proto,
			   int uid,
               const char * desc,
               unsigned int leaseduration)
{
	unsigned int timestamp;
	FILE * fd;

	if (lease_file6 == NULL) return 0;

	fd = fopen( lease_file6, "a");
	if (fd==NULL) {
		syslog(LOG_ERR, "could not open lease file: %s", lease_file);
		return -1;
	}

	timestamp = (leaseduration > 0) ? time(NULL) + leaseduration : 0;

	/* convert our time to unix time */
	if (timestamp != 0) {
		timestamp -= upnp_time();
	}
	if (rem_client == NULL) {
		rem_client = "";
	}
	if (desc == NULL) {
		desc = "";
	}

	fprintf(fd, "%s;%s;%hu;%s;%hu;%u;%u;%s\n",
	        proto_itoa(proto), int_client, int_port, rem_client, rem_port,
	        uid, timestamp, desc);
	fclose(fd);

	return 0;
}

static int
lease_file6_update(int uid, unsigned int leaseduration)
{
	FILE* fd, *fdt;
	char * p, * p2;
	unsigned short int_port, rem_port;
	char * proto;
	char * int_client;
	char * desc;
	char * rem_client;
	unsigned int timestamp_rule;
	unsigned int timestamp;
	char line[128];
	char tmpfilename[128];
	int uid_rule;
	int tmp;

	if (lease_file6 == NULL) return 0;

	if (strlen(lease_file6) + 7 > sizeof(tmpfilename)) {
		syslog(LOG_ERR, "Lease filename is too long");
		return -1;
	}

	snprintf( tmpfilename, sizeof(tmpfilename), "%sXXXXXX", lease_file6);

	fd = fopen( lease_file6, "r");
	if (fd==NULL) {
		return 0;
	}

	tmp = mkstemp(tmpfilename);
	if (tmp==-1) {
		fclose(fd);
		syslog(LOG_ERR, "could not open temporary lease file");
		return -1;
	}

	fchmod(tmp, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	fdt = fdopen(tmp, "a");

	timestamp = (leaseduration > 0) ? time(NULL) + leaseduration : 0;

	/* convert our time to unix time */
	if (timestamp != 0) {
		timestamp -= upnp_time();
	}

	while(fgets(line, sizeof(line), fd)) {

		syslog(LOG_DEBUG, "parsing lease file line '%s'", line);
		proto = line;
		p = strchr(line, ';');
		if(!p) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(p++) = '\0';
		p2 = strchr(p, ';');
		if(!p2) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(p2++) = '\0';
		int_port = (unsigned short)atoi(p2);
		int_client = p;
		p = strchr(p2, ';');
		if(!p) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(p++) = '\0';
		p2 = strchr(p, ';');
		if(!p2) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(p2++) = '\0';
		rem_port = (unsigned short)atoi(p2);
		rem_client = p;
		p = strchr(p2, ';');
		if(!p) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(p++) = '\0';
		p2 = strchr(p, ';');
		if(!p2) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(p2++) = '\0';
		desc = strchr(p2, ';');
		uid_rule = atoi(p);
		if(!desc) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(desc++) = '\0';
		timestamp_rule = (unsigned int)strtoul(p2, NULL, 10);

		if (uid == uid_rule) {
			timestamp_rule = timestamp;
		}

		fprintf(fdt, "%s;%s;%hu;%s;%hu;%u;%u;%s\n",
	        proto, int_client, int_port, rem_client, rem_port,
	        uid, timestamp_rule, desc);
	}

	fclose(fdt);
	fclose(fd);

	if (rename(tmpfilename, lease_file6) < 0) {
		syslog(LOG_ERR, "could not rename temporary lease file to %s", lease_file6);
		remove(tmpfilename);
	}

	return 0;
}

static int
lease_file6_remove(const char * int_client, unsigned short int_port, int proto, int uid)
{
	FILE* fd, *fdt;
	int tmp, uid_tmp;
	char buf[512], buf2[512];
	char str[32];
	char tmpfilename[128];
	char *p, *p2;
	int str_size, buf_size;


	if (lease_file6 == NULL) return 0;

	if (strlen(lease_file6) + 7 > sizeof(tmpfilename)) {
		syslog(LOG_ERR, "Lease filename is too long");
		return -1;
	}

	snprintf( tmpfilename, sizeof(tmpfilename), "%sXXXXXX", lease_file6);

	fd = fopen( lease_file6, "r");
	if (fd==NULL) {
		return 0;
	}

	snprintf( str, sizeof(str), "%s;%s;%u", proto_itoa(proto), int_client, int_port);
	str_size = strlen(str);

	tmp = mkstemp(tmpfilename);
	if (tmp==-1) {
		fclose(fd);
		syslog(LOG_ERR, "could not open temporary lease file");
		return -1;
	}
	fchmod(tmp, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	fdt = fdopen(tmp, "a");

	buf[sizeof(buf)-1] = 0;
	while( fgets(buf, sizeof(buf)-1, fd) != NULL) {
		buf_size = strlen(buf);

		if (uid > 0) {
			strncpy(buf2, buf, buf_size);
			// Internal Host
			p = strchr(buf2, ';');
			*(p++) = '\0';
			// Internal Port
			p = strchr(p, ';');
			*(p++) = '\0';
			// External Host
			p = strchr(p, ';');
			*(p++) = '\0';
			// External Port
			p = strchr(p, ';');
			*(p++) = '\0';
			// uid
			p = strchr(p, ';');
			*(p++) = '\0';
			p2 = strchr(p, ';');
			*(p2++) = '\0';
			uid_tmp = atoi(p);
			if (uid != uid_tmp) {
				fwrite(buf, buf_size, 1, fdt);
			}

		} else if (buf_size < str_size || strncmp(str, buf, str_size)!=0) {
			fwrite(buf, buf_size, 1, fdt);
		}
	}

	fclose(fdt);
	fclose(fd);

	if (rename(tmpfilename, lease_file6) < 0) {
		syslog(LOG_ERR, "could not rename temporary lease file to %s", lease_file6);
		remove(tmpfilename);
	}

	return 0;

}

int lease_file6_expire()
{
	FILE* fd, *fdt;
	char * p, * p2;
	int tmp;
	char buf[512];
	char line[512];
	char tmpfilename[128];
	char * desc;
	int buf_size;
	unsigned int timestamp;
	time_t current_unix_time;

	if (lease_file6 == NULL) return 0;

	if (strlen(lease_file6) + 7 > sizeof(tmpfilename)) {
		syslog(LOG_ERR, "Lease filename is too long");
		return -1;
	}

	snprintf( tmpfilename, sizeof(tmpfilename), "%sXXXXXX", lease_file6);

	fd = fopen( lease_file6, "r");
	if (fd==NULL) {
		return 0;
	}

	current_unix_time = time(NULL);

	tmp = mkstemp(tmpfilename);
	if (tmp==-1) {
		fclose(fd);
		syslog(LOG_ERR, "could not open temporary lease file");
		return -1;
	}
	fchmod(tmp, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	fdt = fdopen(tmp, "a");

	buf[sizeof(buf)-1] = 0;
	while(fgets(line, sizeof(line), fd)) {
		strncpy(buf, line, sizeof(buf));

		syslog(LOG_DEBUG, "Expire: parsing lease file line '%s'", line);
		// Internal Host
		p = strchr(line, ';');
		if(!p) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(p++) = '\0';
		// Internal Port
		p2 = strchr(p, ';');
		if(!p2) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(p2++) = '\0';
		// External Host
		p = strchr(p2, ';');
		if(!p) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(p++) = '\0';
		// External Port
		p2 = strchr(p, ';');
		if(!p2) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(p2++) = '\0';
		// uid
		p = strchr(p2, ';');
		if(!p) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(p++) = '\0';
		// Timestamp
		p2 = strchr(p, ';');
		if(!p2) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(p2++) = '\0';
		// descr
		desc = strchr(p2, ';');
		if(!desc) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(desc++) = '\0';
		/*timestamp = (unsigned int)atoi(p2);*/
		timestamp = (unsigned int)strtoul(p2, NULL, 10);
		syslog(LOG_DEBUG, "Expire: timestamp is '%u'", timestamp);
		syslog(LOG_DEBUG, "Expire: current timestamp is '%u'", (unsigned int)current_unix_time);
		if((timestamp > 0 && timestamp <= (unsigned int)current_unix_time) || timestamp <= 0) {
			continue;
		}

		buf_size = strlen(buf);
		fwrite(buf, buf_size, 1, fdt);
	}

	fclose(fdt);
	fclose(fd);

	if (rename(tmpfilename, lease_file6) < 0) {
		syslog(LOG_ERR, "could not rename temporary lease file to %s", lease_file6);
		remove(tmpfilename);
	}

	return 0;
}

/* reload_from_lease_file()
 * read lease_file and add the rules contained
 */
int reload_from_lease_file6()
{
	FILE * fd;
	char * p, * p2;
	unsigned short int_port, rem_port;
	char * proto;
	char * int_client;
	char * desc;
	char * rem_client;
	unsigned int leaseduration;
	unsigned int timestamp;
	time_t current_time;
	time_t current_unix_time;
	char line[128];
	int r, uid;

	if(!lease_file6) return -1;
	fd = fopen( lease_file6, "r");
	if (fd==NULL) {
		syslog(LOG_ERR, "could not open lease file: %s", lease_file6);
		return -1;
	}
	if(unlink(lease_file6) < 0) {
		syslog(LOG_WARNING, "could not unlink file %s : %m", lease_file6);
	}

	current_time = upnp_time();
	current_unix_time = time(NULL);

	while(fgets(line, sizeof(line), fd)) {
		syslog(LOG_DEBUG, "parsing lease file line '%s'", line);
		proto = line;
		p = strchr(line, ';');
		if(!p) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(p++) = '\0';
		p2 = strchr(p, ';');
		if(!p2) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(p2++) = '\0';
		int_port = (unsigned short)atoi(p2);
		int_client = p;
		p = strchr(p2, ';');
		if(!p) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(p++) = '\0';
		p2 = strchr(p, ';');
		if(!p2) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(p2++) = '\0';
		rem_port = (unsigned short)atoi(p2);
		rem_client = p;
		p = strchr(p2, ';');
		if(!p) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(p++) = '\0';
		p2 = strchr(p, ';');
		if(!p2) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(p2++) = '\0';
		desc = strchr(p2, ';');
		uid = atoi(p);
		if(!desc) {
			syslog(LOG_ERR, "unrecognized data in lease file");
			continue;
		}
		*(desc++) = '\0';
		/*timestamp = (unsigned int)atoi(p2);*/
		timestamp = (unsigned int)strtoul(p2, NULL, 10);
		/* trim description */
		while(isspace(*desc))
			desc++;
		p = desc;
		while(*(p+1))
			p++;
		while(isspace(*p) && (p > desc))
			*(p--) = '\0';

		if(timestamp > 0) {
			if(timestamp <= (unsigned int)current_unix_time) {
				syslog(LOG_NOTICE, "already expired lease in lease file");
				continue;
			} else {
				leaseduration = timestamp - current_unix_time;
				timestamp = leaseduration + current_time; /* convert to our time */
			}
		} else {
			leaseduration = 0;	/* default value */
		}

		r = upnp_add_inboundpinhole(rem_client, rem_port, int_client, int_port, proto_atoi(proto),
                        		desc, leaseduration, &uid);
		if(r == -1) {
			syslog(LOG_ERR, "Failed to add %s:%hu -> %s:%hu protocol %s",
			       rem_client, rem_port, int_client, int_port, proto);
		} else if(r == -2) {
			/* Add the redirection again to the lease file */
			lease_file6_add(rem_client, rem_port, int_client, int_port, proto_atoi(proto),
			               uid, desc, timestamp);
		}
	}
	fclose(fd);

	return 0;
}

#endif

int
upnp_find_inboundpinhole(const char * raddr, unsigned short rport,
                         const char * iaddr, unsigned short iport,
                         int proto, char * desc, int desc_len, unsigned int * leasetime)
{
#if defined(USE_PF) || defined(USE_NETFILTER)
	int uid;
	uid = find_pinhole(NULL, raddr, rport, iaddr, iport, proto,
	                   desc, desc_len, leasetime);
	return uid;
#else
	return -42;
#endif
}

/* upnp_add_inboundpinhole()
 * returns:  1 on success
 *          -1 Pinhole space exhausted
 *          -4 invalid arguments
 *         -42 not implemented
 * TODO : return uid on success (positive) or error value (negative)
 */
int
upnp_add_inboundpinhole(const char * raddr,
                        unsigned short rport,
                        const char * iaddr,
                        unsigned short iport,
                        int proto,
                        char * desc,
                        unsigned int leasetime,
                        int * uid)
{
	int r;
	time_t current;
	unsigned int timestamp;
	struct in6_addr address;

	r = inet_pton(AF_INET6, iaddr, &address);
	if(r <= 0) {
		syslog(LOG_ERR, "inet_pton(%d, %s, %p) FAILED",
		       AF_INET6, iaddr, &address);
		return -4;
	}
	current = upnp_time();
	timestamp = current + leasetime;
	r = 0;

	*uid = upnp_find_inboundpinhole(raddr, rport, iaddr, iport, proto, NULL, 0, NULL);
	if(*uid >= 0) {
		syslog(LOG_INFO, "Pinhole for inbound traffic from [%s]:%hu to [%s]:%hu with proto %d found uid=%d. Updating it.", raddr, rport, iaddr, iport, proto, *uid);
		r = upnp_update_inboundpinhole(*uid, leasetime);
#ifdef ENABLE_LEASEFILE
		if (r >= 0) {
			lease_file6_remove(iaddr, iport, proto, -1);
			lease_file6_add(raddr, rport, iaddr, iport, proto, *uid, desc, timestamp);
		}
#endif /* ENABLE_LEASEFILE */
		return (r >= 0) ? 1 : r;
	}
#if defined(USE_PF) || defined(USE_NETFILTER)
	*uid = add_pinhole (ext_if_name6, raddr, rport,
	                    iaddr, iport, proto, desc, timestamp);
#ifdef ENABLE_LEASEFILE
	if (*uid >= 0) {
		lease_file6_remove(iaddr, iport, proto, -1);
		lease_file6_add(raddr, rport, iaddr, iport, proto, *uid, desc, timestamp);
	}
#endif /* ENABLE_LEASEFILE */
	return *uid >= 0 ? 1 : -1;
#else
	return -42;	/* not implemented */
#endif
}

#if 0
int
upnp_add_inboundpinhole_internal(const char * raddr, unsigned short rport,
                       const char * iaddr, unsigned short iport,
                       const char * proto, int * uid)
{
	int c = 9999;
	char cmd[256], cmd_raw[256], cuid[42];
#if 0
	static const char cmdval_full_udptcp[] = "ip6tables -I %s %d -p %s -i %s -s %s --sport %hu -d %s --dport %hu -j ACCEPT";
	static const char cmdval_udptcp[] = "ip6tables -I %s %d -p %s -i %s --sport %hu -d %s --dport %hu -j ACCEPT";
	static const char cmdval_full_udplite[] = "ip6tables -I %s %d -p %s -i %s -s %s -d %s -j ACCEPT";
	static const char cmdval_udplite[] = "ip6tables -I %s %d -p %s -i %s -d %s -j ACCEPT";
	// raw table command
	static const char cmdval_full_udptcp_raw[] = "ip6tables -t raw -I PREROUTING %d -p %s -i %s -s %s --sport %hu -d %s --dport %hu -j TRACE";
	static const char cmdval_udptcp_raw[] = "ip6tables -t raw -I PREROUTING %d -p %s -i %s --sport %hu -d %s --dport %hu -j TRACE";
	static const char cmdval_full_udplite_raw[] = "ip6tables -t raw -I PREROUTING %d -p %s -i %s -s %s -d %s -j TRACE";
	static const char cmdval_udplite_raw[] = "ip6tables -t raw -I PREROUTING %d -p %s -i %s -d %s -j TRACE";
#endif
	/*printf("%s\n", raddr);*/
	if(raddr!=NULL)
	{
#ifdef IPPROTO_UDPLITE
		if(atoi(proto) == IPPROTO_UDPLITE)
		{
	/*		snprintf(cmd, sizeof(cmd), cmdval_full_udplite, miniupnpd_forward_chain, line_number, proto, ext_if_name, raddr, iaddr);
			snprintf(cmd_raw, sizeof(cmd_raw), cmdval_full_udplite_raw, line_number, proto, ext_if_name, raddr, iaddr);*/
		}
		else
#endif
		{
	/*		snprintf(cmd, sizeof(cmd), cmdval_full_udptcp, miniupnpd_forward_chain, line_number, proto, ext_if_name, raddr, rport, iaddr, iport);
			snprintf(cmd_raw, sizeof(cmd_raw), cmdval_full_udptcp_raw, line_number, proto, ext_if_name, raddr, rport, iaddr, iport);*/
		}
	}
	else
	{
#ifdef IPPROTO_UDPLITE
		if(atoi(proto) == IPPROTO_UDPLITE)
		{
			/*snprintf(cmd, sizeof(cmd), cmdval_udplite, miniupnpd_forward_chain, line_number, proto, ext_if_name, iaddr);
			snprintf(cmd_raw, sizeof(cmd_raw), cmdval_udplite_raw, line_number, proto, ext_if_name, iaddr);*/
		}
		else
#endif
		{
			/*snprintf(cmd, sizeof(cmd), cmdval_udptcp, miniupnpd_forward_chain, line_number, proto, ext_if_name, rport, iaddr, iport);
			snprintf(cmd_raw, sizeof(cmd_raw), cmdval_udptcp_raw, line_number, proto, ext_if_name, rport, iaddr, iport);
*/
		}
	}
#ifdef DEBUG
	syslog(LOG_INFO, "Adding following ip6tables rule:");
	syslog(LOG_INFO, "  -> %s", cmd);
	syslog(LOG_INFO, "  -> %s", cmd_raw);
#endif
	/* TODO Add a better checking error.*/
	if(system(cmd) < 0 || system(cmd_raw) < 0)
	{
		return 0;
	}
	srand(time(NULL));
	snprintf(cuid, sizeof(cuid), "%.4d", rand()%c);
	*uid = atoi(cuid);
	printf("\t_add_ uid: %s\n", cuid);
	return 1;
}
#endif

/* upnp_get_pinhole_info()
 * return values :
 *   0   OK
 *  -1   Internal error
 *  -2   NOT FOUND (no such entry)
 *  ..
 *  -42  Not implemented
 */
int
upnp_get_pinhole_info(unsigned short uid,
                      char * raddr, int raddrlen,
                      unsigned short * rport,
                      char * iaddr, int iaddrlen,
                      unsigned short * iport,
                      int * proto, char * desc, int desclen,
                      unsigned int * leasetime,
                      unsigned int * packets)
{
	/* Call Firewall specific code to get IPv6 pinhole infos */
#if defined(USE_PF) || defined(USE_NETFILTER)
	int r;
	unsigned int timestamp;
	u_int64_t packets_tmp;
	/*u_int64_t bytes_tmp;*/

	r = get_pinhole_info(uid, raddr, raddrlen, rport,
	                     iaddr, iaddrlen, iport,
	                     proto, desc, desclen,
	                     leasetime ? &timestamp : NULL,
	                     packets ? &packets_tmp : NULL,
	                     NULL/*&bytes_tmp*/);
	if(r >= 0) {
		if(leasetime) {
			time_t current_time;
			current_time = upnp_time();
			if(timestamp > (unsigned int)current_time)
				*leasetime = timestamp - current_time;
			else
				*leasetime = 0;
		}
		if(packets)
			*packets = (unsigned int)packets_tmp;
	}
	return r;
#else
	UNUSED(uid);
	UNUSED(raddr); UNUSED(raddrlen); UNUSED(rport);
	UNUSED(iaddr); UNUSED(iaddrlen); UNUSED(iport);
	UNUSED(proto); UNUSED(desc); UNUSED(desclen);
	UNUSED(leasetime); UNUSED(packets);
	return -42;	/* not implemented */
#endif
}

int
upnp_get_pinhole_uid_by_index(int index)
{
#if defined (USE_NETFILTER)
	return get_pinhole_uid_by_index(index);
#else
	UNUSED(index);
	return -42;
#endif /* defined (USE_NETFILTER) */
}

int
upnp_update_inboundpinhole(unsigned short uid, unsigned int leasetime)
{
#if defined(USE_PF) || defined(USE_NETFILTER)
	unsigned int timestamp;
	int ret;

	timestamp = upnp_time() + leasetime;
	ret = update_pinhole(uid, timestamp);
#ifdef ENABLE_LEASEFILE
	if (ret == 0)
		lease_file6_update(uid, timestamp);
#endif
	return ret;
#else
	UNUSED(uid); UNUSED(leasetime);

	return -42; /* not implemented */
#endif
}

int
upnp_delete_inboundpinhole(unsigned short uid)
{
#if defined(USE_PF) || defined(USE_NETFILTER)
	int ret;
	ret = delete_pinhole(uid);
#ifdef ENABLE_LEASEFILE
	if (ret == 0)
		lease_file6_remove((const char *)"*", 0, 0, uid);
#endif

	return ret;
#else
	UNUSED(uid);

	return -1;
#endif
}

#if 0
/*
 * Result:
 * 	 1: Found Result
 * 	-4: No result
 * 	-5: Result in another table
 * 	-6: Result in another chain
 * 	-7: Result in a chain not a rule
*/
int
upnp_check_pinhole_working(const char * uid,
                           char * eaddr,
                           char * iaddr,
                           unsigned short * eport,
                           unsigned short * iport,
                           char * protocol,
                           int * rulenum_used)
{
	/* TODO : to be implemented */
#if 0
	FILE * fd;
	time_t expire = upnp_time();
	char buf[1024], filename[] = "/var/log/kern.log", expire_time[32]="";
	int res = -4, str_len;

	str_len = strftime(expire_time, sizeof(expire_time), "%b %d %H:%M:%S", localtime(&expire));

	fd = fopen(filename, "r");
	if (fd==NULL)
	{
		syslog(LOG_ERR, "Get_rule: could not open file: %s", filename);
		return -1;
	}

	syslog(LOG_INFO, "Get_rule: Starting getting info in file %s for %s\n", filename, uid);
	buf[sizeof(buf)-1] = 0;
	while(fgets(buf, sizeof(buf)-1, fd) != NULL && res != 1)
	{
		//printf("line: %s\n", buf);
		char * r, * t, * c, * p;
		// looking for something like filter:FORWARD:rule: or filter:MINIUPNPD:rule:
		r = strstr(buf, ":rule:");
		p = strstr(buf, ":policy:");
		t = strstr(buf, "TRACE:"); // table pointeur
		t += 7;
		c = t + 7; // chain pointeur
		if(r)
		{
			printf("\t** Found %.*s\n", 24 ,t);
			char * src, * dst, * sport, * dport, * proto, * line;
			char time[15]="", src_addr[40], dst_addr[40], proto_tmp[8];
			int proto_int;
			strncpy(time, buf, sizeof(time));
			/*if(compare_time(time, expire_time)<0)
			{
				printf("\t\tNot corresponding time\n");
				continue;
			}*/

			line = r + 6;
			printf("\trule line = %d\n", atoi(line));

			src = strstr(buf, "SRC=");
			src += 4;
			snprintf(src_addr, sizeof(src_addr), "%.*s", 39, src);
#if 0
			del_char(src_addr);
			add_char(src_addr);
#endif

			dst = strstr(buf, "DST=");
			dst += 4;
			snprintf(dst_addr, sizeof(dst_addr), "%.*s", 39, dst);
#if 0
			del_char(dst_addr);
			add_char(dst_addr);
#endif

			proto = strstr(buf, "PROTO=");
			proto += 6;
			proto_int = atoi(protocol);
			if(proto_int == IPPROTO_UDP)
				strcpy(proto_tmp, "UDP");
			else if(proto_int == IPPROTO_TCP)
				strcpy(proto_tmp, "TCP");
#ifdef IPPROTO_UDPLITE
			else if(proto_int == IPPROTO_UDPLITE)
				strcpy(proto_tmp, "UDPLITE");
#endif
			else
				strcpy(proto_tmp, "UnsupportedProto");

	//		printf("\tCompare eaddr: %s // protocol: %s\n\t     to  addr: %s // protocol: %.*s\n", eaddr, proto_tmp, src_addr, strlen(proto_tmp), proto);
	//		printf("\tCompare iaddr: %s // protocol: %s\n\t     to  addr: %s // protocol: %.*s\n", iaddr, proto_tmp, dst_addr, strlen(proto_tmp), proto);
			// TODO Check time
			// Check that the paquet found in trace correspond to the one we are looking for
			if( /*(strcmp(eaddr, src_addr) == 0) &&*/ (strcmp(iaddr, dst_addr) == 0) && (strncmp(proto_tmp, proto, strlen(proto_tmp))==0))
			{
				sport = strstr(buf, "SPT=");
				sport += 4;
				dport = strstr(buf, "DPT=");
				dport += 4;
				printf("\tCompare eport: %hu\n\t     to   port: %d\n", *eport, atoi(sport));
				printf("\tCompare iport: %hu\n\t     to   port: %d\n", *iport, atoi(dport));
				if(/*eport != atoi(sport) &&*/ *iport != atoi(dport))
				{
					printf("\t\tPort not corresponding\n");
					continue;
				}
				printf("\ttable found: %.*s\n", 6, t);
				printf("\tchain found: %.*s\n", 9, c);
				// Check that the table correspond to the filter table
				if(strncmp(t, "filter", 6)==0)
				{
					// Check that the table correspond to the MINIUPNP table
					if(strncmp(c, "MINIUPNPD", 9)==0)
					{
						*rulenum_used = atoi(line);
						res = 1;
					}
					else
					{
						res = -6;
						continue;
					}
				}
				else
				{
					res = -5;
					continue;
				}
			}
			else
			{
				printf("Packet information not corresponding\n");
				continue;
			}
		}
		if(!r && p)
		{
			printf("\t** Policy case\n");
			char * src, * dst, * sport, * dport, * proto, * line;
			char time[15], src_addr[40], dst_addr[40], proto_tmp[8];
			int proto_int;
			strncpy(time, buf, sizeof(time));
			/*if(compare_time(time, expire_time)<0)
			{
				printf("\t\tNot corresponding time\n");
				continue;
			}*/

			line = p + 8;
			printf("\trule line = %d\n", atoi(line));

			src = strstr(buf, "SRC=");
			src += 4;
			snprintf(src_addr, sizeof(src_addr), "%.*s", 39, src);
#if 0
			del_char(src_addr);
			add_char(src_addr);
#endif

			dst = strstr(buf, "DST=");
			dst += 4;
			snprintf(dst_addr, sizeof(dst_addr), "%.*s", 39, dst);
#if 0
			del_char(dst_addr);
			add_char(dst_addr);
#endif

			proto = strstr(buf, "PROTO=");
			proto += 6;
			proto_int = atoi(protocol);
			if(proto_int == IPPROTO_UDP)
				strcpy(proto_tmp, "UDP");
			else if(proto_int == IPPROTO_TCP)
				strcpy(proto_tmp, "TCP");
#ifdef IPPROTO_UDPLITE
			else if(proto_int == IPPROTO_UDPLITE)
				strcpy(proto_tmp, "UDPLITE");
#endif
			else
				strcpy(proto_tmp, "UnsupportedProto");

	//		printf("\tCompare eaddr: %s // protocol: %s\n\t     to  addr: %s // protocol: %.*s\n", eaddr, proto_tmp, src_addr, strlen(proto_tmp), proto);
	//		printf("\tCompare iaddr: %s // protocol: %s\n\t     to  addr: %s // protocol: %.*s\n", iaddr, proto_tmp, dst_addr, strlen(proto_tmp), proto);
			// Check that the paquet found in trace correspond to the one we are looking for
			if( (strcmp(eaddr, src_addr) == 0) && (strcmp(iaddr, dst_addr) == 0) && (strncmp(proto_tmp, proto, 5)==0))
			{
				sport = strstr(buf, "SPT=");
				sport += 4;
				dport = strstr(buf, "DPT=");
				dport += 4;
				printf("\tCompare eport: %hu\n\t     to   port: %d\n", *eport, atoi(sport));
				printf("\tCompare iport: %hu\n\t     to   port: %d\n", *iport, atoi(dport));
				if(*eport != atoi(sport) && *iport != atoi(dport))
				{
					printf("\t\tPort not corresponding\n");
					continue;
				}
				else
				{
					printf("Find a corresponding policy trace in the chain: %.*s\n", 10, c);
					res = -7;
					continue;
				}
			}
			else
				continue;
		}
	}
	fclose(fd);
	return res;
#else
	return -42;	/* to be implemented */
#endif
}
#endif

int
upnp_clean_expired_pinholes(unsigned int * next_timestamp)
{
	int ret = 0;
#if defined(USE_PF) || defined(USE_NETFILTER)
	ret = clean_pinhole_list(next_timestamp);
#else
	UNUSED(next_timestamp);
#endif
#ifdef ENABLE_LEASEFILE
	lease_file6_expire();
#endif

	return ret;
}

#endif /* ENABLE_UPNPPINHOLE */
