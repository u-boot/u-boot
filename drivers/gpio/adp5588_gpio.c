// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * GPIO Chip driver for Analog Devices
 * ADP5588/ADP5587 I/O Expander and QWERTY Keypad Controller
 *
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 *
 * Based on Michael Hennerich's Linux driver:
 * Michael Hennerich <michael.hennerich@analog.com>
 *
 */

#include <dm.h>
#include <i2c.h>
#include <asm-generic/gpio.h>

#define ADP5588_MAXGPIO     18
#define ADP5588_BANK(offs)  ((offs) >> 3)
#define ADP5588_BIT(offs)   (1u << ((offs) & 0x7))

#define DEV_ID          0x00    /* Device ID */
#define GPIO_DAT_STAT1  0x14    /* GPIO Data Status, Read twice to clear */
#define GPIO_DAT_STAT2  0x15    /* GPIO Data Status, Read twice to clear */
#define GPIO_DAT_STAT3  0x16    /* GPIO Data Status, Read twice to clear */
#define GPIO_DAT_OUT1   0x17    /* GPIO DATA OUT */
#define GPIO_DAT_OUT2   0x18    /* GPIO DATA OUT */
#define GPIO_DAT_OUT3   0x19    /* GPIO DATA OUT */
#define GPIO_INT_EN1    0x1A    /* GPIO Interrupt Enable */
#define GPIO_INT_EN2    0x1B    /* GPIO Interrupt Enable */
#define GPIO_INT_EN3    0x1C    /* GPIO Interrupt Enable */
#define KP_GPIO1        0x1D    /* Keypad or GPIO Selection */
#define KP_GPIO2        0x1E    /* Keypad or GPIO Selection */
#define KP_GPIO3        0x1F    /* Keypad or GPIO Selection */
#define GPIO_DIR1       0x23    /* GPIO Data Direction */
#define GPIO_DIR2       0x24    /* GPIO Data Direction */
#define GPIO_DIR3       0x25	/* GPIO Data Direction */
#define GPIO_PULL1      0x2C    /* GPIO Pull Disable */
#define GPIO_PULL2      0x2D    /* GPIO Pull Disable */
#define GPIO_PULL3      0x2E    /* GPIO Pull Disable */
#define ID_MASK	        0x0F

struct adp5588_gpio {
	u8 dat_out[3];
	u8 dir[3];
};

static int adp5588_gpio_read(struct udevice *dev, u8 reg)
{
	int ret;
	u8 val;

	ret = dm_i2c_read(dev, reg, &val, 1);

	if (ret < 0) {
		pr_err("%s: read error\n", __func__);
		return ret;
	}

	return val;
}

static int adp5588_gpio_write(struct udevice *dev, u8 reg, u8 val)
{
	int ret;

	ret = dm_i2c_write(dev, reg, &val, 1);
	if (ret < 0) {
		pr_err("%s: write error\n", __func__);
		return ret;
	}

	return 0;
}

static int adp5588_get_value(struct udevice *dev, u32 offset)
{
	struct adp5588_gpio *plat = dev_get_plat(dev);
	unsigned int bank = ADP5588_BANK(offset);
	unsigned int bit = ADP5588_BIT(offset);
	int val;

	if (plat->dir[bank] & bit)
		val = plat->dat_out[bank];
	else
		val = adp5588_gpio_read(dev, GPIO_DAT_STAT1 + bank);

	return !!(val & bit);
}

static int adp5588_set_value(struct udevice *dev, u32 offset,
			     int32_t value)
{
	unsigned int bank, bit;
	int ret;
	struct adp5588_gpio *plat = dev_get_plat(dev);

	bank = ADP5588_BANK(offset);
	bit = ADP5588_BIT(offset);

	if (value)
		plat->dat_out[bank] |= bit;
	else
		plat->dat_out[bank] &= ~bit;

	ret = adp5588_gpio_write(dev, GPIO_DAT_OUT1 + bank,
				 plat->dat_out[bank]);

	return ret;
}

static int adp5588_direction_input(struct udevice *dev, u32 offset)
{
	int ret;
	unsigned int bank;
	struct adp5588_gpio *plat = dev_get_plat(dev);

	bank = ADP5588_BANK(offset);

	plat->dir[bank] &= ~ADP5588_BIT(offset);
	ret = adp5588_gpio_write(dev, GPIO_DIR1 + bank, plat->dir[bank]);

	return ret;
}

static int adp5588_direction_output(struct udevice *dev,
				    u32 offset, int value)
{
	int ret;
	unsigned int bank, bit;
	struct adp5588_gpio *plat = dev_get_plat(dev);

	bank = ADP5588_BANK(offset);
	bit = ADP5588_BIT(offset);

	plat->dir[bank] |= bit;

	if (value)
		plat->dat_out[bank] |= bit;
	else
		plat->dat_out[bank] &= ~bit;

	ret = adp5588_gpio_write(dev, GPIO_DAT_OUT1 + bank,
				 plat->dat_out[bank]);
	ret |= adp5588_gpio_write(dev, GPIO_DIR1 + bank,
				 plat->dir[bank]);

	return ret;
}

static int adp5588_ofdata_platdata(struct udevice *dev)
{
	struct adp5588_gpio *plat = dev_get_plat(dev);
	struct gpio_dev_priv *priv = dev_get_uclass_priv(dev);
	int node = dev_of_offset(dev);
	int ret, i, revid;

	priv->gpio_count = ADP5588_MAXGPIO;
	priv->bank_name = fdt_get_name(gd->fdt_blob, node, NULL);

	ret = adp5588_gpio_read(dev, DEV_ID);
	if (ret < 0)
		return ret;

	revid = ret & ID_MASK;

	printf("ADP5588 Detected: Rev %x, Rev ID %x\n", ret, revid);

	for (i = 0, ret = 0; i <= ADP5588_BANK(ADP5588_MAXGPIO); i++) {
		plat->dat_out[i] = adp5588_gpio_read(dev, GPIO_DAT_OUT1 + i);
		plat->dir[i] = adp5588_gpio_read(dev, GPIO_DIR1 + i);
		ret |= adp5588_gpio_write(dev, KP_GPIO1 + i, 0);
		ret |= adp5588_gpio_write(dev, GPIO_PULL1 + i, 0);
		ret |= adp5588_gpio_write(dev, GPIO_INT_EN1 + i, 0);
		if (ret) {
			pr_err("%s: Initialization error\n", __func__);
			return ret;
		}
	}

	return 0;
}

static const struct dm_gpio_ops adp5588_ops = {
	.direction_input  = adp5588_direction_input,
	.direction_output = adp5588_direction_output,
	.get_value		  = adp5588_get_value,
	.set_value		  = adp5588_set_value,
};

static const struct udevice_id adp5588_of_match_list[] = {
	{ .compatible = "adi,adp5588"},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(gpio_adp5588) = {
	.name					  = "gpio_adp5588",
	.id						  = UCLASS_GPIO,
	.ops					  = &adp5588_ops,
	.of_match				  = adp5588_of_match_list,
	.of_to_plat		  = adp5588_ofdata_platdata,
	.plat_auto = sizeof(struct adp5588_gpio),
	.flags					  = DM_FLAG_PRE_RELOC,
};
