/* $Id: nftnlrdr_misc.c,v 1.19 2024/03/11 23:28:21 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2015 Tomofumi Hayashi
 * (c) 2019 Paul Chambers
 * (c) 2019-2024 Thomas Bernard
 *
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <string.h>
#include <stddef.h>
#include <syslog.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <errno.h>

#include <linux/netfilter.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/ipv6.h>

#include <libmnl/libmnl.h>
#include <libnftnl/table.h>
#include <libnftnl/chain.h>
#include <libnftnl/rule.h>
#include <libnftnl/expr.h>

#include "../commonrdr.h"
#include "nftnlrdr_misc.h"
#include "../macros.h"


#ifdef DEBUG
#define d_printf(x) do { printf x; } while (0)
#else
#define d_printf(x)
#endif

#if defined(DEBUG) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) && (__GNUC__ >= 3)
/* disambiguate log messages by adding position in source. GNU C99 or later. Pesky trailing comma... */
#define log_error( msg, ...)	syslog(LOG_ERR, "%s[%d]: " msg, __func__, __LINE__, ##__VA_ARGS__ )
#define log_debug( msg, ...)	syslog(LOG_DEBUG, "%s[%d]: " msg, __func__, __LINE__, ##__VA_ARGS__ )
#else
/* original style */
#define log_error(args...)	syslog(LOG_ERR, args)
#define log_debug(args...)	syslog(LOG_DEBUG, args)
#endif


#define RULE_CACHE_INVALID  0
#define RULE_CACHE_VALID    1

const char * nft_table = "filter";
const char * nft_nat_table = "filter";
const char * nft_prerouting_chain = "prerouting_miniupnpd";
const char * nft_postrouting_chain = "postrouting_miniupnpd";
const char * nft_forward_chain = "miniupnpd";
int nft_nat_family = NFPROTO_INET;
int nft_ipv4_family = NFPROTO_INET;
int nft_ipv6_family = NFPROTO_INET;

static struct mnl_socket *mnl_sock = NULL;
static uint32_t mnl_portid = 0;
static uint32_t mnl_seq = 0;

// FILTER
struct rule_list head_filter = LIST_HEAD_INITIALIZER(head_filter);
// DNAT
struct rule_list head_redirect = LIST_HEAD_INITIALIZER(head_redirect);
// SNAT
struct rule_list head_peer = LIST_HEAD_INITIALIZER(head_peer);

static uint32_t rule_list_filter_validate = RULE_CACHE_INVALID;
static uint32_t rule_list_redirect_validate = RULE_CACHE_INVALID;
static uint32_t rule_list_peer_validate = RULE_CACHE_INVALID;


/*
 * return : 0 for OK, -1 for error
 */
int
nft_mnl_connect(void)
{
	mnl_sock = mnl_socket_open(NETLINK_NETFILTER);
	if (mnl_sock == NULL) {
		log_error("mnl_socket_open() FAILED: %m");
		return -1;
	}
	if (mnl_socket_bind(mnl_sock, 0, MNL_SOCKET_AUTOPID) < 0) {
		log_error("mnl_socket_bind() FAILED: %m");
		return -1;
	}
	mnl_portid = mnl_socket_get_portid(mnl_sock);
	syslog(LOG_INFO, "mnl_socket bound, port_id=%u", mnl_portid);
	return 0;
}

void
nft_mnl_disconnect(void)
{
	if (mnl_sock != NULL) {
		mnl_socket_close(mnl_sock);
		mnl_sock = NULL;
	}
}

#ifdef DEBUG
void
print_rule(const char *func, int line, const struct nftnl_rule *rule)
{
	fprintf(stdout, "%s[%d]: ", func, line);
	nftnl_rule_fprintf(stdout, rule, NFTNL_OUTPUT_DEFAULT, 0);
}

void
print_rule_t(const char *func, int line, const rule_t *r)
{
	fprintf(stdout, "%s[%d]: ", func, line);
	printf("%s %s %d %hu => %hu => %hu\n", r->table, r->chain, (int)r->type,
	       r->sport, r->dport, r->nat_port);
}

/* print out the "filter" and "nat" tables */
void
print_redirect_rules(const char * ifname)
{
	rule_t *p;
	int i;
	UNUSED(ifname);

	refresh_nft_cache_filter();
	i = 1;
	LIST_FOREACH(p, &head_filter, entry) {
		print_rule_t("filter", i++, p);
	}

	refresh_nft_cache_redirect();
	i = 1;
	LIST_FOREACH(p, &head_redirect, entry) {
		print_rule_t("redirect", i++, p);
	}

	refresh_nft_cache_peer();
	i = 1;
	LIST_FOREACH(p, &head_peer, entry) {
		print_rule_t("peer", 0, p);
	}
}
#endif

static enum rule_reg_type *
get_reg_type_ptr(rule_t *r, uint32_t dreg)
{
	switch (dreg) {
	case NFT_REG_1:
		return &r->reg1_type;
	case NFT_REG_2:
		return &r->reg2_type;
	default:
		return NULL;
	}
}

static uint32_t *
get_reg_val_ptr(rule_t *r, uint32_t dreg)
{
	switch (dreg) {
	case NFT_REG_1:
		return &r->reg1_val;
	case NFT_REG_2:
		return &r->reg2_val;
	default:
		return NULL;
	}
}

static void
set_reg (rule_t *r, uint32_t dreg, enum rule_reg_type type, uint32_t val)
{
	if (dreg == NFT_REG_1) {
		r->reg1_type = type;
		if (type == RULE_REG_IMM_VAL) {
			r->reg1_val = val;
		}
	} else if (dreg == NFT_REG_2) {
		r->reg2_type = type;
		if (type == RULE_REG_IMM_VAL) {
			r->reg2_val = val;
		}
	} else if (dreg == NFT_REG_VERDICT) {
		if (r->type == RULE_FILTER) {
			r->filter_action = val;
		}
	} else {
		log_error("unknown reg:%d", dreg);
	}
	return ;
}

