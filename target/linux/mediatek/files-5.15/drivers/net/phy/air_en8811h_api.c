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

#ifdef CONFIG_AIROHA_EN8811H_PHY_DEBUGFS
static const char * const tx_rx_string[32] = {
	"Tx Reverse, Rx Normal",
	"Tx Normal, Rx Normal",
	"Tx Reverse, Rx Reverse",
	"Tx Normal, Rx Reverse",
};
static struct trrg_s _fldRW_tr_reg__EcVarTrainingGain_ECNC_C8h = {
	.TrRGDesc.DescVal = 0x81900302,
	.RgMask =   0x0000000C
};
static struct trrg_s _fldRW_tr_reg__EcVarTrainingTime_ECNC_C8h = {
	.TrRGDesc.DescVal = 0x81900F04,
	.RgMask =   0x0000FFF0
};
#endif
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

int __air_mii_cl45_read(struct phy_device *phydev, int devad, u16 reg)
{
	int ret = 0;
	int data;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);

	ret = __air_mii_cl22_write(mbus, addr, MII_MMD_ACC_CTL_REG, devad);
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

int __air_mii_cl45_write(struct phy_device *phydev,
					int devad, u16 reg, u16 write_data)
{
	int ret = 0;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);

	ret = __air_mii_cl22_write(mbus, addr, MII_MMD_ACC_CTL_REG, devad);
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
	struct mii_bus *mbus = phydev_mdio_bus(phydev);

	mutex_lock(&mbus->mdio_lock);
	data = __air_mii_cl45_read(phydev, devad, reg);
	mutex_unlock(&mbus->mdio_lock);
	return data;
}

int air_mii_cl45_write(struct phy_device *phydev,
				int devad, u16 reg, u16 write_data)
{
	int ret = 0;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);

	mutex_lock(&mbus->mdio_lock);
	ret = __air_mii_cl45_write(phydev, devad, reg, write_data);
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
	unsigned int data;

	mutex_lock(&mbus->mdio_lock);
	data = __air_pbus_reg_read(phydev, pbus_address);
	mutex_unlock(&mbus->mdio_lock);
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

	ret = __air_mii_cl22_write(mbus, (addr + 8),
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
	ret = __air_pbus_reg_write(phydev, pbus_address, pbus_data);
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
	ret = __air_mii_cl22_write(mbus, addr, 0x1F, 4);
	ret |= __air_mii_cl22_write(mbus, addr, 0x10, 0);
	ret |= __air_mii_cl22_write(mbus, addr,
			0x11, ((pbus_address >> 16) & 0xffff));
	ret |= __air_mii_cl22_write(mbus, addr,
			0x12, (pbus_address & 0xffff));
	ret |= __air_mii_cl22_write(mbus, addr,
			0x13, ((pbus_data >> 16) & 0xffff));
	ret |= __air_mii_cl22_write(mbus, addr, 0x14, (pbus_data & 0xffff));
	ret |= __air_mii_cl22_write(mbus, addr, 0x1F, 0);
	if (ret) {
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
	ret = __air_mii_cl22_write(mbus, addr, 0x1F, 4);
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
	pbus_data = pbus_data_low | (pbus_data_high << 16);
	ret |= __air_mii_cl22_write(mbus, addr, 0x1F, 0);
	if (ret) {
		dev_err(dev, "__air_mii_cl22_write, ret: %d\n", ret);
		return PBUS_INVALID_DATA;
	}
	return pbus_data;
}

unsigned int air_buckpbus_reg_read(struct phy_device *phydev,
			unsigned int pbus_address)
{
	unsigned int data;
	struct mii_bus *mbus = phydev_mdio_bus(phydev);

	mutex_lock(&mbus->mdio_lock);
	data = __air_buckpbus_reg_read(phydev, pbus_address);
	mutex_unlock(&mbus->mdio_lock);
	return data;
}

int air_buckpbus_reg_write(struct phy_device *phydev,
		unsigned int pbus_address, unsigned int pbus_data)
{
	int ret = 0;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);

	mutex_lock(&mbus->mdio_lock);
	ret = __air_buckpbus_reg_write(phydev, pbus_address, pbus_data);
	mutex_unlock(&mbus->mdio_lock);
	if (ret) {
		dev_err(dev, "__air_buckpbus_reg_write, ret: %d\n", ret);
		return ret;
	}
	return ret;
}

int air_surge_protect_cfg(struct phy_device *phydev)
{
	int ret = 0;
	struct device *dev = phydev_dev(phydev);
	struct en8811h_priv *priv = phydev->priv;

	if (priv->surge) {
		ret = air_mii_cl45_write(phydev, 0x1e, 0x800c, 0x0);
		ret |= air_mii_cl45_write(phydev, 0x1e, 0x800d, 0x0);
		ret |= air_mii_cl45_write(phydev, 0x1e, 0x800e, 0x1100);
		ret |= air_mii_cl45_write(phydev, 0x1e, 0x800f, 0x00b0);
		if (ret < 0)
			return ret;
		dev_info(dev, "surge protection mode - 5R\n");
	} else
		dev_info(dev, "surge protection mode - 0R\n");
	return ret;
}

int air_cko_cfg(struct phy_device *phydev)
{
	int ret = 0;
	struct device *dev = phydev_dev(phydev);
	struct en8811h_priv *priv = phydev->priv;
	u32 pbus_value = 0;

	if (!priv->cko) {
		pbus_value = air_buckpbus_reg_read(phydev, 0xcf958);
		pbus_value &= ~BIT(26);
		ret = air_buckpbus_reg_write(phydev, 0xcf958, pbus_value);
		if (ret < 0)
			return ret;

		dev_info(dev, "CKO Output mode - Disabled\n");
	} else
		dev_info(dev, "CKO Output mode - Enabled\n");
	return ret;
}

int air_ref_clk_speed(struct phy_device *phydev, int para)
{
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev), ret;
	struct en8811h_priv *priv = phydev->priv;

	mutex_lock(&mbus->mdio_lock);
	ret = __air_mii_cl22_write(mbus, addr, 0x1f, 0x0);
	/* Get real speed from vendor register */
	ret = __air_mii_cl22_read(mbus, addr, AIR_AUX_CTRL_STATUS);
	if (ret < 0)
		goto unlock;
	switch (ret & AIR_AUX_CTRL_STATUS_SPEED_MASK) {
	case AIR_AUX_CTRL_STATUS_SPEED_2500:
		if (para == AIR_PARA_PRIV)
			priv->speed = SPEED_2500;
		else
			phydev->speed = SPEED_2500;
		break;
	case AIR_AUX_CTRL_STATUS_SPEED_1000:
		if (para == AIR_PARA_PRIV)
			priv->speed = SPEED_1000;
		else
			phydev->speed = SPEED_1000;
		break;
	case AIR_AUX_CTRL_STATUS_SPEED_100:
		if (para == AIR_PARA_PRIV)
			priv->speed = SPEED_100;
		else
			phydev->speed = SPEED_100;
		break;
	default:
		if (para == AIR_PARA_PRIV)
			priv->speed = SPEED_2500;
		else
			phydev->speed = SPEED_2500;
		dev_err(dev, "%s: Default Speed: 0x%x\n", __func__, ret);
		break;
	}
unlock:
	mutex_unlock(&mbus->mdio_lock);
	return ret;
}


#if defined(CONFIG_OF)
int en8811h_of_init(struct phy_device *phydev)
{
	struct device *dev = phydev_dev(phydev);
	struct device_node *of_node = dev->of_node;
	struct en8811h_priv *priv = phydev->priv;
	int val = 0;

	dev_info(dev, "%s: start\n", __func__);
	if (of_find_property(of_node, "airoha,polarity", NULL)) {
		if (of_property_read_u32(of_node, "airoha,polarity",
					 &val) != 0) {
			dev_err(dev, "airoha,polarity value is invalid.");
			return -EINVAL;
		}
		if (val < AIR_POL_TX_REV_RX_NOR ||
		    val > AIR_POL_TX_NOR_RX_REV) {
			dev_err(dev,
				   "airoha,polarity value %u out of range.",
				   val);
			return -EINVAL;
		}
		priv->pol = val;
	} else
		priv->pol = AIR_POL_TX_NOR_RX_NOR;

	if (of_find_property(of_node, "airoha,surge", NULL)) {
		if (of_property_read_u32(of_node, "airoha,surge",
					 &val) != 0) {
			dev_err(dev, "airoha,surge value is invalid.");
			return -EINVAL;
		}
		if (val < 0 || val > 1) {
			dev_err(dev,
				   "airoha,surge value %u out of range.",
				   val);
			return -EINVAL;
		}
		priv->surge = val;
	} else
		priv->surge = AIR_SURGE_0R;

	if (of_find_property(of_node, "airoha,cko-en", NULL)) {
		if (of_property_read_u32(of_node, "airoha,cko-en",
					 &val) != 0) {
			dev_err(dev, "airoha,cko-en value is invalid.");
			return -EINVAL;
		}
		if (val < AIR_CKO_DIS ||
			val > AIR_CKO_EN) {
			dev_err(dev,
				   "airoha,cko-en value %u out of range.",
				   val);
			return -EINVAL;
		}
		priv->cko = val;
	} else
		priv->cko = AIR_CKO_DIS;

	if (of_find_property(of_node, "airoha,phy-handle", NULL))
		priv->phy_handle = true;
	else
		priv->phy_handle = false;

	return 0;
}
#else
int en8811h_of_init(struct phy_device *phydev)
{
	return -ESRCH;
}
#endif /* CONFIG_OF */

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

