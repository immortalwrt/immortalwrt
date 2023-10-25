// SPDX-License-Identifier: GPL-2.0+
/*************************************************
 * FILE NAME:  air_en8811h_api.c
 * PURPOSE:
 *      EN8811H PHY Driver for Linux
 * NOTES:
 *
 *  Copyright (C) 2023 Airoha Technology Corp.
 *************************************************/

/* INCLUDE FILE DECLARATIONS
*/
#include <linux/uaccess.h>
#include <linux/trace_seq.h>
#include <linux/seq_file.h>
#include <linux/u64_stats_sync.h>
#include <linux/dma-mapping.h>
#include <linux/netdevice.h>
#include <linux/ctype.h>
#include <linux/of_mdio.h>
#include <linux/of_address.h>
#include <linux/mii.h>
#include <linux/phy.h>
#include <linux/debugfs.h>

#include "air_en8811h.h"
#include "air_en8811h_api.h"

/*
struct air_phy_debug {
	struct dentry *root;
};
struct air_phy_debug air_debug;
*/

static const char * const tx_rx_string[32] = {
	"Tx Reverse, Rx Normal",
	"Tx Normal, Rx Normal",
	"Tx Reverse, Rx Reverse",
	"Tx Normal, Rx Reverse",
};

/* Airoha MII read function */
static int __air_mii_cl22_read(struct mii_bus *ebus,
					int addr, unsigned int phy_register)
{
	int read_data;
#if (KERNEL_VERSION(4, 16, 0) < LINUX_VERSION_CODE)
	read_data = __mdiobus_read(ebus, addr, phy_register);
#else
	read_data = ebus->read(ebus, addr, phy_register);
#endif
	return read_data;
}
/* Airoha MII write function */
static int __air_mii_cl22_write(struct mii_bus *ebus, int addr,
			unsigned int phy_register, unsigned int write_data)
{
	int ret = 0;
#if (KERNEL_VERSION(4, 16, 0) < LINUX_VERSION_CODE)
	ret = __mdiobus_write(ebus, addr, phy_register, write_data);
#else
	ret = ebus->write(ebus, addr, phy_register, write_data);
#endif
	return ret;
}

/* Airoha MII read function */
int air_mii_cl22_read(struct mii_bus *ebus, int addr, unsigned int phy_register)
{
	int read_data;

	mutex_lock(&ebus->mdio_lock);
	read_data = __air_mii_cl22_read(ebus, addr, phy_register);
	mutex_unlock(&ebus->mdio_lock);
	return read_data;
}
/* Airoha MII write function */
int air_mii_cl22_write(struct mii_bus *ebus, int addr,
			unsigned int phy_register, unsigned int write_data)
{
	int ret = 0;

	mutex_lock(&ebus->mdio_lock);
	ret = __air_mii_cl22_write(ebus, addr, phy_register, write_data);
	mutex_unlock(&ebus->mdio_lock);
	return ret;
}

static int __air_mii_cl45_read(struct phy_device *phydev, int devad, u16 reg)
{
	int ret = 0;
	int data;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);

	ret |= __air_mii_cl22_write(mbus, addr, MII_MMD_ACC_CTL_REG, devad);
	ret |= __air_mii_cl22_write(mbus, addr, MII_MMD_ADDR_DATA_REG, reg);
	ret |= __air_mii_cl22_write(mbus, addr,
			MII_MMD_ACC_CTL_REG, MMD_OP_MODE_DATA | devad);
	if (ret) {
		dev_err(dev, "__air_mii_cl22_write, ret: %d\n", ret);
		return INVALID_DATA;
	}
	data = __air_mii_cl22_read(mbus, addr, MII_MMD_ADDR_DATA_REG);
	return data;
}

static int __air_mii_cl45_write(struct phy_device *phydev,
					int devad, u16 reg, u16 write_data)
{
	int ret = 0;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);

	ret |= __air_mii_cl22_write(mbus, addr, MII_MMD_ACC_CTL_REG, devad);
	ret |= __air_mii_cl22_write(mbus, addr, MII_MMD_ADDR_DATA_REG, reg);
	ret |= __air_mii_cl22_write(mbus, addr,
		MII_MMD_ACC_CTL_REG, MMD_OP_MODE_DATA | devad);
	ret |= __air_mii_cl22_write(mbus, addr,
			MII_MMD_ADDR_DATA_REG, write_data);
	if (ret) {
		dev_err(dev, "__air_mii_cl22_write, ret: %d\n", ret);
		return ret;
	}
	return 0;
}

int air_mii_cl45_read(struct phy_device *phydev, int devad, u16 reg)
{
	int data;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);

	mutex_lock(&mbus->mdio_lock);
	data = __air_mii_cl45_read(phydev, devad, reg);
	mutex_unlock(&mbus->mdio_lock);
	if (data == INVALID_DATA) {
		dev_err(dev, "__airoha_cl45_read fail\n");
		return INVALID_DATA;
	}
	return data;
}

int air_mii_cl45_write(struct phy_device *phydev,
				int devad, u16 reg, u16 write_data)
{
	int ret = 0;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);

	mutex_lock(&mbus->mdio_lock);
	ret |= __air_mii_cl45_write(phydev, devad, reg, write_data);
	mutex_unlock(&mbus->mdio_lock);
	if (ret) {
		dev_err(dev, "__airoha_cl45_write, ret: %d\n", ret);
		return ret;
	}
	return ret;
}

