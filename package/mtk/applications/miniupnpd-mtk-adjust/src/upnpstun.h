/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2018 Pali Rohár
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef UPNPSTUN_H_INCLUDED
#define UPNPSTUN_H_INCLUDED

int perform_stun(const char *if_name, const char *if_addr, const char *stun_host, unsigned short stun_port, struct in_addr *ext_addr, int *restrictive_nat);

#endif
