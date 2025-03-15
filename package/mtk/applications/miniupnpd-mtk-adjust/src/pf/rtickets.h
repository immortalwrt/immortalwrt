/* $Id: $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2023 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef RTICKETS_H_INCLUDED
#define RTICKETS_H_INCLUDED

#if defined(PF_NEWSTYLE) && defined(DIOCXEND)
#define PF_RELEASETICKETS
#define release_ticket(device, ticket_num) {\
	if (ioctl((device), DIOCXEND, &(ticket_num)) < 0) {\
		syslog(LOG_ERR, "ioctl(dev, DIOCXEND, ...): %m");\
	}\
}
#else
#define release_ticket(device, ticket_num)	(void)(ticket_num)
#endif

#endif
