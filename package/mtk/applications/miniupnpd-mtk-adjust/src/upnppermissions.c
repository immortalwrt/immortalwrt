/* $Id: upnppermissions.c,v 1.20 2020/10/30 21:37:35 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2023 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#ifdef ENABLE_REGEX
#include <regex.h>
#endif

#include "config.h"
#include "macros.h"
#include "upnppermissions.h"

static int
isodigit(char c)
{
	return '0' <= c && c >= '7';
}

static char
hex2chr(char c)
{
	if(c >= 'a')
		return c - 'a';
	if(c >= 'A')
		return c - 'A';
	return c - '0';
}

static char
unescape_char(const char * s, int * seqlen)
{
	char c;
	int len;

	if(s[0] != '\\')
	{
		c = s[0];
		len = 1;
	}
	else
	{
		s++;
		c = s[0];
		len = 2;
		switch(s[0])
		{
		case 'a':  c = '\a'; break;
		case 'b':  c = '\b'; break;
		case 'f':  c = '\f'; break;
		case 'n':  c = '\n'; break;
		case 'r':  c = '\r'; break;
		case 't':  c = '\t'; break;
		case 'v':  c = '\v'; break;
		/* no need: escape the char itself
		case '\\': c = '\\'; break;
		case '\'': c = '\''; break;
		case '"':  c = '"';  break;
		case '?':  c = '?';  break;
		*/
		case 'x':
			if(isxdigit(s[1]) && isxdigit(s[2]))
			{
				c = (hex2chr(s[1]) << 4) + hex2chr(s[2]);
				len = 4;
			}
			break;
		default:
			if(isodigit(s[1]) && isodigit(s[2]) && isodigit(s[3]))
			{
				c = (hex2chr(s[0]) << 6) + (hex2chr(s[1]) << 3) + hex2chr(s[2]);
				len = 4;
			}
		}
	}

	if(seqlen)
		*seqlen = len;
	return c;
}

/* get_next_token(s, &token, raw)
 * put the unquoted/unescaped token in token and returns
 * a pointer to the begining of the next token
 * Do not unescape if raw is true */
static char *
get_next_token(const char * s, char ** token, int raw)
{
	char deli;
	const char * end;

	/* skip any whitespace */
	for(; isspace(*s); s++)
		if(*s == '\0' || *s == '\n')
		{
			if(token)
				*token = NULL;
			return (char *) s;
		}

	/* find the start */
	if(*s == '"' || *s == '\'')
	{
		deli = *s;
		s++;
	}
	else
		deli = 0;
	/* find the end */
	end = s;
	for(; *end != '\0' && *end != '\n' && (deli ? *end != deli : !isspace(*end));
	    end++)
		if(*end == '\\')
		{
			end++;
			if(*end == '\0')
				break;
		}

	/* save the token */
	if(token)
	{
		unsigned int token_len;
		unsigned int i;

		token_len = end - s;
		*token = strndup(s, token_len);
		if(!*token)
			return NULL;

		for(i = 0; (*token)[i] != '\0'; i++)
		{
			int sequence_len;

			if((*token)[i] != '\\')
				continue;

			if(raw && deli && (*token)[i + 1] != deli)
				continue;
			(*token)[i] = unescape_char(*token + i, &sequence_len);
			memmove(*token + i + 1, *token + i + sequence_len,
			        token_len - i - sequence_len);
		}
		if (i == 0)
		{
			/* behavior of realloc(p, 0) is implementation-defined, so better set it to NULL.
			 * https://github.com/miniupnp/miniupnp/issues/652#issuecomment-1518922139 */
			free(*token);
			*token = NULL;
		}
		else
		{
			char * tmp = realloc(*token, i);
			if (tmp != NULL)
				*token = tmp;
			else
				syslog(LOG_ERR, "%s: failed to reallocate to %u bytes",
				       "get_next_token()", i);
		}
	}

	/* return the beginning of the next token */
	if(deli && *end == deli)
		end++;
	while(isspace(*end))
		end++;
	return (char *) end;
}

/* read_permission_line()
 * parse the a permission line which format is :
 * (deny|allow) [0-9]+(-[0-9]+) ip/mask [0-9]+(-[0-9]+) regex
 * ip/mask is either 192.168.1.1/24 or 192.168.1.1/255.255.255.0
 */
