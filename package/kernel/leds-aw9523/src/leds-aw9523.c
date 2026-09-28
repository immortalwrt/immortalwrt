// SPDX-License-Identifier: GPL-2.0-only
/*
 * LED driver for the Awinic AW9523/AW9523B 16-channel I2C GPIO/LED expander,
 * driving its outputs in constant-current (LED) mode.
 *
 * The AW9523B exposes 16 pins (P0_0..P0_7, P1_0..P1_7). Each pin can be a GPIO
 * (handled by pinctrl-aw9523) or an LED constant-current sink. This driver binds
 * when the chip is used purely for LEDs ("awinic,aw9523-led"); each DT child is
 * one LED channel with a "reg" = DIM channel index (0..15, DIM register
 * 0x20 + index) and the usual "color"/"function" so it appears in
 * /sys/class/leds as e.g. "green:power".
 *
 * Reset: the chip is reset over I2C (SOFT_RESET register) - this driver does NOT
 * touch any hardware RSTN GPIO, deliberately. On at least one board (the QDM530
 * CPE) requesting the RSTN gpio and pulsing it wedged the QUP I2C controller at
 * probe; left alone (RSTN at its power-on level) the chip answers fine.
 *
 * Copyright (C) 2026
 * Modelled on drivers/leds/leds-aw200xx.c.
 */

#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/property.h>
#include <linux/regmap.h>

#define AW9523_REG_CHIPID	0x10
#define AW9523_CHIPID		0x23
#define AW9523_REG_GCR		0x11
#define AW9523_GCR_ISEL_IMAX	0x00	/* ISEL[1:0]=00 -> full IMAX range */
/* LED-mode select, one bit per pin on each port: 0 = LED (const current),
 * 1 = GPIO. Port 0 at 0x12, port 1 at 0x13. */
#define AW9523_REG_LED_MODE(port)	(0x12 + (port))
#define AW9523_REG_DIM_BASE	0x20	/* per-channel current, 0x20..0x2f */
#define AW9523_NUM_CHANNELS	16
#define AW9523_REG_SOFT_RESET	0x7f
#define AW9523_SOFT_RESET_VAL	0x00
#define AW9523_REG_MAX		0x7f

struct aw9523_led {
	struct led_classdev cdev;
	struct aw9523 *chip;
	u8 channel;		/* 0..15; DIM register = AW9523_REG_DIM_BASE + channel */
};

struct aw9523 {
	struct regmap *regmap;
	struct mutex lock;	/* serialises register access */
	struct aw9523_led leds[AW9523_NUM_CHANNELS];
	unsigned int num_leds;
};

/*
 * The AW9523 DIM (constant-current) registers 0x20..0x2f map to pins in an
 * irregular order; derive the LED-mode port/bit for a DIM channel index:
 *   channel  0..3  -> P1_0..P1_3  (port 1, bits 0..3)
 *   channel  4..11 -> P0_0..P0_7  (port 0, bits 0..7)
 *   channel 12..15 -> P1_4..P1_7  (port 1, bits 4..7)
 */
static void aw9523_channel_to_port_bit(u8 channel, u8 *port, u8 *bit)
{
	if (channel <= 3) {
		*port = 1;
		*bit = channel;
	} else if (channel <= 11) {
		*port = 0;
		*bit = channel - 4;
	} else {
		*port = 1;
		*bit = channel - 8;
	}
}

static int aw9523_brightness_set(struct led_classdev *cdev,
				 enum led_brightness brightness)
{
	struct aw9523_led *led = container_of(cdev, struct aw9523_led, cdev);
	struct aw9523 *chip = led->chip;
	int ret;

	mutex_lock(&chip->lock);
	ret = regmap_write(chip->regmap,
			   AW9523_REG_DIM_BASE + led->channel, brightness);
	mutex_unlock(&chip->lock);

	return ret;
}