static void
parse_rule_immediate(struct nftnl_expr *e, rule_t *r)
{
	uint32_t dreg, reg_val, reg_len;

	dreg = nftnl_expr_get_u32(e, NFTNL_EXPR_IMM_DREG);

	if (dreg == NFT_REG_VERDICT) {
		reg_val = nftnl_expr_get_u32(e, NFTNL_EXPR_IMM_VERDICT);
	} else {
		const void * p = nftnl_expr_get(e, NFTNL_EXPR_IMM_DATA, &reg_len);
		if (p == NULL) {
			log_error("nftnl_expr_get() failed for reg:%u", dreg);
			return;
		} else switch(reg_len) {
			case sizeof(uint32_t):
				reg_val = *(const uint32_t *)p;
				break;
			case sizeof(uint16_t):
				reg_val = *(const uint16_t *)p;
				break;
			default:
				log_error("nftnl_expr_get() reg_len=%u", reg_len);
				return;
		}
	}

	set_reg(r, dreg, RULE_REG_IMM_VAL, reg_val);
}

static void
parse_rule_counter(struct nftnl_expr *e, rule_t *r)
{
	r->type = RULE_COUNTER;
	r->bytes = nftnl_expr_get_u64(e, NFTNL_EXPR_CTR_BYTES);
	r->packets = nftnl_expr_get_u64(e, NFTNL_EXPR_CTR_PACKETS);
}

static void
parse_rule_meta(struct nftnl_expr *e, rule_t *r)
{
	uint32_t key = nftnl_expr_get_u32(e, NFTNL_EXPR_META_KEY);
	uint32_t dreg = nftnl_expr_get_u32(e, NFTNL_EXPR_META_DREG);
	enum rule_reg_type reg_type;

	/* ToDo: body of both cases are identical - bug? */
	switch (key) {
	case NFT_META_IIF:
		reg_type = RULE_REG_IIF;
		set_reg(r, dreg, reg_type, 0);
		break;
	case NFT_META_OIF:
		reg_type = RULE_REG_IIF;
		set_reg(r, dreg, reg_type, 0);
		break;
	default:
		log_debug("parse_rule_meta :Not support key %d\n", key);
		break;
	}
}

static void
parse_rule_nat(struct nftnl_expr *e, rule_t *r)
{
	uint32_t addr_min_reg, addr_max_reg, proto_min_reg, proto_max_reg;
	uint16_t proto_min_val = 0;
	uint32_t * reg_val_ptr;
	r->type = RULE_NAT;

	r->nat_type = nftnl_expr_get_u32(e, NFTNL_EXPR_NAT_TYPE);
	r->family = nftnl_expr_get_u32(e, NFTNL_EXPR_NAT_FAMILY);
	addr_min_reg = nftnl_expr_get_u32(e, NFTNL_EXPR_NAT_REG_ADDR_MIN);
	addr_max_reg = nftnl_expr_get_u32(e, NFTNL_EXPR_NAT_REG_ADDR_MAX);
	/* see expr_add_nat() :
 	 * NFTNL_EXPR_NAT_REG_PROTO_MIN/NFTNL_EXPR_NAT_REG_PROTO_MAX is used
 	 * for destination port */
	proto_min_reg = nftnl_expr_get_u32(e, NFTNL_EXPR_NAT_REG_PROTO_MIN);
	proto_max_reg = nftnl_expr_get_u32(e, NFTNL_EXPR_NAT_REG_PROTO_MAX);

	if (addr_min_reg != addr_max_reg ||
	    proto_min_reg != proto_max_reg) {
		log_error( "Unsupport proto/addr range for NAT");
	}

	reg_val_ptr = get_reg_val_ptr(r, proto_min_reg);
	if (reg_val_ptr != NULL) {
		proto_min_val = htons((uint16_t)*reg_val_ptr);
		syslog(LOG_DEBUG, "%s: proto_min_reg %u : %08x => %hd", "parse_rule_nat", proto_min_reg, *reg_val_ptr, proto_min_val);
	} else {
		syslog(LOG_ERR, "%s: invalid proto_min_reg %u", "parse_rule_nat", proto_min_reg);
	}
	reg_val_ptr = get_reg_val_ptr(r, addr_min_reg);
	if (reg_val_ptr != NULL) {
		/* destination address */
		r->nat_addr = (in_addr_t)*reg_val_ptr;
		/* destination port */
		if (proto_min_reg == NFT_REG_1 || proto_min_reg == NFT_REG_2) {
 		r->nat_port = proto_min_val;
		}
	} else {
		syslog(LOG_ERR, "%s: invalid addr_min_reg %u", "parse_rule_nat", addr_min_reg);
	}

	set_reg(r, NFT_REG_1, RULE_REG_NONE, 0);
	set_reg(r, NFT_REG_2, RULE_REG_NONE, 0);
}

static void
parse_rule_payload(struct nftnl_expr *e, rule_t *r)
{
	uint32_t  base, dreg, offset, len;
	uint32_t  *regptr;

	dreg = nftnl_expr_get_u32(e, NFTNL_EXPR_PAYLOAD_DREG);
	base = nftnl_expr_get_u32(e, NFTNL_EXPR_PAYLOAD_BASE);
	offset = nftnl_expr_get_u32(e, NFTNL_EXPR_PAYLOAD_OFFSET);
	len = nftnl_expr_get_u32(e, NFTNL_EXPR_PAYLOAD_LEN);
	regptr = get_reg_type_ptr(r, dreg);
	if (regptr == NULL) {
		syslog(LOG_INFO, "%s: unsupported dreg %u", "parse_rule_payload", dreg);
		return;
	}

	switch (base) {
	case NFT_PAYLOAD_NETWORK_HEADER:
		if (offset == offsetof(struct iphdr, daddr) &&
		    len == sizeof(in_addr_t)) {
			*regptr = RULE_REG_IP_DEST_ADDR;
		} else if (offset == offsetof(struct iphdr, saddr) &&
			   len == sizeof(in_addr_t)) {
			*regptr = RULE_REG_IP_SRC_ADDR;
		} else if (offset == offsetof(struct iphdr, saddr) &&
			   len == sizeof(in_addr_t) * 2) {
			*regptr = RULE_REG_IP_SD_ADDR;
		} else if (offset == offsetof(struct iphdr, protocol) &&
			   len == sizeof(uint8_t)) {
			*regptr = RULE_REG_IP_PROTO;
		} else if (offset == offsetof(struct ipv6hdr, nexthdr) &&
			   len == sizeof(uint8_t)) {
			*regptr = RULE_REG_IP6_PROTO;
		} else if (offset == offsetof(struct ipv6hdr, daddr) &&
		    len == sizeof(struct in6_addr)) {
			*regptr = RULE_REG_IP6_DEST_ADDR;
		} else if (offset == offsetof(struct ipv6hdr, saddr) &&
			   len == sizeof(struct in6_addr)) {
			*regptr = RULE_REG_IP6_SRC_ADDR;
		} else if (offset == offsetof(struct ipv6hdr, saddr) &&
			   len == sizeof(struct in6_addr) * 2) {
			*regptr = RULE_REG_IP6_SD_ADDR;
		} else {
			syslog(LOG_WARNING,
				   "%s: Unsupported payload: (dreg:%u, base:NETWORK_HEADER, offset:%u, len:%u)",
				   "parse_rule_payload", dreg, offset, len);
		}
		break;
	case NFT_PAYLOAD_TRANSPORT_HEADER:
		/* in both UDP and TCP headers, source port is at offset 0,
		 * destination port at offset 2 */
		if (offset == offsetof(struct tcphdr, dest) &&
		    len == sizeof(uint16_t)) {
			*regptr = RULE_REG_TCP_DPORT;
		} else if (offset == offsetof(struct tcphdr, source) &&
			   len == sizeof(uint16_t)) {
			*regptr = RULE_REG_TCP_SPORT;
		} else if (offset == offsetof(struct tcphdr, source) &&
			   len == sizeof(uint16_t) * 2) {
			*regptr = RULE_REG_TCP_SD_PORT;
		} else {
			syslog(LOG_WARNING,
				   "%s: Unsupported payload: (dreg:%u, base:TRANSPORT_HEADER, offset:%u, len:%u)",
				   "parse_rule_payload", dreg, offset, len);
		}
		break;
	default:
		syslog(LOG_WARNING,
			   "%s: Unsupported payload: (dreg:%u, base:%u, offset:%u, len:%u)",
			   "parse_rule_payload", dreg, base, offset, len);
		break;
	}

}