int
read_permission_line(struct upnpperm * perm,
                     char * p)
{
	char * q;
	int n_bits;
	int i;

	/* zero memory : see https://github.com/miniupnp/miniupnp/issues/652 */
	memset(perm, 0, sizeof(struct upnpperm));

	/* first token: (allow|deny) */
	while(isspace(*p))
		p++;
	if(0 == memcmp(p, "allow", 5))
	{
		perm->type = UPNPPERM_ALLOW;
		p += 5;
	}
	else if(0 == memcmp(p, "deny", 4))
	{
		perm->type = UPNPPERM_DENY;
		p += 4;
	}
	else
	{
		return -1;
	}
	while(isspace(*p))
		p++;

	/* second token: eport or eport_min-eport_max */
	if(!isdigit(*p))
		return -1;
	for(q = p; isdigit(*q); q++);
	if(*q=='-')
	{
		*q = '\0';
		i = atoi(p);
		if(i > 65535)
			return -1;
		perm->eport_min = (u_short)i;
		q++;
		p = q;
		while(isdigit(*q))
			q++;
		*q = '\0';
		i = atoi(p);
		if(i > 65535)
			return -1;
		perm->eport_max = (u_short)i;
		if(perm->eport_min > perm->eport_max)
			return -1;
	}
	else if(isspace(*q))
	{
		*q = '\0';
		i = atoi(p);
		if(i > 65535)
			return -1;
		perm->eport_min = perm->eport_max = (u_short)i;
	}
	else
	{
		return -1;
	}
	p = q + 1;
	while(isspace(*p))
		p++;

	/* third token:  ip/mask */
	if(!isdigit(*p))
		return -1;
	for(q = p; isdigit(*q) || (*q == '.'); q++);
	if(*q=='/')
	{
		*q = '\0';
		if(!inet_aton(p, &perm->address))
			return -1;
		q++;
		p = q;
		while(isdigit(*q))
			q++;
		if(*q == '.')
		{
			while(*q == '.' || isdigit(*q))
				q++;
			if(!isspace(*q))
				return -1;
			*q = '\0';
			if(!inet_aton(p, &perm->mask))
				return -1;
		}
		else if(!isspace(*q))
			return -1;
		else
		{
			*q = '\0';
			n_bits = atoi(p);
			if(n_bits > 32)
				return -1;
			perm->mask.s_addr = htonl(n_bits ? (0xffffffffu << (32 - n_bits)) : 0);
		}
	}
	else if(isspace(*q))
	{
		*q = '\0';
		if(!inet_aton(p, &perm->address))
			return -1;
		perm->mask.s_addr = 0xffffffffu;
	}
	else
	{
		return -1;
	}
	p = q + 1;

	/* fourth token: iport or iport_min-iport_max */
	while(isspace(*p))
		p++;
	if(!isdigit(*p))
		return -1;
	for(q = p; isdigit(*q); q++);
	if(*q=='-')
	{
		*q = '\0';
		i = atoi(p);
		if(i > 65535)
			return -1;
		perm->iport_min = (u_short)i;
		q++;
		p = q;
		while(isdigit(*q))
			q++;
		*q = '\0';
		i = atoi(p);
		if(i > 65535)
			return -1;
		perm->iport_max = (u_short)i;
		if(perm->iport_min > perm->iport_max)
			return -1;
	}
	else if(isspace(*q) || *q == '\0')
	{
		*q = '\0';
		i = atoi(p);
		if(i > 65535)
			return -1;
		perm->iport_min = perm->iport_max = (u_short)i;
	}
	else
	{
		return -1;
	}
	p = q;

	/* fifth token: (optional) regex */
	p = get_next_token(p, &perm->re, 1);
	if(!p)
	{
		fprintf(stderr, "err when copying regex: out of memory\n");
		return -1;
	}
	if(perm->re)
	{
		if(perm->re[0] == '\0')
		{
			free(perm->re);
			perm->re = NULL;
		}
		else
		{
#ifdef ENABLE_REGEX
			/* icase: if case matters, it must be someone doing something nasty */
			int err;
			err = regcomp(&perm->regex, perm->re,
			              REG_EXTENDED | REG_ICASE | REG_NOSUB);
			if(err)
			{
				char errbuf[256];
				regerror(err, &perm->regex, errbuf, sizeof(errbuf));
				fprintf(stderr, "err when compiling regex \"%s\": %s\n",
				        perm->re, errbuf);
				free(perm->re);
				perm->re = NULL;
				return -1;
			}
#else
			fprintf(stderr, "MiniUPnP is not compiled with ENABLE_REGEX. "
			        "Please remove any regex filter and restart.\n");
			free(perm->re);
			perm->re = NULL;
			return -1;
#endif
		}
	}

#ifdef DEBUG
	printf("perm rule added : %s %hu-%hu %08x/%08x %hu-%hu %s\n",
	       (perm->type==UPNPPERM_ALLOW) ? "allow" : "deny",
	       perm->eport_min, perm->eport_max, ntohl(perm->address.s_addr),
	       ntohl(perm->mask.s_addr), perm->iport_min, perm->iport_max,
	       perm->re ? perm->re : "");
#endif
	return 0;
}