static int aw9523_hw_init(struct device *dev, struct aw9523 *chip)
{
	unsigned int chipid;
	int ret;

	/* Soft reset over I2C - no RSTN GPIO handling (see file header). */
	ret = regmap_write(chip->regmap, AW9523_REG_SOFT_RESET,
			   AW9523_SOFT_RESET_VAL);
	if (ret)
		return dev_err_probe(dev, ret, "soft reset failed\n");

	ret = regmap_read(chip->regmap, AW9523_REG_CHIPID, &chipid);
	if (ret)
		return dev_err_probe(dev, ret, "cannot read chip id\n");

	if (chipid != AW9523_CHIPID)
		return dev_err_probe(dev, -ENODEV,
				     "bad chip id 0x%02x (expected 0x%02x)\n",
				     chipid, AW9523_CHIPID);

	/* Full constant-current range. */
	ret = regmap_write(chip->regmap, AW9523_REG_GCR, AW9523_GCR_ISEL_IMAX);
	if (ret)
		return dev_err_probe(dev, ret, "cannot set GCR\n");

	/*
	 * Start with every pin in GPIO mode (reset default); the per-LED setup
	 * below switches only the used pins into LED mode.
	 */
	ret = regmap_write(chip->regmap, AW9523_REG_LED_MODE(0), 0xff);
	if (ret)
		return ret;

	return regmap_write(chip->regmap, AW9523_REG_LED_MODE(1), 0xff);
}

static int aw9523_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct aw9523 *chip;
	int ret;
	static const struct regmap_config aw9523_regmap_config = {
		.reg_bits = 8,
		.val_bits = 8,
		.max_register = AW9523_REG_MAX,
	};

	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->regmap = devm_regmap_init_i2c(client, &aw9523_regmap_config);
	if (IS_ERR(chip->regmap))
		return dev_err_probe(dev, PTR_ERR(chip->regmap),
				     "regmap init failed\n");

	ret = devm_mutex_init(dev, &chip->lock);
	if (ret)
		return ret;

	ret = aw9523_hw_init(dev, chip);
	if (ret)
		return ret;

	i2c_set_clientdata(client, chip);

	device_for_each_child_node_scoped(dev, child) {
		struct led_init_data init_data = {};
		struct aw9523_led *led;
		u8 port, bit;
		u32 channel;

		ret = fwnode_property_read_u32(child, "reg", &channel);
		if (ret) {
			dev_err(dev, "missing reg property\n");
			continue;
		}

		if (channel >= AW9523_NUM_CHANNELS) {
			dev_err(dev, "reg %u out of range (0..%u)\n",
				channel, AW9523_NUM_CHANNELS - 1);
			continue;
		}

		led = &chip->leds[chip->num_leds];
		led->chip = chip;
		led->channel = channel;
		led->cdev.max_brightness = 255;
		led->cdev.brightness_set_blocking = aw9523_brightness_set;

		/* switch this channel's pin into LED (constant-current) mode */
		aw9523_channel_to_port_bit(channel, &port, &bit);
		ret = regmap_update_bits(chip->regmap, AW9523_REG_LED_MODE(port),
					 BIT(bit), 0);
		if (ret)
			return ret;

		init_data.fwnode = child;

		ret = devm_led_classdev_register_ext(dev, &led->cdev,
						     &init_data);
		if (ret)
			return dev_err_probe(dev, ret,
					     "failed to register LED for channel %u\n",
					     channel);

		chip->num_leds++;
	}

	if (!chip->num_leds)
		return dev_err_probe(dev, -EINVAL, "no LEDs defined\n");

	return 0;
}

static const struct of_device_id aw9523_of_match[] = {
	{ .compatible = "awinic,aw9523-led" },
	{ }
};
MODULE_DEVICE_TABLE(of, aw9523_of_match);

static const struct i2c_device_id aw9523_i2c_id[] = {
	{ "aw9523-led" },
	{ }
};
MODULE_DEVICE_TABLE(i2c, aw9523_i2c_id);

static struct i2c_driver aw9523_driver = {
	.driver = {
		.name = "leds-aw9523",
		.of_match_table = aw9523_of_match,
	},
	.probe = aw9523_probe,
	.id_table = aw9523_i2c_id,
};
module_i2c_driver(aw9523_driver);

MODULE_DESCRIPTION("Awinic AW9523B constant-current LED driver");
MODULE_LICENSE("GPL");