/* EN8811H PBUS read function */
static unsigned int __air_pbus_reg_read(struct phy_device *phydev,
		unsigned int pbus_address)
{
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);
	struct device *dev = phydev_dev(phydev);
	unsigned int pbus_data_low, pbus_data_high;
	unsigned int pbus_data;
	int ret = 0;

	ret |= __air_mii_cl22_write(mbus, (addr + 8),
			0x1F, (pbus_address >> 6));
	pbus_data_low = __air_mii_cl22_read(mbus, (addr + 8),
				((pbus_address >> 2) & 0xf));
	pbus_data_high = __air_mii_cl22_read(mbus, (addr + 8), 0x10);
	pbus_data = (pbus_data_high << 16) + pbus_data_low;
	if (ret) {
		dev_err(dev, "%s: ret: %d\n", __func__, ret);
		return ret;
	}
	return pbus_data;
}

unsigned int air_pbus_reg_read(struct phy_device *phydev,
		unsigned int pbus_address)
{
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	struct device *dev = phydev_dev(phydev);
	int ret = 0;
	unsigned int data;

	mutex_lock(&mbus->mdio_lock);
	data = __air_pbus_reg_read(phydev, pbus_address);
	mutex_unlock(&mbus->mdio_lock);
	if (ret) {
		dev_err(dev, "%s: ret: %d\n", __func__, ret);
		return ret;
	}
	return data;
}

/* EN8811H PBUS write function */
static int __air_pbus_reg_write(struct phy_device *phydev,
		unsigned int pbus_address, unsigned long pbus_data)
{
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);
	struct device *dev = phydev_dev(phydev);
	int ret = 0;

	ret |= __air_mii_cl22_write(mbus, (addr + 8),
			0x1F, (pbus_address >> 6));
	ret |= __air_mii_cl22_write(mbus, (addr + 8),
			((pbus_address >> 2) & 0xf), (pbus_data & 0xFFFF));
	ret |= __air_mii_cl22_write(mbus, (addr + 8),
			0x10, (pbus_data >> 16));
	if (ret) {
		dev_err(dev, "%s: ret: %d\n", __func__, ret);
		return ret;
	}
	return 0;
}

int air_pbus_reg_write(struct phy_device *phydev,
		unsigned int pbus_address, unsigned int pbus_data)
{
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	struct device *dev = phydev_dev(phydev);
	int ret = 0;

	mutex_lock(&mbus->mdio_lock);
	ret |= __air_pbus_reg_write(phydev, pbus_address, pbus_data);
	mutex_unlock(&mbus->mdio_lock);
	if (ret) {
		dev_err(dev, "%s: ret: %d\n", __func__, ret);
		return ret;
	}
	return 0;
}
/* EN8811H BUCK write function */
static int __air_buckpbus_reg_write(struct phy_device *phydev,
			unsigned int pbus_address, unsigned int pbus_data)
{
	int ret = 0;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);

	/* page 4 */
	ret |= __air_mii_cl22_write(mbus, addr, 0x1F, 4);
	ret |= __air_mii_cl22_write(mbus, addr, 0x10, 0);
	ret |= __air_mii_cl22_write(mbus, addr,
			0x11, ((pbus_address >> 16) & 0xffff));
	ret |= __air_mii_cl22_write(mbus, addr,
			0x12, (pbus_address & 0xffff));
	ret |= __air_mii_cl22_write(mbus, addr,
			0x13, ((pbus_data >> 16) & 0xffff));
	ret |= __air_mii_cl22_write(mbus, addr, 0x14, (pbus_data & 0xffff));
	ret |= __air_mii_cl22_write(mbus, addr, 0x1F, 0);
	if (ret < 0) {
		dev_err(dev, "__air_mii_cl22_write, ret: %d\n", ret);
		return ret;
	}
	return 0;
}

/* EN8811H BUCK read function */
static unsigned int __air_buckpbus_reg_read(struct phy_device *phydev,
			unsigned int pbus_address)
{
	unsigned int pbus_data = 0, pbus_data_low, pbus_data_high;
	int ret = 0;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);

	/* page 4 */
	ret |= __air_mii_cl22_write(mbus, addr, 0x1F, 4);
	ret |= __air_mii_cl22_write(mbus, addr, 0x10, 0);
	ret |= __air_mii_cl22_write(mbus, addr,
			0x15, ((pbus_address >> 16) & 0xffff));
	ret |= __air_mii_cl22_write(mbus, addr,
			0x16, (pbus_address & 0xffff));
	if (ret) {
		dev_err(dev, "__air_mii_cl22_write, ret: %d\n", ret);
		return PBUS_INVALID_DATA;
	}

	pbus_data_high = __air_mii_cl22_read(mbus, addr, 0x17);
	pbus_data_low = __air_mii_cl22_read(mbus, addr, 0x18);
	pbus_data = (pbus_data_high << 16) + pbus_data_low;
	ret |= __air_mii_cl22_write(mbus, addr, 0x1F, 0);
	if (ret) {
		dev_err(dev, "__air_mii_cl22_write, ret: %d\n", ret);
		return ret;
	}
	return pbus_data;
}

unsigned int air_buckpbus_reg_read(struct phy_device *phydev,
			unsigned int pbus_address)
{
	unsigned int data;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);

	mutex_lock(&mbus->mdio_lock);
	data = __air_buckpbus_reg_read(phydev, pbus_address);
	mutex_unlock(&mbus->mdio_lock);
	if (data == INVALID_DATA) {
		dev_err(dev, "__air_buckpbus_reg_read fail\n");
		return INVALID_DATA;
	}
	return data;
}

int air_buckpbus_reg_write(struct phy_device *phydev,
		unsigned int pbus_address, unsigned int pbus_data)
{
	int ret = 0;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);

	mutex_lock(&mbus->mdio_lock);
	ret |= __air_buckpbus_reg_write(phydev, pbus_address, pbus_data);
	mutex_unlock(&mbus->mdio_lock);
	if (ret) {
		dev_err(dev, "__air_buckpbus_reg_write, ret: %d\n", ret);
		return ret;
	}
	return ret;
}

