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
#else
#define phydev_mdio_bus(_dev) (_dev->mdio.bus)
#define phydev_addr(_dev) (_dev->mdio.addr)
#define phydev_dev(_dev) (&_dev->mdio.dev)
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
#ifdef CONFIG_AIROHA_EN8811H_PHY_DEBUGFS
int airphy_debugfs_init(struct phy_device *phydev);
void airphy_debugfs_remove(struct phy_device *phydev);
#endif /*CONFIG_AIROHA_EN8811H_PHY_DEBUGFS*/
#endif /* End of __EN8811H_API_H */
