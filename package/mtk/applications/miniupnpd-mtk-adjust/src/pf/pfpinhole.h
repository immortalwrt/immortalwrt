/* $Id: pfpinhole.h,v 1.9 2012/05/01 22:37:53 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2012-2016 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef PFPINHOLE_H_INCLUDED
#define PFPINHOLE_H_INCLUDED

#ifdef ENABLE_UPNPPINHOLE

int find_pinhole(const char * ifname,
                 const char * rem_host, unsigned short rem_port,
                 const char * int_client, unsigned short int_port,
                 int proto,
                 char *desc, int desc_len, unsigned int * timestamp);

int add_pinhole(const char * ifname,
                const char * rem_host, unsigned short rem_port,
                const char * int_client, unsigned short int_port,
                int proto, const char * desc, unsigned int timestamp);

int delete_pinhole(unsigned short uid);

int
get_pinhole_info(unsigned short uid,
                 char * rem_host, int rem_hostlen, unsigned short * rem_port,
                 char * int_client, int int_clientlen, unsigned short * int_port,
                 int * proto, char * desc, int desclen,
                 unsigned int * timestamp,
                 u_int64_t * packets, u_int64_t * bytes);

int update_pinhole(unsigned short uid, unsigned int timestamp);

int clean_pinhole_list(unsigned int * next_timestamp);

#endif /* ENABLE_UPNPPINHOLE */

#endif