static int air_resolve_an_speed(struct phy_device *phydev)
{
	int lpagb = 0, advgb = 0, common_adv_gb = 0;
	int lpa = 0, adv = 0, common_adv = 0;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);

	dev_dbg(dev, "AN mode!\n");
	dev_dbg(dev, "SPEED 1000/100!\n");
	lpagb = air_mii_cl22_read(mbus,
				addr, MII_STAT1000);
	if (lpagb < 0)
		return lpagb;
	advgb = air_mii_cl22_read(mbus,
				addr, MII_CTRL1000);
	if (adv < 0)
		return adv;
	common_adv_gb = (lpagb & (advgb << 2));

	lpa = air_mii_cl22_read(mbus, addr, MII_LPA);
	if (lpa < 0)
		return lpa;
	adv = air_mii_cl22_read(mbus,
				addr, MII_ADVERTISE);
	if (adv < 0)
		return adv;
	phydev->pause = GET_BIT(adv, 10);
	phydev->asym_pause = GET_BIT(adv, 11);
	common_adv = (lpa & adv);

	phydev->speed = SPEED_UNKNOWN;
	phydev->duplex = DUPLEX_HALF;
	if (common_adv_gb & (LPA_1000FULL | LPA_1000HALF)) {
		phydev->speed = SPEED_1000;
		if (common_adv_gb & LPA_1000FULL)
			phydev->duplex = DUPLEX_FULL;
	} else if (common_adv & (LPA_100FULL | LPA_100HALF)) {
		phydev->speed = SPEED_100;
		if (common_adv & LPA_100FULL)
			phydev->duplex = DUPLEX_FULL;
	} else {
		if (common_adv & LPA_10FULL)
			phydev->duplex = DUPLEX_FULL;
	}
	return 0;
}

int air_get_autonego(struct phy_device *phydev, int *an)
{
	int reg;
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);

	reg = air_mii_cl22_read(mbus, addr, MII_BMCR);
	if (reg < 0)
		return -EINVAL;
	if (reg & BMCR_ANENABLE)
		*an = AUTONEG_ENABLE;
	else
		*an = AUTONEG_DISABLE;
	return 0;
}

static int air_read_status(struct phy_device *phydev)
{
	int ret = 0, reg = 0, an = AUTONEG_DISABLE, bmcr = 0;
	u32 pbus_value = 0;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);

	phydev->speed = SPEED_UNKNOWN;
	phydev->duplex = DUPLEX_UNKNOWN;
	phydev->pause = 0;
	phydev->asym_pause = 0;
	phydev->link = 0;
	phydev->autoneg = AUTONEG_DISABLE;
	reg = air_mii_cl22_read(mbus, addr, MII_BMSR);
	if (reg < 0) {
		dev_err(dev, "MII_BMSR reg %d!\n", reg);
		return reg;
	}
	reg = air_mii_cl22_read(mbus, addr, MII_BMSR);
	if (reg < 0) {
		dev_err(dev, "MII_BMSR reg %d!\n", reg);
		return reg;
	}
	if (reg & BMSR_LSTATUS) {
		phydev->link = 1;
		ret = air_get_autonego(phydev, &an);
		if (ret < 0)
			return ret;
		phydev->autoneg = an;
		pbus_value = air_buckpbus_reg_read(phydev, 0x109D4);
		if (0x10 & pbus_value) {
			phydev->speed = SPEED_2500;
			phydev->duplex = DUPLEX_FULL;
		} else {
			ret = air_get_autonego(phydev, &an);
			if (phydev->autoneg == AUTONEG_ENABLE) {
				ret = air_resolve_an_speed(phydev);
				if (ret < 0)
					return ret;
			} else {
				dev_dbg(dev, "Force mode!\n");
				bmcr = air_mii_cl22_read(mbus, addr, MII_BMCR);

				if (bmcr < 0)
					return bmcr;

				if (bmcr & BMCR_FULLDPLX)
					phydev->duplex = DUPLEX_FULL;
				else
					phydev->duplex = DUPLEX_HALF;

				if (bmcr & BMCR_SPEED1000)
					phydev->speed = SPEED_1000;
				else if (bmcr & BMCR_SPEED100)
					phydev->speed = SPEED_100;
				else
					phydev->speed = SPEED_UNKNOWN;
			}
		}
	}

	return ret;
}
#ifdef CONFIG_AIROHA_EN8811H_PHY_DEBUGFS
static void air_polarity_help(void)
{
	pr_notice("\nUsage:\n"
			"[debugfs] = /sys/kernel/debug/mdio-bus\':[phy_addr]\n"
			"echo [tx polarity] [rx polarity] > /[debugfs]/polarity\n"
			"option: tx_normal, tx_reverse, rx_normal, rx_revers\n");
}

static int air_set_polarity(struct phy_device *phydev, int tx_rx)
{
	int ret = 0;
	unsigned int pbus_data = 0;

	pr_debug("\nPolarit %s\n", tx_rx_string[tx_rx]);
	pbus_data = air_buckpbus_reg_read(phydev, 0xca0f8) & ~(BIT(0) | BIT(1));
	pbus_data |= tx_rx;
	ret = air_buckpbus_reg_write(phydev, 0xca0f8, pbus_data);
	if (ret < 0)
		pr_notice("\n%s:air_buckpbus_reg_write fail\n", __func__);
	pbus_data = air_buckpbus_reg_read(phydev, 0xca0f8);
	pr_notice("\nPolarity %s confirm....(%02lx)\n",
			tx_rx_string[tx_rx], pbus_data & (BIT(0) | BIT(1)));

	return ret;
}

