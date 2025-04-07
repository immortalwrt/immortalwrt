// SPDX-License-Identifier: GPL-2.0
/*************************************************
 * FILE NAME:  air_en8811h_main.c
 * PURPOSE:
 *      EN8811H PHY Driver for Linux
 * NOTES:
 *
 *  Copyright (C) 2023 Airoha Technology Corp.
 *************************************************/

/* INCLUDE FILE DECLARATIONS
*/
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/phy.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/crc32.h>
#include <linux/debugfs.h>
#include "air_en8811h_api.h"
#include "air_en8811h.h"

MODULE_DESCRIPTION("Airoha EN8811H PHY Drivers");
MODULE_AUTHOR("Airoha");
MODULE_LICENSE("GPL");

/**************************
 * GPIO5  <-> BASE_T_LED0,
 * GPIO4  <-> BASE_T_LED1,
 * GPIO3  <-> BASE_T_LED2,
 **************************/
/* User-defined.B */
/* #define AIR_MD32_FW_CHECK */
/* #define AIR_IVY_LOAD */
#ifdef AIR_IVY_LOAD
/* #define AIR_IVY_CHECK */
#endif
#define AIR_LED_SUPPORT
#ifdef AIR_LED_SUPPORT
static const struct air_base_t_led_cfg led_cfg[3] = {
/********************************************************************
 *Enable,     GPIO,        LED Polarity,    LED ON,     LED Blink
*********************************************************************/
	{1,	 AIR_LED0_GPIO5, AIR_ACTIVE_HIGH, AIR_LED0_ON, AIR_LED0_BLK},
	{1,	 AIR_LED1_GPIO4, AIR_ACTIVE_HIGH, AIR_LED1_ON, AIR_LED1_BLK},
	{1,	 AIR_LED2_GPIO3, AIR_ACTIVE_HIGH, AIR_LED2_ON, AIR_LED2_BLK},
};
static const u16 led_dur = UNIT_LED_BLINK_DURATION << AIR_LED_BLK_DUR_64M;
#endif
/* User-defined.E */

/***********************************************************
 *                  F U N C T I O N S
 ***********************************************************/
#ifdef AIR_MD32_FW_CHECK
static void air_mdio_read_buf(struct phy_device *phydev, unsigned long address,
			const struct firmware *fw, unsigned int *crc32)
{
	unsigned int offset;
	int ret = 0;
	unsigned int pbus_data_low, pbus_data_high;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);
	char *buf = kmalloc(fw->size, GFP_KERNEL);

	if (!buf)
		return -ENOMEM;
	memset(buf, '\0', fw->size);
	/* page 4 */
	ret |= air_mii_cl22_write(mbus, addr, 0x1F, 4);
	/* address increment*/
	ret |= air_mii_cl22_write(mbus, addr, 0x10, 0x8000);
	ret |= air_mii_cl22_write(mbus, addr,
			0x15, (unsigned int)((address >> 16) & 0xffff));
	ret |= air_mii_cl22_write(mbus, addr,
			0x16, (unsigned int)(address & 0xffff));
	for (offset = 0; offset < fw->size; offset += 4) {
		pbus_data_high = air_mii_cl22_read(mbus, addr, 0x17);
		pbus_data_low = air_mii_cl22_read(mbus, addr, 0x18);
		buf[offset + 0] = pbus_data_low & 0xff;
		buf[offset + 1] = (pbus_data_low & 0xff00) >> 8;
		buf[offset + 2] = pbus_data_high & 0xff;
		buf[offset + 3] = (pbus_data_high & 0xff00) >> 8;
	}
	msleep(100);
	*crc32 = ~crc32(~0, buf, fw->size);
	ret |= air_mii_cl22_write(mbus, addr, 0x1F, 0);
	kfree(buf);
	if (ret) {
		dev_info(dev, "%s 0x%lx FAIL(ret:%d)\n",
				__func__, address, ret);
	}
}
#endif
static int air_mdio_write_buf(struct phy_device *phydev,
		unsigned long address, const struct firmware *fw)
{
	unsigned int write_data, offset;
	int ret = 0;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);
	/* page 4 */
	ret = air_mii_cl22_write(mbus, addr, 0x1F, 4);
	if (ret < 0) {
		dev_err(dev, "air_mii_cl22_write, ret: %d\n", ret);
		return ret;
	}
	/* address increment*/
	ret = air_mii_cl22_write(mbus, addr, 0x10, 0x8000);
	if (ret < 0) {
		dev_err(dev, "air_mii_cl22_write, ret: %d\n", ret);
		return ret;
	}
	ret = air_mii_cl22_write(mbus, addr, 0x11,
					(u32)((address >> 16) & 0xffff));
	if (ret < 0) {
		dev_err(dev, "air_mii_cl22_write, ret: %d\n", ret);
		return ret;
	}
	ret = air_mii_cl22_write(mbus, addr, 0x12, (u32)(address & 0xffff));
	if (ret < 0) {
		dev_err(dev, "air_mii_cl22_write, ret: %d\n", ret);
		return ret;
	}

	for (offset = 0; offset < fw->size; offset += 4) {
		write_data = (fw->data[offset + 3] << 8) | fw->data[offset + 2];
		ret = air_mii_cl22_write(mbus, addr, 0x13, write_data);
		if (ret < 0) {
			dev_err(dev, "air_mii_cl22_write, ret: %d\n", ret);
			return ret;
		}
		write_data = (fw->data[offset + 1] << 8) | fw->data[offset];
		ret = air_mii_cl22_write(mbus, addr, 0x14, write_data);
		if (ret < 0) {
			dev_err(dev, "air_mii_cl22_write, ret: %d\n", ret);
			return ret;
		}
	}
	ret = air_mii_cl22_write(mbus, addr, 0x1F, 0);
	if (ret < 0) {
		dev_err(dev, "air_mii_cl22_write, ret: %d\n", ret);
		return ret;
	}
	return 0;
}
#ifdef AIR_IVY_LOAD
static int modify_reg_bits(struct phy_device *phydev,
	unsigned int reg, unsigned int mask, unsigned int set)
{
	unsigned int write_data;
	int ret;

	write_data = air_buckpbus_reg_read(phydev, reg);
	write_data &= ~mask;
	write_data |= set;
	ret = air_buckpbus_reg_write(phydev, reg, write_data);
	if (ret < 0)
		return ret;

	return 0;
}