/*
 *
 * Note: Currently support only NFT_REG_1
 */
static void
parse_rule_cmp(struct nftnl_expr *e, rule_t *r)
{
	uint32_t data_len = 0;
	const void *data_val;
	uint32_t op, sreg;

	op = nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_OP);

	if (op != NFT_CMP_EQ) {
		/* not a cmp expression, so bail out early */
		return;
	}

	sreg = nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_SREG);

	if (sreg != NFT_REG_1) {
		log_error( "parse_rule_cmp: Unsupport reg:%d", sreg);
		return;
	}

	data_val = nftnl_expr_get(e, NFTNL_EXPR_CMP_DATA, &data_len);
	if (data_val == NULL) {
		log_error( "parse_rule_cmp: nftnl_expr_get(NFTNL_EXPR_CMP_DATA) returned NULL");
		return;
	}

	switch (r->reg1_type) {
	case RULE_REG_IIF:
		if (data_len == sizeof(uint32_t)) {
			r->ingress_ifidx = *(const uint32_t *)data_val;
		}
		break;
	case RULE_REG_IP_SRC_ADDR:
		if (data_len == sizeof(in_addr_t)) {
			r->saddr = *(const in_addr_t *)data_val;
		}
		break;
	case RULE_REG_IP6_SRC_ADDR:
		if (data_len == sizeof(struct in6_addr)) {
			r->saddr6 = *(const struct in6_addr *)data_val;
		}
		break;
	case RULE_REG_IP_DEST_ADDR:
		if (data_len == sizeof(in_addr_t)) {
			r->daddr = *(const in_addr_t *)data_val;
		}
		break;
	case RULE_REG_IP6_DEST_ADDR:
		if (data_len == sizeof(struct in6_addr)) {
			r->daddr6 = *(const struct in6_addr *)data_val;
		}
		break;
	case RULE_REG_IP_SD_ADDR:
		if (data_len == sizeof(in_addr_t) * 2) {
			const in_addr_t *addrp = (const in_addr_t *)data_val;
			r->saddr = addrp[0];
			r->daddr = addrp[1];
		}
		break;
	case RULE_REG_IP6_SD_ADDR:
		if (data_len == sizeof(struct in6_addr) * 2) {
			const struct in6_addr *addrp6 = (const struct in6_addr *)data_val;
			r->saddr6 = addrp6[0];
			r->daddr6 = addrp6[1];
		}
		break;
	case RULE_REG_IP_PROTO:
	case RULE_REG_IP6_PROTO:
		if (data_len == sizeof(uint8_t)) {
			r->proto = *(const uint8_t *)data_val;
		}
		break;
	case RULE_REG_TCP_SPORT:
		if (data_len == sizeof(uint16_t)) {
			r->sport = ntohs(*(const uint16_t *)data_val);
		}
		break;
	case RULE_REG_TCP_DPORT:
		if (data_len == sizeof(uint16_t)) {
			r->dport = ntohs(*(const uint16_t *)data_val);
		}
		break;
	case RULE_REG_TCP_SD_PORT:
		if (data_len == sizeof(uint16_t) * 2) {
			const uint16_t * ports = (const uint16_t *)data_val;
			r->sport = ntohs(ports[0]);
			r->dport = ntohs(ports[1]);
		}
		break;
	default:
		log_debug("Unknown cmp (r1type:%d, data_len:%d, op:%d)",
			   r->reg1_type, data_len, op);
		/* return early - don't modify r->reg1_type */
		return;
	}

	r->reg1_type = RULE_REG_NONE;
	return;
}

static int
rule_expr_cb(struct nftnl_expr *e, rule_t *r)
{
	const char *attr_name = nftnl_expr_get_str(e, NFTNL_EXPR_NAME);

	if (strncmp("cmp", attr_name, sizeof("cmp")) == 0) {
		parse_rule_cmp(e, r);
	} else if (strncmp("nat", attr_name, sizeof("nat")) == 0) {
		parse_rule_nat(e, r);
	} else if (strncmp("meta", attr_name, sizeof("meta")) == 0) {
		parse_rule_meta(e, r);
	} else if (strncmp("counter", attr_name, sizeof("counter")) == 0) {
		parse_rule_counter(e, r);
	} else if (strncmp("payload", attr_name, sizeof("payload")) == 0) {
		parse_rule_payload(e, r);
	} else if (strncmp("immediate", attr_name, sizeof("immediate")) == 0) {
		parse_rule_immediate(e, r);
	} else {
		log_debug("unknown attr: %s\n", attr_name);
	}

	return MNL_CB_OK;
}

struct table_cb_data {
	const char * table;
	const char * chain;
	enum rule_type type;
};

