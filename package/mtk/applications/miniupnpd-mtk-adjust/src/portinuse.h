/* $Id: $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2014 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef __PORTINUSE_H__
#define __PORTINUSE_H__

#ifdef CHECK_PORTINUSE
/* portinuse()
 * determine wether a port is already in use
 * on a given interface.
 * returns: 0 not in use, > 0 in use
 *         -1 in case of error */
int
port_in_use(const char *if_name,
            unsigned port, int proto,
            const char *iaddr, unsigned iport);
#endif /* CHECK_PORTINUSE */

#endif