static int air_mdio_load_ivy(struct phy_device *phydev,
		unsigned long address, const struct firmware *fw)
{
	unsigned int write_data = 0, offset, read_data;
	int ret = 0, retry;
#ifdef AIR_IVY_CHECK
	int error = 0;
#endif
	struct device *dev = phydev_dev(phydev);

	ret = air_buckpbus_reg_write(phydev,
						0xcf924, 0x12);
	if (ret < 0)
		return ret;
	write_data = air_buckpbus_reg_read(phydev, 0xcfa28);
	write_data |= BIT(0);
	ret = air_buckpbus_reg_write(phydev,
					0xcfa28, write_data);
	write_data = air_buckpbus_reg_read(phydev, 0xcfa28);
	if (ret < 0)
		return ret;
	msleep(100);
	for (offset = 0; offset < fw->size; offset += 4) {
		write_data = (fw->data[offset + 3] << 24) | (fw->data[offset + 2] << 16);
		write_data |= ((fw->data[offset + 1] << 8) | fw->data[offset]);
		ret = air_buckpbus_reg_write(phydev,
						address, write_data);
#ifdef AIR_IVY_CHECK
		read_data = air_buckpbus_reg_read(phydev, address);
		if (write_data != read_data) {
			dev_info(dev, "%x: write_data(0x%x) != read_data(0x%x)\n",
				address, write_data, read_data);
			error++;
		}
#endif
		address += 1;
	}
#ifdef AIR_IVY_CHECK
	if (error)
		dev_err(dev, "Check ivy Fail(%d)\n", error);
	else
		dev_err(dev, "Check ivy Pass\n");
#endif
	ret = modify_reg_bits(phydev, 0xCFA28, BIT(0), 0);
	if (ret < 0)
		return ret;

	ret = modify_reg_bits(phydev, 0xCFA28, 0, BIT(16));
	if (ret < 0)
		return ret;
	ret |= air_buckpbus_reg_write(phydev,
					0xDC065, 0x80);
	ret |= air_buckpbus_reg_write(phydev,
					0xDC064, 0x0);
	if (ret < 0)
		return ret;
	retry = 5;
	do {
		msleep(300);
		ret = air_buckpbus_reg_write(phydev,
					0xDC064, 0x0);
		if (ret < 0)
			return ret;
		write_data = air_buckpbus_reg_read(phydev, 0xDC065);
		if (write_data == 0x80)
			break;
		if (!retry)
			dev_err(dev, "0xDC065 is not ready.(0x%x)\n", write_data);
	} while (retry--);

	ret = modify_reg_bits(phydev, 0xCFA28, BIT(16), 0);
	if (ret < 0)
		return ret;
	ret = modify_reg_bits(phydev, 0xCFA28, 0, BIT(24));
	if (ret < 0)
		return ret;
	retry = 10;
	do {
		msleep(300);
		write_data = air_buckpbus_reg_read(phydev, 0xCFA38);
		if (write_data == EN8811H_PHY_IVY_READY) {
			dev_info(dev, "IVY ready!\n");
			break;
		}
		if (!retry)
			dev_err(dev, "IVY is not ready.(0x%x)\n", write_data);
	} while (retry--);
	return 0;
}
#endif
static int en8811h_load_firmware(struct phy_device *phydev)
{
	struct device *dev = phydev_dev(phydev);
	const struct firmware *fw;
	const char *firmware;
	int ret = 0;
	u32 pbus_value = 0;
#ifdef AIR_MD32_FW_CHECK
	unsigned int d_crc32 = 0, crc32 = 0;
	int retry = 0;
#endif
	struct en8811h_priv *priv = phydev->priv;

	priv->init_stage = AIR_INIT_FW_LOADING;
#ifdef AIR_IVY_LOAD
	firmware = EN8811H_IVY;
	ret = request_firmware_direct(&fw, firmware, dev);
	if (ret < 0) {
		dev_err(dev,
			"failed to load firmware %s, ret: %d\n", firmware, ret);
		return ret;
	}
	priv->ivy_crc32 = ~crc32(~0, fw->data, fw->size);
	dev_info(dev, "%s: crc32=0x%x\n",
		firmware, ~crc32(~0, fw->data, fw->size));
	/* Download ivy */
	ret = air_mdio_load_ivy(phydev, 0xd4000, fw);
	release_firmware(fw);
	if (ret < 0) {
		dev_err(dev,
			"air_mdio_write_buf 0xd4000 fail, ret: %d\n", ret);
		goto release;
	}
#endif
	ret = air_buckpbus_reg_write(phydev,
					0x0f0018, 0x0);
	if (ret < 0)
		return ret;
	pbus_value = air_buckpbus_reg_read(phydev, 0x800000);
	pbus_value |= BIT(11);
	ret = air_buckpbus_reg_write(phydev,
					0x800000, pbus_value);
	if (ret < 0)
		return ret;
	firmware = EN8811H_MD32_DM;
	ret = request_firmware_direct(&fw, firmware, dev);
	if (ret < 0) {
		dev_err(dev,
			"failed to load firmware %s, ret: %d\n", firmware, ret);
		return ret;
	}
	priv->dm_crc32 = ~crc32(~0, fw->data, fw->size);
	dev_info(dev, "%s: crc32=0x%x\n",
		firmware, ~crc32(~0, fw->data, fw->size));
	/* Download DM */
	ret = air_mdio_write_buf(phydev, 0x00000000, fw);
	release_firmware(fw);
	if (ret < 0) {
		dev_err(dev,
			"air_mdio_write_buf 0x00000000 fail, ret: %d\n", ret);
		goto release;
	}

	firmware = EN8811H_MD32_DSP;
	ret = request_firmware_direct(&fw, firmware, dev);
	if (ret < 0) {
		dev_info(dev,
			"failed to load firmware %s, ret: %d\n", firmware, ret);
		return ret;
	}
	priv->dsp_crc32 = ~crc32(~0, fw->data, fw->size);
	dev_info(dev, "%s: crc32=0x%x\n",
		firmware, ~crc32(~0, fw->data, fw->size));
	/* Download PM */
	ret = air_mdio_write_buf(phydev, 0x00100000, fw);
	if (ret < 0) {
		dev_err(dev,
			"air_mdio_write_buf 0x00100000 fail , ret: %d\n", ret);
		goto release;
	}
	pbus_value = air_buckpbus_reg_read(phydev, 0x800000);
	pbus_value &= ~BIT(11);
	ret = air_buckpbus_reg_write(phydev, 0x800000, pbus_value);
	if (ret < 0)
		goto release;
#ifdef AIR_MD32_FW_CHECK
	crc32 = ~crc32(~0, fw->data, fw->size);
	/* Check PM */
	air_mdio_read_buf(phydev, 0x100000, fw, &d_crc32);
	if (d_crc32 == crc32)
		dev_info(dev, "0x00100000 Check Sum Pass.\n");
	else {
		dev_info(dev, "0x00100000 Check Sum Fail.\n");
		dev_info(dev, "CRC32 0x%x != Caculated CRC32 0x%x\n",
					crc32, d_crc32);
	}
	release_firmware(fw);
	retry = MAX_RETRY;
	do {
		ret = air_buckpbus_reg_write(phydev, 0x0f0018, 0x01);
		if (ret < 0)
			return ret;
		msleep(100);
		pbus_value = air_buckpbus_reg_read(phydev, 0x0f0018);
		retry--;
		if (retry == 0) {
			dev_err(dev,
				"Release Software Reset fail , ret: %d\n",
						pbus_value);
			goto release;
		}
	} while (pbus_value != 0x1);
	dev_info(dev,
		"Release Software Reset successful.\n");
#else
	release_firmware(fw);
	ret = air_buckpbus_reg_write(phydev, 0x0f0018, 0x01);
	if (ret < 0)
		return ret;
#endif
	return 0;
release:
	release_firmware(fw);
	return ret;
}

