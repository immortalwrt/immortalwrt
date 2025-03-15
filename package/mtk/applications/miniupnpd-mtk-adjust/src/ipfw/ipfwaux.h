/* $Id: ipfwaux.h,v 1.6 2015/09/04 14:20:58 nanard Exp $ */
/*
 * MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2009-2012 Jardel Weyrich
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution
 */
#ifndef IPFWAUX_H
#define IPFWAUX_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_fw.h>

#define IP_FW_BASE	(IP_FW_ADD - 5)
#define IP_FW_INIT	(IP_FW_BASE + 1)
#define IP_FW_TERM	(IP_FW_BASE + 2)

int ipfw_exec(int optname, void * optval, uintptr_t optlen);
void ipfw_free_ruleset(struct ip_fw ** rules);
int ipfw_fetch_ruleset(struct ip_fw ** rules, int * total_fetched, int count);
int ipfw_validate_protocol(int value);
int ipfw_validate_ifname(const char * const value);

#endif

