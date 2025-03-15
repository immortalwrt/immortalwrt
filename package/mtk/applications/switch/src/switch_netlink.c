/*
 * switch_netlink.c: switch(netlink) set API
 *
 * Author: Sirui Zhao <Sirui.Zhao@mediatek.com>
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>

#include "switch_netlink.h"

static struct nl_sock *user_sock;
static struct nl_cache *cache;
static struct genl_family *family;
static struct nlattr *attrs[MT753X_ATTR_TYPE_MAX + 1];

static int wait_handler(struct nl_msg *msg, void *arg)
{
	int *finished = arg;

	*finished = 1;
	return NL_STOP;
}

static int list_swdevs(struct nl_msg *msg, void *arg)
{
	struct mt753x_attr *val = arg;
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));

	if (nla_parse(attrs, MT753X_ATTR_TYPE_MAX, genlmsg_attrdata(gnlh, 0),
		      genlmsg_attrlen(gnlh, 0), NULL) < 0)
		goto done;

	if (gnlh->cmd == MT753X_CMD_REPLY) {
		if (attrs[MT753X_ATTR_TYPE_MESG]) {
			val->dev_info =
				nla_get_string(attrs[MT753X_ATTR_TYPE_MESG]);
			printf("register switch dev:\n%s", val->dev_info);
		}
		else {
			fprintf(stderr, "ERROR:No switch dev now\n");
			goto done;
		}
	} else
		goto done;
	return 0;
done:
	return NL_SKIP;
}

static int construct_attrs(struct nl_msg *msg, void *arg)
{
	struct mt753x_attr *val = arg;
	int type = val->type;

	if (val->dev_id > -1)
		NLA_PUT_U32(msg, MT753X_ATTR_TYPE_DEV_ID, val->dev_id);

	if (val->op == 'r') {
		if (val->phy_dev != -1)
			NLA_PUT_U32(msg, MT753X_ATTR_TYPE_PHY_DEV, val->phy_dev);
		if (val->port_num >= 0)
			NLA_PUT_U32(msg, MT753X_ATTR_TYPE_PHY, val->port_num);
		NLA_PUT_U32(msg, type, val->reg);
	} else if (val->op == 'w') {
		if (val->phy_dev != -1)
			NLA_PUT_U32(msg, MT753X_ATTR_TYPE_PHY_DEV, val->phy_dev);
		if (val->port_num >= 0)
			NLA_PUT_U32(msg, MT753X_ATTR_TYPE_PHY, val->port_num);
		NLA_PUT_U32(msg, type, val->reg);
		NLA_PUT_U32(msg, MT753X_ATTR_TYPE_VAL, val->value);
	} else {
		printf("construct_attrs_message\n");
		NLA_PUT_STRING(msg, type, "hello");
	}
	return 0;

nla_put_failure:
	return -1;
}

static int spilt_attrs(struct nl_msg *msg, void *arg)
{
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct mt753x_attr *val = arg;
	char *str;

	if (nla_parse(attrs, MT753X_ATTR_TYPE_MAX, genlmsg_attrdata(gnlh, 0),
		      genlmsg_attrlen(gnlh, 0), NULL) < 0)
		goto done;

	if ((gnlh->cmd == MT753X_CMD_WRITE) || (gnlh->cmd == MT753X_CMD_READ)) {
		if (attrs[MT753X_ATTR_TYPE_MESG]) {
			str = nla_get_string(attrs[MT753X_ATTR_TYPE_MESG]);
			printf(" %s\n", str);
			if (!strncmp(str, "No", 2))
				goto done;
		}
		if (attrs[MT753X_ATTR_TYPE_REG]) {
			val->reg =
			    nla_get_u32(attrs[MT753X_ATTR_TYPE_REG]);
		}
		if (attrs[MT753X_ATTR_TYPE_VAL]) {
			val->value =
			    nla_get_u32(attrs[MT753X_ATTR_TYPE_VAL]);
		}
	}
	else
		goto done;

	return 0;
done:
	return NL_SKIP;
}

static int mt753x_request_callback(int cmd, int (*spilt)(struct nl_msg *, void *),
				   int (*construct)(struct nl_msg *, void *),
				   void *arg)
{
	struct nl_msg *msg;
	struct nl_cb *callback = NULL;
	int finished;
	int flags = 0;
	int err;

	/*Allocate an netllink message buffer*/
	msg = nlmsg_alloc();
	if (!msg) {
		fprintf(stderr, "Failed to allocate netlink message\n");
		exit(1);
	}
	if (!construct) {
		if (cmd == MT753X_CMD_REQUEST)
			flags |= NLM_F_REQUEST;
		else
			flags |= NLM_F_DUMP;
	}
	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, genl_family_get_id(family),
		    0, flags, cmd, 0);

	/*Fill attaribute of netlink message by construct function*/
	if (construct) {
		err = construct(msg, arg);
		if (err < 0) {
			fprintf(stderr, "attributes error\n");
			goto nal_put_failure;
		}
	}

	/*Allocate an new callback handler*/
	callback = nl_cb_alloc(NL_CB_CUSTOM);
	if (!callback) {
		fprintf(stderr, "Failed to allocate callback handler\n");
		exit(1);
	}

	/*Send netlink message*/
	err = nl_send_auto_complete(user_sock, msg);
	if (err < 0) {
		fprintf(stderr, "nl_send_auto_complete failied:%d\n", err);
		goto out;
	}
	finished = 0;
	if (spilt)
		nl_cb_set(callback, NL_CB_VALID, NL_CB_CUSTOM, spilt, arg);

	if (construct)
		nl_cb_set(callback, NL_CB_ACK, NL_CB_CUSTOM, wait_handler,
			  &finished);
	else
		nl_cb_set(callback, NL_CB_FINISH, NL_CB_CUSTOM, wait_handler,
			  &finished);

	/*receive message from kernel request*/
	err = nl_recvmsgs(user_sock, callback);
	if (err < 0)
		goto out;

	/*wait until an ACK is received for the latest not yet acknowledge*/
	if (!finished)
		err = nl_wait_for_ack(user_sock);