static int en8811h_init_up(struct phy_device *phydev)
{
	int ret = 0, retry, reg_value;
	struct device *dev = phydev_dev(phydev);
	unsigned int pbus_value;
	struct en8811h_priv *priv = phydev->priv;

	dev_info(dev, "%s start\n", __func__);
	ret = en8811h_load_firmware(phydev);
	if (ret < 0) {
		dev_err(dev, "EN8811H load firmware fail.\n");
		return ret;
	}
	retry = MAX_RETRY;
	do {
		mdelay(300);
		reg_value = air_mii_cl45_read(phydev, 0x1e, 0x8009);
		if (reg_value == EN8811H_PHY_READY) {
			dev_info(dev, "EN8811H PHY ready!\n");
			priv->init_stage = AIR_INIT_FW_READY;
			break;
		}
		if (retry == 0) {
			dev_err(dev, "MD32 FW is not ready.(Status 0x%x)\n", reg_value);
			pbus_value = air_buckpbus_reg_read(phydev, 0x3b3c);
			dev_err(dev,
				"Check MD32 FW Version(0x3b3c) : %08x\n", pbus_value);
			dev_err(dev,
				"%s fail!\n", __func__);
			priv->init_stage = AIR_INIT_FW_FAIL;
		}
	} while (retry--);
	return 0;
}

