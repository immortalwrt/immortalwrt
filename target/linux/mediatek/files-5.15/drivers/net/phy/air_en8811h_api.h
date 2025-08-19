/* SPDX-License-Identifier: GPL-2.0 */
/*************************************************
 * FILE NAME:  air_en8811h_api.h
 * PURPOSE:
 *      EN8811H PHY Driver for Linux
 * NOTES:
 *
 *  Copyright (C) 2023 Airoha Technology Corp.
 *************************************************/
#ifndef __EN8811H_API_H
#define __EN8811H_API_H
#include <linux/version.h>

#if (KERNEL_VERSION(4, 5, 0) > LINUX_VERSION_CODE)
#define phydev_mdio_bus(_dev) (_dev->bus)
#define phydev_addr(_dev) (_dev->addr)
#define phydev_dev(_dev) (&_dev->dev)
#define phydev_kobj(_dev) (&_dev->dev.kobj)
#else
#define phydev_mdio_bus(_dev) (_dev->mdio.bus)
#define phydev_addr(_dev) (_dev->mdio.addr)
#define phydev_dev(_dev) (&_dev->mdio.dev)
#define phydev_kobj(_dev) (&_dev->mdio.dev.kobj)
#endif

#define DEBUGFS_COUNTER		        "counter"
#define DEBUGFS_DRIVER_INFO	        "drvinfo"
#define DEBUGFS_PORT_MODE           "port_mode"
#define DEBUGFS_BUCKPBUS_OP         "buckpbus_op"
#define DEBUGFS_PBUS_OP             "pbus_op"
#define DEBUGFS_POLARITY            "polarity"
#define DEBUGFS_LINK_STATUS         "link_status"
#define DEBUGFS_DBG_REG_SHOW        "dbg_regs_show"
#define DEBUGFS_TEMPERATURE         "temp"
#define DEBUGFS_LP_SPEED            "lp_speed"
#define DEBUGFS_MII_CL22_OP         "cl22_op"
#define DEBUGFS_MII_CL45_OP         "cl45_op"
#define DEBUGFS_CABLE_DIAG          "cable_diag"
#define DEBUGFS_LED_MODE            "led_mode"
#define DEBUGFS_TX_COMP             "tx_comp"

#define CMD_MAX_LENGTH 128

/* bits range: for example AIR_BITS_RANGE(16, 4) = 0x0F0000 */
#ifndef AIR_BITS_RANGE
#define AIR_BITS_RANGE(offset, range) GENMASK((offset) + (range) - 1U, (offset))
#endif /* End of AIR_BITS_RANGE */

/* bits offset right: for example AIR_BITS_OFF_R(0x1234, 8, 4) = 0x2 */
#ifndef AIR_BITS_OFF_R
#define AIR_BITS_OFF_R(val, offset, range) (((val) >> (offset)) & GENMASK((range) - 1U, 0))
#endif /* End of AIR_BITS_OFF_R */

/* bits offset left: for example AIR_BITS_OFF_L(0x1234, 8, 4) = 0x400 */
#ifndef AIR_BITS_OFF_L
#define AIR_BITS_OFF_L(val, offset, range) (((val) & GENMASK((range) - 1U, 0)) << (offset))
#endif /* End of AIR_BITS_OFF_L */

#define AIR_EN8811H_SET_VALUE(__out__, __val__, __offset__, __length__)	\
{							\
	(__out__) &= ~AIR_BITS_RANGE((__offset__), (__length__));			\
	(__out__) |= AIR_BITS_OFF_L((__val__), (__offset__), (__length__));	\
}

#define CTL1000_PORT_TYPE           (0x0400)
#define CTL1000_TEST_NORMAL         (0x0000)
#define CTL1000_TEST_TM1            (0x2000)
#define CTL1000_TEST_TM2            (0x4000)
#define CTL1000_TEST_TM3            (0x6000)
#define CTL1000_TEST_TM4            (0x8000)
#define MMD_DEV_VSPEC1              (0x1e)

enum air_port_mode {
	AIR_PORT_MODE_FORCE_100,
	AIR_PORT_MODE_FORCE_1000,
	AIR_PORT_MODE_FORCE_2500,
	AIR_PORT_MODE_AUTONEGO,
	AIR_PORT_MODE_POWER_DOWN,
	AIR_PORT_MODE_POWER_UP,
	AIR_PORT_MODE_SSC_DISABLE,
	AIR_PORT_MODE_SSC_ENABLE,
	AIR_PORT_MODE_LAST = 0xFF,
};

enum air_polarity {
	AIR_POL_TX_REV_RX_NOR,
	AIR_POL_TX_NOR_RX_NOR,
	AIR_POL_TX_REV_RX_REV,
	AIR_POL_TX_NOR_RX_REV,
	AIR_POL_TX_NOR_RX_LAST = 0xff,
};

/* Link mode bit indices */
enum air_link_mode_bit {
	AIR_LINK_MODE_10baseT_Half_BIT	 = 0,
	AIR_LINK_MODE_10baseT_Full_BIT	 = 1,
	AIR_LINK_MODE_100baseT_Half_BIT	 = 2,
	AIR_LINK_MODE_100baseT_Full_BIT	 = 3,
	AIR_LINK_MODE_1000baseT_Full_BIT = 4,
	AIR_LINK_MODE_2500baseT_Full_BIT = 5,
};