/* callback.
 * return values :
 *   MNL_CB_ERROR : an error has occurred. Stop callback runqueue.
 *   MNL_CB_STOP : top callback runqueue.
 *   MNL_CB_OK : no problems has occurred.
 */
static int
table_cb(const struct nlmsghdr *nlh, void *data)
{
	int result = MNL_CB_OK;
	struct nftnl_rule *rule;
	struct nftnl_expr_iter *itr;
#define CB_DATA(field) ((struct table_cb_data *)data)->field

	syslog(LOG_DEBUG, "table_cb(%p, %p) %s %s %d", nlh, data, CB_DATA(table), CB_DATA(chain), CB_DATA(type));
	rule = nftnl_rule_alloc();
	if (rule == NULL) {
		log_error("nftnl_rule_alloc() FAILED");
		return MNL_CB_ERROR;
	}
	if (nftnl_rule_nlmsg_parse(nlh, rule) < 0) {
		log_error("nftnl_rule_nlmsg_parse FAILED");
		result = MNL_CB_ERROR;
	} else {
		rule_t *r = malloc(sizeof(rule_t));
		if (r == NULL) {
			syslog(LOG_ERR, "%s: failed to allocate %u bytes",
			       "table_cb", (unsigned)sizeof(rule_t));
			result = MNL_CB_ERROR;
		} else {
			const char *chain;

			memset(r, 0, sizeof(rule_t));

			chain = nftnl_rule_get_str(rule, NFTNL_RULE_CHAIN);
			if (strcmp(chain, nft_prerouting_chain) == 0 ||
				strcmp(chain, nft_postrouting_chain) == 0 ||
				strcmp(chain, nft_forward_chain) == 0) {
				r->table = strdup(nftnl_rule_get_str(rule, NFTNL_RULE_TABLE));
				r->chain = strdup(chain);
				r->family = nftnl_rule_get_u32(rule, NFTNL_RULE_FAMILY);
				if (nftnl_rule_is_set(rule, NFTNL_RULE_USERDATA)) {
					const char *descr;
					descr = (const char *) nftnl_rule_get_data(rule, NFTNL_RULE_USERDATA,
															 &r->desc_len);
					if (r->desc_len > 0) {
						r->desc = malloc(r->desc_len + 1);
						if (r->desc != NULL) {
							memcpy(r->desc, descr, r->desc_len);
							r->desc[r->desc_len] = '\0';
						} else {
							syslog(LOG_ERR, "failed to allocate %u bytes for desc", r->desc_len);
						}
					}
				}

				r->handle = nftnl_rule_get_u64(rule, NFTNL_RULE_HANDLE);
				r->type = CB_DATA(type);

				itr = nftnl_expr_iter_create(rule);
				if (itr == NULL) {
					syslog(LOG_ERR, "%s: nftnl_expr_iter_create() FAILED",
					       "table_cb");
				} else {
					struct nftnl_expr *expr;

					while ((expr = nftnl_expr_iter_next(itr)) != NULL) {
						rule_expr_cb(expr, r);
					}
					nftnl_expr_iter_destroy(itr);
				}

				switch (r->type) {
				case RULE_NAT:
					switch (r->nat_type) {
					case NFT_NAT_SNAT:
						LIST_INSERT_HEAD(&head_peer, r, entry);
						r = NULL;
						break;
					case NFT_NAT_DNAT:
						LIST_INSERT_HEAD(&head_redirect, r, entry);
						r = NULL;
						break;
					default:
						syslog(LOG_WARNING, "unknown nat type %d", r->nat_type);
					}
					break;

				case RULE_FILTER:
					LIST_INSERT_HEAD(&head_filter, r, entry);
					r = NULL;
					break;

				default:
					syslog(LOG_INFO, "unknown rule type %d", r->type);
					break;
				}
			} else {
				syslog(LOG_WARNING, "unknown chain '%s'", chain);
			}
			if (r != NULL) {
				free(r);
			}
		}
	}
	nftnl_rule_free(rule);
	return result;
}
#undef CB_DATA


int
refresh_nft_cache_filter(void)
{
	if (rule_list_filter_validate != RULE_CACHE_VALID) {
		if (refresh_nft_cache(&head_filter, nft_table, nft_forward_chain, nft_ipv4_family, RULE_FILTER) < 0)
			return -1;
		rule_list_filter_validate = RULE_CACHE_VALID;
	}
	return 0;
}

int
refresh_nft_cache_peer(void)
{
	if (rule_list_peer_validate != RULE_CACHE_VALID) {
		if (refresh_nft_cache(&head_peer, nft_nat_table, nft_postrouting_chain, nft_nat_family, RULE_NAT) < 0)
			return -1;
		rule_list_peer_validate = RULE_CACHE_VALID;
	}
	return 0;
}

int
refresh_nft_cache_redirect(void)
{
	if (rule_list_redirect_validate != RULE_CACHE_VALID) {
		if (refresh_nft_cache(&head_redirect, nft_nat_table, nft_prerouting_chain, nft_nat_family, RULE_NAT) < 0)
			return -1;
		rule_list_redirect_validate = RULE_CACHE_VALID;
	}
	return 0;
}

void
flush_nft_cache(struct rule_list *head)
{
	rule_t *p1, *p2;

	p1 = LIST_FIRST(head);
	while (p1 != NULL) {
		p2 = (rule_t *)LIST_NEXT(p1, entry);
		if (p1->desc != NULL) {
			free(p1->desc);
		}
		if (p1->table != NULL) {
			free(p1->table);
		}
		if (p1->chain != NULL) {
			free(p1->chain);
		}
		free(p1);
		p1 = p2;
	}
	LIST_INIT(head);
}

/*
 * return -1 in case of error, 0 if OK
 */