static int air_set_mode(struct phy_device *phydev, int dbg_mode)
{
	int ret = 0, val = 0;
	unsigned int pbus_data = 0;
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);

	switch (dbg_mode) {
	case AIR_PORT_MODE_FORCE_100:
		pr_notice("\nForce 100M\n");
		val = air_mii_cl22_read(mbus, addr, MII_ADVERTISE) | BIT(8);
		ret = air_mii_cl22_write(mbus, addr, MII_ADVERTISE, val);
		if (unlikely(ret < 0))
			break;
		val = air_mii_cl22_read(mbus, addr, MII_CTRL1000) & ~BIT(9);
		ret = air_mii_cl22_write(mbus, addr, MII_CTRL1000, val);
		if (unlikely(ret < 0))
			break;
		val = air_mii_cl45_read(phydev, 0x7, 0x20) & ~BIT(7);
		ret = air_mii_cl45_write(phydev, 0x7, 0x20, val);
		if (unlikely(ret < 0))
			break;
		val = air_mii_cl22_read(mbus, addr, MII_BMCR) | BIT(9);
		ret = air_mii_cl22_write(mbus, addr, MII_BMCR, val);
		if (unlikely(ret < 0))
			break;
		break;
	case AIR_PORT_MODE_FORCE_1000:
		pr_notice("\nForce 1000M\n");
		val = air_mii_cl22_read(mbus, addr, MII_ADVERTISE) & ~BIT(8);
		ret = air_mii_cl22_write(mbus, addr, MII_ADVERTISE, val);
		if (unlikely(ret < 0))
			break;
		val = air_mii_cl22_read(mbus, addr, MII_CTRL1000) | BIT(9);
		ret = air_mii_cl22_write(mbus, addr, MII_CTRL1000, val);
		if (unlikely(ret < 0))
			break;
		val = air_mii_cl45_read(phydev, 0x7, 0x20) & ~BIT(7);
		ret = air_mii_cl45_write(phydev, 0x7, 0x20, val);
		if (unlikely(ret < 0))
			break;
		val = air_mii_cl22_read(mbus, addr, MII_BMCR) | BIT(9);
		ret = air_mii_cl22_write(mbus, addr, MII_BMCR, val);
		if (unlikely(ret < 0))
			break;
		break;
	case AIR_PORT_MODE_FORCE_2500:
		pr_notice("\nForce 2500M\n");
		val = air_mii_cl22_read(mbus, addr, MII_ADVERTISE) & ~BIT(8);
		ret = air_mii_cl22_write(mbus, addr, MII_ADVERTISE, val);
		if (unlikely(ret < 0))
			break;
		val = air_mii_cl22_read(mbus, addr, MII_CTRL1000) & ~BIT(9);
		ret = air_mii_cl22_write(mbus, addr, MII_CTRL1000, val);
		if (unlikely(ret < 0))
			break;
		val = air_mii_cl45_read(phydev, 0x7, 0x20) | BIT(7);
		ret = air_mii_cl45_write(phydev, 0x7, 0x20, val);
		if (unlikely(ret < 0))
			break;
		val = air_mii_cl22_read(mbus, addr, MII_BMCR) | BIT(9);
		ret = air_mii_cl22_write(mbus, addr, MII_BMCR, val);
		if (unlikely(ret < 0))
			break;
		break;
	case AIR_PORT_MODE_AUTONEGO:
		pr_notice("\nAutonego mode\n");
		val = air_mii_cl22_read(mbus, addr, MII_ADVERTISE) | BIT(8);
		ret = air_mii_cl22_write(mbus, addr, MII_ADVERTISE, val);
		if (unlikely(ret < 0))
			break;
		val = air_mii_cl22_read(mbus, addr, MII_CTRL1000) | BIT(9);
		ret = air_mii_cl22_write(mbus, addr, MII_CTRL1000, val);
		if (unlikely(ret < 0))
			break;
		val = air_mii_cl45_read(phydev, 0x7, 0x20) | BIT(7);
		ret = air_mii_cl45_write(phydev, 0x7, 0x20, val);
		if (unlikely(ret < 0))
			break;
		val = air_mii_cl22_read(mbus, addr, MII_BMCR) | BIT(9);
		ret = air_mii_cl22_write(mbus, addr, MII_BMCR, val);
		if (unlikely(ret < 0))
			break;
		break;
	case AIR_PORT_MODE_POWER_DOWN:
		pr_notice("\nPower Down\n");
		val = air_mii_cl22_read(mbus, addr, MII_BMCR) | BIT(11);
		ret = air_mii_cl22_write(mbus, addr, MII_BMCR, val);
		if (unlikely(ret < 0))
			break;
		break;
	case AIR_PORT_MODE_POWER_UP:
		pr_notice("\nPower Up\n");
		val = air_mii_cl22_read(mbus, addr, MII_BMCR) & ~BIT(11);
		ret = air_mii_cl22_write(mbus, addr, MII_BMCR, val);
		if (unlikely(ret < 0))
			break;
		break;
	case AIR_PORT_MODE_FC_DIS:
		pr_notice("\nFlow Control Disabled\n");
		pbus_data = air_buckpbus_reg_read(phydev, 0xe0008);
		pbus_data &= ~(BIT(6) | BIT(7));
		ret = air_buckpbus_reg_write(phydev, 0xe0008, pbus_data);
		if (unlikely(ret < 0))
			break;
		pbus_data = air_buckpbus_reg_read(phydev, 0xe000c);
		pbus_data &= ~(BIT(0) | BIT(1));
		ret = air_buckpbus_reg_write(phydev, 0xe000c, pbus_data);
		if (unlikely(ret < 0))
			break;
		break;
	case AIR_PORT_MODE_FC_EN:
		pr_notice("\nFlow Control Enabled\n");
		pbus_data = air_buckpbus_reg_read(phydev, 0xe0008);
		pbus_data |= (BIT(6) | BIT(7));
		ret = air_buckpbus_reg_write(phydev, 0xe0008, pbus_data);
		if (unlikely(ret < 0))
			break;
		pbus_data = air_buckpbus_reg_read(phydev, 0xe000c);
		pbus_data |= (BIT(0) | BIT(1));
		ret = air_buckpbus_reg_write(phydev, 0xe000c, pbus_data);
		if (unlikely(ret < 0))
			break;
		break;
	default:
		pr_notice("\nWrong Port mode\n");
		break;
	}
	return ret;
}

