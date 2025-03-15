// SPDX-License-Identifier: GPL-2.0+
#include <linux/bitfield.h>
#include <linux/module.h>
#include <linux/phy.h>

static int gpy211_phy_config_init(struct phy_device *phydev)
{
	int sgmii_reg = phy_read_mmd(phydev, MDIO_MMD_VEND1, 8);

	if (sgmii_reg != 0x24e2){
		phy_write_mmd(phydev, MDIO_MMD_VEND1, 8, 0x24e2);
	}
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 1, 0x03f0);
	return 0;
}

int gpy211_phy_probe(struct phy_device *phydev)
{
	int sgmii_reg = phy_read_mmd(phydev, MDIO_MMD_VEND1, 8);

	/* enable 2.5G SGMII rate adaption */
	phy_write_mmd(phydev, MDIO_MMD_VEND1, 8, 0x24e2);

	return 0;
}

static int gpy211_get_features(struct phy_device *phydev)
{
	int ret;

	ret = genphy_read_abilities(phydev);
	if (ret)
		return ret;

	/* GPY211 with rate adaption supports 100M/1G/2.5G speed. */
	linkmode_clear_bit(ETHTOOL_LINK_MODE_10baseT_Half_BIT,
			   phydev->supported);
	linkmode_clear_bit(ETHTOOL_LINK_MODE_10baseT_Full_BIT,
			   phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_2500baseX_Full_BIT,
			 phydev->supported);

	return 0;
}

static struct phy_driver gpy211_phy_driver[] = {
	{
		PHY_ID_MATCH_MODEL(0x67c9de0a),
		.name		= "Intel GPY211 PHY",
		.config_init	= gpy211_phy_config_init,
		.probe		= gpy211_phy_probe,
		.get_features	= gpy211_get_features,
	}
};

module_phy_driver(gpy211_phy_driver);

static struct mdio_device_id __maybe_unused gpy211_phy_tbl[] = {
	{ PHY_ID_MATCH_VENDOR(0x67c9de00) },
	{ }
};

MODULE_DESCRIPTION("Intel GPY211 PHY driver with rate adaption");
MODULE_AUTHOR("Landen Chao <landen.chao@mediatek.com>");
MODULE_LICENSE("GPL");

MODULE_DEVICE_TABLE(mdio, gpy211_phy_tbl);
