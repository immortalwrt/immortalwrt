/* $Id: pcplearndscp.c $ */
/* MiniUPnP project
 * Website : http://miniupnp.free.fr/
 * Author : Miroslav Bagljas

Copyright (c) 2013 by Cisco Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * The name of the author may not be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>

#include "upnpglobalvars.h"
#include "pcplearndscp.h"
#include "macros.h"

#ifdef PCP_SADSCP

void
print_dscp(void) {
	unsigned int i;

	for (i=0; i < num_dscp_values; i++){
		syslog(LOG_DEBUG, "Appname %*.s, del %d, loss %d, jitter %d, dscp %d",
				dscp_values_list[i].app_name_len,
				dscp_values_list[i].app_name, dscp_values_list[i].delay,
				dscp_values_list[i].loss, dscp_values_list[i].jitter,
				dscp_values_list[i].dscp_value
				);
	}
}

int
read_learn_dscp_line(struct dscp_values *dscpvalues, char *p, int debug_flag)
{
	char * q;
	size_t len;
	unsigned int sizeof_first_token = sizeof("set_learn_dscp") - 1;
	int af_value;
	int cs_value;

	/* first token: (set_learn_dscp) skip it */
	while(isspace(*p))
		p++;
	if(0 == memcmp(p, "set_learn_dscp", sizeof_first_token))
	{
		p += sizeof_first_token;
	}
	else
	{
		return -1;
	}
	while(isspace(*p))
		p++;

	/* second token: name of the application */
	// if
	if(!(*p == '"'))
		return -1;
	p++;
	for(q = p; !(*q == '"'); q++);
	len = q - p;
	if (len != 0) {
		dscpvalues->app_name = strndup(p, len);
	} else {
		dscpvalues->app_name = NULL;
	}
	dscpvalues->app_name_len = len;
	p = q + 1;

	/* third token:  delay */
	while(isspace(*p))
		p++;
	if(!isdigit(*p))
		goto exit_err_and_cleanup;
	for(q = p; isdigit(*q); q++);
	if(isspace(*q))
	{
		*q = '\0';
		dscpvalues->delay = (unsigned char)atoi(p);
		if (dscpvalues->delay >= 3) {
			INIT_PRINT_ERR("Wrong delay value %d in \n", dscpvalues->delay);
			INIT_PRINT_ERR("Delay can be from set {0,1,2} 0=low delay, 1=medium delay, 2=high delay\n");
			goto exit_err_and_cleanup;
		}
	}
	else
	{
		goto exit_err_and_cleanup;
	}
	p = q + 1;

	/* fourth token:  loss */
	while(isspace(*p))
		p++;
	if(!isdigit(*p))
		goto exit_err_and_cleanup;

	for(q = p; isdigit(*q); q++);
	if(isspace(*q))
	{
		*q = '\0';
		dscpvalues->loss = (unsigned char)atoi(p);
		if (dscpvalues->loss >= 3) {
			INIT_PRINT_ERR("Wrong loss value %d \n", dscpvalues->loss);
			INIT_PRINT_ERR("Delay can be from set {0,1,2} 0=low loss, 1=medium loss, 2=high loss\n");
			goto exit_err_and_cleanup;
		}
	}
	else
	{
		goto exit_err_and_cleanup;
	}
	p = q + 1;

	/* fifth token:  jitter */
	while(isspace(*p))
		p++;
	if(!isdigit(*p))
				goto exit_err_and_cleanup;
	for(q = p; isdigit(*q); q++);
	if(isspace(*q))
	{
		*q = '\0';
		dscpvalues->jitter = (unsigned char)atoi(p);
		if (dscpvalues->jitter >= 3) {
			INIT_PRINT_ERR("Wrong jitter value %d \n", dscpvalues->jitter);
			INIT_PRINT_ERR("Delay can be from set {0,1,2} 0=low jitter, 1=medium jitter, 2=high jitter \n");
			goto exit_err_and_cleanup;
		}
	}
	else
	{
		goto exit_err_and_cleanup;
	}
	p = q + 1;
	while(isspace(*p))
		p++;
	/*{
	}*/
	p = q + 1;

	/* sixth token: DSCP value */
	while(isspace(*p))
		p++;
	if(!isdigit(*p) && !( toupper(*p) == 'A' && toupper(*(p+1)) == 'F') &&
			!( toupper(*p) == 'C' && toupper(*(p+1)) == 'S') &&
			!( toupper(*p) == 'E' && toupper(*(p+1)) == 'F')
			)
		goto exit_err_and_cleanup;
