/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "sw.h"
#include "fal_port_ctrl.h"
#include "hsl_api.h"
#include "hsl.h"
#include "sfp_phy.h"
#include "aos_timer.h"
#include "hsl_phy.h"
#include <linux/kconfig.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/phy.h>
#include "ssdk_plat.h"

/******************************************************************************
*
* sfp_phy_init -
*
*/
static int
sfp_phy_probe(struct phy_device *pdev)
{
	SSDK_INFO("sfp phy is probed!\n");
	return 0;
}

static void
sfp_phy_remove(struct phy_device *pdev)
{
	return;
}

static int
sfp_phy_config_aneg(struct phy_device *pdev)
{

	return 0;
}

static int
sfp_phy_aneg_done(struct phy_device *pdev)
{

	return SFP_ANEG_DONE;
}

static int
sfp_read_status(struct phy_device *pdev)
{
	sw_error_t rv;
	a_bool_t status;
	fal_port_t port;
	a_uint32_t addr;
	struct qca_phy_priv *priv = pdev->priv;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
	addr = pdev->mdio.addr;
#else
	addr = pdev->addr;
#endif
	port = qca_ssdk_phy_addr_to_port(priv->device_id, addr);
	rv = fal_port_link_status_get(priv->device_id, port, &status);
	if (!rv) {
		pdev->link = status;
		if (pdev->link) {
			pdev->speed = SPEED_10000;
			pdev->duplex = DUPLEX_FULL;
		}
	}

	return rv;
}

static struct phy_driver sfp_phy_driver = {
	.name		= "QCA SFP",
	.phy_id		= SFP_PHY,
	.phy_id_mask = SFP_PHY_MASK,
	.probe		= sfp_phy_probe,
	.remove		= sfp_phy_remove,
	.config_aneg	= sfp_phy_config_aneg,
	.aneg_done	= sfp_phy_aneg_done,
	.read_status	= sfp_read_status,
	.features	= PHY_BASIC_FEATURES,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
	.mdiodrv.driver	= { .owner = THIS_MODULE },
#else
	.driver		= { .owner = THIS_MODULE },
#endif
};

int sfp_phy_device_setup(a_uint32_t dev_id, a_uint32_t port, a_uint32_t phy_id)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
	struct phy_device *phydev;
	struct qca_phy_priv *priv;
	a_uint32_t addr = 0;
	struct mii_bus *bus;

	priv = ssdk_phy_priv_data_get(dev_id);
	/*create phy device*/
#if defined(IN_PHY_I2C_MODE)
	if (hsl_port_phy_access_type_get(dev_id, port) == PHY_I2C_ACCESS) {
		addr = qca_ssdk_port_to_phy_mdio_fake_addr(dev_id, port);
	} else
#endif
	{
		addr = qca_ssdk_port_to_phy_addr(dev_id, port);
	}
	bus = ssdk_miibus_get_by_device(dev_id);
	phydev = phy_device_create(bus, addr, phy_id, false, NULL);
	if (IS_ERR(phydev) || phydev == NULL) {
		SSDK_ERROR("Failed to create phy device!\n");
		return SW_NOT_SUPPORTED;
	}
	/*register phy device*/
	phy_device_register(phydev);

	phydev->priv = priv;
#endif
	return 0;
}

void sfp_phy_device_remove(a_uint32_t dev_id, a_uint32_t port)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
	struct phy_device *phydev = NULL;
	a_uint32_t addr = 0;
	struct mii_bus *bus;

	bus = ssdk_miibus_get_by_device(dev_id);
#if defined(IN_PHY_I2C_MODE)
	if (hsl_port_phy_access_type_get(dev_id, port) == PHY_I2C_ACCESS) {
		addr = qca_ssdk_port_to_phy_mdio_fake_addr(dev_id, port);
	} else
#endif
	{
		addr = qca_ssdk_port_to_phy_addr(dev_id, port);
	}
	if (addr < PHY_MAX_ADDR) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0))
		phydev = bus->phy_map[addr];
#else
		phydev = mdiobus_get_phy(bus, addr);
#endif
	}
	if (phydev)
		phy_device_remove(phydev);
#endif
}

int sfp_phy_init(a_uint32_t dev_id, a_uint32_t port_bmp)
{
	a_uint32_t port_id = 0;

	SSDK_INFO("sfp phy init for port 0x%x!\n", port_bmp);

	for (port_id = 0; port_id < SW_MAX_NR_PORT; port_id ++) {
		if (port_bmp & (0x1 << port_id)) {
			sfp_phy_device_setup(dev_id, port_id, SFP_PHY);
		}
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
	phy_driver_register(&sfp_phy_driver, THIS_MODULE);
#else
	phy_driver_register(&sfp_phy_driver);
#endif
	return 0;
}

void sfp_phy_exit(a_uint32_t dev_id, a_uint32_t port_bmp)
{
	a_uint32_t port_id = 0;

	phy_driver_unregister(&sfp_phy_driver);

	for (port_id = 0; port_id < SW_MAX_NR_PORT; port_id ++) {
		if (port_bmp & (0x1 << port_id)) {
			sfp_phy_device_remove(dev_id, port_id);
		}
	}

}