int airoha_control_flag(struct phy_device *phydev, int mask, int val)
{
	u32 pbus_value = 0;
	int ret;
	struct device *dev = phydev_dev(phydev);

	pbus_value = air_buckpbus_reg_read(phydev, 0x3a9c);
	dev_dbg(dev, "%d:pbus_value 0x%x!\n", __LINE__, pbus_value);
	switch (val) {
	case 0:
		pbus_value &= ~BIT(mask);
		break;
	case 1:
		pbus_value |= BIT(mask);
		break;
	default:
		dev_err(dev, "Wrong value %d!\n", val);
		return -EINVAL;
	}
	dev_dbg(dev, "%d:pbus_value 0x%x!\n", __LINE__, pbus_value);
	ret = air_buckpbus_reg_write(phydev,
					0x3a9c, pbus_value);
	if (ret < 0)
		return ret;
	return 0;
}

#ifdef CONFIG_AIROHA_EN8811H_PHY_DEBUGFS
static int air_read_status(struct phy_device *phydev)
{
	int ret = 0, reg = 0;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);
	struct en8811h_priv *priv = phydev->priv;

	priv->speed = SPEED_UNKNOWN;
	priv->duplex = DUPLEX_UNKNOWN;
	priv->pause = 0;
	priv->asym_pause = 0;

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
		priv->link = 1;
		ret = air_ref_clk_speed(phydev, AIR_PARA_PRIV);
		if (ret < 0)
			return ret;
		reg = air_mii_cl22_read(mbus,
					addr, MII_ADVERTISE);
		if (reg < 0)
			return reg;
		priv->pause = GET_BIT(reg, 10);
		priv->asym_pause = GET_BIT(reg, 11);
	} else
		priv->link = 0;

	priv->duplex = DUPLEX_FULL;
	return 0;
}

static void air_polarity_help(void)
{
	pr_notice("\nUsage:\n"
			"[debugfs] = /sys/kernel/debug/mdio-bus\':[phy_addr]\n"
			"echo [tx polarity] [rx polarity] > /[debugfs]/polarity\n"
			"option: tx_normal, tx_reverse, rx_normal, rx_revers\n");
}

static int air_set_polarity(struct phy_device *phydev, unsigned int tx_rx)
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
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);
	struct en8811h_priv *priv = phydev->priv;

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
		priv->need_an = 1;
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
		priv->need_an = 1;
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
		priv->need_an = 1;
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
		val = air_mii_cl22_read(mbus, addr, MII_BMCR);
		val |= BMCR_ANENABLE;
		ret = air_mii_cl22_write(mbus, addr, MII_BMCR, val);
		if (unlikely(ret < 0))
			break;
		if (priv->need_an) {
			val = air_mii_cl22_read(mbus, addr, MII_BMCR);
			val |= BMCR_ANRESTART;
			ret = air_mii_cl22_write(mbus, addr, MII_BMCR, val);
			if (unlikely(ret < 0))
				break;
			priv->need_an = 0;
			pr_notice("\nRe-an\n");
		}
		break;
	case AIR_PORT_MODE_POWER_DOWN:
		pr_notice("\nPower Down\n");
		val = air_mii_cl22_read(mbus, addr, MII_BMCR) | BIT(11);
		ret = air_mii_cl22_write(mbus, addr, MII_BMCR, val);
		break;
	case AIR_PORT_MODE_POWER_UP:
		pr_notice("\nPower Up\n");
		val = air_mii_cl22_read(mbus, addr, MII_BMCR) & ~BIT(11);
		ret = air_mii_cl22_write(mbus, addr, MII_BMCR, val);
		break;
	default:
		pr_notice("\nWrong Port mode\n");
		break;
	}
	return ret;
}

static int airoha_led_control(struct phy_device *phydev, int force_mode)
{
	int ret = 0, id, on_evt;
	struct en8811h_priv *priv = phydev->priv;

	for (id = 0; id < EN8811H_LED_COUNT; id++) {
		if (force_mode) {
			on_evt = air_mii_cl45_read(phydev, 0x1f,
					LED_ON_CTRL(id));
			on_evt &= ~(LED_ON_EVT_LINK_2500M |
				LED_ON_EVT_LINK_100M | LED_ON_EVT_LINK_1000M);
			if (force_mode == AIR_LED_FORCE_ON)
				on_evt |= LED_ON_EVT_FORCE;
			else
				on_evt &= ~LED_ON_EVT_FORCE;

			ret = air_mii_cl45_write(phydev, 0x1f,
					LED_ON_CTRL(id), on_evt);
			if (ret < 0)
				return ret;
			ret = air_mii_cl45_write(phydev, 0x1f,
					LED_BLK_CTRL(id), 0);
		} else {
			ret = air_mii_cl45_write(phydev, 0x1f,
					LED_ON_CTRL(id), priv->on_crtl[id]);
			ret |= air_mii_cl45_write(phydev, 0x1f,
					LED_BLK_CTRL(id), priv->blk_crtl[id]);
		}
		if (ret < 0)
			return ret;
	}
	return 0;
}

static void airphy_led_mode_help(void)
{
	pr_notice("\nUsage:\n"
			"[debugfs] = /sys/kernel/debug/mdio-bus\':[phy_addr]\n"
			"echo 0 > /[debugfs]/led_mode\n"
			"echo 1 > /[debugfs]/led_mode\n"
			"echo normal > /[debugfs]/led_mode\n");
}

static ssize_t airphy_led_mode(struct file *file, const char __user *ptr,
					size_t len, loff_t *off)
{
	struct phy_device *phydev = file->private_data;
	char buf[32], cmd[32];
	int count = len, ret = 0;
	int num = 0;

	memset(buf, 0, 32);
	memset(cmd, 0, 32);

	if (count > sizeof(buf) - 1)
		return -EINVAL;
	if (copy_from_user(buf, ptr, len))
		return -EFAULT;

	num = sscanf(buf, "%8s", cmd);
	if (num < 1 || num > 2)
		return -EFAULT;

	if (!strncmp("0", cmd, strlen("0")))
		ret = airoha_led_control(phydev, AIR_LED_FORCE_OFF);
	else if (!strncmp("1", cmd, strlen("1")))
		ret = airoha_led_control(phydev, AIR_LED_FORCE_ON);
	else if (!strncmp("normal", cmd, strlen("normal")))
		ret = airoha_led_control(phydev, AIR_LED_NORMAL);
	else if (!strncmp("help", cmd, strlen("help")))
		airphy_led_mode_help();

	if (ret < 0)
		return ret;

	return count;
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
	seq_printf(seq, "| MD32 FW Version      : %08x\n", priv->firmware_version);
	val = air_buckpbus_reg_read(phydev, 0x3a9c);
	if (val & BIT(3))
		seq_puts(seq, "| Surge Protection     : 5R\n");
	else
		seq_puts(seq, "| Surge Protection     : 0R\n");
	val = air_buckpbus_reg_read(phydev, 0xcf958);
	if (val & BIT(26))
		seq_puts(seq, "| Co-Clock Ouput       : Enable\n");
	else
		seq_puts(seq, "| Co-Clock Ouput       : Disable\n");
	val = (air_buckpbus_reg_read(phydev, 0xca0f8) & 0x3);
	seq_printf(seq, "| Tx, Rx Polarity      : %s(%02d)\n",
						tx_rx_string[val], val);
	seq_printf(seq, "| Init Stage           : %02d\n",
						priv->init_stage);
	seq_puts(seq, "\n");

	return 0;
}

static int airphy_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, airphy_info_show, inode->i_private);
}

/**
 * airphy_fcm_counter_show - FCM Counter
 * @seq: Pointer to the sequence file structure.
 * @phydev: target phy_device struct
 */
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
	seq_puts(seq, "| Pause to System side     :");
	pkt_cnt = air_buckpbus_reg_read(phydev, 0xe00A4);
	seq_printf(seq, "%010u |\n", pkt_cnt);
	seq_puts(seq, "| Pause from Line side     :");
	pkt_cnt = air_buckpbus_reg_read(phydev, 0xe0098);
	seq_printf(seq, "%010u |\n", pkt_cnt);
	seq_puts(seq, "| Pause from System side   :");
	pkt_cnt = air_buckpbus_reg_read(phydev, 0xe0080);
	seq_printf(seq, "%010u |\n", pkt_cnt);
	seq_puts(seq, "| Pause to Line side       :");
	pkt_cnt = air_buckpbus_reg_read(phydev, 0xe008C);
	seq_printf(seq, "%010u |\n", pkt_cnt);
	ret = air_buckpbus_reg_write(phydev, 0xe0074, 0x3);
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
	seq_printf(seq, "%010u |\n", pkt_cnt);
	ret = air_buckpbus_reg_write(phydev, 0xC602C, 0x4);
	if (ret < 0)
		return ret;
	return 0;
}
/**
 * airphy_mac_counter - Internal MAC counter
 * @seq: Pointer to the sequence file structure.
 * @phydev: target phy_device struct
 * NOTE: MAC counter should not be polled continuously.
 */
