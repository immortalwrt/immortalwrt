/* $Id: testupnpdescgen.c,v 1.37 2020/11/04 21:02:29 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2021 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* for mkdir */
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "macros.h"
#include "config.h"
#include "upnpglobalvars.h"
#include "upnpdescgen.h"
#include "upnpdescstrings.h"
#include "getifaddr.h"

char uuidvalue_igd[] = "uuid:12345678-0000-0000-0000-000000abcd01";
char uuidvalue_wan[] = "uuid:12345678-0000-0000-0000-000000abcd02";
char uuidvalue_wcd[] = "uuid:12345678-0000-0000-0000-000000abcd03";
char serialnumber[] = "12345678";
char modelnumber[] = "1";
char presentationurl[] = "http://192.168.0.1:8080/";
/*char presentationurl[] = "";*/
#ifdef ENABLE_MANUFACTURER_INFO_CONFIGURATION
char friendly_name[] = OS_NAME " router";
char manufacturer_name[] = ROOTDEV_MANUFACTURER;
char manufacturer_url[] = ROOTDEV_MANUFACTURERURL;
char model_name[] = ROOTDEV_MODELNAME;
char model_description[] = ROOTDEV_MODELDESCRIPTION;
char model_url[] = ROOTDEV_MODELURL;
#endif /* ENABLE_MANUFACTURER_INFO_CONFIGURATION */
#ifdef RANDOMIZE_URLS
char random_url[RANDOM_URL_MAX_LEN] = "RANDOM";
#endif /* RANDOMIZE_URLS */
unsigned int upnp_configid = 666;

const char * use_ext_ip_addr = NULL;
const char * ext_if_name = "eth0";

int runtime_flags = 0;

int getifaddr(const char * ifname, char * buf, int len, struct in_addr * addr, struct in_addr * mask)
{
	UNUSED(ifname);
	UNUSED(addr);
	UNUSED(mask);
	strncpy(buf, "1.2.3.4", len);
	return 0;
}

int addr_is_reserved(struct in_addr * addr)
{
	UNUSED(addr);
	return 0;
}

int upnp_get_portmapping_number_of_entries(void)
{
	return 42;
}

int get_wan_connection_status(const char * ifname)
{
	UNUSED(ifname);
	return 2;
}

/* To be improved */
int
xml_pretty_print(const char * s, int len, FILE * f)
{
	int n = 0, i;
	int elt_close = 0;
	int c, indent = 0;

	if(!s)
		return n;
	while(len > 0)
	{
		c = *(s++);	len--;
		switch(c)
		{
		case '<':
			if(len>0 && *s == '/')
				elt_close++;
			else if(len>0 && *s == '?')
				elt_close = 1;
			else
				elt_close = 0;
			if(elt_close!=1)
			{
				if(elt_close > 1)
					indent--;
				fputc('\n', f); n++;
				for(i=indent; i>0; i--)
					fputc(' ', f);
				n += indent;
			}
			fputc(c, f); n++;
			break;
		case '>':
			fputc(c, f); n++;
			if(elt_close==1)
			{
				/*fputc('\n', f); n++; */
				/* elt_close = 0; */
				if(indent > 0)
					indent--;
			}
			else if(elt_close == 0)
				indent++;
			break;
		case '\n':
			/* remove existing LF */
			break;
		default:
			fputc(c, f); n++;
		}
	}
	return n;
}

/* stupid test */
const char * str1 = "Prefix123String";
const char * str2 = "123String";

void stupid_test(void)
{
	printf("str1:'%s' str2:'%s'\n", str1, str2);
	printf("str1:%p str2:%p str2-str1:%ld\n", str1, str2, (long)(str2-str1));
}

/* main */

