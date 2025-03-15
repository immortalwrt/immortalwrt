#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/ethtool.h>
#include <linux/mdio.h>
#include <linux/sockios.h>

#include "mii_mgr.h"

static bool is_silent;

#define INFO(...)			\
do {					\
	if (!is_silent) {		\
		printf(__VA_ARGS__);	\
	}				\
} while (0)

void show_usage(void)
{
	INFO("mii_mgr -g -i [ifname] -p [phy number] -r [register number]\n"
	     "  Get: mii_mgr -g -p 3 -r 4\n\n"
	     "mii_mgr -s -p [phy number] -r [register number] -v [0xvalue]\n"
	     "  Set: mii_mgr -s -p 4 -r 1 -v 0xff11\n"
	     "#NOTE: Without -i , eth0 is default ifname!\n"
	     " -z: silent mode\n"
	     "----------------------------------------------------------------------------------------\n"
	     "Get: mii_mgr_cl45 -g -p [port number] -d [dev number] -r [register number]\n"
	     "Example: mii_mgr_cl45 -g -p 3 -d 0x5 -r 0x4\n\n"
	     "Set: mii_mgr_cl45 -s -p [port number] -d [dev number] -r [register number] -v [value]\n"
	     "Example: mii_mgr_cl45 -s -p 4 -d 0x6 -r 0x1 -v 0xff11\n\n");
}

static void fill_mii_ioctl(struct mii_ioctl_data *mii, uint16_t phy_id,
			   uint16_t reg_num, uint16_t *val)
{
	mii->phy_id  = phy_id;
	mii->reg_num = reg_num;
	mii->val_in  = *val;
	mii->val_out = 0;
}


static void fill_mtk_mii_ioctl(struct mtk_mii_ioctl_data *mtk_mii, uint16_t phy_id,
			      uint16_t reg_num, unsigned int *val)
{
	mtk_mii->phy_id  = phy_id;
	mtk_mii->reg_num = reg_num;
	mtk_mii->val_in  = *val;
	mtk_mii->val_out = 0;
}

static int __phy_op(char *ifname, uint16_t phy_id, uint16_t reg_num, unsigned int *val, uint16_t cmd, int is_priv)
{
	static int sd = -1;

	struct ifreq ifr;
	struct mii_ioctl_data mii;
	struct mtk_mii_ioctl_data mtk_mii;
	int err;

	if (sd < 0)
		sd = socket(AF_INET, SOCK_DGRAM, 0);

	if (sd < 0)
		return sd;

	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);
	ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';

	if (is_priv) {
		fill_mtk_mii_ioctl(&mtk_mii, phy_id, reg_num, val);
		ifr.ifr_data = (char *)&mtk_mii;
	} else {
		fill_mii_ioctl(&mii, phy_id, reg_num, (uint16_t *)val);
		ifr.ifr_data = (char *)&mii;
	}

	err = ioctl(sd, cmd, &ifr);
	if (err)
		return -errno;

	if ((cmd == MTKETH_MII_WRITE) || (cmd == MTKETH_MII_WRITE_CL45) ||
	    (cmd == SIOCSMIIREG))
		*val = (is_priv) ? mtk_mii.val_in : mii.val_in;
	else
		*val = (is_priv) ? mtk_mii.val_out : mii.val_out;

	return 0;
}

int main(int argc, char *argv[])
{
	int opt;
	char options[] = "gsui:p:d:r:v:?t";
	int is_write = 0,is_cl45 = 0;
	int is_priv = 1;
	unsigned int port=0, dev=0,reg_num=0,val=0;
	char ifname[IFNAMSIZ] = "eth0";
	uint16_t phy_id=0;
	uint16_t cmd;

	if (argc < 6) {
		show_usage();
		return 0;
	}

	while ((opt = getopt(argc, argv, options)) != -1) {
		switch (opt) {
			case 'g':
				is_write=0;
				break;
			case 's':
				is_write=1;
				break;
			case 'u':
				is_priv = 0;
				break;
			case 'i':
				strncpy(ifname, optarg, 5);
				ifname[IFNAMSIZ - 1] = '\0';
				break;
			case 'p':
				port = strtoul(optarg, NULL, 16);
				if (port > INT_MAX)
					return -EINVAL;
				break;
			case 'd':
				dev = strtoul(optarg, NULL, 16);
				if (dev > INT_MAX)
					return -EINVAL;
				is_cl45 = 1;
				break;
			case 'r':
				reg_num = strtoul(optarg, NULL, 16);
				if (reg_num > INT_MAX)
					return -EINVAL;
				break;
			case 'v':
				val = strtoul(optarg, NULL, 16);
				if (val > INT_MAX)
					return -EINVAL;
				break;
			case 'z':
				is_silent = true;
				break;
			case '?':
				show_usage();
				break;
		}
	}

	if(is_cl45)
		phy_id = mdio_phy_id_c45(port, dev);
	else
		phy_id = port;

	if (is_write) {
		if (is_priv)
			cmd = (is_cl45) ? MTKETH_MII_WRITE_CL45 :
					  MTKETH_MII_WRITE;
		else
			cmd = SIOCSMIIREG;

		__phy_op(ifname,phy_id,reg_num, &val, cmd, is_priv);

		if(is_cl45)
			INFO("Set: port%x dev%Xh_reg%Xh = 0x%04X\n",
				port, dev, reg_num, val);
		else
			INFO("Set: phy[%x].reg[%x] = %04x\n",
				port, reg_num, val);
	}
	else {
		if (is_priv)
			cmd = (is_cl45) ? MTKETH_MII_READ_CL45 :
					  MTKETH_MII_READ;
		else
			cmd = SIOCGMIIREG;

		__phy_op(ifname,phy_id,reg_num, &val, cmd, is_priv);

		if(is_cl45)
			INFO("Get: port%x dev%Xh_reg%Xh = 0x%04X\n",
				port, dev, reg_num, val);
		else
			INFO("Get: phy[%x].reg[%x] = %04x\n",
				port, reg_num, val);
	}

	return 0;
}