#ifdef AIR_LED_SUPPORT
static int airoha_led_set_usr_def(struct phy_device *phydev, u8 entity,
				int polar, u16 on_evt, u16 blk_evt)
{
	int ret = 0;

	if (polar == AIR_ACTIVE_HIGH)
		on_evt |= LED_ON_POL;
	else
		on_evt &= ~LED_ON_POL;

	ret = air_mii_cl45_write(phydev, 0x1f,
			LED_ON_CTRL(entity), on_evt | LED_ON_EN);
	if (ret < 0)
		return ret;
	ret = air_mii_cl45_write(phydev, 0x1f, LED_BLK_CTRL(entity), blk_evt);
	if (ret < 0)
		return ret;
	return 0;
}

static int airoha_led_set_mode(struct phy_device *phydev, u8 mode)
{
	u16 cl45_data;
	int err = 0;
	struct device *dev = phydev_dev(phydev);

	cl45_data = air_mii_cl45_read(phydev, 0x1f, LED_BCR);
	switch (mode) {
	case AIR_LED_MODE_DISABLE:
		cl45_data &= ~LED_BCR_EXT_CTRL;
		cl45_data &= ~LED_BCR_MODE_MASK;
		cl45_data |= LED_BCR_MODE_DISABLE;
		break;
	case AIR_LED_MODE_USER_DEFINE:
		cl45_data |= LED_BCR_EXT_CTRL;
		cl45_data |= LED_BCR_CLK_EN;
		break;
	default:
		dev_err(dev, "LED mode%d is not supported!\n", mode);
		return -EINVAL;
	}
	err = air_mii_cl45_write(phydev, 0x1f, LED_BCR, cl45_data);
	if (err < 0)
		return err;
	return 0;
}

static int airoha_led_set_state(struct phy_device *phydev, u8 entity, u8 state)
{
	u16 cl45_data = 0;
	int err;

	cl45_data = air_mii_cl45_read(phydev, 0x1f, LED_ON_CTRL(entity));
	if (state == 1)
		cl45_data |= LED_ON_EN;
	else
		cl45_data &= ~LED_ON_EN;

	err = air_mii_cl45_write(phydev, 0x1f, LED_ON_CTRL(entity), cl45_data);
	if (err < 0)
		return err;
	return 0;
}