static int airphy_info_show(struct seq_file *seq, void *v)
{
	struct phy_device *phydev = seq->private;
	struct en8811h_priv *priv = phydev->priv;
	unsigned int val = 0;

	seq_puts(seq, "<<AIR EN8811H Driver info>>\n");
	seq_printf(seq, "| Driver Version       : %s\n",
							EN8811H_DRIVER_VERSION);
	val = air_buckpbus_reg_read(phydev, 0xcf914);
	seq_printf(seq, "| Boot mode            : %s\n",
		((val & BIT(24)) >> 24) ? "Flash" : "Download Code");
	seq_printf(seq, "| EthMD32.dm.bin  CRC32: %08x\n",
				priv->dm_crc32);
	seq_printf(seq, "| EthMD32.DSP.bin CRC32: %08x\n",
				priv->dsp_crc32);
	val = air_buckpbus_reg_read(phydev, 0x3b3c);
	seq_printf(seq, "| MD32 FW Version      : %08x\n", val);
	val = air_mii_cl45_read(phydev, 0x1e, 0x8009);
	seq_printf(seq, "| MD32 FW Status       : %08x\n",
				air_mii_cl45_read(phydev, 0x1e, 0x8009));
	val = (air_buckpbus_reg_read(phydev, 0xca0f8) & 0x3);
	seq_printf(seq, "| Tx, Rx Polarity      : %s(%02d)\n",
						tx_rx_string[val], val);

	seq_puts(seq, "\n");

	return 0;
}

static int airphy_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, airphy_info_show, inode->i_private);
}

static int airphy_fcm_counter_show(struct phy_device *phydev,
				struct seq_file *seq)
{
	int ret = 0;
	u32 pkt_cnt = 0;

	seq_puts(seq, "|\t<<FCM Counter>>\n");
	seq_puts(seq, "| Rx from Line side_S      :");
	pkt_cnt = air_buckpbus_reg_read(phydev, 0xe0090);
	seq_printf(seq, "%010u |\n", pkt_cnt);
	seq_puts(seq, "| Rx from Line side_T      :");
	pkt_cnt = air_buckpbus_reg_read(phydev, 0xe0094);
	seq_printf(seq, "%010u |\n", pkt_cnt);
	seq_puts(seq, "| Tx to System side_S      :");
	pkt_cnt = air_buckpbus_reg_read(phydev, 0xe009c);
	seq_printf(seq, "%010u |\n", pkt_cnt);
	seq_puts(seq, "| Tx to System side_T      :");
	pkt_cnt = air_buckpbus_reg_read(phydev, 0xe00A0);
	seq_printf(seq, "%010u |\n", pkt_cnt);
	seq_puts(seq, "| Rx from System side_S    :");
	pkt_cnt = air_buckpbus_reg_read(phydev, 0xe0078);
	seq_printf(seq, "%010u |\n", pkt_cnt);
	seq_puts(seq, "| Rx from System side_T    :");
	pkt_cnt = air_buckpbus_reg_read(phydev, 0xe007C);
	seq_printf(seq, "%010u |\n", pkt_cnt);
	seq_puts(seq, "| Tx to Line side_S        :");
	pkt_cnt = air_buckpbus_reg_read(phydev, 0xe0084);
	seq_printf(seq, "%010u |\n", pkt_cnt);
	seq_puts(seq, "| Tx to Line side_T        :");
	pkt_cnt = air_buckpbus_reg_read(phydev, 0xe0088);
	seq_printf(seq, "%010u |\n", pkt_cnt);
	ret = air_buckpbus_reg_write(phydev, 0xe0074, 0xf);
	if (ret < 0)
		return ret;
	return 0;
}

static int airphy_ss_counter_show(struct phy_device *phydev,
				struct seq_file *seq)
{
	int ret = 0;
	u32 pkt_cnt = 0;

	seq_puts(seq, "|\t<<SS Counter>>\n");
	ret = air_buckpbus_reg_write(phydev, 0xC602C, 0x3);
	if (ret < 0)
		return ret;
	seq_puts(seq, "| TX Start                 :");
	pkt_cnt = air_buckpbus_reg_read(phydev, 0xC60B0);
	seq_printf(seq, "%010u |\n", pkt_cnt);
	seq_puts(seq, "| TX Terminal              :");
	pkt_cnt = air_buckpbus_reg_read(phydev, 0xC60B4);
	seq_printf(seq, "%010u |\n", pkt_cnt);
	seq_puts(seq, "| RX Start                 :");
	pkt_cnt = air_buckpbus_reg_read(phydev, 0xC60BC);
	seq_printf(seq, "%010u |\n", pkt_cnt);
	seq_puts(seq, "| RX Terminal              :");
	pkt_cnt = air_buckpbus_reg_read(phydev, 0xC60C0);
	seq_printf(seq, "%010u |\n\n", pkt_cnt);
	ret = air_buckpbus_reg_write(phydev, 0xC602C, 0x4);
	if (ret < 0)
		return ret;
	return 0;
}

