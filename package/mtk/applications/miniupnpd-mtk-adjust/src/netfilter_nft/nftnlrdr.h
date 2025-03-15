/*
 * MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2015 Tomofumi Hayashi
 * (c) 2019 Paul Chambers
 * 
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution.
 */

#ifndef NFTNLRDR_H_INCLUDED
#define NFTNLRDR_H_INCLUDED

#include "../commonrdr.h"

int init_redirect(void);
void shutdown_redirect(void);

int
add_redirect_rule2(const char * ifname,
		   const char * rhost, unsigned short eport,
		   const char * iaddr, unsigned short iport, int proto,
		   const char * desc, unsigned int timestamp);

int
add_peer_redirect_rule2(const char * ifname,
			const char * rhost, unsigned short rport,
			const char * eaddr, unsigned short eport,
			const char * iaddr, unsigned short iport, int proto,
			const char * desc, unsigned int timestamp);

int
add_filter_rule2(const char * ifname,
		 const char * rhost, const char * iaddr,
		 unsigned short eport, unsigned short iport,
		 int proto, const char * desc);

int
delete_redirect_and_filter_rules(unsigned short eport, int proto);

int
delete_filter_rule(const char * ifname, unsigned short port, int proto);

int
add_peer_dscp_rule2(const char * ifname,
		    const char * rhost, unsigned short rport,
		    unsigned char dscp,
		    const char * iaddr, unsigned short iport, int proto,
		    const char * desc, unsigned int timestamp);

int
get_peer_rule_by_index(int index,
		       char * ifname, unsigned short * eport,
		       char * iaddr, int iaddrlen, unsigned short * iport,
		       int * proto, char * desc, int desclen,
		       char * rhost, int rhostlen, unsigned short * rport,
		       unsigned int * timestamp,
		       u_int64_t * packets, u_int64_t * bytes);
int
get_nat_redirect_rule(const char * nat_chain_name, const char * ifname,
		      unsigned short eport, int proto,
		      char * iaddr, int iaddrlen, unsigned short * iport,
		      char * desc, int desclen,
		      char * rhost, int rhostlen,
		      unsigned int * timestamp,
		      u_int64_t * packets, u_int64_t * bytes);
int
get_redirect_rule_by_index(int index,
			   char * ifname, unsigned short * eport,
			   char * iaddr, int iaddrlen, unsigned short * iport,
			   int * proto, char * desc, int desclen,
			   char * rhost, int rhostlen,
			   unsigned int * timestamp,
			   u_int64_t * packets, u_int64_t * bytes);

unsigned short *
get_portmappings_in_range(unsigned short startport, unsigned short endport,
			  int proto, unsigned int * number);

/* in nfct_get.c */
int get_nat_ext_addr(struct sockaddr* src, struct sockaddr *dst, uint8_t proto,
		     struct sockaddr* ret_ext);

#endif
