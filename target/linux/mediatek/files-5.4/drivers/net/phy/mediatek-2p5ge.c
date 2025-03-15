// SPDX-License-Identifier: GPL-2.0+
#include <linux/bitfield.h>
#include <linux/firmware.h>
#include <linux/module.h>
#include <linux/nvmem-consumer.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/phy.h>

#define MEDAITEK_2P5GE_PHY_DMB_FW "mediatek-2p5ge-phy-dmb.bin"
#define MEDIATEK_2P5GE_PHY_PMB_FW "mediatek-2p5ge-phy-pmb.bin"

#define MD32_EN_CFG	0x18
#define   MD32_EN	BIT(0)

static int mt798x_2p5ge_phy_config_init(struct phy_device *phydev)
{
	int ret;
	int i;
	const struct firmware *fw;
	struct device *dev = &phydev->mdio.dev;
	struct device_node *np;
	void __iomem *dmb_addr;
	void __iomem *pmb_addr;
	void __iomem *mcucsr_base;
	u16 reg;

	np = of_find_compatible_node(NULL, NULL, "mediatek,2p5gphy-fw");
	if (!np)
		return -ENOENT;

	dmb_addr = of_iomap(np, 0);
	if (!dmb_addr)
		return -ENOMEM;
	pmb_addr = of_iomap(np, 1);
	if (!pmb_addr)
		return -ENOMEM;
	mcucsr_base = of_iomap(np, 2);
	if (!mcucsr_base)
		return -ENOMEM;

	ret = request_firmware(&fw, MEDAITEK_2P5GE_PHY_DMB_FW, dev);
	if (ret) {
		dev_err(dev, "failed to load firmware: %s, ret: %d\n",
			MEDAITEK_2P5GE_PHY_DMB_FW, ret);
		return ret;
	}
	for (i = 0; i < fw->size - 1; i += 4)
		writel(*((uint32_t *)(fw->data + i)), dmb_addr + i);
	release_firmware(fw);

	ret = request_firmware(&fw, MEDIATEK_2P5GE_PHY_PMB_FW, dev);
	if (ret) {
		dev_err(dev, "failed to load firmware: %s, ret: %d\n",
			MEDIATEK_2P5GE_PHY_PMB_FW, ret);
		return ret;
	}
	for (i = 0; i < fw->size - 1; i += 4)
		writel(*((uint32_t *)(fw->data + i)), pmb_addr + i);
	release_firmware(fw);

	reg = readw(mcucsr_base + MD32_EN_CFG);
	writew(reg | MD32_EN, mcucsr_base + MD32_EN_CFG);
	dev_info(dev, "Firmware loading/trigger ok.\n");

	return 0;
}

static int mt798x_2p5ge_phy_get_features(struct phy_device *phydev)
{
	int ret;

	ret = genphy_read_abilities(phydev);
	if (ret)
		return ret;

	linkmode_set_bit(ETHTOOL_LINK_MODE_100baseT_Half_BIT,
			 phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_100baseT_Full_BIT,
			 phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
			 phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
			 phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_Autoneg_BIT, phydev->supported);

	return 0;
}

static struct phy_driver mtk_gephy_driver[] = {
	{
		PHY_ID_MATCH_EXACT(0x00339c11),
		.name		= "MediaTek MT798x 2.5GbE PHY",
		.config_init	= mt798x_2p5ge_phy_config_init,
		.config_aneg    = genphy_c45_config_aneg,
		.get_features	= mt798x_2p5ge_phy_get_features,
		//.config_intr	= genphy_no_config_intr,
		//.handle_interrupt = genphy_no_ack_interrupt,
		//.suspend	= genphy_suspend,
		//.resume		= genphy_resume,
	},
};

module_phy_driver(mtk_gephy_driver);

static struct mdio_device_id __maybe_unused mtk_2p5ge_phy_tbl[] = {
	{ PHY_ID_MATCH_VENDOR(0x00339c00) },
	{ }
};

MODULE_DESCRIPTION("MediaTek 2.5Gb Ethernet PHY driver");
MODULE_AUTHOR("SkyLake Huang <SkyLake.Huang@mediatek.com>");
MODULE_LICENSE("GPL");

MODULE_DEVICE_TABLE(mdio, mtk_2p5ge_phy_tbl);