static int airphy_counter_show(struct seq_file *seq, void *v)
{
	struct phy_device *phydev = seq->private;
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int ret = 0, addr = phydev_addr(phydev);
	u32 pkt_cnt = 0;

	ret = air_read_status(phydev);
	if (ret < 0)
		return ret;
	seq_puts(seq, "==========AIR PHY COUNTER==========\n");
	ret = airphy_fcm_counter_show(phydev, seq);
	if (ret < 0)
		return ret;
	if (phydev->link && phydev->speed == SPEED_2500) {
		seq_puts(seq, "|\t<<LS Counter>>\n");
		ret = air_buckpbus_reg_write(phydev, 0x30718, 0x10);
		if (ret < 0)
			return ret;
		ret = air_buckpbus_reg_write(phydev, 0x30718, 0x0);
		if (ret < 0)
			return ret;
		seq_puts(seq, "|\tBefore EF\n");
		seq_puts(seq, "| TX to Line side_S        :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x3071c);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| TX to Line side_T        :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x30720);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| RX from Line side_S      :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x3072c);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| RX from Line side_T      :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x30730);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| TX_ENC                   :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x30724);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| RX_DEC                   :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x30728);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "|\tAfter EF\n");
		seq_puts(seq, "| TX to Line side_S        :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x30734);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| TX to Line side_T        :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x30738);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| RX from Line side_S      :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x30764);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| RX from Line side_T      :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x30768);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		ret = air_buckpbus_reg_write(phydev, 0x30718, 0x13);
		if (ret < 0)
			return ret;
		ret = air_buckpbus_reg_write(phydev, 0x30718, 0x3);
		if (ret < 0)
			return ret;
		ret = air_buckpbus_reg_write(phydev, 0x30718, 0x10);
		if (ret < 0)
			return ret;
		ret = air_buckpbus_reg_write(phydev, 0x30718, 0x0);
		if (ret < 0)
			return ret;
		seq_puts(seq, "|\t<<MAC Counter>>\n");
		seq_puts(seq, "| TX Error from System side:");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x131000);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| RX Error to System side  :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x132000);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| TX from System side      :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x131004);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| RX to System Side        :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x132004);
		seq_printf(seq, "%010u |\n", pkt_cnt);
	}
	if (phydev->link && ((phydev->speed != SPEED_2500))) {
		seq_puts(seq, "|\t<<LS Counter>>\n");
		ret = air_mii_cl22_write(mbus, addr, 0x1f, 1);
		if (ret < 0)
			return ret;
		seq_puts(seq, "| RX from Line side        :");
		pkt_cnt = air_mii_cl22_read(mbus, addr, 0x12) & 0x7fff;
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| RX Error from Line side  :");
		pkt_cnt = air_mii_cl22_read(mbus, addr, 0x17) & 0xff;
		seq_printf(seq, "%010u |\n", pkt_cnt);
		ret = air_mii_cl22_write(mbus, addr, 0x1f, 0);
		if (ret < 0)
			return ret;
		ret = air_mii_cl22_write(mbus, addr, 0x1f, 0x52B5);
		if (ret < 0)
			return ret;
		ret = air_mii_cl22_write(mbus, addr, 0x10, 0xBF92);
		if (ret < 0)
			return ret;
		seq_puts(seq, "| TX to Line side          :");
		pkt_cnt = (air_mii_cl22_read(mbus, addr, 0x11) & 0x7ffe) >> 1;
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| TX Error to Line side    :");
		pkt_cnt = air_mii_cl22_read(mbus, addr, 0x12);
		pkt_cnt &= 0x7f;
		seq_printf(seq, "%010u |\n", pkt_cnt);
		ret = air_mii_cl22_write(mbus, addr, 0x1f, 0);
		if (ret < 0)
			return ret;
	}
	if (phydev->link) {
		ret = airphy_ss_counter_show(phydev, seq);
		if (ret < 0)
			return ret;
	}
	return ret;
}

static int airphy_counter_open(struct inode *inode, struct file *file)
{
	return single_open(file, airphy_counter_show, inode->i_private);
}

static ssize_t airphy_polarity_write(struct file *file, const char __user *ptr,
					size_t len, loff_t *off)
{
	struct phy_device *phydev = file->private_data;
	char buf[32], param1[32], param2[32];
	int count = len, ret = 0, tx_rx = 0;

	memset(buf, 0, 32);
	memset(param1, 0, 32);
	memset(param2, 0, 32);

	if (count > sizeof(buf) - 1)
		return -EINVAL;
	if (copy_from_user(buf, ptr, len))
		return -EFAULT;
	if (sscanf(buf, "%s %s", param1, param2) == -1)
		return -EFAULT;

	if (!strncmp("tx_normal", param1, strlen("tx_normal"))) {
		if (!strncmp("rx_normal", param2, strlen("rx_normal")))
			tx_rx = AIR_POL_TX_NOR_RX_NOR;
		else if (!strncmp("rx_reverse", param2, strlen("rx_reverse")))
			tx_rx = AIR_POL_TX_NOR_RX_REV;
		else {
			pr_notice("\nRx param is not correct.\n");
			return -EINVAL;
		}
	} else if (!strncmp("tx_reverse", param1, strlen("tx_reverse"))) {
		if (!strncmp("rx_normal", param2, strlen("rx_normal")))
			tx_rx = AIR_POL_TX_REV_RX_NOR;
		else if (!strncmp("rx_reverse", param2, strlen("rx_reverse")))
			tx_rx = AIR_POL_TX_REV_RX_REV;
		else {
			pr_notice("\nRx param is not correct.\n");
			return -EINVAL;
		}
	} else {
		air_polarity_help();
		return count;
	}
	pr_notice("\nSet Polarity %s\n", tx_rx_string[tx_rx]);
	ret = air_set_polarity(phydev, tx_rx);
	if (ret < 0)
		return ret;
	return count;
}
static void airphy_port_mode_help(void)
{
	pr_notice("\nUsage:\n"
			"[debugfs] = /sys/kernel/debug/mdio-bus\':[phy_addr]\n"
			"echo [mode] [para] > /[debugfs]/port_mode\n"
			"echo re-an > /[debugfs]/port_mode\n"
			"echo auto > /[debugfs]/port_mode\n"
			"echo 2500 > /[debugfs]/port_mode\n"
			"echo 1000 > /[debugfs]/port_mode\n"
			"echo 100 > /[debugfs]/port_mode\n"
			"echo power up/down >  /[debugfs]/port_mode\n");
}

