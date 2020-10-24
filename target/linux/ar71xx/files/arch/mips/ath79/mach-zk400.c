/*
 *  Lute ZK400 board support
 *
 *  Copyright (C) 2020 AmadeusGhost
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define zk400_GPIO_LED_WLAN		12

#define zk400_GPIO_LED_WAN		11
#define zk400_GPIO_LED_LAN1		14
#define zk400_GPIO_LED_LAN2		15
#define zk400_GPIO_LED_LAN3		16
#define zk400_GPIO_LED_LAN4		4

#define zk400_GPIO_BTN_RESET		17

#define zk400_KEYS_POLL_INTERVAL	20	/* msecs */
#define zk400_KEYS_DEBOUNCE_INTERVAL	(3 * zk400_KEYS_POLL_INTERVAL)

#define zk400_MAC0_OFFSET		0
#define zk400_MAC1_OFFSET		6
#define zk400_WMAC_CALDATA_OFFSET	0x1000

static struct gpio_led zk400_leds_gpio[] __initdata = {
	{
		.name		= "zk400:green:wan",
		.gpio		= zk400_GPIO_LED_WAN,
		.active_low	= 1,
	}, {
		.name		= "zk400:green:lan1",
		.gpio		= zk400_GPIO_LED_LAN1,
		.active_low	= 1,
	}, {
		.name		= "zk400:green:lan2",
		.gpio		= zk400_GPIO_LED_LAN2,
		.active_low	= 1,
	}, {
		.name		= "zk400:green:lan3",
		.gpio		= zk400_GPIO_LED_LAN3,
		.active_low	= 1,
	}, {
		.name		= "zk400:green:lan4",
		.gpio		= zk400_GPIO_LED_LAN4,
		.active_low	= 1,
	}, {
		.name		= "zk400:green:wlan",
		.gpio		= zk400_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button zk400_gpio_keys[] __initdata = {
	{
		.desc = "reset",
		.type = EV_KEY,
		.code = KEY_RESTART,
		.debounce_interval = zk400_KEYS_DEBOUNCE_INTERVAL,
		.gpio    = zk400_GPIO_BTN_RESET,
		.active_low  = 1,
	},
};

static void __init zk400_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(zk400_leds_gpio),
			zk400_leds_gpio);

	ath79_register_gpio_keys_polled(-1, zk400_KEYS_POLL_INTERVAL,
			ARRAY_SIZE(zk400_gpio_keys),
			zk400_gpio_keys);

	ath79_register_wmac(art + zk400_WMAC_CALDATA_OFFSET, NULL);

	ath79_setup_ar933x_phy4_switch(true, true);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + zk400_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, art + zk400_MAC1_OFFSET, 0);

	ath79_register_mdio(0, 0x0);

	/* WAN port */
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.speed = SPEED_100;
	ath79_register_eth(0);

	/* LAN ports */
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.phy_mask = BIT(1);
	ath79_switch_data.phy_poll_mask |= BIT(0);
	ath79_switch_data.phy4_mii_en = 1;
	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_ZK400, "ZK400", "Lute ZK400",
	     zk400_setup);