int
main(int argc, char * * argv)
{
	int force_igd1 = 0;
	char * rootDesc;
	int rootDescLen;
	char * s;
	int l;
	FILE * f;

	for(l = 1; l < argc; l++) {
		if(0 == strcmp(argv[l], "--help") || 0 == strcmp(argv[l], "-h")) {
			printf("Usage:\t%s [options]\n", argv[0]);
			printf("options:\n");
#ifdef IGD_V2
			printf("\t--forceigdv1    Force versions of devices to be 1\n");
#else
			printf("\tNone\n");
#endif
			return 0;
#ifdef IGD_V2
		} else if(0 == strcmp(argv[l], "--forceigdv1")) {
			force_igd1 = 1;
#endif
		} else {
			fprintf(stderr, "unknown option %s\n", argv[l]);
		}
	}

	if(mkdir("testdescs", 0777) < 0) {
		if(errno != EEXIST) {
			perror("mkdir");
		}
	}
	printf("Root Description :\n");
	rootDesc = genRootDesc(&rootDescLen, force_igd1);
	xml_pretty_print(rootDesc, rootDescLen, stdout);
	f = fopen("testdescs/rootdesc.xml", "w");
	if(f) {
		xml_pretty_print(rootDesc, rootDescLen, f);
		fclose(f);
	}
	free(rootDesc);
	printf("\n-------------\n");
	printf("WANIPConnection Description :\n");
	s = genWANIPCn(&l, force_igd1);
	xml_pretty_print(s, l, stdout);
	f = fopen("testdescs/wanipc_scpd.xml", "w");
	if(f) {
		xml_pretty_print(s, l, f);
		fclose(f);
	}
	free(s);
	printf("\n-------------\n");
	printf("WANConfig Description :\n");
	s = genWANCfg(&l, force_igd1);
	xml_pretty_print(s, l, stdout);
	f = fopen("testdescs/wanconfig_scpd.xml", "w");
	if(f) {
		xml_pretty_print(s, l, f);
		fclose(f);
	}
	free(s);
	printf("\n-------------\n");
#ifdef ENABLE_L3F_SERVICE
	printf("Layer3Forwarding service :\n");
	s = genL3F(&l, force_igd1);
	xml_pretty_print(s, l, stdout);
	f = fopen("testdescs/l3f_scpd.xml", "w");
	if(f) {
		xml_pretty_print(s, l, f);
		fclose(f);
	}
	free(s);
	printf("\n-------------\n");
#endif
#ifdef ENABLE_6FC_SERVICE
	printf("WANIPv6FirewallControl service :\n");
	s = gen6FC(&l, force_igd1);
	xml_pretty_print(s, l, stdout);
	f = fopen("testdescs/wanipv6fc_scpd.xml", "w");
	if(f) {
		xml_pretty_print(s, l, f);
		fclose(f);
	}
	free(s);
	printf("\n-------------\n");
#endif
#ifdef ENABLE_DP_SERVICE
	printf("DeviceProtection service :\n");
	s = genDP(&l, force_igd1);
	xml_pretty_print(s, l, stdout);
	f = fopen("testdescs/dp_scpd.xml", "w");
	if(f) {
		xml_pretty_print(s, l, f);
		fclose(f);
	}
	free(s);
	printf("\n-------------\n");
#endif
#ifdef ENABLE_EVENTS
	s = getVarsWANIPCn(&l);
	xml_pretty_print(s, l, stdout);
	free(s);
	printf("\n-------------\n");
	s = getVarsWANCfg(&l);
	xml_pretty_print(s, l, stdout);
	free(s);
	printf("\n-------------\n");
#ifdef ENABLE_L3F_SERVICE
	s = getVarsL3F(&l);
	xml_pretty_print(s, l, stdout);
	free(s);
	printf("\n-------------\n");
#ifdef ENABLE_6FC_SERVICE
	s = getVars6FC(&l);
	xml_pretty_print(s, l, stdout);
	free(s);
	printf("\n-------------\n");
#endif
#ifdef ENABLE_DP_SERVICE
	s = getVarsDP(&l);
	xml_pretty_print(s, l, stdout);
	free(s);
	printf("\n-------------\n");
#endif
#endif
#endif
/*
	stupid_test();
*/
	return 0;
}

