/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * author: Ryan Wagoner and Thomas Bernard
 * (c) 2007 Darren Reed
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <kstat.h>
#include <sys/sysmacros.h>

#include "../getifstats.h"

int
getifstats(const char * ifname, struct ifdata * data)
{
	char buffer[64], *s;
	kstat_named_t *kn;
	kstat_ctl_t *kc;
	int instance;
	kstat_t *ksp;

	if (data == NULL)
		goto error;

	if (ifname == NULL || *ifname == '\0')
		goto error;

	s = (char *)ifname + strlen(ifname);
	s--;
	while ((s > ifname) && isdigit(*s))
		s--;

	s++;
	instance = atoi(s);
	strlcpy(buffer, ifname, MIN(s - ifname + 1, 64));

	kc = kstat_open();
	if (kc != NULL) {
		ksp = kstat_lookup(kc, buffer, instance, (char *)ifname);
		if (ksp && (kstat_read(kc, ksp, NULL) != -1)) {
			/* found the right interface */
			if (sizeof(long) == 8) {
				kn = kstat_data_lookup(ksp, "rbytes64");
				if (kn != NULL) {
					data->ibytes = kn->value.i64;
				}
				kn = kstat_data_lookup(ksp, "ipackets64");
				if (kn != NULL) {
					data->ipackets = kn->value.i64;
				}
				kn = kstat_data_lookup(ksp, "obytes64");
				if (kn != NULL) {
					data->obytes = kn->value.i64;
				}
				kn = kstat_data_lookup(ksp, "opackets64");
				if (kn != NULL) {
					data->opackets = kn->value.i64;
				}
			} else {
				kn = kstat_data_lookup(ksp, "rbytes");
				if (kn != NULL) {
					data->ibytes = kn->value.i32;
				}
				kn = kstat_data_lookup(ksp, "ipackets");
				if (kn != NULL) {
					data->ipackets = kn->value.i32;
				}
				kn = kstat_data_lookup(ksp, "obytes");
				if (kn != NULL) {
					data->obytes = kn->value.i32;
				}
				kn = kstat_data_lookup(ksp, "opackets");
				if (kn != NULL) {
					data->ipackets = kn->value.i32;
				}
			}
			kn = kstat_data_lookup(ksp, "ifspeed");
			if (kn != NULL) {
				data->baudrate = kn->value.i32;
			}
			kstat_close(kc);
			return 0;	/* ok */
		}
		syslog(LOG_ERR, "kstat_lookup/read() failed: %m");
		kstat_close(kc);
		return -1;
	} else {
		syslog(LOG_ERR, "kstat_open() failed: %m");
	}
error:
	return -1;	/* not found or error */
}