static int en8811h_led_init(struct phy_device *phydev)
{

	unsigned long led_gpio = 0, reg_value = 0;
	u16 cl45_data = led_dur;
	int ret = 0, id;
	struct device *dev = phydev_dev(phydev);
	struct en8811h_priv *priv = phydev->priv;

	ret = air_mii_cl45_write(phydev, 0x1f, LED_BLK_DUR, cl45_data);
	if (ret < 0)
		return ret;
	cl45_data >>= 1;
	ret = air_mii_cl45_write(phydev, 0x1f, LED_ON_DUR, cl45_data);
	if (ret < 0)
		return ret;
	ret = airoha_led_set_mode(phydev, AIR_LED_MODE_USER_DEFINE);
	if (ret != 0) {
		dev_err(dev, "led_set_mode fail(ret:%d)!\n", ret);
		return ret;
	}
	for (id = 0; id < EN8811H_LED_COUNT; id++) {
		/* LED0 <-> GPIO5, LED1 <-> GPIO4, LED0 <-> GPIO3 */
		if (led_cfg[id].gpio != (id + (AIR_LED0_GPIO5 - (2 * id)))) {
			dev_err(dev, "LED%d uses incorrect GPIO%d !\n",
							id, led_cfg[id].gpio);
			return -EINVAL;
		}
		ret = airoha_led_set_state(phydev, id, led_cfg[id].en);
		if (ret != 0) {
			dev_err(dev, "led_set_state fail(ret:%d)!\n", ret);
			return ret;
		}
		if (led_cfg[id].en == 1) {
			led_gpio |= BIT(led_cfg[id].gpio);
			ret = airoha_led_set_usr_def(phydev, id,
				led_cfg[id].pol, led_cfg[id].on_cfg,
				led_cfg[id].blk_cfg);
			if (ret != 0) {
				dev_err(dev, "led_set_usr_def fail!\n");
				return ret;
			}
		}
		priv->on_crtl[id] = air_mii_cl45_read(phydev, 0x1f,
				LED_ON_CTRL(id));
		priv->blk_crtl[id] = air_mii_cl45_read(phydev, 0x1f,
				LED_BLK_CTRL(id));
	}
	reg_value = air_buckpbus_reg_read(phydev, 0xcf8b8) | led_gpio;
	ret = air_buckpbus_reg_write(phydev, 0xcf8b8, reg_value);
	if (ret < 0)
		return ret;
	dev_info(dev, "LED initialize OK !\n");
	return 0;
}
#endif /* AIR_LED_SUPPORT */

#if (KERNEL_VERSION(4, 5, 0) < LINUX_VERSION_CODE)
static int en8811h_get_features(struct phy_device *phydev)
{
	int ret;
	struct device *dev = phydev_dev(phydev);

	dev_dbg(dev, "%s()\n", __func__);
	ret = air_pbus_reg_write(phydev, 0xcf928, 0x0);
	if (ret < 0)
		return ret;
	ret = genphy_read_abilities(phydev);
	if (ret)
		return ret;
	/* EN8811H supports 100M/1G/2.5G speed. */
	linkmode_clear_bit(ETHTOOL_LINK_MODE_10baseT_Half_BIT,
			phydev->supported);
	linkmode_clear_bit(ETHTOOL_LINK_MODE_10baseT_Full_BIT,
			phydev->supported);
	linkmode_clear_bit(ETHTOOL_LINK_MODE_100baseT_Half_BIT,
			phydev->supported);
	linkmode_clear_bit(ETHTOOL_LINK_MODE_1000baseT_Half_BIT,
			phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_100baseT_Full_BIT,
			phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
			phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
			phydev->supported);
	return 0;
}
#endif