int
refresh_nft_cache(struct rule_list *head, const char *table, const char *chain, uint32_t family, enum rule_type type)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct table_cb_data data;
	struct nftnl_rule *rule;
	int ret;
	ssize_t n;

	if (mnl_sock == NULL) {
		log_error("netlink not connected");
		return -1;
	}
	flush_nft_cache(head);

	rule = nftnl_rule_alloc();
	if (rule == NULL) {
		log_error("nftnl_rule_alloc() FAILED");
		return -1;
	}

	mnl_seq = time(NULL);
	nlh = nftnl_rule_nlmsg_build_hdr(buf, NFT_MSG_GETRULE, family,
					NLM_F_DUMP, mnl_seq);
	nftnl_rule_set_str(rule, NFTNL_RULE_TABLE, table);
	nftnl_rule_set_str(rule, NFTNL_RULE_CHAIN, chain);
	nftnl_rule_nlmsg_build_payload(nlh, rule);
	nftnl_rule_free(rule);

	if (mnl_socket_sendto(mnl_sock, nlh, nlh->nlmsg_len) < 0) {
		log_error("mnl_socket_sendto() FAILED: %m");
		return -1;
	}

	data.table = table;
	data.chain = chain;
	data.type = type;
	do {
		n = mnl_socket_recvfrom(mnl_sock, buf, sizeof(buf));
		if (n < 0) {
			syslog(LOG_ERR, "%s: mnl_socket_recvfrom: %m",
			       "refresh_nft_cache");
			return -1;
		} else if (n == 0) {
			break;
		}
		ret = mnl_cb_run(buf, n, mnl_seq, mnl_portid, table_cb, &data);
		if (ret <= -1 /*== MNL_CB_ERROR*/) {
			syslog(LOG_ERR, "%s: mnl_cb_run returned %d",
			       "refresh_nft_cache", ret);
			return -1;
		}
	} while(ret >= 1 /*== MNL_CB_OK*/);
	/* ret == MNL_CB_STOP */

	return 0;
}

static void
expr_add_payload(struct nftnl_rule *r, uint32_t base, uint32_t dreg,
                 uint32_t offset, uint32_t len)
{
	struct nftnl_expr *e;

	e = nftnl_expr_alloc("payload");
	if (e == NULL) {
		log_error("nftnl_expr_alloc(\"%s\") FAILED", "payload");
		return;
	}

	nftnl_expr_set_u32(e, NFTNL_EXPR_PAYLOAD_BASE, base);
	nftnl_expr_set_u32(e, NFTNL_EXPR_PAYLOAD_DREG, dreg);
	nftnl_expr_set_u32(e, NFTNL_EXPR_PAYLOAD_OFFSET, offset);
	nftnl_expr_set_u32(e, NFTNL_EXPR_PAYLOAD_LEN, len);

	nftnl_rule_add_expr(r, e);
}

static void
expr_add_cmp(struct nftnl_rule *r, uint32_t sreg, uint32_t op,
	     const void *data, uint32_t data_len)
{
	struct nftnl_expr *e;

	e = nftnl_expr_alloc("cmp");
	if (e == NULL) {
		log_error("nftnl_expr_alloc(\"%s\") FAILED", "cmp");
		return;
	}

	nftnl_expr_set_u32(e, NFTNL_EXPR_CMP_SREG, sreg);
	nftnl_expr_set_u32(e, NFTNL_EXPR_CMP_OP, op);
	nftnl_expr_set(e, NFTNL_EXPR_CMP_DATA, data, data_len);

	nftnl_rule_add_expr(r, e);
}

static void
expr_add_meta(struct nftnl_rule *r, uint32_t meta_key, uint32_t dreg)
{
	struct nftnl_expr *e;

	e = nftnl_expr_alloc("meta");
	if (e == NULL) {
		log_error("nftnl_expr_alloc(\"%s\") FAILED", "meta");
		return;
	}

	nftnl_expr_set_u32(e, NFTNL_EXPR_META_KEY, meta_key);
	nftnl_expr_set_u32(e, NFTNL_EXPR_META_DREG, dreg);

	nftnl_rule_add_expr(r, e);
}

static void
expr_set_reg_val_u32(struct nftnl_rule *r, enum nft_registers dreg, uint32_t val)
{
	struct nftnl_expr *e;
	e = nftnl_expr_alloc("immediate");
	if (e == NULL) {
		log_error("nftnl_expr_alloc(\"%s\") FAILED", "immediate");
		return;
	}
	nftnl_expr_set_u32(e, NFTNL_EXPR_IMM_DREG, dreg);
	nftnl_expr_set_u32(e, NFTNL_EXPR_IMM_DATA, val);
	nftnl_rule_add_expr(r, e);
}

static void
expr_set_reg_val_u16(struct nftnl_rule *r, enum nft_registers dreg, uint16_t val)
{
	struct nftnl_expr *e;
	e = nftnl_expr_alloc("immediate");
	if (e == NULL) {
		log_error("nftnl_expr_alloc(\"%s\") FAILED", "immediate");
		return;
	}
	nftnl_expr_set_u32(e, NFTNL_EXPR_IMM_DREG, dreg);
	nftnl_expr_set_u16(e, NFTNL_EXPR_IMM_DATA, val);
	nftnl_rule_add_expr(r, e);
}

static void
expr_set_reg_verdict(struct nftnl_rule *r, uint32_t val)
{
	struct nftnl_expr *e;
	e = nftnl_expr_alloc("immediate");
	if (e == NULL) {
		log_error("nftnl_expr_alloc(\"%s\") FAILED", "immediate");
		return;
	}
	nftnl_expr_set_u32(e, NFTNL_EXPR_IMM_DREG, NFT_REG_VERDICT);
	nftnl_expr_set_u32(e, NFTNL_EXPR_IMM_VERDICT, val);
	nftnl_rule_add_expr(r, e);
}

static void
expr_add_nat(struct nftnl_rule *r, uint32_t t, uint32_t family,
	     in_addr_t addr_min, uint16_t proto_min, uint32_t flags)
{
	struct nftnl_expr *e;
	UNUSED(flags);

	e = nftnl_expr_alloc("nat");
	if (e == NULL) {
		log_error("nftnl_expr_alloc(\"%s\") FAILED", "nat");
		return;
	}

	nftnl_expr_set_u32(e, NFTNL_EXPR_NAT_TYPE, t);
	nftnl_expr_set_u32(e, NFTNL_EXPR_NAT_FAMILY, family);

	/* To IP Address */
	expr_set_reg_val_u32(r, NFT_REG_1, (uint32_t)addr_min);
	nftnl_expr_set_u32(e, NFTNL_EXPR_NAT_REG_ADDR_MIN, NFT_REG_1);
	nftnl_expr_set_u32(e, NFTNL_EXPR_NAT_REG_ADDR_MAX, NFT_REG_1);
	/* To Port */
	expr_set_reg_val_u16(r, NFT_REG_2, proto_min);
	nftnl_expr_set_u32(e, NFTNL_EXPR_NAT_REG_PROTO_MIN, NFT_REG_2);
	nftnl_expr_set_u32(e, NFTNL_EXPR_NAT_REG_PROTO_MAX, NFT_REG_2);

	nftnl_rule_add_expr(r, e);
}