static ssize_t airphy_port_mode(struct file *file, const char __user *ptr,
					size_t len, loff_t *off)
{
	struct phy_device *phydev = file->private_data;
	char buf[32], cmd[32], param[32];
	int count = len, ret = 0;
	int num = 0, val = 0;

	memset(buf, 0, 32);
	memset(cmd, 0, 32);
	memset(param, 0, 32);

	if (count > sizeof(buf) - 1)
		return -EINVAL;
	if (copy_from_user(buf, ptr, len))
		return -EFAULT;

	num = sscanf(buf, "%s %s", cmd, param);
	if (num < 1 || num > 3)
		return -EFAULT;

	if (!strncmp("auto", cmd, strlen("auto")))
		ret = air_set_mode(phydev, AIR_PORT_MODE_AUTONEGO);
	else if (!strncmp("2500", cmd, strlen("2500")))
		ret = air_set_mode(phydev, AIR_PORT_MODE_FORCE_2500);
	else if (!strncmp("1000", cmd, strlen("1000")))
		ret = air_set_mode(phydev, AIR_PORT_MODE_FORCE_1000);
	else if (!strncmp("100", cmd, strlen("100")))
		ret = air_set_mode(phydev, AIR_PORT_MODE_FORCE_100);
	else if (!strncmp("re-an", cmd, strlen("re-an"))) {
		val = phy_read(phydev, MII_BMCR) | BIT(9);
		ret = phy_write(phydev, MII_BMCR, val);
	} else if (!strncmp("power", cmd, strlen("power"))) {
		if (!strncmp("down", param, strlen("down")))
			ret = air_set_mode(phydev, AIR_PORT_MODE_POWER_DOWN);
		else if (!strncmp("up", param, strlen("up")))
			ret = air_set_mode(phydev, AIR_PORT_MODE_POWER_UP);
	} else if (!strncmp("int_sw_release", cmd, strlen("int_sw_release"))) {
		ret = air_pbus_reg_write(phydev, 0xcf928, 0x0);
		pr_notice("\ninternal sw reset released\n");
	} else if (!strncmp("help", cmd, strlen("help"))) {
		airphy_port_mode_help();
	}

	if (ret < 0)
		return ret;

	return count;
}

static void airphy_debugfs_buckpbus_help(void)
{
	pr_notice("\nUsage:\n"
			"[debugfs] = /sys/kernel/debug/mdio-bus\':[phy_addr]\n"
			"Read:\n"
			"echo r [buckpbus_register] > /[debugfs]/buckpbus_op\n"
			"Write:\n"
			"echo w [buckpbus_register] [value] > /[debugfs]/buckpbus_op\n");
}


static ssize_t airphy_debugfs_buckpbus(struct file *file,
					const char __user *buffer, size_t count,
					loff_t *data)
{
	struct phy_device *phydev = file->private_data;
	char buf[64];
	int ret = 0;
	unsigned int reg, val;

	memset(buf, 0, 64);
	if (count > sizeof(buf) - 1)
		return -EINVAL;
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	if (buf[0] == 'w') {
		if (sscanf(buf, "w %x %x", &reg, &val) == -1)
			return -EFAULT;

		pr_notice("\nphy=%d, reg=0x%x, val=0x%x\n",
			phydev_addr(phydev), reg, val);
		ret = air_buckpbus_reg_write(phydev, reg, val);
		if (ret < 0) {
			pr_notice("\nbuckpbus_reg_write fail\n");
			return -EIO;
		}
		val = air_buckpbus_reg_read(phydev, reg);
		pr_notice("\nphy=%d, reg=0x%x, val=0x%x confirm..\n",
			phydev_addr(phydev), reg, val);
	} else if (buf[0] == 'r') {
		if (sscanf(buf, "r %x", &reg) == -1)
			return -EFAULT;

		val = air_buckpbus_reg_read(phydev, reg);
		pr_notice("\nphy=%d, reg=0x%x, val=0x%x\n",
		phydev_addr(phydev), reg, val);
	} else
		airphy_debugfs_buckpbus_help();

	return count;
}

static ssize_t airphy_debugfs_pbus(struct file *file,
					const char __user *buffer, size_t count,
					loff_t *data)
{
	struct phy_device *phydev = file->private_data;
	char buf[64];
	int ret = 0;
	unsigned int reg, val;

	memset(buf, 0, 64);
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	if (buf[0] == 'w') {
		if (sscanf(buf, "w %x %x", &reg, &val) == -1)
			return -EFAULT;

		pr_notice("\nphy=%d, reg=0x%x, val=0x%x\n",
			phydev_addr(phydev), reg, val);
		ret = air_pbus_reg_write(phydev, reg, val);
		if (ret < 0) {
			pr_notice("\npbus_reg_write fail\n");
			return -EIO;
		}
		val = air_pbus_reg_read(phydev, reg);
		pr_notice("\nphy=%d, reg=0x%x, val=0x%x confirm..\n",
			phydev_addr(phydev), reg, val);
	} else if (buf[0] == 'r') {
		if (sscanf(buf, "r %x", &reg) == -1)
			return -EFAULT;

		val = air_pbus_reg_read(phydev, reg);
		pr_notice("\nphy=%d, reg=0x%x, val=0x%x\n",
		phydev_addr(phydev), reg, val);
	} else
		airphy_debugfs_buckpbus_help();

	return count;
}