static int en8811h_probe(struct phy_device *phydev)
{
	int ret = 0;
	int pid1 = 0, pid2 = 0;
	u32 pbus_value = 0;
	struct device *dev = phydev_dev(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);
	struct en8811h_priv *priv;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	phydev->priv = priv;
	ret = air_pbus_reg_write(phydev, 0xcf928, 0x0);
	if (ret < 0)
		goto priv_free;

	pid1 = air_mii_cl22_read(mbus, addr, MII_PHYSID1);
	pid2 = air_mii_cl22_read(mbus, addr, MII_PHYSID2);
	dev_info(dev, "PHY = %x - %x\n", pid1, pid2);
	if ((pid1 != EN8811H_PHY_ID1) || (pid2 != EN8811H_PHY_ID2)) {
		dev_err(dev, "EN8811H dose not exist!!\n");
		goto priv_free;
	}

	priv->init_stage = AIR_INIT_START;
	ret = air_buckpbus_reg_write(phydev, 0x1e00d0, 0xf);
	ret |= air_buckpbus_reg_write(phydev, 0x1e0228, 0xf0);
	if (ret < 0)
		goto priv_free;

	priv->mcu_needs_restart = false;

	pbus_value = air_buckpbus_reg_read(phydev, 0xcf914);
	dev_info(dev, "Bootmode: %s\n",
			(GET_BIT(pbus_value, 24) ? "Flash" : "Download Code"));
	ret = en8811h_of_init(phydev);
	if (ret < 0)
		goto priv_free;

#ifdef CONFIG_AIROHA_EN8811H_PHY_DEBUGFS
	ret = airphy_debugfs_init(phydev);
	if (ret < 0) {
		dev_err(dev, "air_debug_procfs_init fail. (ret=%d)\n", ret);
		airphy_debugfs_remove(phydev);
		goto priv_free;
	}
#endif /* CONFIG_AIROHA_EN8811H_PHY_DEBUGFS */

	if (priv->phy_handle) {
		dev_info(dev, "EN8811H Probe OK! (%s)\n", EN8811H_DRIVER_VERSION);
	} else {
		ret = en8811h_init_up(phydev);
		if (ret < 0)
			goto priv_free;

		priv->init_stage = AIR_INIT_CONFIG;
		ret = air_mii_cl45_write(phydev, 0x1e, 0x800c, 0x0);
		ret |= air_mii_cl45_write(phydev, 0x1e, 0x800d, 0x0);
		ret |= air_mii_cl45_write(phydev, 0x1e, 0x800e, 0x1101);
		ret |= air_mii_cl45_write(phydev, 0x1e, 0x800f, 0x0002);
		if (ret < 0)
			goto priv_free;

		pbus_value = air_buckpbus_reg_read(phydev, 0xca0f8);
		pbus_value &= ~0x3;
#if defined(CONFIG_OF)
		pbus_value |= priv->pol;
#else
		pbus_value |= (EN8811H_RX_POL_NORMAL | EN8811H_TX_POL_NORMAL);
#endif
		ret = air_buckpbus_reg_write(phydev, 0xca0f8, pbus_value);
		if (ret < 0)
			goto priv_free;

		pbus_value = air_buckpbus_reg_read(phydev, 0xca0f8);
		dev_info(dev, "Tx, Rx Polarity : %08x\n", pbus_value);
		priv->firmware_version = air_buckpbus_reg_read(phydev, 0x3b3c);
		dev_info(dev, "MD32 FW Version : %08x\n", priv->firmware_version);
		ret = air_surge_protect_cfg(phydev);
		if (ret < 0) {
			dev_err(dev,
				"air_surge_protect_cfg fail. (ret=%d)\n", ret);
			goto priv_free;
		}

		ret = air_cko_cfg(phydev);
		if (ret < 0) {
			dev_err(dev,
				"air_cko_cfg fail. (ret=%d)\n", ret);
			goto priv_free;
		}

#if defined(AIR_LED_SUPPORT)
		ret = en8811h_led_init(phydev);
		if (ret < 0) {
			dev_err(dev, "en8811h_led_init fail. (ret=%d)\n", ret);
			goto priv_free;
		}
#endif

		priv->init_stage = AIR_INIT_SUCESS;
		dev_info(dev, "EN8811H initialize OK! (%s)\n", EN8811H_DRIVER_VERSION);
	}
	return 0;
priv_free:
	kfree(priv);
	return ret;
}

void en8811h_remove(struct phy_device *phydev)
{

	struct en8811h_priv *priv = phydev->priv;
	struct device *dev = phydev_dev(phydev);

	dev_dbg(dev, "%s: start\n", __func__);
	if (priv) {
		dev_info(dev, "%s: airphy_debugfs_remove\n", __func__);
#ifdef CONFIG_AIROHA_EN8811H_PHY_DEBUGFS
		airphy_debugfs_remove(phydev);
#endif /*CONFIG_AIROHA_EN8811H_PHY_DEBUGFS*/
		kfree(priv);
	}
}