out:
	if (callback)
		nl_cb_put(callback);

nal_put_failure:
	nlmsg_free(msg);
	return err;
}

void mt753x_netlink_free(void)
{
	if (family)
		nl_object_put((struct nl_object *)family);
	if (cache)
		nl_cache_free(cache);
	if (user_sock)
		nl_socket_free(user_sock);
	user_sock = NULL;
	cache = NULL;
	family = NULL;
}

int mt753x_netlink_init(const char *name)
{
	int ret;

	user_sock = NULL;
	cache = NULL;
	family = NULL;

	/*Allocate an new netlink socket*/
	user_sock = nl_socket_alloc();
	if (!user_sock) {
		fprintf(stderr, "Failed to create user socket\n");
		goto err;
	}
	/*Connetct the genl controller*/
	if (genl_connect(user_sock)) {
		fprintf(stderr, "Failed to connetct to generic netlink\n");
		goto err;
	}
	/*Allocate an new nl_cache*/
	ret = genl_ctrl_alloc_cache(user_sock, &cache);
	if (ret < 0) {
		fprintf(stderr, "Failed to allocate netlink cache\n");
		goto err;
	}

	if (name == NULL)
		return -EINVAL;

	/*Look up generic netlik family by "mt753x" in the provided cache*/
	family = genl_ctrl_search_by_name(cache, name);
	if (!family) {
		//fprintf(stderr,"switch(mt753x) API not be prepared\n");
		goto err;
	}
	return 0;
err:
	mt753x_netlink_free();
	return -EINVAL;
}

void mt753x_list_swdev(struct mt753x_attr *arg, int cmd)
{
	int err;

	err = mt753x_request_callback(cmd, list_swdevs, NULL, arg);
	if (err < 0)
		fprintf(stderr, "mt753x list dev error\n");
}

static int mt753x_request(struct mt753x_attr *arg, int cmd)
{
	int err;

	err = mt753x_request_callback(cmd, spilt_attrs, construct_attrs, arg);
	if (err < 0) {
		fprintf(stderr, "mt753x deal request error\n");
		return err;
	}
	return 0;
}

static int phy_operate_netlink(char op, struct mt753x_attr *arg,
			       unsigned int port_num, unsigned int phy_dev,
			       unsigned int offset, unsigned int *value)
{
	int ret = 0;
	struct mt753x_attr *attr = arg;

	attr->port_num = port_num;
	attr->phy_dev = phy_dev;
	attr->reg = offset;
	attr->value = -1;
	attr->type = MT753X_ATTR_TYPE_REG;

	switch (op)
	{
		case 'r':
			attr->op = 'r';
			ret = mt753x_request(attr, MT753X_CMD_READ);
			*value = attr->value;
			break;
		case 'w':
			attr->op = 'w';
			attr->value = *value;
			ret = mt753x_request(attr, MT753X_CMD_WRITE);
			break;
		default:
			break;
	}

	return ret;
}

int reg_read_netlink(struct mt753x_attr *arg, unsigned int offset,
		     unsigned int *value)
{
	int ret;

	ret = phy_operate_netlink('r', arg, -1, -1, offset, value);
	return ret;
}

int reg_write_netlink(struct mt753x_attr *arg, unsigned int offset,
		      unsigned int value)
{
	int ret;

	ret = phy_operate_netlink('w', arg, -1, -1, offset, &value);
	return ret;
}