//	for(q = p; isdigit(*q) || (toupper(*q) == 'A') || (toupper(*q) == 'F'); q++);
	for(q = p; isdigit(*q) || isalpha(*q); q++);
	if(isspace(*q) || *q == '\0')
	{
		*q = '\0';
		if (toupper(*p) == 'A' && toupper(*(p+1)) == 'F'){
			p = p+2;
			if (*p == '\0') {
				dscpvalues->dscp_value = 0;
			}
			else if (!isdigit(*p)) {
				goto exit_err_and_cleanup;
			}
			else {
			af_value = atoi(p);
			switch(af_value) {
			case 11:
				dscpvalues->dscp_value = 10;
				break;
			case 12:
				dscpvalues->dscp_value = 12;
				break;
			case 13:
				dscpvalues->dscp_value = 14;
				break;
			case 21:
				dscpvalues->dscp_value = 18;
				break;
			case 22:
				dscpvalues->dscp_value = 20;
				break;
			case 23:
				dscpvalues->dscp_value = 22;
				break;
			case 31:
				dscpvalues->dscp_value = 26;
				break;
			case 32:
				dscpvalues->dscp_value = 28;
				break;
			case 33:
				dscpvalues->dscp_value = 30;
				break;
			case 41:
				dscpvalues->dscp_value = 34;
				break;
			case 42:
				dscpvalues->dscp_value = 36;
				break;
			case 43:
				dscpvalues->dscp_value = 38;
				break;
			default:
				INIT_PRINT_ERR("Unknown AF value %d \n", af_value);
				goto exit_err_and_cleanup;
			}
			}
		}
		else if (toupper(*p) == 'C' && toupper(*(p+1)) == 'S'){
			p=p+2;
			if (*p == '\0') {
				dscpvalues->dscp_value = 0;
			}
			else if (!isdigit(*p)) {
				INIT_PRINT_ERR("Not digit after CS but %c \n", *p);
				goto exit_err_and_cleanup;
			}
			else {
				cs_value = atoi(p);
				switch(cs_value) {
				case 1:
					dscpvalues->dscp_value = 8;
					break;
				case 2:
					dscpvalues->dscp_value = 16;
					break;
				case 3:
					dscpvalues->dscp_value = 24;
					break;
				case 4:
					dscpvalues->dscp_value = 32;
					break;
				case 5:
					dscpvalues->dscp_value = 40;
					break;
				case 6:
					dscpvalues->dscp_value = 48;
					break;
				case 7:
					dscpvalues->dscp_value = 56;
					break;
				default:
					INIT_PRINT_ERR("Unknown CS value %d \n", cs_value);
					goto exit_err_and_cleanup;
				}
			}
		}
		else if (toupper(*p) == 'E' && toupper(*(p+1)) == 'F'){
			dscpvalues->dscp_value = 46;
		}
		else {
			dscpvalues->dscp_value = (unsigned char)atoi(p);
		}
	}
	else
	{
		goto exit_err_and_cleanup;
	}

	return 0;

exit_err_and_cleanup:
	free(dscpvalues->app_name);
	dscpvalues->app_name = NULL;
	dscpvalues->app_name_len = 0;
	return -1;


}

#endif