static int en8811h_restart_up(struct phy_device *phydev)
{
	int ret, retry, reg_value;
	u32 pbus_value;
	struct device *dev = phydev_dev(phydev);
	struct en8811h_priv *priv = phydev->priv;

	dev_info(dev, "%s start\n", __func__);
	ret = air_mii_cl45_write(phydev, 0x1e, 0x8009, 0x0);
	if (ret < 0)
		return ret;
	ret = air_buckpbus_reg_write(phydev, EN8811H_FW_CTRL_1,
				     EN8811H_FW_CTRL_1_START);
	if (ret < 0)
		return ret;
	ret = air_buckpbus_reg_write(phydev, EN8811H_FW_CTRL_1,
				     EN8811H_FW_CTRL_1_FINISH);
	if (ret < 0)
		return ret;
	retry = MAX_RETRY;
	do {
		mdelay(300);
		reg_value = air_mii_cl45_read(phydev, 0x1e, 0x8009);
		if (reg_value == EN8811H_PHY_READY) {
			priv->init_stage = AIR_INIT_FW_READY;
			dev_info(dev, "EN8811H PHY ready!\n");
			break;
		}
		if (retry == 0) {
			dev_err(dev, "MD32 FW is not ready.(Status 0x%x)\n", reg_value);
			pbus_value = air_buckpbus_reg_read(phydev, 0x3b3c);
			dev_err(dev,
				"Check MD32 FW Version(0x3b3c) : %08x\n", pbus_value);
			dev_err(dev,
				"%s fail!\n", __func__);
			priv->init_stage = AIR_INIT_FW_FAIL;
		}
	} while (retry--);
	return 0;
}

static int en8811h_config_init(struct phy_device *phydev)
{
	int ret = 0;
	u32 pbus_value = 0;
	struct device *dev = phydev_dev(phydev);
	struct en8811h_priv *priv = phydev->priv;

	ret = air_buckpbus_reg_write(phydev, 0x1e00d0, 0xf);
	ret |= air_buckpbus_reg_write(phydev, 0x1e0228, 0xf0);
	if (ret < 0)
		return ret;

	/* If restart happened in .probe(), no need to restart now */
	if (priv->mcu_needs_restart) {
		ret = en8811h_restart_up(phydev);
		if (ret < 0)
			goto priv_free;
	} else {
		ret = en8811h_init_up(phydev);
		if (ret < 0)
			goto priv_free;
		/* Next calls to .config_init() mcu needs to restart */
		priv->mcu_needs_restart = true;
	}

	priv->init_stage = AIR_INIT_CONFIG;
	ret = air_mii_cl45_write(phydev, 0x1e, 0x800c, 0x0);
	ret |= air_mii_cl45_write(phydev, 0x1e, 0x800d, 0x0);
	ret |= air_mii_cl45_write(phydev, 0x1e, 0x800e, 0x1101);
	ret |= air_mii_cl45_write(phydev, 0x1e, 0x800f, 0x0002);
	if (ret < 0)
		goto priv_free;

	/* Serdes polarity */
	pbus_value = air_buckpbus_reg_read(phydev, 0xca0f8);
	pbus_value &= ~0x3;
#if defined(CONFIG_OF)
	pbus_value |= priv->pol;
#else
	pbus_value |= (EN8811H_RX_POL_NORMAL | EN8811H_TX_POL_NORMAL);
#endif

	ret = air_buckpbus_reg_write(phydev, 0xca0f8, pbus_value);
	if (ret < 0)
		goto priv_free;

	pbus_value = air_buckpbus_reg_read(phydev, 0xca0f8);
	dev_info(dev, "Tx, Rx Polarity : %08x\n", pbus_value);
	priv->firmware_version = air_buckpbus_reg_read(phydev, 0x3b3c);
	dev_info(dev, "MD32 FW Version : %08x\n", priv->firmware_version);

	ret = air_surge_protect_cfg(phydev);
	if (ret < 0) {
		dev_err(dev,
			"air_surge_protect_cfg fail. (ret=%d)\n", ret);
		goto priv_free;
	}

	ret = air_cko_cfg(phydev);
	if (ret < 0) {
		dev_err(dev,
			"air_cko_cfg fail. (ret=%d)\n", ret);
		goto priv_free;
	}

#if defined(AIR_LED_SUPPORT)
	ret = en8811h_led_init(phydev);
	if (ret < 0) {
		dev_err(dev, "en8811h_led_init fail. (ret=%d)\n", ret);
		goto priv_free;
	}
#endif

	priv->init_stage = AIR_INIT_SUCESS;
	dev_info(dev, "EN8811H initialize OK! (%s)\n", EN8811H_DRIVER_VERSION);
	return 0;

priv_free:
	kfree(priv);
	return ret;
}
static int en8811h_get_rate_matching(struct phy_device *phydev,
				    phy_interface_t iface)
{
	return RATE_MATCH_PAUSE;
}

