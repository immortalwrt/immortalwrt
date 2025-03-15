/*
 * switch_netlink.h: switch(netlink) set API
 * 
 * Author: Sirui Zhao <Sirui.Zhao@mediatek.com>
 */
#ifndef MT753X_NETLINK_H
#define MT753X_NETLINK_H

#define MT753X_GENL_NAME "mt753x"
#define MT753X_DSA_GENL_NAME "mt753x_dsa"
#define MT753X_GENL_VERSION 0X1

/*add your cmd to here*/
enum {
	MT753X_CMD_UNSPEC = 0, /*Reserved*/
	MT753X_CMD_REQUEST,    /*user->kernelrequest/get-response*/
	MT753X_CMD_REPLY,      /*kernel->user event*/
	MT753X_CMD_READ,
	MT753X_CMD_WRITE,
	__MT753X_CMD_MAX,
};
#define MT753X_CMD_MAX (__MT753X_CMD_MAX - 1)

/*define attar types */
enum
{
	MT753X_ATTR_TYPE_UNSPEC = 0,
	MT753X_ATTR_TYPE_MESG, /*MT753X message*/
	MT753X_ATTR_TYPE_PHY,
	MT753X_ATTR_TYPE_PHY_DEV,
	MT753X_ATTR_TYPE_REG,
	MT753X_ATTR_TYPE_VAL,
	MT753X_ATTR_TYPE_DEV_NAME,
	MT753X_ATTR_TYPE_DEV_ID,
	__MT753X_ATTR_TYPE_MAX,
};
#define MT753X_ATTR_TYPE_MAX (__MT753X_ATTR_TYPE_MAX - 1)

struct mt753x_attr {
	int port_num;
	int phy_dev;
	int reg;
	int value;
	int type;
	char op;
	char *dev_info;
	int dev_name;
	int dev_id;
};

int mt753x_netlink_init(const char *name);
void mt753x_netlink_free(void);
void mt753x_list_swdev(struct mt753x_attr *arg, int cmd);
int reg_read_netlink(struct mt753x_attr *arg, unsigned int offset,
		     unsigned int *value);
int reg_write_netlink(struct mt753x_attr *arg, unsigned int offset,
		      unsigned int value);
int phy_cl22_read_netlink(struct mt753x_attr *arg, unsigned int port_num,
			  unsigned int phy_addr, unsigned int *value);
int phy_cl22_write_netlink(struct mt753x_attr *arg, unsigned int port_num,
			   unsigned int phy_addr, unsigned int value);
int phy_cl45_read_netlink(struct mt753x_attr *arg, unsigned int port_num,
			  unsigned int phy_dev, unsigned int phy_addr,
			  unsigned int *value);
int phy_cl45_write_netlink(struct mt753x_attr *arg, unsigned int port_num,
			   unsigned int phy_dev, unsigned int phy_addr,
			   unsigned int value);
int phy_dump_netlink(struct mt753x_attr *arg, int phy_addr);

#endif
