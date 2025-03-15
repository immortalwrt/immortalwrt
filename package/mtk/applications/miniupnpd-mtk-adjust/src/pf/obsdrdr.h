/* $Id: obsdrdr.h,v 1.25 2020/05/29 21:48:57 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2020 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef OBSDRDR_H_INCLUDED
#define OBSDRDR_H_INCLUDED

#include "../commonrdr.h"

/* add_redirect_rule2() uses DIOCCHANGERULE ioctl
 * proto can take the values IPPROTO_UDP or IPPROTO_TCP
 */
int
add_redirect_rule2(const char * ifname,
                   const char * rhost, unsigned short eport,
                   const char * iaddr, unsigned short iport, int proto,
                   const char * desc, unsigned int timestamp);

/* add_filter_rule2() uses DIOCCHANGERULE ioctl
 * proto can take the values IPPROTO_UDP or IPPROTO_TCP
 */
int
add_filter_rule2(const char * ifname,
                 const char * rhost, const char * iaddr,
                 unsigned short eport, unsigned short iport,
                 int proto, const char * desc);


/* get_redirect_rule() gets internal IP and port from
 * interface, external port and protocl
 */
#if 0
int
get_redirect_rule(const char * ifname, unsigned short eport, int proto,
                  char * iaddr, int iaddrlen, unsigned short * iport,
                  char * desc, int desclen,
                  u_int64_t * packets, u_int64_t * bytes);

int
get_redirect_rule_by_index(int index,
                           char * ifname, unsigned short * eport,
                           char * iaddr, int iaddrlen, unsigned short * iport,
                           int * proto, char * desc, int desclen,
                           u_int64_t * packets, u_int64_t * bytes);
#endif

/* delete_redirect_rule()
 */
int
delete_redirect_rule(const char * ifname, unsigned short eport, int proto);

/* delete_redirect_and_filter_rules()
 */
int
delete_redirect_and_filter_rules(const char * ifname, unsigned short eport,
                                 int proto);

int
delete_filter_rule(const char * ifname, unsigned short port, int proto);

#ifdef TEST
int
clear_redirect_rules(void);
int
clear_filter_rules(void);
int
clear_nat_rules(void);
#endif

#endif