static int en8811h_config_aneg(struct phy_device *phydev)
{
	bool changed = false;
	int err, val;

	val = 0;
	if (linkmode_test_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
			      phydev->advertising))
		val |= MDIO_AN_10GBT_CTRL_ADV2_5G;
	err =  phy_modify_mmd_changed(phydev, MDIO_MMD_AN, MDIO_AN_10GBT_CTRL,
				      MDIO_AN_10GBT_CTRL_ADV2_5G, val);
	if (err < 0)
		return err;
	if (err > 0)
		changed = true;

	return __genphy_config_aneg(phydev, changed);
}

static int en8811h_update_link(struct phy_device *phydev)
{
	int status = 0, bmcr;
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int addr = phydev_addr(phydev);

	bmcr = air_mii_cl22_read(mbus, addr, MII_BMCR);
	if (bmcr < 0)
		return bmcr;
	/* Autoneg is being started, therefore disregard BMSR value and
	 * report link as down.
	 */
	if (bmcr & BMCR_ANRESTART)
		goto done;
	status = air_mii_cl22_read(mbus, addr, MII_BMSR);
	if (status < 0)
		return status;
done:
	phydev->link = status & BMSR_LSTATUS ? 1 : 0;
	phydev->autoneg_complete = status & BMSR_ANEGCOMPLETE ? 1 : 0;
	return 0;
}

static int en8811h_read_status(struct phy_device *phydev)
{
	struct en8811h_priv *priv = phydev->priv;
	u32 pbus_value;
	int old_link = phydev->link, ret;

	ret = en8811h_update_link(phydev);
	if (ret)
		return ret;
	/* why bother the PHY if nothing can have changed */
	if (old_link && phydev->link)
		return 0;
	phydev->speed = SPEED_UNKNOWN;
	phydev->duplex = DUPLEX_UNKNOWN;
	phydev->pause = 0;
	phydev->asym_pause = 0;
	phydev->rate_matching = RATE_MATCH_PAUSE;

	ret = genphy_read_lpa(phydev);
	if (ret < 0)
		return ret;

	/* Get link partner 2.5GBASE-T ability from vendor register */
	pbus_value = air_buckpbus_reg_read(phydev, EN8811H_2P5G_LPA);
	if (ret < 0)
		return ret;
	linkmode_mod_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
			 phydev->lp_advertising,
			 pbus_value & EN8811H_2P5G_LPA_2P5G);

	phydev->duplex = DUPLEX_FULL;
	if (phydev->autoneg_complete)
		phy_resolve_aneg_pause(phydev);

	if (!phydev->link)
		return 0;

	ret = air_ref_clk_speed(phydev, AIR_PARA_PHYDEV);
	if (ret < 0)
		return ret;
	/* Firmware before version 24011202 has no vendor register 2P5G_LPA.
	 * Assume link partner advertised it if connected at 2500Mbps.
	 */
	if (priv->firmware_version < 0x24011202) {
		linkmode_mod_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
				 phydev->lp_advertising,
				 phydev->speed == SPEED_2500);
	}
	if (phydev->speed <= SPEED_1000)
		phydev->pause = 1;
	return 0;
}

static struct phy_driver en8811h_driver[] = {
{
	.phy_id         = EN8811H_PHY_ID,
	.name           = "Airoha EN8811H",
	.phy_id_mask    = 0x0ffffff0,
	.probe          = en8811h_probe,
	.remove         = en8811h_remove,
#if (KERNEL_VERSION(4, 5, 0) < LINUX_VERSION_CODE)
	.get_features   = en8811h_get_features,
	.read_mmd       = __air_mii_cl45_read,
	.write_mmd      = __air_mii_cl45_write,
#endif
	.config_init        = en8811h_config_init,
	.read_status        = en8811h_read_status,
	.get_rate_matching  = en8811h_get_rate_matching,
	.config_aneg        = en8811h_config_aneg,
	.resume             = genphy_resume,
	.suspend            = genphy_suspend,
} };

int __init en8811h_phy_driver_register(void)
{
	int ret;
#if (KERNEL_VERSION(4, 5, 0) > LINUX_VERSION_CODE)
	ret = phy_driver_register(en8811h_driver);
#else
	ret = phy_driver_register(en8811h_driver, THIS_MODULE);
#endif
	if (!ret)
		return 0;

	phy_driver_unregister(en8811h_driver);
	return ret;
}

void __exit en8811h_phy_driver_unregister(void)
{
	phy_driver_unregister(en8811h_driver);
}

module_init(en8811h_phy_driver_register);
module_exit(en8811h_phy_driver_unregister);
