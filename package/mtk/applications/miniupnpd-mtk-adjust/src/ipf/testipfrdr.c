/* $Id: testipfrdr.c,v 1.3 2007/10/01 16:21:23 nanard Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <netinet/in.h>
#include <sys/types.h>
#include "ipfrdr.h"

/* test program for ipfrdr.c */

int runtime_flags = 0;

void
list_eports_tcp(void)
{
	unsigned short * port_list;
	unsigned int number = 0;
	unsigned int i;
	port_list = get_portmappings_in_range(0, 65535, IPPROTO_TCP, &number);
	printf("%u ports redirected (TCP) :", number);
	for(i = 0; i < number; i++)
	{
		printf(" %hu", port_list[i]);
	}
	printf("\n");
	free(port_list);
	port_list = get_portmappings_in_range(0, 65535, IPPROTO_UDP, &number);
	printf("%u ports redirected (UDP) :", number);
	for(i = 0; i < number; i++)
	{
		printf(" %hu", port_list[i]);
	}
	printf("\n");
	free(port_list);
}

int
main(int argc, char * * argv)
{
	char c;
	
	openlog("testipfrdrd", LOG_CONS|LOG_PERROR, LOG_USER);
	if(init_redirect() < 0)
	{
		fprintf(stderr, "init_redirect() failed\n");
		return 1;
	}
	
	printf("List rdr ports :\n");
	list_eports_tcp();
	
	printf("Add redirection !\n");
	add_redirect_rule2("xennet0", "*", 12345, "192.168.1.100", 54321, IPPROTO_UDP,
	                   "redirection description", 0);
	add_redirect_rule2("xennet0", "8.8.8.8", 12345, "192.168.1.100", 54321, IPPROTO_TCP,
	                   "redirection description", 0);
	
	printf("Check redirect rules with \"ipnat -l\" then press any key.\n");
	c = getchar();
	
	printf("List rdr ports :\n");
	list_eports_tcp();
	
	printf("Delete redirection !\n");
	delete_redirect_rule("xennet0", 12345, IPPROTO_UDP);
	delete_redirect_rule("xennet0", 12345, IPPROTO_TCP);
	
	printf("List rdr ports :\n");
	list_eports_tcp();
	
	return 0;
}