static int airphy_link_status(struct seq_file *seq, void *v)
{
	int ret = 0;
	struct phy_device *phydev = seq->private;

	ret = air_read_status(phydev);
	if (ret < 0)
		return ret;

	seq_printf(seq, "%s Information:\n", dev_name(phydev_dev(phydev)));
	seq_printf(seq, "\tPHYAD: %02d\n", phydev_addr(phydev));
	seq_printf(seq, "\tLink Status: %s\n", phydev->link ? "UP" : "DOWN");
	if (phydev->link) {
		seq_printf(seq, "\tAuto-Nego: %s\n",
				phydev->autoneg ? "on" : "off");
		seq_puts(seq, "\tSpeed: ");
		if (phydev->speed == SPEED_UNKNOWN)
			seq_printf(seq, "Unknown! (%i)\n", phydev->speed);
		else
			seq_printf(seq, "%uMb/s\n", phydev->speed);

		seq_printf(seq, "\tDuplex: %s\n",
				phydev->duplex ? "Full" : "Half");
		seq_puts(seq, "\n");
	}

	return ret;
}

static int airphy_link_status_open(struct inode *inode, struct file *file)
{
	return single_open(file, airphy_link_status, inode->i_private);
}

static int dbg_regs_show(struct seq_file *seq, void *v)
{
	struct phy_device *phydev = seq->private;
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);

	seq_puts(seq, "\t<<DEBUG REG DUMP>>\n");
	seq_printf(seq, "| RG_MII_BMSR           : 0x%08x |\n",
		   air_mii_cl22_read(mbus, addr, MII_BMSR));
	seq_printf(seq, "| RG_MII_REF_CLK        : 0x%08x |\n",
		   air_mii_cl22_read(mbus, addr, 0x1d));
	seq_printf(seq, "| RG_SYS_LINK_MODE      : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0xe0004));
	seq_printf(seq, "| RG_FCM_CTRL           : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0xe000C));
	seq_printf(seq, "| RG_SS_PAUSE_TIME      : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0xe0020));
	seq_printf(seq, "| RG_MIN_IPG_NUM        : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0xe002C));
	seq_printf(seq, "| RG_CTROL_0            : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0xc0000));
	seq_printf(seq, "| RG_LINK_STATUS        : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0xc0b04));
	seq_printf(seq, "| RG_LINK_PARTNER_AN    : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0xc0014));
	seq_printf(seq, "| RG_FN_PWR_CTRL_STATUS : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0x1020c));
	seq_printf(seq, "| RG_WHILE_LOOP_COUNT   : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0x3A48));

	return 0;
}

static int airphy_dbg_regs_show_open(struct inode *inode, struct file *file)
{
	return single_open(file, dbg_regs_show, inode->i_private);
}

static const struct file_operations airphy_info_fops = {
	.owner = THIS_MODULE,
	.open = airphy_info_open,
	.read = seq_read,
	.llseek = noop_llseek,
};

static const struct file_operations airphy_counter_fops = {
	.owner = THIS_MODULE,
	.open = airphy_counter_open,
	.read = seq_read,
	.llseek = noop_llseek,
};

static const struct file_operations airphy_debugfs_buckpbus_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.write = airphy_debugfs_buckpbus,
	.llseek = noop_llseek,
};

static const struct file_operations airphy_debugfs_pbus_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.write = airphy_debugfs_pbus,
	.llseek = noop_llseek,
};

static const struct file_operations airphy_port_mode_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.write = airphy_port_mode,
	.llseek = noop_llseek,
};

static const struct file_operations airphy_polarity_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.write = airphy_polarity_write,
	.llseek = noop_llseek,
};

static const struct file_operations airphy_link_status_fops = {
	.owner = THIS_MODULE,
	.open = airphy_link_status_open,
	.read = seq_read,
	.llseek = noop_llseek,
};

static const struct file_operations airphy_dbg_reg_show_fops = {
	.owner = THIS_MODULE,
	.open = airphy_dbg_regs_show_open,
	.read = seq_read,
	.llseek = noop_llseek,
};

int airphy_debugfs_init(struct phy_device *phydev)
{
	int ret = 0;
	struct en8811h_priv *priv = phydev->priv;
	struct dentry *dir = priv->debugfs_root;

	dev_dbg(phydev_dev(phydev), "%s: start\n", __func__);
	dir = debugfs_create_dir(dev_name(phydev_dev(phydev)), NULL);
	if (!dir) {
		dev_err(phydev_dev(phydev), "%s:err at %d\n",
					 __func__, __LINE__);
		ret = -ENOMEM;
	}
	debugfs_create_file(DEBUGFS_DRIVER_INFO, S_IFREG | 0444,
					dir, phydev,
					&airphy_info_fops);
	debugfs_create_file(DEBUGFS_COUNTER, S_IFREG | 0444,
					dir, phydev,
					&airphy_counter_fops);
	debugfs_create_file(DEBUGFS_BUCKPBUS_OP, S_IFREG | 0200,
					dir, phydev,
					&airphy_debugfs_buckpbus_fops);
	debugfs_create_file(DEBUGFS_PBUS_OP, S_IFREG | 0200,
					dir, phydev,
					&airphy_debugfs_pbus_fops);
	debugfs_create_file(DEBUGFS_PORT_MODE, S_IFREG | 0200,
					dir, phydev,
					&airphy_port_mode_fops);
	debugfs_create_file(DEBUGFS_POLARITY, S_IFREG | 0200,
					dir, phydev,
					&airphy_polarity_fops);
	debugfs_create_file(DEBUGFS_LINK_STATUS, S_IFREG | 0444,
					dir, phydev,
					&airphy_link_status_fops);
	debugfs_create_file(DEBUGFS_DBG_REG_SHOW, S_IFREG | 0444,
					dir, phydev,
					&airphy_dbg_reg_show_fops);

	priv->debugfs_root = dir;
	return ret;
}

void air_debugfs_remove(struct phy_device *phydev)
{
	struct en8811h_priv *priv = phydev->priv;

	debugfs_remove_recursive(priv->debugfs_root);
	priv->debugfs_root = NULL;
}
#endif /*CONFIG_AIROHA_EN8811H_PHY_DEBUGFS*/
