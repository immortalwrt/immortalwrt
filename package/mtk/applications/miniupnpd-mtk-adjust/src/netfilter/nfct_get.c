#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>

#ifdef USE_NFCT
#include <libmnl/libmnl.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

#include <linux/netfilter/nf_conntrack_tcp.h>

struct data_cb_s
{
	struct sockaddr_storage * ext;
	uint8_t found;
};

static int data_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nf_conntrack *ct;
	struct data_cb_s * d = (struct data_cb_s*) data;
	struct sockaddr_in* ext4 = (struct sockaddr_in*) d->ext;

	ct = nfct_new();
	if (ct == NULL)
		return MNL_CB_OK;
	nfct_nlmsg_parse(nlh, ct);

	if (data) {
		ext4->sin_addr.s_addr = nfct_get_attr_u32(ct, ATTR_REPL_IPV4_DST);
		ext4->sin_port =        nfct_get_attr_u16(ct, ATTR_REPL_PORT_DST);
	}
	d->found = 1;
	nfct_destroy(ct);

	return MNL_CB_OK;
}

int get_nat_ext_addr(struct sockaddr* src, struct sockaddr *dst, uint8_t proto,
                     struct sockaddr_storage* ret_ext)
{
	struct mnl_socket *nl;
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfh;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	unsigned int seq, portid;
	struct nf_conntrack *ct;
	int ret;
	struct data_cb_s data;

	if ((!src)&&(!dst)) {
		return 0;
	}

	if (src->sa_family != dst->sa_family) {
		return 0;
	}

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
//		perror("mnl_socket_open");
		goto free_nl;
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
//		perror("mnl_socket_bind");
		goto free_nl;
	}
	portid = mnl_socket_get_portid(nl);

	memset(buf, 0, sizeof(buf));
	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = (NFNL_SUBSYS_CTNETLINK << 8) | IPCTNL_MSG_CT_GET;
	nlh->nlmsg_flags = NLM_F_REQUEST|NLM_F_ACK;
	nlh->nlmsg_seq = seq = time(NULL);

	nfh = mnl_nlmsg_put_extra_header(nlh, sizeof(struct nfgenmsg));
	nfh->nfgen_family = src->sa_family;
	nfh->version = NFNETLINK_V0;
	nfh->res_id = 0;

	ct = nfct_new();
	if (ct == NULL) {
		goto free_nl;
	}

	nfct_set_attr_u8(ct, ATTR_L3PROTO, src->sa_family);
	if (src->sa_family == AF_INET) {
		struct sockaddr_in *src4 = (struct sockaddr_in *)src;
		struct sockaddr_in *dst4 = (struct sockaddr_in *)dst;
		nfct_set_attr_u32(ct, ATTR_IPV4_SRC, src4->sin_addr.s_addr);
		nfct_set_attr_u32(ct, ATTR_IPV4_DST, dst4->sin_addr.s_addr);
		nfct_set_attr_u16(ct, ATTR_PORT_SRC, src4->sin_port);
		nfct_set_attr_u16(ct, ATTR_PORT_DST, dst4->sin_port);
	} else if (src->sa_family == AF_INET6) {
		struct sockaddr_in6 *src6 = (struct sockaddr_in6 *)src;
		struct sockaddr_in6 *dst6 = (struct sockaddr_in6 *)dst;
		nfct_set_attr(ct, ATTR_IPV6_SRC, &src6->sin6_addr);
		nfct_set_attr(ct, ATTR_IPV6_DST, &dst6->sin6_addr);
		nfct_set_attr_u16(ct, ATTR_PORT_SRC, src6->sin6_port);
		nfct_set_attr_u16(ct, ATTR_PORT_DST, dst6->sin6_port);
	}
	nfct_set_attr_u8(ct, ATTR_L4PROTO, proto);

	nfct_nlmsg_build(nlh, ct);

	ret = mnl_socket_sendto(nl, nlh, nlh->nlmsg_len);
	if (ret == -1) {
		goto free_ct;
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	data.ext = ret_ext;
	data.found = 0;
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, seq, portid, data_cb, &data);
		if (ret <= MNL_CB_STOP)
			break;
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}

free_ct:
    nfct_destroy(ct);
