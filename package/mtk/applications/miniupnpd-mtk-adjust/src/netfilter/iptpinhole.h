/* $Id: iptpinhole.h,v 1.5 2012/05/08 20:41:45 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2012-2016 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */
#ifndef IPTPINHOLE_H_INCLUDED
#define IPTPINHOLE_H_INCLUDED

#ifdef ENABLE_UPNPPINHOLE
#include <sys/types.h>

int find_pinhole(const char * ifname,
                 const char * rem_host, unsigned short rem_port,
                 const char * int_client, unsigned short int_port,
                 int proto,
                 char *desc, int desc_len, unsigned int * timestamp);

int add_pinhole(const char * ifname,
                const char * rem_host, unsigned short rem_port,
                const char * int_client, unsigned short int_port,
                int proto, const char *desc, unsigned int timestamp);

int update_pinhole(unsigned short uid, unsigned int timestamp);

int delete_pinhole(unsigned short uid);

int
get_pinhole_info(unsigned short uid,
                 char * rem_host, int rem_hostlen, unsigned short * rem_port,
                 char * int_client, int int_clientlen,
                 unsigned short * int_port,
                 int * proto, char * desc, int desclen,
                 unsigned int * timestamp,
                 u_int64_t * packets, u_int64_t * bytes);

int get_pinhole_uid_by_index(int index);

int clean_pinhole_list(unsigned int * next_timestamp);

#endif /* ENABLE_UPNPPINHOLE */

#endif