struct nftnl_rule *
rule_set_snat(uint8_t family, uint8_t proto,
	      in_addr_t rhost, unsigned short rport,
	      in_addr_t ehost, unsigned short eport,
	      in_addr_t ihost, unsigned short iport,
	      const char *descr,
	      const char *handle)
{
	struct nftnl_rule *r = NULL;
	uint16_t dport, sport;
	UNUSED(handle);

	r = nftnl_rule_alloc();
	if (r == NULL) {
		log_error("nftnl_rule_alloc() FAILED");
		return NULL;
	}

	nftnl_rule_set_u32(r, NFTNL_RULE_FAMILY, family);
	nftnl_rule_set_str(r, NFTNL_RULE_TABLE, nft_nat_table);
	nftnl_rule_set_str(r, NFTNL_RULE_CHAIN, nft_postrouting_chain);

	if (descr != NULL && *descr != '\0') {
		nftnl_rule_set_data(r, NFTNL_RULE_USERDATA,
							descr, strlen(descr));
	}

	/* Destination IP */
	expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
	                 offsetof(struct iphdr, daddr), sizeof(uint32_t));
	expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &ihost, sizeof(uint32_t));

	/* Source IP */
	expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
	                 offsetof(struct iphdr, saddr), sizeof(in_addr_t));
	expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &rhost, sizeof(in_addr_t));

	/* Protocol */
	expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
	                 offsetof(struct iphdr, protocol), sizeof(uint8_t));
	expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &proto, sizeof(uint8_t));

	/* Source and Destination Port of Protocol */
	if (proto == IPPROTO_TCP) {
		/* Destination Port */
		dport = htons(iport);
		expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
		                 offsetof(struct tcphdr, dest), sizeof(uint16_t));
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &dport, sizeof(uint16_t));

		/* Source Port */
		sport = htons(rport);
		expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
		                 offsetof(struct tcphdr, source), sizeof(uint16_t));
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &sport, sizeof(uint16_t));
	} else if (proto == IPPROTO_UDP) {
		/* Destination Port */
		dport = htons(iport);
		expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
		                 offsetof(struct udphdr, dest), sizeof(uint16_t));
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &dport, sizeof(uint16_t));

		/* Source Port */
		sport = htons(rport);
		expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
		                 offsetof(struct udphdr, source), sizeof(uint16_t));
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &sport, sizeof(uint16_t));
	}

	expr_add_nat(r, NFT_NAT_SNAT, NFPROTO_IPV4, ehost, htons(eport), 0);

	debug_rule(r);

	return r;
}

struct nftnl_rule *
rule_set_dnat(uint8_t family, const char * ifname, uint8_t proto,
	      in_addr_t rhost, unsigned short eport,
	      in_addr_t ihost, uint32_t iport,
	      const char *descr,
	      const char *handle)
{
	struct nftnl_rule *r = NULL;
	uint16_t dport;
	uint64_t handle_num;
	uint32_t if_idx;

	r = nftnl_rule_alloc();
	if (r == NULL) {
		log_error("nftnl_rule_alloc() FAILED");
		return NULL;
	}

	nftnl_rule_set_u32(r, NFTNL_RULE_FAMILY, family);
	nftnl_rule_set_str(r, NFTNL_RULE_TABLE, nft_nat_table);
	nftnl_rule_set_str(r, NFTNL_RULE_CHAIN, nft_prerouting_chain);

	if (descr != NULL && *descr != '\0') {
		nftnl_rule_set_data(r, NFTNL_RULE_USERDATA,
							descr, strlen(descr));
	}

	if (handle != NULL) {
		handle_num = atoll(handle);
		nftnl_rule_set_u64(r, NFTNL_RULE_POSITION, handle_num);
	}

	if (ifname != NULL) {
		if_idx = (uint32_t)if_nametoindex(ifname);
		expr_add_meta(r, NFT_META_IIF, NFT_REG_1);
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &if_idx,
			     sizeof(uint32_t));
	}

	/* Source IP */
	if (rhost != 0) {
		expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
		                 offsetof(struct iphdr, saddr), sizeof(in_addr_t));
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &rhost, sizeof(in_addr_t));
	}

	/* Protocol */
	expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
	                 offsetof(struct iphdr, protocol), sizeof(uint8_t));
	expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &proto, sizeof(uint8_t));

	if (proto == IPPROTO_TCP) {
		dport = htons(eport);
		expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
		                 offsetof(struct tcphdr, dest), sizeof(uint16_t));
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &dport, sizeof(uint16_t));
	} else if (proto == IPPROTO_UDP) {
		dport = htons(eport);
		expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
		                 offsetof(struct udphdr, dest), sizeof(uint16_t));
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &dport, sizeof(uint16_t));
	}

	expr_add_nat(r, NFT_NAT_DNAT, NFPROTO_IPV4, ihost, htons(iport), 0);

	debug_rule(r);

	return r;
}

struct nftnl_rule *
rule_set_filter(uint8_t family, const char * ifname, uint8_t proto,
		in_addr_t rhost, in_addr_t iaddr,
		unsigned short eport, unsigned short iport,
		unsigned short rport, const char *descr, const char *handle)
{
	struct nftnl_rule *r = NULL;

	r = nftnl_rule_alloc();
	if (r == NULL) {
		log_error("nftnl_rule_alloc() FAILED");
		return NULL;
	}

	r = rule_set_filter_common(r, family, ifname, proto, eport, iport, rport, descr, handle);

	/* Destination IP */
	expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
	                 offsetof(struct iphdr, daddr), sizeof(uint32_t));
	expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &iaddr, sizeof(uint32_t));

	/* Source IP */
	if (rhost != 0) {
		expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
		                 offsetof(struct iphdr, saddr), sizeof(in_addr_t));
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &rhost,
			     sizeof(in_addr_t));
	}

	/* Protocol */
	expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
	                 offsetof(struct iphdr, protocol), sizeof(uint8_t));
	expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &proto, sizeof(uint8_t));

	expr_set_reg_verdict(r, NF_ACCEPT);

	debug_rule(r);

	return r;
}

/*
 * Create the IP6 filter rule
 * called by add_pinhole() and update_pinhole()
 * eport is always 0
 * iport is the destination port of the filter rule
 * rport is the (optional) source port of the rule
 */
