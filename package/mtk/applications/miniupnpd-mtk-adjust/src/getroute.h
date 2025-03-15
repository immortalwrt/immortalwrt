/* $Id: getroute.h,v 1.3 2013/02/06 10:50:04 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2013 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef  GETROUTE_H_INCLUDED
#define  GETROUTE_H_INCLUDED

int
get_src_for_route_to(const struct sockaddr * dst,
                     void * src, size_t * src_len,
                     int * index);

#endif

