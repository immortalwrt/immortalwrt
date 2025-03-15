/* $Id: upnppermissions.h,v 1.14 2023/02/11 23:02:17 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2023 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef UPNPPERMISSIONS_H_INCLUDED
#define UPNPPERMISSIONS_H_INCLUDED

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "config.h"

#ifdef ENABLE_REGEX
#include <regex.h>
#endif

/* UPnP permission rule samples:
 * allow 1024-65535 192.168.3.0/24 1024-65535
 * deny 0-65535 192.168.1.125/32 0-65535 */
struct upnpperm {
	enum {UPNPPERM_ALLOW=1, UPNPPERM_DENY=2 } type;
				/* is it an allow or deny permission rule ? */
	u_short eport_min, eport_max;	/* external port range */
	struct in_addr address, mask;	/* ip/mask */
	u_short iport_min, iport_max;	/* internal port range */
	char * re;
#ifdef ENABLE_REGEX
	regex_t regex;	/* matching regex */
#endif
};

/* read_permission_line()
 * returns: 0 line read okay
 *          -1 error reading line
 *
 * line sample :
 *  allow 1024-65535 192.168.3.0/24 1024-65535
 *  allow 22 192.168.4.33/32 22
 *  deny 0-65535 0.0.0.0/0 0-65535 */
int
read_permission_line(struct upnpperm * perm,
                     char * p);

void
free_permission_line(struct upnpperm * perm);

/* check_upnp_rule_against_permissions()
 * returns: 0 if the upnp rule should be rejected,
 *          1 if it could be accepted */
int
check_upnp_rule_against_permissions(const struct upnpperm * permary,
                                    int n_perms,
                                    u_short eport, struct in_addr address,
                                    u_short iport, const char * desc);

/**
 * Build an array of all allowed external ports (for the address and internal port)
 */
void
get_permitted_ext_ports(uint32_t * allowed,
                        const struct upnpperm * permary, int n_perms,
                        in_addr_t addr, u_short iport);

#ifdef USE_MINIUPNPDCTL
void
write_permlist(int fd, const struct upnpperm * permary,
               int nperms);
#endif

#endif