struct nftnl_rule *
rule_set_filter6(uint8_t family, const char * ifname, uint8_t proto,
		struct in6_addr *rhost6, struct in6_addr *iaddr6,
		unsigned short eport, unsigned short iport,
		unsigned short rport, const char *descr, const char *handle)
{
	struct nftnl_rule *r = NULL;

	r = nftnl_rule_alloc();
	if (r == NULL) {
		log_error("nftnl_rule_alloc() FAILED");
		return NULL;
	}

	r = rule_set_filter_common(r, family, ifname, proto, eport, iport, rport, descr, handle);

	/* Destination IP */
	expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
	                 offsetof(struct ipv6hdr, daddr), sizeof(struct in6_addr));
	expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, iaddr6, sizeof(struct in6_addr));

	/* Source IP */
	if (rhost6) {
		expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
		                 offsetof(struct ipv6hdr, saddr), sizeof(struct in6_addr));
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, rhost6, sizeof(struct in6_addr));
	}

	/* Protocol */
	expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
	                 offsetof(struct ipv6hdr, nexthdr), sizeof(uint8_t));
	expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &proto, sizeof(uint8_t));

	expr_set_reg_verdict(r, NF_ACCEPT);

	debug_rule(r);

	return r;
}

/*
 * Create common parts for the filter rules (IPv4 or IPv6)
 * eport is ignored
 * iport is the destination port of the filter rule
 * rport is the (optional) source port of the rule
 */
struct nftnl_rule *
rule_set_filter_common(struct nftnl_rule *r, uint8_t family, const char * ifname,
		uint8_t proto, unsigned short eport, unsigned short iport,
		unsigned short rport, const char *descr, const char *handle)
{
	uint16_t dport, sport;
	uint64_t handle_num;
	uint32_t if_idx;
	UNUSED(eport);

	nftnl_rule_set_u32(r, NFTNL_RULE_FAMILY, family);
	nftnl_rule_set_str(r, NFTNL_RULE_TABLE, nft_table);
	nftnl_rule_set_str(r, NFTNL_RULE_CHAIN, nft_forward_chain);

	if (descr != NULL && *descr != '\0') {
		nftnl_rule_set_data(r, NFTNL_RULE_USERDATA,
							descr, strlen(descr));
	}

	if (handle != NULL) {
		handle_num = atoll(handle);
		nftnl_rule_set_u64(r, NFTNL_RULE_POSITION, handle_num);
	}

	if (ifname != NULL) {
		if_idx = (uint32_t)if_nametoindex(ifname);
		expr_add_meta(r, NFT_META_IIF, NFT_REG_1);
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &if_idx,
			     sizeof(uint32_t));
	}

	/* Destination Port */
	dport = htons(iport);
	if (proto == IPPROTO_TCP) {
		expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
		                 offsetof(struct tcphdr, dest), sizeof(uint16_t));
	} else if (proto == IPPROTO_UDP) {
		expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
		                 offsetof(struct udphdr, dest), sizeof(uint16_t));
	}
	expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &dport, sizeof(uint16_t));

	/* Source Port */
	if (rport != 0) {
		sport = htons(rport);
		if (proto == IPPROTO_TCP) {
			expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
			                 offsetof(struct tcphdr, source), sizeof(uint16_t));
		} else if (proto == IPPROTO_UDP) {
			expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
			                 offsetof(struct udphdr, source), sizeof(uint16_t));
		}
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &sport, sizeof(uint16_t));
	}

	return r;
}

struct nftnl_rule *
rule_del_handle(rule_t *rule)
{
	struct nftnl_rule *r = NULL;

	r = nftnl_rule_alloc();
	if (r == NULL) {
		log_error("nftnl_rule_alloc() FAILED");
		return NULL;
	}

	if (rule->type == RULE_NAT) {
		// NAT Family is not chain/rule family
		nftnl_rule_set_u32(r, NFTNL_RULE_FAMILY, nft_nat_family);
	} else {
		nftnl_rule_set_u32(r, NFTNL_RULE_FAMILY, rule->family);
	}

	nftnl_rule_set_str(r, NFTNL_RULE_TABLE, rule->table);
	nftnl_rule_set_str(r, NFTNL_RULE_CHAIN, rule->chain);
	nftnl_rule_set_u64(r, NFTNL_RULE_HANDLE, rule->handle);

	return r;
}

static void
nft_mnl_batch_put(char *buf, uint16_t type, uint32_t seq)
{
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfg;

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = type;
	nlh->nlmsg_flags = NLM_F_REQUEST;
	nlh->nlmsg_seq = seq;

	nfg = mnl_nlmsg_put_extra_header(nlh, sizeof(*nfg));
	nfg->nfgen_family = AF_INET;
	nfg->version = NFNETLINK_V0;
	nfg->res_id = NFNL_SUBSYS_NFTABLES;
}

int
nft_send_rule(struct nftnl_rule * rule, uint16_t cmd, enum rule_chain_type chain_type)
{
	int result = -1;
	struct nlmsghdr *nlh;
	struct mnl_nlmsg_batch *batch;
	char buf[MNL_SOCKET_BUFFER_SIZE*2];

	batch = start_batch(buf, MNL_SOCKET_BUFFER_SIZE);
	if (batch != NULL)
	{
		switch (chain_type) {
			case RULE_CHAIN_FILTER:
				rule_list_filter_validate = RULE_CACHE_INVALID;
				break;
			case RULE_CHAIN_PEER:
				rule_list_peer_validate = RULE_CACHE_INVALID;
				break;
			case RULE_CHAIN_REDIRECT:
				rule_list_redirect_validate = RULE_CACHE_INVALID;
				break;
		}
		nlh = nftnl_rule_nlmsg_build_hdr(mnl_nlmsg_batch_current(batch),
		                                 cmd,
		                                 nftnl_rule_get_u32(rule, NFTNL_RULE_FAMILY),
		                                 NLM_F_APPEND|NLM_F_CREATE|NLM_F_ACK,
		                                 mnl_seq++);

		nftnl_rule_nlmsg_build_payload(nlh, rule);
		nftnl_rule_free(rule);

		result = send_batch(batch);
		if (result < 0) {
			syslog(LOG_ERR, "%s(%p, %d, %d) send_batch failed %d",
			       "nft_send_rule", rule, (int)cmd, (int)chain_type, result);
		}
	}

	return result;
}