int phy_cl22_read_netlink(struct mt753x_attr *arg, unsigned int port_num,
			  unsigned int phy_addr, unsigned int *value)
{
	int ret;

	ret = phy_operate_netlink('r', arg, port_num, -1, phy_addr, value);
	return ret;
}

int phy_cl22_write_netlink(struct mt753x_attr *arg, unsigned int port_num,
			   unsigned int phy_addr, unsigned int value)
{
	int ret;

	ret = phy_operate_netlink('w', arg, port_num, -1, phy_addr, &value);
	return ret;
}

int phy_cl45_read_netlink(struct mt753x_attr *arg, unsigned int port_num,
			  unsigned int phy_dev, unsigned int phy_addr,
			  unsigned int *value)
{
	int ret;

	ret = phy_operate_netlink('r', arg, port_num, phy_dev, phy_addr, value);
	return ret;
}

int phy_cl45_write_netlink(struct mt753x_attr *arg, unsigned int port_num,
			   unsigned int phy_dev, unsigned int phy_addr,
			   unsigned int value)
{
	int ret;

	ret = phy_operate_netlink('w', arg, port_num, phy_dev, phy_addr, &value);
	return ret;
}

void dump_extend_phy_reg(struct mt753x_attr *arg, int port_no, int from,
			int to, int is_local, int page_no)
{
        unsigned int temp = 0;
        int r31 = 0;
        int i = 0;

        if (is_local == 0) {
            printf("\n\nGlobal Register Page %d\n",page_no);
            printf("===============");
            r31 |= 0 << 15; //global
            r31 |= ((page_no&0x7) << 12); //page no
            phy_cl22_write_netlink(arg, port_no, 31, r31); //select global page x
            for (i = 16; i < 32; i++) {
                if(i%8 == 0)
                    printf("\n");
		phy_cl22_read_netlink(arg, port_no, i, &temp);
                printf("%02d: %04X ", i, temp);
            }
        } else {
            printf("\n\nLocal Register Port %d Page %d\n",port_no, page_no);
            printf("===============");
            r31 |= 1 << 15; //local
            r31 |= ((page_no&0x7) << 12); //page no
            phy_cl22_write_netlink(arg, port_no, 31, r31); //select global page x
            for (i = 16; i < 32; i++) {
                if (i%8 == 0) {
                    printf("\n");
                }
		phy_cl22_read_netlink(arg, port_no, i, &temp);
                printf("%02d: %04X ",i, temp);
            }
        }
        printf("\n");
}

int phy_dump_netlink(struct mt753x_attr *arg, int phy_addr)
{
	int i;
	int ret;
	unsigned int offset, value;

	if (phy_addr == 32) {
		/*dump all phy register*/
		for (i = 0; i < 5; i++) {
			printf("\n[Port %d]=============", i);
			for (offset = 0; offset < 16; offset++) {
				if (offset % 8 == 0)
					printf("\n");
				ret = phy_cl22_read_netlink(arg, i, offset, &value);
				printf("%02d: %04X ", offset, value);
			}
		}
	} else {
		printf("\n[Port %d]=============", phy_addr);
		for (offset = 0; offset < 16; offset++) {
			if (offset % 8 == 0)
				printf("\n");
			ret = phy_cl22_read_netlink(arg, phy_addr, offset, &value);
			printf("%02d: %04X ", offset, value);
		}
	}
	printf("\n");
	for (offset = 0; offset < 5; offset++) { //global register  page 0~4
		if (phy_addr == 32) //dump all phy register
			dump_extend_phy_reg(arg, 0, 16, 31, 0, offset);
		else
			dump_extend_phy_reg(arg, phy_addr, 16, 31, 0, offset);
	}

	if (phy_addr == 32) {	//dump all phy register
		for (offset = 0; offset < 5; offset++) { //local register port 0-port4
			dump_extend_phy_reg(arg, offset, 16, 31, 1, 0); //dump local page 0
			dump_extend_phy_reg(arg, offset, 16, 31, 1, 1); //dump local page 1
			dump_extend_phy_reg(arg, offset, 16, 31, 1, 2); //dump local page 2
			dump_extend_phy_reg(arg, offset, 16, 31, 1, 3); //dump local page 3
		}
	} else {
		dump_extend_phy_reg(arg, phy_addr, 16, 31, 1, 0); //dump local page 0
		dump_extend_phy_reg(arg, phy_addr, 16, 31, 1, 1); //dump local page 1
		dump_extend_phy_reg(arg, phy_addr, 16, 31, 1, 2); //dump local page 2
		dump_extend_phy_reg(arg, phy_addr, 16, 31, 1, 3); //dump local page 3
	}
	return ret;
}