void
free_permission_line(struct upnpperm * perm)
{
	if(perm->re)
	{
		free(perm->re);
		perm->re = NULL;
#ifdef ENABLE_REGEX
		regfree(&perm->regex);
#endif
	}
}

#ifdef USE_MINIUPNPDCTL
void
write_permlist(int fd, const struct upnpperm * permary,
               int nperms)
{
	int l;
	const struct upnpperm * perm;
	int i;
	char buf[128];
	write(fd, "Permissions :\n", 14);
	for(i = 0; i<nperms; i++)
	{
		perm = permary + i;
		l = snprintf(buf, sizeof(buf), "%02d %s %hu-%hu %08x/%08x %hu-%hu",
	       i,
	       (perm->type==UPNPPERM_ALLOW)?"allow":"deny",
	       perm->eport_min, perm->eport_max, ntohl(perm->address.s_addr),
	       ntohl(perm->mask.s_addr), perm->iport_min, perm->iport_max);
		if(l<0)
			return;
		write(fd, buf, l);
		if(perm->re)
		{
			write(fd, " ", 1);
			write(fd, perm->re, strlen(perm->re));
		}
		write(fd, "\n", 1);
	}
}
#endif

/* match_permission()
 * returns: 1 if eport, address, iport matches the permission rule
 *          0 if no match */
static int
match_permission(const struct upnpperm * perm,
                 u_short eport, struct in_addr address, u_short iport,
                 const char * desc)
{
	if( (eport < perm->eport_min) || (perm->eport_max < eport))
		return 0;
	if( (iport < perm->iport_min) || (perm->iport_max < iport))
		return 0;
	if( (address.s_addr & perm->mask.s_addr)
	   != (perm->address.s_addr & perm->mask.s_addr) )
		return 0;
#ifdef ENABLE_REGEX
	if(desc && perm->re && regexec(&perm->regex, desc, 0, NULL, 0) == REG_NOMATCH)
		return 0;
#else
	UNUSED(desc);
#endif
	return 1;
}

#if 0
/* match_permission_internal()
 * returns: 1 if address, iport matches the permission rule
 *          0 if no match */
static int
match_permission_internal(const struct upnpperm * perm,
                          struct in_addr address, u_short iport)
{
	if( (iport < perm->iport_min) || (perm->iport_max < iport))
		return 0;
	if( (address.s_addr & perm->mask.s_addr)
	   != (perm->address.s_addr & perm->mask.s_addr) )
		return 0;
	return 1;
}
#endif

int
check_upnp_rule_against_permissions(const struct upnpperm * permary,
                                    int n_perms,
                                    u_short eport, struct in_addr address,
                                    u_short iport, const char * desc)
{
	int i;
	for(i=0; i<n_perms; i++)
	{
		if(match_permission(permary + i, eport, address, iport, desc))
		{
			syslog(LOG_DEBUG,
			       "UPnP permission rule %d matched : port mapping %s",
			       i, (permary[i].type == UPNPPERM_ALLOW)?"accepted":"rejected"
			       );
			return (permary[i].type == UPNPPERM_ALLOW);
		}
	}
	syslog(LOG_DEBUG, "no permission rule matched : accept by default (n_perms=%d)", n_perms);
	return 1;	/* Default : accept */
}

void
get_permitted_ext_ports(uint32_t * allowed,
                        const struct upnpperm * permary, int n_perms,
                        in_addr_t addr, u_short iport)
{
	int i, j;

	/* build allowed external ports array */
	memset(allowed, 0xff, 65536 / 8);	/* everything allowed by default */

	for (i = n_perms - 1; i >= 0; i--)
	{
		if( (addr & permary[i].mask.s_addr)
		  != (permary[i].address.s_addr & permary[i].mask.s_addr) )
			continue;
		if( (iport < permary[i].iport_min) || (permary[i].iport_max < iport))
			continue;
		for (j = (int)permary[i].eport_min ; j <= (int)permary[i].eport_max; )
		{
			if ((j % 32) == 0 && ((int)permary[i].eport_max >= (j + 31)))
			{
				/* 32bits at once */
				allowed[j / 32] = (permary[i].type == UPNPPERM_ALLOW) ? 0xffffffff : 0;
				j += 32;
			}
			else
			{
				do
				{
					/* one bit at once */
					if (permary[i].type == UPNPPERM_ALLOW)
						allowed[j / 32] |= (1 << (j % 32));
					else
						allowed[j / 32] &= ~(1 << (j % 32));
					j++;
				}
				while ((j % 32) != 0 && (j <= (int)permary[i].eport_max));
			}
		}
	}
}