static int airphy_mac_counter_show(struct seq_file *seq,
		struct phy_device *phydev)
{
	int ret = 0;
	u32 pkt_cnt = 0;

	ret = air_buckpbus_reg_write(phydev, 0xdc036, 0xb);
	if (ret < 0)
		return ret;
	pkt_cnt = air_buckpbus_reg_read(phydev, 0xdc037);
	if (pkt_cnt == 0x98) {
		seq_puts(seq, "|\t<<MAC Counter>>\n");
		seq_puts(seq, "| Tx Error from System side:");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x131000);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| Rx Error to System side  :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x132000);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| Tx from System side      :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x131004);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| Rx to System Side        :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x132004);
		seq_printf(seq, "%010u |\n", pkt_cnt);
	} else
		seq_printf(seq, "MAC is not ready.(0x%x)\n", pkt_cnt);
	return 0;
}

static int airphy_counter_show(struct seq_file *seq, void *v)
{
	struct phy_device *phydev = seq->private;
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int ret = 0, addr = phydev_addr(phydev);
	u32 pkt_cnt = 0;
	struct en8811h_priv *priv = phydev->priv;

	ret = air_read_status(phydev);
	if (ret < 0)
		return ret;
	seq_puts(seq, "==========AIR PHY COUNTER==========\n");
	if (priv->link) {
		ret = airphy_ss_counter_show(phydev, seq);
		if (ret < 0)
			return ret;
	}
	ret = airphy_fcm_counter_show(phydev, seq);
	if (ret < 0)
		return ret;
	if (priv->link) {
		ret = airphy_mac_counter_show(seq, phydev);
		if (ret < 0)
			return ret;
	}
	if (priv->link && priv->speed == SPEED_2500) {
		seq_puts(seq, "|\t<<LS Counter>>\n");
		ret = air_buckpbus_reg_write(phydev, 0x30718, 0x10);
		if (ret < 0)
			return ret;
		ret = air_buckpbus_reg_write(phydev, 0x30718, 0x0);
		if (ret < 0)
			return ret;
		seq_puts(seq, "|\tBefore EF\n");
		seq_puts(seq, "| Tx to Line side_S        :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x3071c);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| Tx to Line side_T        :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x30720);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| Tx_ENC                   :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x30724);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| Rx from Line side_S      :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x3072c);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| Rx from Line side_T      :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x30730);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| Rx_DEC                   :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x30728);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "|\tAfter EF\n");
		seq_puts(seq, "| Tx to Line side_S        :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x30734);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| Tx to Line side_T        :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x30738);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| Rx from Line side_S      :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x30764);
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| Rx from Line side_T      :");
		pkt_cnt = air_buckpbus_reg_read(phydev, 0x30768);
		seq_printf(seq, "%010u |\n\n", pkt_cnt);
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
	}
	if (priv->link && ((priv->speed != SPEED_2500))) {
		seq_puts(seq, "|\t<<LS Counter>>\n");
		ret = air_mii_cl22_write(mbus, addr, 0x1f, 1);
		if (ret < 0)
			return ret;
		seq_puts(seq, "| Rx from Line side        :");
		pkt_cnt = air_mii_cl22_read(mbus, addr, 0x12) & 0x7fff;
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| Rx Error from Line side  :");
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
		seq_puts(seq, "| Tx to Line side          :");
		pkt_cnt = (air_mii_cl22_read(mbus, addr, 0x11) & 0x7ffe) >> 1;
		seq_printf(seq, "%010u |\n", pkt_cnt);
		seq_puts(seq, "| Tx Error to Line side    :");
		pkt_cnt = air_mii_cl22_read(mbus, addr, 0x12);
		pkt_cnt &= 0x7f;
		seq_printf(seq, "%010u |\n\n", pkt_cnt);
		ret = air_mii_cl22_write(mbus, addr, 0x1f, 0);
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
	if (sscanf(buf, "%12s %12s", param1, param2) == -1)
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
			"echo ssc ena/dis > /[debugfs]/port_mode\n"
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

	num = sscanf(buf, "%8s %8s", cmd, param);
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
	} else if (!strncmp("ssc", cmd, strlen("ssc"))) {
		if (!strncmp("dis", param, strlen("dis")))
			ret = air_set_mode(phydev, AIR_PORT_MODE_SSC_DISABLE);
		else if (!strncmp("ena", param, strlen("ena")))
			ret = air_set_mode(phydev, AIR_PORT_MODE_SSC_ENABLE);
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
	int ret = 0, i;
	unsigned int reg, val, num;

	memset(buf, 0, 64);
	if (count > sizeof(buf) - 1)
		return -EINVAL;
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	if (buf[0] == 'w') {
		if (sscanf(buf, "w %15x %15x", &reg, &val) == -1)
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
		if (sscanf(buf, "r %15x", &reg) == -1)
			return -EFAULT;

		val = air_buckpbus_reg_read(phydev, reg);
		pr_notice("\nphy=%d, reg=0x%x, val=0x%x\n",
		phydev_addr(phydev), reg, val);
	} else if (buf[0] == 'x') {
		if (sscanf(buf, "x %15x %6d", &reg, &num) == -1)
			return -EFAULT;
		if (num > 0x1000 || num == 0) {
			pr_notice("\nphy%d: number(0x%x) is invalid number\n",
				phydev_addr(phydev), num);
			return -EFAULT;
		}
		for (i = 0; i < num; i++) {
			val = air_buckpbus_reg_read(phydev, (reg + (i * 4)));
			pr_notice("phy=%d, reg=0x%x, val=0x%x",
				phydev_addr(phydev), reg + (i * 4), val);
			pr_notice("");
		}
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
		if (sscanf(buf, "w %15x %15x", &reg, &val) == -1)
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
		if (sscanf(buf, "r %15x", &reg) == -1)
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
	struct en8811h_priv *priv = phydev->priv;

	ret = air_read_status(phydev);
	if (ret < 0)
		return ret;

	seq_printf(seq, "%s Information:\n", dev_name(phydev_dev(phydev)));
	seq_printf(seq, "\tPHYAD: %02d\n", phydev_addr(phydev));
	seq_printf(seq, "\tLink Status: %s\n", priv->link ? "UP" : "DOWN");
	if (priv->link) {
		ret = air_get_autonego(phydev, &priv->an);
		if (ret < 0)
			return ret;
		seq_printf(seq, "\tAuto-Nego: %s\n",
				priv->an ? "on" : "off");
		seq_puts(seq, "\tSpeed: ");
		if (priv->speed == SPEED_UNKNOWN)
			seq_printf(seq, "Unknown! (%i)\n", priv->speed);
		else
			seq_printf(seq, "%uMb/s\n", priv->speed);

		seq_printf(seq, "\tDuplex: %s\n",
				priv->duplex ? "Full" : "Half");
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
	int addr = phydev_addr(phydev), ret, reg;

	seq_puts(seq, "\t<<DEBUG REG DUMP>>\n");
	for (reg = MII_BMCR; reg <= MII_STAT1000; reg++) {
		seq_printf(seq, "| RG_MII_REG_%02x         : 0x%08x |\n",
					reg, air_mii_cl22_read(mbus, addr, reg));
	}
	seq_printf(seq, "| RG_ABILITY_2G5        : 0x%08x |\n",
		    air_mii_cl45_read(phydev, 0x7, 0x20));
	seq_printf(seq, "| RG_LINK_PARTNER_2G5   : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0x3b30));
	ret = air_mii_cl22_write(mbus, addr, 0x1f, 0x0);
	if (ret < 0)
		return 0;
	ret = air_mii_cl22_read(mbus, addr, 0x1d);
	seq_printf(seq, "| RG_MII_REF_CLK        : 0x%08x |\n",
		   ret);
	seq_printf(seq, "| RG_PHY_ANA            : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0xca0f8));
	seq_printf(seq, "| RG_HW_STRAP1          : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0xcf910));
	seq_printf(seq, "| RG_HW_STRAP2          : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0xcf914));
	seq_printf(seq, "| RG_SYS_LINK_MODE      : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0xe0004));
	seq_printf(seq, "| RG_FCM_CTRL           : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0xe000C));
	seq_printf(seq, "| RG_SS_PAUSE_TIME      : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0xe0020));
	seq_printf(seq, "| RG_MIN_IPG_NUM        : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0xe002C));
	seq_printf(seq, "| RG_CSR_AN0            : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0xc0000));
	seq_printf(seq, "| RG_SS_LINK_STATUS     : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0xc0b04));
	seq_printf(seq, "| RG_LINK_PARTNER_AN    : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0xc0014));
	seq_printf(seq, "| RG_FN_PWR_CTRL_STATUS : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0x1020c));
	seq_printf(seq, "| RG_MD32_FW_READY      : 0x%08x |\n",
		   air_mii_cl45_read(phydev, 0x1e, 0x8009));
	seq_printf(seq, "| RG_RX_SYNC_CNT        : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0x3A64));
	seq_printf(seq, "| RG_WHILE_LOOP_COUNT   : 0x%08x |\n",
		   air_buckpbus_reg_read(phydev, 0x3A48));
	return 0;
}

static int airphy_dbg_regs_show_open(struct inode *inode, struct file *file)
{
	return single_open(file, dbg_regs_show, inode->i_private);
}


static int airphy_temp_show(struct seq_file *seq, void *v)
{
	struct phy_device *phydev = seq->private;
	int ret = 0;
	u32 pbus_value = 0;

	seq_puts(seq, "<<AIR EN8811H Temp>>\n");
	ret |= air_mii_cl45_write(phydev, 0x1e, 0x800e, 0x1100);
	ret |= air_mii_cl45_write(phydev, 0x1e, 0x800f, 0xe5);
	if (ret < 0) {
		pr_notice("\nmii_cl45_write fail\n");
		return -EIO;
	}
	pbus_value = air_buckpbus_reg_read(phydev, 0x3B38);
	seq_printf(seq, "| Temperature  : %dC |\n",
						pbus_value);
	seq_puts(seq, "\n");

	return 0;
}

static int airphy_temp_show_open(struct inode *inode, struct file *file)
{
	return single_open(file, airphy_temp_show, inode->i_private);
}


static unsigned int air_read_lp_speed(struct phy_device *phydev)
{
	int val = 0, ret = 0;
	int lpa, lpagb;
	int count = 15;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);
	struct en8811h_priv *priv = phydev->priv;

	if (priv->firmware_version < 0x24011202) {
		val = air_mii_cl22_read(mbus, addr, MII_BMCR) | BIT(9);
		ret = air_mii_cl22_write(mbus, addr, MII_BMCR, val);
		if (unlikely(ret < 0))
			return ret;
		msleep(1500);
		do {
			msleep(100);
			ret = air_mii_cl45_read(phydev, MDIO_MMD_AN, 0x21);
			ret &= BIT(5);
			if (ret)
				break;
			count--;
		} while (count);

		count = 10;
		do {
			msleep(500);
			val = air_mii_cl22_read(mbus, addr, MII_BMSR);
			if (val < 0) {
				dev_err(dev, "MII_BMSR reg 0x%x!\n", val);
				return val;
			}
			val = air_mii_cl22_read(mbus, addr, MII_BMSR);
			if (val < 0) {
				dev_err(dev, "MII_BMSR reg 0x%x!\n", val);
				return val;
			}
			dev_dbg(dev, "val 0x%x\n", val);
			if (val & BMSR_LSTATUS) {
				val = air_mii_cl22_read(mbus, addr, MII_LPA);
				if (val < 0)
					return val;
				lpa = (val & (BIT(5) | BIT(6) | BIT(7) | BIT(8))) >> 5;
				val = air_mii_cl22_read(mbus, addr, MII_STAT1000);
				if (val < 0)
					return val;
				lpagb = GET_BIT(val, 11) << 4;
				ret |= (lpagb | lpa);
				return ret;
			}
		} while (count--);
	} else {
		ret = air_mii_cl22_read(mbus, addr, MII_BMSR);
		if (ret < 0) {
			dev_err(dev, "MII_BMSR reg 0x%x!\n", ret);
			return ret;
		}
		ret = air_mii_cl22_read(mbus, addr, MII_BMSR);
		if (ret < 0) {
			dev_err(dev, "MII_BMSR reg 0x%x!\n", ret);
			return ret;
		}
		dev_dbg(dev, "val 0x%x\n", ret);
		if (ret & BMSR_LSTATUS) {
			ret = air_buckpbus_reg_read(phydev, 0x3b30);
			ret = GET_BIT(ret, 0) << 5;
			lpa = air_mii_cl22_read(mbus, addr, MII_LPA);
			if (lpa < 0)
				return lpa;
			lpa &= (BIT(5) | BIT(6) | BIT(7) | BIT(8));
			lpa >>= 5;
			lpagb = air_mii_cl22_read(mbus, addr, MII_STAT1000);
			if (lpagb < 0)
				return lpagb;
			lpagb = GET_BIT(lpagb, 11) << 4;
			ret |= (lpagb | lpa);
			return ret;
		}
	}
	return 0;
}

static int airphy_lp_speed(struct seq_file *seq, void *v)
{
	int ret = 0, did1 = 0, i;
	struct phy_device *phydev = seq->private;
	static const struct {
		unsigned int bit_index;
		const char *name;
	} mode_defs[] = {
		{ AIR_LINK_MODE_10baseT_Half_BIT,
		"10baseT/Half" },
		{ AIR_LINK_MODE_10baseT_Full_BIT,
		"10baseT/Full" },
		{ AIR_LINK_MODE_100baseT_Half_BIT,
		"100baseT/Half" },
		{ AIR_LINK_MODE_100baseT_Full_BIT,
		"100baseT/Full" },
		{ AIR_LINK_MODE_1000baseT_Full_BIT,
		"1000baseT/Full" },
		{ AIR_LINK_MODE_2500baseT_Full_BIT,
		"2500baseT/Full" }
	};

	seq_printf(seq, "%s Link Partner Ability:\n",
			dev_name(phydev_dev(phydev)));
	ret = air_read_lp_speed(phydev);
	if (ret < 0)
		return ret;
	for (i = 0; i < ARRAY_SIZE(mode_defs); i++) {
		if (ret & BIT(mode_defs[i].bit_index)) {
			seq_printf(seq, "\t\t\t %s\n",
						mode_defs[i].name);
			did1++;
		}
	}
	if (did1 == 0)
		seq_puts(seq, "\t\t\t Not reported\n");

	return 0;
}

static int airphy_lp_speed_open(struct inode *inode, struct file *file)
{
	return single_open(file, airphy_lp_speed, inode->i_private);
}

static void airphy_debugfs_mii_cl22_help(void)
{
	pr_notice("\nUsage:\n"
			"[debugfs] = /sys/kernel/debug/mdio-bus\':[phy_addr]\n"
			"Read:\n"
			"echo r [phy_register] > /[debugfs]/mii_mgr_op\n"
			"Write:\n"
			"echo w [phy_register] [value] > /[debugfs]/mii_mgr_op\n");
}


static ssize_t airphy_debugfs_cl22(struct file *file,
					const char __user *buffer, size_t count,
					loff_t *data)
{
	struct phy_device *phydev = file->private_data;
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);
	char buf[64];
	int ret = 0;
	unsigned int reg, val;

	memset(buf, 0, 64);
	if (count > sizeof(buf) - 1)
		return -EINVAL;
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	if (buf[0] == 'w') {
		if (sscanf(buf, "w %15x %15x", &reg, &val) == -1)
			return -EFAULT;

		pr_notice("\nphy=%d, reg=0x%x, val=0x%x\n",
			phydev_addr(phydev), reg, val);
		ret = air_mii_cl22_write(mbus, addr, reg, val);
		if (ret < 0) {
			pr_notice("\nmii_cl22_write fail\n");
			return -EIO;
		}
		val = air_mii_cl22_read(mbus, addr, reg);
		pr_notice("\nphy=%d, reg=0x%x, val=0x%x confirm..\n",
			phydev_addr(phydev), reg, val);
	} else if (buf[0] == 'r') {
		if (sscanf(buf, "r %15x", &reg) == -1)
			return -EFAULT;

		val = air_mii_cl22_read(mbus, addr, reg);
		pr_notice("\nphy=%d, reg=0x%x, val=0x%x\n",
		phydev_addr(phydev), reg, val);
	} else
		airphy_debugfs_mii_cl22_help();

	return count;
}

static void airphy_debugfs_mii_cl45_help(void)
{
	pr_notice("\nUsage:\n"
			"[debugfs] = /sys/kernel/debug/mdio-bus\':[phy_addr]\n"
			"Read:\n"
			"echo r [device number] [phy_register] > /[debugfs]/mii_mgr_cl45_op\n"
			"Write:\n"
			"echo w [device number] [phy_register] [value] > /[debugfs]/mii_mgr_cl45_op\n");
}


static ssize_t airphy_debugfs_cl45(struct file *file,
					const char __user *buffer, size_t count,
					loff_t *data)
{
	struct phy_device *phydev = file->private_data;
	char buf[64];
	int ret = 0;
	unsigned int reg, val, devnum;

	memset(buf, 0, 64);
	if (count > sizeof(buf) - 1)
		return -EINVAL;
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	if (buf[0] == 'w') {
		if (sscanf(buf, "w %15x %15x %15x", &devnum, &reg, &val) == -1)
			return -EFAULT;

		pr_notice("\nphy=%d, devnum=0x%x, reg=0x%x, val=0x%x\n",
			phydev_addr(phydev), devnum, reg, val);
		ret = air_mii_cl45_write(phydev, devnum, reg, val);
		if (ret < 0) {
			pr_notice("\nmii_cl45_write fail\n");
			return -EIO;
		}
		val = air_mii_cl45_read(phydev, devnum, reg);
		pr_notice("\nphy=%d, devnum=0x%x, reg=0x%x, val=0x%x confirm..\n",
			phydev_addr(phydev), devnum, reg, val);
	} else if (buf[0] == 'r') {
		if (sscanf(buf, "r %15x %15x", &devnum, &reg) == -1)
			return -EFAULT;

		val = air_mii_cl45_read(phydev, devnum, reg);
		pr_notice("\nphy=%d, devnum=0x%x, reg=0x%x, val=0x%x\n",
		phydev_addr(phydev), devnum, reg, val);
	} else
		airphy_debugfs_mii_cl45_help();

	return count;
}

static void
air_cable_pair_swap(int pair0, int pair1,
	struct air_cable_test_rsl *ptr_cable)
{
	int tmp_status;
	unsigned int tmp_length;

	tmp_status = ptr_cable->status[pair0];
	tmp_length = ptr_cable->length[pair0];
	ptr_cable->status[pair0] = ptr_cable->status[pair1];
	ptr_cable->length[pair0] = ptr_cable->length[pair1];
	ptr_cable->status[pair1] = tmp_status;
	ptr_cable->length[pair1] = tmp_length;
}

int air_wait_md32_fw(struct phy_device *phydev)
{
	int reg;
	struct device *dev = phydev_dev(phydev);
	int retry = 60;

	do {
		msleep(1000);
		reg = air_mii_cl45_read(phydev, 0x1e, 0x8009);
		if (reg == EN8811H_PHY_READY) {
			dev_dbg(dev, "%s: PHY ready!\n", __func__);
			break;
		}
		if (!(retry % 10))
			dev_info(dev, "Waiting for MD32 FW ready....(%d)", reg);

		if (!retry) {
			dev_err(dev, "%s: MD32 FW is not ready.(Status: 0x%x)\n",
							__func__, reg);
			return -EPERM;
		}
	} while (retry--);
	return 0;
}

static int air_cable_normal(struct phy_device *phydev, int step_col_12, int step_col_13)
{
	int ret = 0, reg;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);

	ret |= air_mii_cl22_write(mbus, addr, 0x1f, 0);
	reg = ((air_mii_cl22_read(mbus, addr, 0x11)  & BIT(12)) >> 12);
	if (!reg) {
		reg = air_mii_cl22_read(mbus,
				addr, MII_BMCR);
		ret |= air_mii_cl22_write(mbus,
				addr, MII_BMCR, reg | BMCR_PDOWN);
		ret |= air_mii_cl22_write(mbus,
				addr, MII_BMCR, reg & ~BMCR_PDOWN);
		dev_dbg(dev, "%s Power Down -> Power UP.\n", __func__);
		msleep(1000);
		ret |= air_mii_cl22_write(mbus, addr, 0x1f, 0x1);
		ret |= air_mii_cl22_write(mbus, addr, 0x18, 0x8000);
		ret |= air_wait_md32_fw(phydev);
		if (ret < 0) {
			dev_err(dev,
				"%s: cable normal 1-1 fail!\n", __func__);
			return -EPERM;
		}
	}
	ret |= air_mii_cl45_write(phydev, 0x1e, 0x800e, 0x1100);
	ret |= air_mii_cl45_write(phydev, 0x1e, 0x800f, 0xd4);
	ret |= air_wait_md32_fw(phydev);
	if (ret < 0) {
		dev_err(dev,
			"%s: cable normal 1-2 fail!\n", __func__);
		return -EPERM;
	}
	return 0;
}

static int airphy_cable_step2(struct phy_device *phydev)
{
	int ret = 0;
	int step_col_12 = 0;
	int step_col_13 = 0;
	struct device *dev = phydev_dev(phydev);

	ret = air_cable_normal(phydev, step_col_12, step_col_13);
	if (ret < 0) {
		dev_err(dev, "%s: air_cable_normal fail (ret=%d)\n",
				__func__, ret);
		return -EPERM;
	}
	step_col_12 = air_mii_cl45_read(phydev, 0x1e, 0x800c);
	step_col_13 = air_mii_cl45_read(phydev, 0x1e, 0x800d);
	dev_dbg(dev, "%s step_col_12 %d, step_col_13 %d\n",
				__func__, step_col_12, step_col_13);
	dev_dbg(dev, "%s: successfull\n", __func__);
	return 0;
}

static int airphy_cable_step4(struct phy_device *phydev, struct air_cable_test_rsl *cable_rsl)
{
	int reg = 0, status;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev), test_pair = 0;
	unsigned int pbus_value = 0, cable_len, cable_info;
	int ret = 0;
	static const char * const pair_str[] = {"A", "B", "C", "D"};

	msleep(1000);
	ret |= air_mii_cl45_write(phydev, 0x1e, 0x800e, 0x1100);
	ret |= air_mii_cl45_write(phydev, 0x1e, 0x800f, 0xFA);
	ret |= air_wait_md32_fw(phydev);
	if (ret < 0) {
		dev_err(dev,
			"%s: cable step4 fail!\n", __func__);
		return -EPERM;
	}
	ret |= air_mii_cl22_write(mbus, addr, 0x1f, 0x0);
	reg = ((air_mii_cl22_read(mbus, addr, 0x11)  & BIT(12)) >> 12);
	for (test_pair = 0; test_pair < 4; test_pair++) {
		cable_info = air_buckpbus_reg_read(phydev, 0x11e0 + test_pair * 4);
		dev_dbg(dev, "pair%s cable_info(0x%x) 0x%x\n",
					pair_str[test_pair], 0x11e0 + test_pair * 4, pbus_value);
		cable_len = cable_info & 0xffff;
		status = cable_info >> 16;
		if (!status) {
			dev_dbg(dev, "Pair %d, term=%d, len=%d\n",
					test_pair, status, cable_len);
		} else {
			cable_rsl->status[test_pair] = status;
			cable_rsl->length[test_pair] = cable_len;
			dev_dbg(dev, "2.pair %s, status %d, cable_len %d\n",
				pair_str[test_pair], cable_rsl->status[test_pair],
				cable_rsl->length[test_pair]);
		}
	}
	if (!reg) {
		dev_dbg(dev, "%s: air_cable_pair_swap\n", __func__);
		air_cable_pair_swap(AIR_PORT_CABLE_TEST_PAIR_A,
				AIR_PORT_CABLE_TEST_PAIR_B, cable_rsl);
		air_cable_pair_swap(AIR_PORT_CABLE_TEST_PAIR_C,
				AIR_PORT_CABLE_TEST_PAIR_D, cable_rsl);
	}
	ret |= air_mii_cl45_write(phydev, 0x1E, 0x800D, 0x0);
	if (ret < 0)
		return ret;
	dev_dbg(dev, "%s successfull\n", __func__);
	return 0;
}

int airphy_cable_step1(struct phy_device *phydev)
{
	int ret = 0, pair, len = 0, reg, retry;
	int max_len = CMD_MAX_LENGTH;
	static const char * const link_str[] = {"X", "100M", "1G", "2.5G", "LinkDown"};
	struct device *dev = phydev_dev(phydev);
	unsigned int pbus_value = 0;
	struct air_cable_test_rsl cable_rsl = {0};
	char str_out[CMD_MAX_LENGTH] = {0};
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev), status;
	struct en8811h_priv *priv = phydev->priv;

	pbus_value = air_buckpbus_reg_read(phydev, 0x3b3c);
	dev_info(dev, "MD32 FW Version: %x\n", pbus_value);
	msleep(1000);
	ret |= air_mii_cl45_write(phydev, 0x1E, 0x800e, 0x1100);
	ret |= air_mii_cl45_write(phydev, 0x1E, 0x800f, 0xd6);
	if (ret < 0)
		return ret;
	retry = 5;
	do {
		status = air_wait_md32_fw(phydev);
		if (!retry) {
			dev_info(dev, "%s: md32 fw is not ready(%d)", __func__, status);
			return -EPERM;
		}
		retry--;
	} while (status);
	pbus_value = air_buckpbus_reg_read(phydev, 0x11fc);
	if ((pbus_value == 1) || (pbus_value == 3)) {
		dev_info(dev, "%s -> No Support!\n",
					link_str[pbus_value]);
	} else if (pbus_value == 2) {
		dev_info(dev, "%s - Link-Up Mode!\n",
					link_str[pbus_value]);
	} else if (pbus_value == 4) {
		dev_info(dev, "Link-Down Mode!\n");
	}
	if ((pbus_value == 2) || (pbus_value == 4)) {
		ret = airphy_cable_step2(phydev);
		if (ret < 0)
			goto phy_reset;
		ret = airphy_cable_step4(phydev, &cable_rsl);
		if (ret < 0)
			goto phy_reset;
	}
	dev_info(dev, "%7s %15s %15s %15s\n", "pair-a", "pair-b", "pair-c", "pair-d");
	dev_info(dev, "%7s %7s %7s %7s %7s %7s %7s %7s\n",
	"status", "length", "status", "length", "status", "length", "status", "length");
	for (pair = 0; pair < 4; pair++) {
		if (cable_rsl.status[pair] == AIR_PORT_CABLE_STATUS_ERROR)
			len += snprintf(str_out + len, max_len - len, "%7s", "error");
		else if (cable_rsl.status[pair] == AIR_PORT_CABLE_STATUS_OPEN)
			len += snprintf(str_out + len, max_len - len, "%7s", " open");
		else if (cable_rsl.status[pair] == AIR_PORT_CABLE_STATUS_SHORT)
			len += snprintf(str_out + len, max_len - len, "%7s", " short");
		else if (cable_rsl.status[pair] == AIR_PORT_CABLE_STATUS_NORMAL)
			len += snprintf(str_out + len, max_len - len, "%7s", "normal");

		len += snprintf(str_out + len, max_len - len, "  %3d.%dm ",
			cable_rsl.length[pair] / 10, cable_rsl.length[pair] % 10);
		priv->pair[pair] = cable_rsl.length[pair];
	}
	dev_info(dev, "%s", str_out);
	dev_info(dev, "%s air_cable_diag sucessfull.\n", __func__);

	return 0;
phy_reset:
	dev_info(dev, "%s air_cable_diag fail.\n", __func__);
	dev_info(dev, "%s phy_reset.\n", __func__);
	ret |= air_mii_cl22_write(mbus, addr, 0x1f, 0x0);
	reg = air_mii_cl22_read(mbus,
			addr, MII_BMCR);
	ret |= air_mii_cl22_write(mbus,
			addr, MII_BMCR, reg | BMCR_PDOWN);
	ret |= air_mii_cl22_write(mbus,
			addr, MII_BMCR, reg & ~BMCR_PDOWN);
	if (ret < 0)
		return ret;
	msleep(1000);
	dev_dbg(dev, "%s Power Down -> Power UP.\n", __func__);

	return 0;
}

static void
air_token_ring_write(
	struct phy_device *phydev, struct trrg_s TrRG, unsigned int WrtVal)
{
	int data;
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);
	unsigned int TmpVal;
	struct hal_tr_data_s TR_DATA;

	mutex_lock(&mbus->mdio_lock);
	data = __air_mii_cl45_read(phydev, 0x1e, 0x148);
	AIR_EN8811H_SET_VALUE(data, 1, 9, 1);
	__air_mii_cl45_write(phydev, 0x1e, 0x148, data);
	/* change page to 0x52b5 */
	__air_mii_cl22_write(mbus, addr, 0x1F, 0x52B5);

	/* write addr */
	TrRG.TrRGDesc.param.WR_RD_CTRL = 1;
	__air_mii_cl22_write(mbus, addr, 0x10, TrRG.TrRGDesc.Raw[1]);
	/* read data */
	TR_DATA.data_hi = __air_mii_cl22_read(mbus, addr, 0x12);
	TR_DATA.data_lo = __air_mii_cl22_read(mbus, addr, 0x11);
	TmpVal = ~TrRG.RgMask &
			(((unsigned int)TR_DATA.data_hi)<<16 | (unsigned int)TR_DATA.data_lo);

	TrRG.TrRGDesc.param.WR_RD_CTRL = 0;
	WrtVal = (WrtVal<<TrRG.TrRGDesc.param.TrRG_LSB) | (TmpVal & ~TrRG.RgMask);
	/* write data */
	__air_mii_cl22_write(mbus, addr, 0x12, (WrtVal>>16) & 0x00FF);
	__air_mii_cl22_write(mbus, addr, 0x11, WrtVal & 0xFFFF);
	__air_mii_cl22_write(mbus, addr, 0x10, TrRG.TrRGDesc.Raw[1]);

	/* change page to 0 */
	__air_mii_cl22_write(mbus, addr, 0x1F, 0x0);

	data = __air_mii_cl45_read(phydev, 0x1E, 0x148);
	AIR_EN8811H_SET_VALUE(data, 0, 0x9, 0x1);
	__air_mii_cl45_write(phydev, 0x1E, 0x148, data);
	mutex_unlock(&mbus->mdio_lock);
}


void airphy_trigger_cable_diag(struct phy_device *phydev)
{
	struct en8811h_priv *priv = phydev->priv;

	priv->running_status = 1;
	air_token_ring_write(phydev, _fldRW_tr_reg__EcVarTrainingTime_ECNC_C8h, 0x2);
	air_token_ring_write(phydev, _fldRW_tr_reg__EcVarTrainingGain_ECNC_C8h, 0x0);
	airphy_cable_step1(phydev);
	air_token_ring_write(phydev, _fldRW_tr_reg__EcVarTrainingTime_ECNC_C8h, 0xf4);
	air_token_ring_write(phydev, _fldRW_tr_reg__EcVarTrainingGain_ECNC_C8h, 0x1);
	priv->running_status = 0;
}

void airphy_dump_cable_diag(struct phy_device *phydev)
{
	u32 PMEM_addr = 0;
	u32 PMEM_value = 0;
	short EC_COEF;
	u8 pair, Test_Cnt;
	struct device *dev = phydev_dev(phydev);

	dev_info(dev, "Cable Diag EC Dump\n");
	for (pair = 0; pair < 4; pair++) {
		for (Test_Cnt = 0; Test_Cnt < 240; Test_Cnt++) { /* EC_FULL_TAPS = 240*/
			PMEM_addr = ((0x118000 + ((Test_Cnt >> 1) * 4)) + (0x200 * pair));
			PMEM_value = air_buckpbus_reg_read(phydev, PMEM_addr);
			EC_COEF = 0;
			if ((Test_Cnt%2) == 0)
				EC_COEF = (short)(PMEM_value & 0xffff);
			else
				EC_COEF = (short)((PMEM_value >> 16) & 0xffff);

			dev_info(dev, "Pair%d: EC_COEF=  %d\n", pair, EC_COEF);
		}
	}
	dev_info(dev, "Cable Diag EC Dump Finish\n");
}

static void airphy_cable_diag_help(void)
{
	pr_notice("\nUsage:\n"
			"[debugfs] = /sys/kernel/debug/mdio-bus\':[phy_addr]\n"
			"echo start > /[debugfs]/cable_diag\n");
}

static ssize_t airphy_cable_diag(struct file *file, const char __user *ptr,
					size_t len, loff_t *off)
{
	struct phy_device *phydev = file->private_data;
	char buf[32], cmd[32];
	int count = len;
	int num = 0;

	memset(buf, 0, 32);
	memset(cmd, 0, 32);

	if (count > sizeof(buf) - 1)
		return -EINVAL;
	if (copy_from_user(buf, ptr, len))
		return -EFAULT;

	num = sscanf(buf, "%8s", cmd);
	if (num != 1)
		return -EFAULT;

	if (!strncmp("start", cmd, strlen("start")))
		airphy_trigger_cable_diag(phydev);
	else if (!strncmp("dump", cmd, strlen("dump")))
		airphy_dump_cable_diag(phydev);
	else
		airphy_cable_diag_help();

	return count;
}

static int air_set_forcexbz(struct phy_device *phydev)
{
	int rv = 0;
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);

	rv = air_mii_cl22_write(mbus, addr, MII_BMCR, 0x1140);
	rv |= air_buckpbus_reg_write(phydev, 0x10204, 0x0);
	rv |= air_mii_cl45_write(phydev, 0x1e, 0x800c, 0x8);
	rv |= air_mii_cl45_write(phydev, 0x1e, 0x800d, 0x0);
	rv |= air_mii_cl45_write(phydev, 0x1e, 0x800e, 0x1100);
	rv |= air_mii_cl45_write(phydev, 0x1e, 0x800f, 0x1);
	if (unlikely(rv < 0))
		return rv;
	return 0;
}

static int air_set_tx_comp(struct phy_device *phydev, int tm_mode)
{
	int ret = 0;
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);
	int u16dat = 0;

	switch (tm_mode) {
	case AIR_TX_COMP_MODE_1000M_TM1:
	case AIR_TX_COMP_MODE_1000M_TM2:
	case AIR_TX_COMP_MODE_1000M_TM3:
	case AIR_TX_COMP_MODE_1000M_TM4_TD:
	case AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_A:
	case AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_B:
	case AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_C:
	case AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_D:
		ret = air_mii_cl22_write(mbus, addr, MII_BMCR,
				BMCR_ANRESTART | BMCR_SPEED1000 | BMCR_FULLDPLX);
		if (unlikely(ret < 0))
			break;
		if (tm_mode == AIR_TX_COMP_MODE_1000M_TM1) {
			u16dat = (CTL1000_TEST_TM1 | CTL1000_PORT_TYPE |
						ADVERTISE_1000FULL | ADVERTISE_1000HALF);
			pr_notice("Tx Compliance 1000M Test mode 1\n");
		} else if (tm_mode == AIR_TX_COMP_MODE_1000M_TM2) {
			u16dat = (CTL1000_TEST_TM2 | CTL1000_PORT_TYPE |
						ADVERTISE_1000FULL | ADVERTISE_1000HALF);
			pr_notice("Tx Compliance 1000M Test mode 2\n");
		} else if (tm_mode == AIR_TX_COMP_MODE_1000M_TM3) {
			u16dat = (CTL1000_TEST_TM3 | CTL1000_PORT_TYPE |
						ADVERTISE_1000FULL | ADVERTISE_1000HALF);
			pr_notice("Tx Compliance 1000M Test mode 3\n");
		} else
			u16dat = (CTL1000_TEST_TM4 | CTL1000_PORT_TYPE |
						ADVERTISE_1000FULL | ADVERTISE_1000HALF);

		ret = air_mii_cl22_write(mbus, addr, MII_CTRL1000, u16dat);
		if (unlikely(ret < 0))
			break;
		/* delay 1s */
		mdelay(1000);
		ret = air_buckpbus_reg_write(phydev, 0x1e0228, 0x0);
		ret |= air_mii_cl22_write(mbus, addr, 0x1F, 0x0);
		if (unlikely(ret < 0))
			break;
		if (tm_mode == AIR_TX_COMP_MODE_1000M_TM4_TD ||
			tm_mode == AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_A ||
			tm_mode == AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_B ||
			tm_mode == AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_C ||
			tm_mode == AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_D) {
			if (tm_mode == AIR_TX_COMP_MODE_1000M_TM4_TD) {
				u16dat = 0xf;
				pr_notice("Tx Compliance 1000M Test mode 4\n");
			} else if (tm_mode == AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_A) {
				u16dat = 0x1;
				pr_notice("Tx Compliance 1000M Test mode 4 PairA\n");
			} else if (tm_mode == AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_B) {
				u16dat = 0x2;
				pr_notice("Tx Compliance 1000M Test mode 4 PairB\n");
			} else if (tm_mode == AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_C) {
				u16dat = 0x4;
				pr_notice("Tx Compliance 1000M Test mode 4 PairC\n");
			} else if (tm_mode == AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_D) {
				u16dat = 0x8;
				pr_notice("Tx Compliance 1000M Test mode 4 PairD\n");
			}
			ret = air_buckpbus_reg_write(phydev, 0x3a20, u16dat);
			if (unlikely(ret < 0))
				break;
			ret = air_mii_cl22_write(mbus, addr, 0x1F, 0x0);
			if (unlikely(ret < 0))
				break;
		}

		ret = air_mii_cl45_write(phydev, MMD_DEV_VSPEC1, 0x145, 0x1010);
		if (unlikely(ret < 0))
			break;
		if (tm_mode == AIR_TX_COMP_MODE_1000M_TM3) {
			ret = air_mii_cl45_write(phydev, MMD_DEV_VSPEC1, 0x143, 0x200);
			if (unlikely(ret < 0))
				break;
			pr_notice("Tx Compliance 1000M Test mode 3\n");
		}
		break;
	case AIR_TX_COMP_MODE_100M_PAIR_A:
	case AIR_TX_COMP_MODE_100M_PAIR_A_DISCRETE:
		ret = air_mii_cl22_write(mbus, addr, MII_BMCR, (BMCR_SPEED100 | BMCR_FULLDPLX));
		ret |= air_mii_cl45_write(phydev, MMD_DEV_VSPEC1, 0x145, 0x5010);
		if (unlikely(ret < 0))
			break;
		/* delay 1s */
		mdelay(1000);
		ret = air_buckpbus_reg_write(phydev, 0x1e0228, 0x0);
		ret |= air_mii_cl22_write(mbus, addr, 0x1F, 0x0);
		if (unlikely(ret < 0))
			break;
		pr_notice("Tx Compliance 100M PairA\n");
		break;
	case AIR_TX_COMP_MODE_100M_PAIR_B:
	case AIR_TX_COMP_MODE_100M_PAIR_B_DISCRETE:
		ret = air_mii_cl22_write(mbus, addr, MII_BMCR, (BMCR_SPEED100 | BMCR_FULLDPLX));
		ret |= air_mii_cl45_write(phydev, MMD_DEV_VSPEC1, 0x145, 0x5018);
		if (unlikely(ret < 0))
			break;
		/* delay 1s */
		mdelay(1000);
		ret = air_buckpbus_reg_write(phydev, 0x1e0228, 0x0);
		ret |= air_mii_cl22_write(mbus, addr, 0x1F, 0x0);
		if (unlikely(ret < 0))
			break;
		pr_notice("Tx Compliance 100M PairB\n");
		break;
	case AIR_TX_COMP_MODE_2500M_TM1:
		ret = air_set_forcexbz(phydev);
		if (unlikely(ret < 0))
			break;
		ret = air_buckpbus_reg_write(phydev, 0x30008, 0x1000007);
		ret |= air_buckpbus_reg_write(phydev, 0x30200, 0x8f601101);
		ret |= air_buckpbus_reg_write(phydev, 0x30004, 0x112101);
		if (unlikely(ret < 0))
			break;
		pr_notice("Tx Compliance 2500M Test mode 1\n");
		break;
	case AIR_TX_COMP_MODE_2500M_TM2:
		ret = air_set_forcexbz(phydev);
		if (unlikely(ret < 0))
			break;
		ret = air_buckpbus_reg_write(phydev, 0x30200, 0x8c611101);
		ret |= air_buckpbus_reg_write(phydev, 0x30004, 0x122101);
		ret |= air_buckpbus_reg_write(phydev, 0x30200, 0x8c601101);
		if (unlikely(ret < 0))
			break;
		pr_notice("Tx Compliance 2500M Test mode 2\n");
		break;
	case AIR_TX_COMP_MODE_2500M_TM3:
		ret = air_set_forcexbz(phydev);
		if (unlikely(ret < 0))
			break;
		ret = air_buckpbus_reg_write(phydev, 0x30200, 0x8c611101);
		ret |= air_buckpbus_reg_write(phydev, 0x30200, 0x89611101);
		ret |= air_buckpbus_reg_write(phydev, 0x30004, 0x132101);
		ret |= air_buckpbus_reg_write(phydev, 0x85024, 0x0);
		ret |= air_buckpbus_reg_write(phydev, 0x30200, 0x89601101);
		if (unlikely(ret < 0))
			break;
		/* delay 1s */
		mdelay(1000);
		ret = air_buckpbus_reg_write(phydev, 0x10608, 0x808);
		if (unlikely(ret < 0))
			break;
		pr_notice("Tx Compliance 2500M Test mode 3\n");
		break;
	case AIR_TX_COMP_MODE_2500M_TM4_TONE_1:
		ret = air_set_forcexbz(phydev);
		if (unlikely(ret < 0))
			break;
		ret = air_buckpbus_reg_write(phydev, 0x30200, 0x8c611101);
		ret |= air_buckpbus_reg_write(phydev, 0x30004, 0x142101);
		ret |= air_buckpbus_reg_write(phydev, 0x30200, 0x8c601101);
		ret |= air_buckpbus_reg_write(phydev, 0x3089c, 0x1ff);
		if (unlikely(ret < 0))
			break;
		pr_notice("Tx Compliance 2500M Test mode 4 Tone 1\n");
		break;
	case AIR_TX_COMP_MODE_2500M_TM4_TONE_2:
		ret = air_set_forcexbz(phydev);
		if (unlikely(ret < 0))
			break;
		ret = air_buckpbus_reg_write(phydev, 0x30200, 0x8c611101);
		ret |= air_buckpbus_reg_write(phydev, 0x30004, 0x242101);
		ret |= air_buckpbus_reg_write(phydev, 0x30200, 0x8c601101);
		ret |= air_buckpbus_reg_write(phydev, 0x3089c, 0x1ff);
		if (unlikely(ret < 0))
			break;
		pr_notice("Tx Compliance 2500M Test mode 4 Tone 2\n");
		break;
	case AIR_TX_COMP_MODE_2500M_TM4_TONE_3:
		ret = air_set_forcexbz(phydev);
		if (unlikely(ret < 0))
			break;
		ret = air_buckpbus_reg_write(phydev, 0x30200, 0x8c611101);
		ret |= air_buckpbus_reg_write(phydev, 0x30004, 0x442101);
		ret |= air_buckpbus_reg_write(phydev, 0x30200, 0x8c601101);
		ret |= air_buckpbus_reg_write(phydev, 0x3089c, 0x1ff);
		if (unlikely(ret < 0))
			break;
		pr_notice("Tx Compliance 2500M Test mode 4 Tone 3\n");
		break;
	case AIR_TX_COMP_MODE_2500M_TM4_TONE_4:
		ret = air_set_forcexbz(phydev);
		if (unlikely(ret < 0))
			break;
		ret = air_buckpbus_reg_write(phydev, 0x30200, 0x8c611101);
		ret |= air_buckpbus_reg_write(phydev, 0x30004, 0x542101);
		ret |= air_buckpbus_reg_write(phydev, 0x30200, 0x8c601101);
		ret |= air_buckpbus_reg_write(phydev, 0x3089c, 0x1ff);
		if (unlikely(ret < 0))
			break;
		pr_notice("Tx Compliance 2500M Test mode 4 Tone 4\n");
		break;
	case AIR_TX_COMP_MODE_2500M_TM4_TONE_5:
		ret = air_set_forcexbz(phydev);
		if (unlikely(ret < 0))
			break;
		ret = air_buckpbus_reg_write(phydev, 0x30200, 0x8c611101);
		ret |= air_buckpbus_reg_write(phydev, 0x30004, 0x642101);
		ret |= air_buckpbus_reg_write(phydev, 0x30200, 0x8c601101);
		ret |= air_buckpbus_reg_write(phydev, 0x3089c, 0x1ff);
		if (unlikely(ret < 0))
			break;
		pr_notice("Tx Compliance 2500M Test mode 4 Tone 5\n");
		break;
	case AIR_TX_COMP_MODE_2500M_TM5:
		ret = air_set_forcexbz(phydev);
		if (unlikely(ret < 0))
			break;
		ret = air_buckpbus_reg_write(phydev, 0x30200, 0x8c611101);
		ret |= air_buckpbus_reg_write(phydev, 0x30004, 0x152101);
		ret |= air_buckpbus_reg_write(phydev, 0x30200, 0x8c601101);
		ret |= air_buckpbus_reg_write(phydev, 0x30080, 0xc000006);
		ret |= air_buckpbus_reg_write(phydev, 0x30898, 0x1ff01d8);
		if (unlikely(ret < 0))
			break;
				pr_notice("Tx Compliance 2500M Test mode 5\n");
		break;
	case AIR_TX_COMP_MODE_2500M_TM6:
		ret = air_set_forcexbz(phydev);
		if (unlikely(ret < 0))
			break;
		ret = air_buckpbus_reg_write(phydev, 0x30200, 0x8c611101);
		ret |= air_buckpbus_reg_write(phydev, 0x30004, 0x162101);
		ret |= air_buckpbus_reg_write(phydev, 0x30200, 0x8c601101);
		if (unlikely(ret < 0))
			break;
		pr_notice("Tx Compliance 2500M Test mode 6\n");
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static void air_set_tx_comp_help(void)
{
	pr_notice("\nUsage:\n"
			"[debugfs] = /sys/kernel/debug/mdio-bus\':[phy_addr]\n"
			"echo [mode] [para] > /[debugfs]/tx_comp\n"
			"echo 2500-tm1 > /[debugfs]/tx_comp\n"
			"echo 2500-tm2 > /[debugfs]/tx_comp\n"
			"echo 2500-tm3 > /[debugfs]/tx_comp\n"
			"echo 2500-tm4 tone1 > /[debugfs]/tx_comp\n"
			"echo 2500-tm4 tone2 > /[debugfs]/tx_comp\n"
			"echo 2500-tm4 tone3 > /[debugfs]/tx_comp\n"
			"echo 2500-tm4 tone4 > /[debugfs]/tx_comp\n"
			"echo 2500-tm4 tone5 > /[debugfs]/tx_comp\n"
			"echo 1000-tm1 > /[debugfs]/tx_comp\n"
			"echo 1000-tm2 > /[debugfs]/tx_comp\n"
			"echo 1000-tm3 > /[debugfs]/tx_comp\n"
			"echo 1000-tm4-td > /[debugfs]/tx_comp\n"
			"echo 1000-tm4-cm paira > /[debugfs]/tx_comp\n"
			"echo 1000-tm4-cm pairb > /[debugfs]/tx_comp\n"
			"echo 1000-tm4-cm pairc > /[debugfs]/tx_comp\n"
			"echo 1000-tm4-cm paird > /[debugfs]/tx_comp\n");
	pr_notice("echo 100-tm paira > /[debugfs]/tx_comp\n"
			"echo 100-tm pairb > /[debugfs]/tx_comp\n");
}

static ssize_t airphy_tx_compliance(struct file *file, const char __user *ptr,
					size_t len, loff_t *off)
{
	struct phy_device *phydev = file->private_data;
	char buf[32], cmd[32], param[32];
	int count = len, ret = 0;
	int num = 0;

	memset(buf, 0, 32);
	memset(cmd, 0, 32);
	memset(param, 0, 32);

	if (count > sizeof(buf) - 1)
		return -EINVAL;
	if (copy_from_user(buf, ptr, len))
		return -EFAULT;

	num = sscanf(buf, "%10s %10s", cmd, param);
	if (num < 1 || num > 3)
		return -EFAULT;

	if (!strncmp("2500-tm1", cmd, strlen("2500-tm1")))
		ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_2500M_TM1);
	else if (!strncmp("2500-tm2", cmd, strlen("2500-tm2")))
		ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_2500M_TM2);
	else if (!strncmp("2500-tm3", cmd, strlen("2500-tm3")))
		ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_2500M_TM3);
	else if (!strncmp("2500-tm4", cmd, strlen("2500-tm4"))) {
		if (!strncmp("tone1", param, strlen("tone1")))
			ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_2500M_TM4_TONE_1);
		else if (!strncmp("tone2", param, strlen("tone2")))
			ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_2500M_TM4_TONE_2);
		else if (!strncmp("tone3", param, strlen("tone3")))
			ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_2500M_TM4_TONE_3);
		else if (!strncmp("tone4", param, strlen("tone4")))
			ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_2500M_TM4_TONE_4);
		else if (!strncmp("tone5", param, strlen("tone5")))
			ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_2500M_TM4_TONE_5);
		else
			air_set_tx_comp_help();
	} else if (!strncmp("2500-tm5", cmd, strlen("2500-tm5")))
		ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_2500M_TM5);
	else if (!strncmp("2500-tm6", cmd, strlen("2500-tm6")))
		ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_2500M_TM6);
	else if (!strncmp("1000-tm1", cmd, strlen("1000-tm1")))
		ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_1000M_TM1);
	else if (!strncmp("1000-tm2", cmd, strlen("1000-tm2")))
		ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_1000M_TM2);
	else if (!strncmp("1000-tm3", cmd, strlen("1000-tm3")))
		ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_1000M_TM3);
	else if (!strncmp("1000-tm4", cmd, strlen("1000-tm4-cm"))) {
		if (!strncmp("paira", param, strlen("paira")))
			ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_A);
		else if (!strncmp("pairb", param, strlen("pairb")))
			ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_B);
		else if (!strncmp("pairc", param, strlen("pairc")))
			ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_C);
		else if (!strncmp("paird", param, strlen("paird")))
			ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_1000M_TM4_CM_PAIR_D);
		else
			air_set_tx_comp_help();
	} else if (!strncmp("1000-tm4-td", cmd, strlen("1000-tm4-td")))
		ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_1000M_TM4_TD);
	else if (!strncmp("100-tm", cmd, strlen("100-tm"))) {
		if (!strncmp("paira", param, strlen("paira")))
			ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_100M_PAIR_A);
		else if (!strncmp("pairb", param, strlen("pairb")))
			ret = air_set_tx_comp(phydev, AIR_TX_COMP_MODE_100M_PAIR_B);
		else
			air_set_tx_comp_help();
	} else if (!strncmp("help", cmd, strlen("help"))) {
		air_set_tx_comp_help();
	}

	if (ret < 0)
		return ret;

	return count;
}

