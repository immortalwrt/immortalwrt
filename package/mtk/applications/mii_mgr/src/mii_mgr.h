/*
 * switch_ioctl.h: switch(ioctl) set API
 */

#ifndef MII_MGR_H
#define MII_MGR_H

#define MTKETH_MII_READ		0x89F3
#define MTKETH_MII_WRITE	0x89F4
#define MTKETH_MII_READ_CL45	0x89FC
#define MTKETH_MII_WRITE_CL45	0x89FD

struct mtk_mii_ioctl_data {
        __u16 phy_id;
        __u16 reg_num;
        __u32 val_in;
        __u32 val_out;
};

#endif /* MII_MGR_H */