int
table_op( enum nf_tables_msg_types op, uint16_t family, const char * name)
{
	int result;
	struct nlmsghdr *nlh;
	struct mnl_nlmsg_batch *batch;
	char buf[MNL_SOCKET_BUFFER_SIZE*2];

	struct nftnl_table *table;

	// log_debug("(%d, %d, %s)", op, family, name);

	table = nftnl_table_alloc();
	if (table == NULL) {
		log_error("out of memory: %m");
		result = -1;
	} else {
		nftnl_table_set_u32(table, NFTNL_TABLE_FAMILY, family);
		nftnl_table_set_str(table, NFTNL_TABLE_NAME, name);

		batch = start_batch(buf, MNL_SOCKET_BUFFER_SIZE);
		if (batch == NULL) {
			log_error("out of memory: %m");
			result = -2;
		} else {
			nlh = nftnl_table_nlmsg_build_hdr(mnl_nlmsg_batch_current(batch),
			                                  op, family,
			                                  (op == NFT_MSG_NEWTABLE ? NLM_F_CREATE : 0) | NLM_F_ACK,
			                                  mnl_seq++);
			nftnl_table_nlmsg_build_payload(nlh, table);

			result = send_batch(batch);
			if (result < 0) {
				syslog(LOG_ERR, "%s(%d, %d, %s) send_batch failed %d",
				       "table_op", (int)op, (int)family, name, result);
			}
		}
		nftnl_table_free(table);
	}
	return result;
}

/*
 * return values :
 *  -2 : out of memory (nftnl_chain_alloc)
 *  -3 : out of memory (start batch)
 *  -4 : failed to build header
 */
int
chain_op(enum nf_tables_msg_types op, uint16_t family, const char * table,
		 const char * name, const char * type, uint32_t hooknum, signed int priority )
{
	int result = -1;
	struct nlmsghdr *nlh;
	struct mnl_nlmsg_batch *batch;
	char buf[MNL_SOCKET_BUFFER_SIZE*2];

	struct nftnl_chain *chain;

	// log_debug("(%d, %d, %s, %s, %s, %d, %d)", op, family, table, name, type, hooknum, priority);

	chain = nftnl_chain_alloc();
	if (chain == NULL) {
		log_error("out of memory: %m");
		result = -2;
	} else {
		nftnl_chain_set_u32(chain, NFTNL_CHAIN_FAMILY, family);
		nftnl_chain_set_str(chain, NFTNL_CHAIN_TABLE, table);
		nftnl_chain_set_str(chain, NFTNL_CHAIN_NAME, name);
		if (op == NFT_MSG_NEWCHAIN) {
			nftnl_chain_set_str(chain, NFTNL_CHAIN_TYPE, type);
			nftnl_chain_set_u32(chain, NFTNL_CHAIN_HOOKNUM, hooknum);
			nftnl_chain_set_s32(chain, NFTNL_CHAIN_PRIO, priority);
		}

		batch = start_batch(buf, MNL_SOCKET_BUFFER_SIZE);
		if (batch == NULL) {
			log_error("out of memory: %m");
			result = -3;
		} else {
			nlh = nftnl_chain_nlmsg_build_hdr(mnl_nlmsg_batch_current(batch),
			                                  op, family,
			                                  (op == NFT_MSG_NEWCHAIN ? NLM_F_CREATE : 0) | NLM_F_ACK,
			                                  mnl_seq++);
			if (nlh == NULL)
			{
				log_error("failed to build header: %m");
				result = -4;
			} else {
				nftnl_chain_nlmsg_build_payload(nlh, chain);

				result = send_batch(batch);
				if (result < 0) {
					syslog(LOG_ERR, "%s(%d, %d, %s, %s, %s, %u, %d) send_batch failed %d",
					       "chain_op", (int)op, (int)family, table, name, type,
						   hooknum, priority, result);
				}
			}
		}
		nftnl_chain_free(chain);
	}
	return result;
}

/**
 * the buffer that you have to use to store the batch must be double
 * of MNL_SOCKET_BUFFER_SIZE
 * @see https://www.netfilter.org/projects/libmnl/doxygen/html/group__batch.html
 */
struct mnl_nlmsg_batch *
start_batch(char *buf, size_t buf_size)
{
	struct mnl_nlmsg_batch *result;
	mnl_seq = time(NULL);

	if (mnl_sock == NULL) {
		log_error("netlink not connected");
		result = NULL;
	} else {
		result = mnl_nlmsg_batch_start(buf, buf_size);
		if (result != NULL) {
			nft_mnl_batch_put(mnl_nlmsg_batch_current(result),
							  NFNL_MSG_BATCH_BEGIN, mnl_seq++);
			mnl_nlmsg_batch_next(result);
		}
	}

	return result;
}

/**
 * return codes :
 * 0  : OK
 * -1 : netlink not connected
 * -2 : mnl_socket_sendto() error
 * -3 : mnl_socket_recvfrom() error
 * -4 : mnl_cb_run() error
 */
int
send_batch(struct mnl_nlmsg_batch *batch)
{
	int ret;
	ssize_t n;
	char buf[MNL_SOCKET_BUFFER_SIZE];

	mnl_nlmsg_batch_next(batch);

	nft_mnl_batch_put(mnl_nlmsg_batch_current(batch), NFNL_MSG_BATCH_END, mnl_seq++);
	mnl_nlmsg_batch_next(batch);

	if (mnl_sock == NULL) {
		log_error("netlink not connected");
		return -1;
	}

	n = mnl_socket_sendto(mnl_sock, mnl_nlmsg_batch_head(batch),
	                      mnl_nlmsg_batch_size(batch));
	if (n == -1) {
		log_error("mnl_socket_sendto() FAILED: %m");
		return -2;
	}
	mnl_nlmsg_batch_stop(batch);

	do {
		n = mnl_socket_recvfrom(mnl_sock, buf, sizeof(buf));
		if (n == -1) {
			log_error("mnl_socket_recvfrom() FAILED: %m");
			return -3;
		} else if (n == 0) {
			break;
		}
		ret = mnl_cb_run(buf, n, 0, mnl_portid, NULL, NULL);
		if (ret <= -1 /*== MNL_CB_ERROR*/) {
			syslog(LOG_ERR, "%s: mnl_cb_run returned %d",
			       "send_batch", ret);
			return -4;
		}
	} while (ret >= 1 /*== MNL_CB_OK*/);
	/* ret == MNL_CB_STOP */
	return 0;
}