static const struct file_operations airphy_lp_speed_fops = {
	.owner = THIS_MODULE,
	.open = airphy_lp_speed_open,
	.read = seq_read,
	.llseek = noop_llseek,
	.release = single_release,
};

static const struct file_operations airphy_info_fops = {
	.owner = THIS_MODULE,
	.open = airphy_info_open,
	.read = seq_read,
	.llseek = noop_llseek,
	.release = single_release,
};

static const struct file_operations airphy_counter_fops = {
	.owner = THIS_MODULE,
	.open = airphy_counter_open,
	.read = seq_read,
	.llseek = noop_llseek,
	.release = single_release,
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
	.release = single_release,
};

static const struct file_operations airphy_dbg_reg_show_fops = {
	.owner = THIS_MODULE,
	.open = airphy_dbg_regs_show_open,
	.read = seq_read,
	.llseek = noop_llseek,
	.release = single_release,
};

static const struct file_operations airphy_temp_fops = {
	.owner = THIS_MODULE,
	.open = airphy_temp_show_open,
	.read = seq_read,
	.llseek = noop_llseek,
	.release = single_release,
};

static const struct file_operations airphy_debugfs_cl22_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.write = airphy_debugfs_cl22,
	.llseek = noop_llseek,
};

static const struct file_operations airphy_debugfs_cl45_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.write = airphy_debugfs_cl45,
	.llseek = noop_llseek,
};