free_nl:
    mnl_socket_close(nl);

	return data.found;
}

#else
#define DST "dst="
#define DST_PORT "dport="
#define SRC "src="
#define SRC_PORT "sport="
#define IP_CONNTRACK_LOCATION	"/proc/net/ip_conntrack"
#define NF_CONNTRACK_LOCATION	"/proc/net/nf_conntrack"

int get_nat_ext_addr(struct sockaddr* src, struct sockaddr *dst, uint8_t proto,
                     struct sockaddr_storage* ret_ext)
{
	FILE *f;
	int af;

	if (!src)
		return -2;

	af = src->sa_family;

	if ((f = fopen(NF_CONNTRACK_LOCATION, "r")) == NULL) {
		if ((f = fopen(IP_CONNTRACK_LOCATION, "r")) == NULL) {
			printf("could not read info about connections from the kernel, "
				"make sure netfilter is enabled in kernel or by modules.\n");
			return -1;
		}
	}

	while (!feof(f)) {
		char line[256], *str;
		memset(line, 0, sizeof(line));
		str = fgets(line, sizeof(line), f);
		if (line[0] != 0) {
			char *token, *saveptr;
			int j;
			uint8_t src_f, src_port_f, dst_f, dst_port_f;
			src_f=src_port_f=dst_f=dst_port_f=0;

			for (j = 1; ; j++, str = NULL) {
				token = strtok_r(str, " ", &saveptr);
				if (token == NULL)
					break;

				if ((j==2)&&(af!=atoi(token)))
					break;
				if ((j==4)&&(proto!=atoi(token)))
					break;
				if (j<=4)
					continue;

				if (strncmp(token, SRC, sizeof(SRC) - 1) == 0) {
					char *srcip = token + sizeof(SRC) - 1;
					uint32_t buf[4];
					memset(buf,0,sizeof(buf));

					if (inet_pton(af, srcip, buf)!=1)
						break;

					if (af==AF_INET) {
						struct sockaddr_in *src4=(struct sockaddr_in*)src;
						if (!src_f)  {
							if (src4->sin_addr.s_addr != buf[0])
								break;
							src_f = 1;
						}
					}
				}
				if (strncmp(token, SRC_PORT, sizeof(SRC_PORT) - 1) == 0) {
					char *src_port = token + sizeof(SRC_PORT) - 1;
					uint16_t port=atoi(src_port);

					if (af==AF_INET) {
						struct sockaddr_in *src4=(struct sockaddr_in*)src;
						if (!src_port_f)  {
							if (ntohs(src4->sin_port) != port)
								break;
							src_port_f = 1;
						}
					}
				}

				if (strncmp(token, DST, sizeof(DST) - 1) == 0) {
					char *dstip = token + sizeof(DST) - 1;
					uint32_t buf[4];
					memset(buf,0,sizeof(buf));
					if (inet_pton(af, dstip, buf)!=1)
						break;
					if (af==AF_INET) {
						struct sockaddr_in *dst4=(struct sockaddr_in*)dst;
						if (!dst_f)  {
							if (dst4->sin_addr.s_addr != buf[0])
								break;
							dst_f = 1;
						} else {
							struct sockaddr_in*ret4=(struct sockaddr_in*)ret_ext;
							ret_ext->ss_family = AF_INET;
							ret4->sin_addr.s_addr = buf[0];
						}
					}
				}
				if (strncmp(token, DST_PORT, sizeof(DST_PORT)-1) == 0) {
					char *dst_port = token + sizeof(DST_PORT) - 1;
					uint16_t port=atoi(dst_port);
					if (af==AF_INET) {
						struct sockaddr_in *dst4=(struct sockaddr_in*)dst;
						if (!dst_port_f)  {
							if (ntohs(dst4->sin_port) != port)
								break;
							dst_port_f = 1;
						} else {
							struct sockaddr_in*ret4=(struct sockaddr_in*)ret_ext;
							ret_ext->ss_family = AF_INET;
							ret4->sin_port = htons(port);
						}
					}
				}
			}
			if (src_f && src_port_f && dst_f && dst_port_f) {
				fclose(f);
				return 1;
			}
		}
	}
	fclose(f);

	return 0;
}
#endif