enum air_led_force {
	AIR_LED_NORMAL = 0,
	AIR_LED_FORCE_OFF,
	AIR_LED_FORCE_ON,
	AIR_LED_FORCE_LAST = 0xff,
};

enum air_para {
	AIR_PARA_PRIV,
	AIR_PARA_PHYDEV,
	AIR_PARA_LAST = 0xff
};

enum air_port_cable_status {
	AIR_PORT_CABLE_STATUS_ERROR,
	AIR_PORT_CABLE_STATUS_OPEN,
	AIR_PORT_CABLE_STATUS_SHORT,
	AIR_PORT_CABLE_STATUS_NORMAL,
	AIR_PORT_CABLE_STATUS_LAST = 0xff
};

enum air_surge {
	AIR_SURGE_0R,
	AIR_SURGE_5R,
	AIR_SURGE_LAST = 0xff
};

enum air_port_cable_test_pair {
	AIR_PORT_CABLE_TEST_PAIR_A,
	AIR_PORT_CABLE_TEST_PAIR_B,
	AIR_PORT_CABLE_TEST_PAIR_C,
	AIR_PORT_CABLE_TEST_PAIR_D,
	AIR_PORT_CABLE_TEST_PAIR_ALL,
	AIR_PORT_CABLE_TEST_PAIR_LAST
};

enum air_cko {
	AIR_CKO_DIS,
	AIR_CKO_EN,
	AIR_CKO_LAST = 0xff
};

enum air_tx_comp_mode {
	AIR_TX_COMP_MODE_100M_PAIR_A,
	AIR_TX_COMP_MODE_100M_PAIR_B,
	AIR_TX_COMP_MODE_100M_PAIR_A_DISCRETE,
	AIR_TX_COMP_MODE_100M_PAIR_B_DISCRETE,
	AIR_TX_COMP_MODE_1000M_TM1,
	AIR_TX_COMP_MODE_1000M_TM2,
	AIR_TX_COMP_MODE_1000M_TM3,
	AIR_TX_COMP_MODE_1000M_TM4_TD,
	AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_A,
	AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_B,
	AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_C,
	AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_D,
	AIR_TX_COMP_MODE_2500M_TM1,
	AIR_TX_COMP_MODE_2500M_TM2,
	AIR_TX_COMP_MODE_2500M_TM3,
	AIR_TX_COMP_MODE_2500M_TM4_TONE_1,
	AIR_TX_COMP_MODE_2500M_TM4_TONE_2,
	AIR_TX_COMP_MODE_2500M_TM4_TONE_3,
	AIR_TX_COMP_MODE_2500M_TM4_TONE_4,
	AIR_TX_COMP_MODE_2500M_TM4_TONE_5,
	AIR_TX_COMP_MODE_2500M_TM5,
	AIR_TX_COMP_MODE_2500M_TM6,
	AIR_TX_COMP_MODE_LAST = 0xFF,
};

struct trrg_param_s {
	unsigned int TrRG_LSB     :5;
	unsigned int Reserved_21  :3;
	unsigned int TrRG_MSB     :5;
	unsigned int Reserved_29  :3;
	unsigned int Reserved_0   :1;
	unsigned int DATA_ADDR    :6;
	unsigned int NODE_ADDR    :4;
	unsigned int CH_ADDR      :2;
	unsigned int WR_RD_CTRL   :1;
	unsigned int Reserved_14  :1;
	unsigned int PKT_XMT_STA  :1;
};

union trrgdesc_s {
	struct trrg_param_s param;
	unsigned short     Raw[2];
	unsigned int       DescVal;
};

struct trrg_s {
	union trrgdesc_s TrRGDesc;
	unsigned int     RgMask;
};

struct hal_tr_data_s {
	unsigned short data_lo;
	unsigned char data_hi;
};

#ifndef unlikely
#  define unlikely(x)	(x)
#endif
int air_pbus_reg_write(struct phy_device *phydev,
	unsigned int pbus_address, unsigned int pbus_data);
int air_mii_cl22_write(struct mii_bus *ebus, int addr,
	unsigned int phy_register, unsigned int write_data);
int air_mii_cl22_read(struct mii_bus *ebus,
	int addr, unsigned int phy_register);
int __air_mii_cl45_read(struct phy_device *phydev, int devad, u16 reg);
int __air_mii_cl45_write(struct phy_device *phydev,
	int devad, u16 reg, u16 write_data);
int air_mii_cl45_read(struct phy_device *phydev, int devad, u16 reg);
int air_mii_cl45_write(struct phy_device *phydev,
	int devad, u16 reg, u16 write_data);
unsigned int air_buckpbus_reg_read(struct phy_device *phydev,
	unsigned int pbus_address);
int air_buckpbus_reg_write(struct phy_device *phydev,
	unsigned int pbus_address, unsigned int pbus_data);
int en8811h_of_init(struct phy_device *phydev);
int air_surge_protect_cfg(struct phy_device *phydev);
int air_ref_clk_speed(struct phy_device *phydev, int para);
int air_cko_cfg(struct phy_device *phydev);
int airoha_control_flag(struct phy_device *phydev, int mask, int val);
#ifdef CONFIG_AIROHA_EN8811H_PHY_DEBUGFS
int airphy_debugfs_init(struct phy_device *phydev);
void airphy_debugfs_remove(struct phy_device *phydev);
#endif /*CONFIG_AIROHA_EN8811H_PHY_DEBUGFS*/
#endif /* End of __EN8811H_API_H */
