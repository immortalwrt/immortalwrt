/* $Id: nftnlrdr_misc.h,v 1.11 2024/03/11 23:35:07 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2015 Tomofumi Hayashi
 * (c) 2019 Paul Chambers
 * (c) 2020-2024 Thomas Bernard
 *
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution.
 */
#include <sys/queue.h>

extern const char * nft_table;
extern const char * nft_nat_table;
extern const char * nft_prerouting_chain;
extern const char * nft_postrouting_chain;
extern const char * nft_forward_chain;
extern int nft_nat_family;
extern int nft_ipv4_family;
extern int nft_ipv6_family;

#define NFT_DESCR_SIZE 1024

enum rule_reg_type { 
	RULE_REG_NONE,
	RULE_REG_IIF,
	RULE_REG_OIF,
	RULE_REG_IP_SRC_ADDR,
	RULE_REG_IP_DEST_ADDR,
	RULE_REG_IP_SD_ADDR, /* source & dest */
	RULE_REG_IP6_SRC_ADDR,
	RULE_REG_IP6_DEST_ADDR,
	RULE_REG_IP6_SD_ADDR, /* source & dest */
	RULE_REG_IP_PROTO,
	RULE_REG_IP6_PROTO,
	RULE_REG_TCP_SPORT,
	RULE_REG_TCP_DPORT,
	RULE_REG_TCP_SD_PORT, /* source & dest */
	RULE_REG_IMM_VAL,     /* immediate */
	RULE_REG_MAX,
};

enum rule_type {
	RULE_NONE,
	RULE_NAT,
	RULE_FILTER,
	RULE_COUNTER,
};

enum rule_chain_type {
	RULE_CHAIN_FILTER,
	RULE_CHAIN_PEER,
	RULE_CHAIN_REDIRECT,
};

typedef struct rule_t {
	LIST_ENTRY(rule_t) entry;
	char * table;
	char * chain;
	uint64_t handle;
	enum rule_type type;
	uint32_t nat_type;
	uint32_t filter_action;
	uint32_t family;
	uint32_t ingress_ifidx;
	uint32_t egress_ifidx;
	in_addr_t saddr;
	struct in6_addr saddr6;
	uint16_t sport;
	in_addr_t daddr;
	struct in6_addr daddr6;
	uint16_t dport;
	in_addr_t nat_addr;
	uint16_t nat_port;
	uint8_t proto;
	enum rule_reg_type reg1_type;
	enum rule_reg_type reg2_type;
	uint32_t reg1_val;
	uint32_t reg2_val;
	uint64_t packets;
	uint64_t bytes;
	char * desc;
	uint32_t desc_len;
} rule_t;

LIST_HEAD(rule_list, rule_t);
extern struct rule_list head_filter;
extern struct rule_list head_redirect;
extern struct rule_list head_peer;

/** called at initialization.
 * establishes persistent connection to mnl/netfilter socket, needs elevated privilege */
int
nft_mnl_connect(void);

/** called at shutdown, to release the mnl/netfilter socket */
void
nft_mnl_disconnect(void);

#ifdef DEBUG
void
print_rule(const char *func, int line, const struct nftnl_rule *rule);

void
print_redirect_rules(const char * ifname);

#define debug_rule(rule)		do { print_rule(__func__, __LINE__, rule); } while (0)

#else
#define debug_rule(rule)
#endif

int
nft_send_rule(struct nftnl_rule * rule, uint16_t cmd, enum rule_chain_type type);
struct nftnl_rule *
rule_set_dnat(uint8_t family, const char * ifname, uint8_t proto,
	      in_addr_t rhost, unsigned short eport,
	      in_addr_t ihost, uint32_t iport,
	      const char *descr,
	      const char *handle);
struct nftnl_rule *
rule_set_snat(uint8_t family, uint8_t proto,
	      in_addr_t rhost, unsigned short rport,
	      in_addr_t ehost, unsigned short eport,
	      in_addr_t ihost, unsigned short iport,
	      const char *descr,
	      const char *handle);
struct nftnl_rule *
rule_set_filter(uint8_t family, const char * ifname, uint8_t proto,
		in_addr_t rhost, in_addr_t iaddr,
		unsigned short eport, unsigned short iport,
		unsigned short rport, const char * descr, const char *handle);
struct nftnl_rule *
rule_set_filter6(uint8_t family, const char * ifname, uint8_t proto,
		struct in6_addr *rhost6, struct in6_addr *iaddr6,
		unsigned short eport, unsigned short iport, 
		unsigned short rport, const char *descr, const char *handle);
struct nftnl_rule *
rule_set_filter_common(struct nftnl_rule *r, uint8_t family, const char * ifname,
		uint8_t proto, unsigned short eport, unsigned short iport, 
		unsigned short rport, const char *descr, const char *handle);
struct nftnl_rule *rule_del_handle(rule_t *r);
int refresh_nft_cache_filter(void);
int refresh_nft_cache_redirect(void);
int refresh_nft_cache_peer(void);
int refresh_nft_cache(struct rule_list *head, const char *table, const char *chain, uint32_t family, enum rule_type type);

int
table_op(enum nf_tables_msg_types op, uint16_t family, const char * name);
int
chain_op(enum nf_tables_msg_types op, uint16_t family, const char * table,
         const char * name, const char * type, uint32_t hooknum, signed int priority );

struct mnl_nlmsg_batch *
start_batch( char *buf, size_t buf_size);
int
send_batch(struct mnl_nlmsg_batch * batch);
