/*
 * switch_ioctl.h: switch(ioctl) set API
 */

#ifndef SWITCH_IOCTL_H
#define SWITCH_IOCTL_H

#define ETH_DEVNAME "eth0"
#define BR_DEVNAME "br-lan"

#define RAETH_MII_READ                  0x89F3
#define RAETH_MII_WRITE                 0x89F4
#define RAETH_ESW_PHY_DUMP              0x89F7

struct esw_reg {
        unsigned int off;
        unsigned int val;
};

struct ra_mii_ioctl_data {
        __u16 phy_id;
        __u16 reg_num;
        __u32 val_in;
        __u32 val_out;
/*
        __u32 port_num;
        __u32 dev_addr;
        __u32 reg_addr;
*/
};

struct ra_switch_ioctl_data {
        unsigned int cmd;
        unsigned int on_off;
        unsigned int port;
        unsigned int bw;
        unsigned int vid;
        unsigned int fid;
        unsigned int port_map;
        unsigned int rx_port_map;
        unsigned int tx_port_map;
        unsigned int igmp_query_interval;
        unsigned int reg_addr;
        unsigned int reg_val;
        unsigned int mode;
        unsigned int qos_queue_num;
        unsigned int qos_type;
        unsigned int qos_pri;
        unsigned int qos_dscp;
        unsigned int qos_table_idx;
        unsigned int qos_weight;
        unsigned char mac[6];
};

extern int chip_name;

int switch_ioctl_init(void);
void switch_ioctl_fini(void);
int reg_read_ioctl(unsigned int offset, unsigned int *value);
int reg_write_ioctl(unsigned int offset, unsigned int value);
int phy_dump_ioctl(unsigned int phy_addr);
int mii_mgr_cl22_read_ioctl(unsigned int port_num, unsigned int reg,
			    unsigned int *value);
int mii_mgr_cl22_write_ioctl(unsigned int port_num, unsigned int reg,
			     unsigned int value);
int mii_mgr_cl45_read_ioctl(unsigned int port_num, unsigned int dev,
			    unsigned int reg, unsigned int *value);
int mii_mgr_cl45_write_ioctl(unsigned int port_num, unsigned int dev,
			     unsigned int reg, unsigned int value);
#endif
