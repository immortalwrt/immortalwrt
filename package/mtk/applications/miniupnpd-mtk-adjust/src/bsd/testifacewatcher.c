/* $Id: testifacewatcher.c,v 1.2 2012/05/21 08:55:10 nanard Exp $ */

#include <syslog.h>

int
OpenAndConfInterfaceWatchSocket(void);

void
ProcessInterfaceWatchNotify(int s);

const char * ext_if_name;
volatile sig_atomic_t should_send_public_address_change_notif = 0;

int main(int argc, char * * argv)
{
	int s;

	ext_if_name = "ep0";
	openlog("testifacewatcher", LOG_CONS|LOG_PERROR, LOG_USER);

	syslog(LOG_DEBUG, "test");
	s = OpenAndConfInterfaceWatchSocket();
	for(;;) {
		if(should_send_public_address_change_notif) {
			syslog(LOG_DEBUG, "should_send_public_address_change_notif !");
			should_send_public_address_change_notif = 0;
		}
		ProcessInterfaceWatchNotify(s);
	}
	closelog();
	return 0;
}