static const struct file_operations airphy_cable_diag_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.write = airphy_cable_diag,
	.llseek = noop_llseek,
};

static const struct file_operations airphy_led_mode_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.write = airphy_led_mode,
	.llseek = noop_llseek,
};

static const struct file_operations airphy_tx_compliance_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.write = airphy_tx_compliance,
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
	debugfs_create_file(DEBUGFS_TEMPERATURE, S_IFREG | 0444,
					dir, phydev,
					&airphy_temp_fops);
	debugfs_create_file(DEBUGFS_LP_SPEED, S_IFREG | 0444,
					dir, phydev,
					&airphy_lp_speed_fops);
	debugfs_create_file(DEBUGFS_MII_CL22_OP, S_IFREG | 0200,
					dir, phydev,
					&airphy_debugfs_cl22_fops);
	debugfs_create_file(DEBUGFS_MII_CL45_OP, S_IFREG | 0200,
					dir, phydev,
					&airphy_debugfs_cl45_fops);
	debugfs_create_file(DEBUGFS_CABLE_DIAG, S_IFREG | 0200,
					dir, phydev,
					&airphy_cable_diag_fops);
	debugfs_create_file(DEBUGFS_LED_MODE, S_IFREG | 0200,
					dir, phydev,
					&airphy_led_mode_fops);
	debugfs_create_file(DEBUGFS_TX_COMP, S_IFREG | 0200,
					dir, phydev,
					&airphy_tx_compliance_fops);
	priv->debugfs_root = dir;
	return ret;
}

void airphy_debugfs_remove(struct phy_device *phydev)
{
	struct en8811h_priv *priv = phydev->priv;

	debugfs_remove_recursive(priv->debugfs_root);
	priv->debugfs_root = NULL;
}
#endif /*CONFIG_AIROHA_EN8811H_PHY_DEBUGFS*/
