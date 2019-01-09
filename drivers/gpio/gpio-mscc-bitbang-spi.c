// SPDX-License-Identifier: (GPL-2.0 OR MIT)
/*
 * Microsemi SoCs pinctrl driver
 *
 * Author: <gregory.clement@bootlin.com>
 * License: Dual MIT/GPL
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>
#include <asm-generic/gpio.h>
#include <asm/io.h>
#include <dm.h>
#include <errno.h>

enum {
	SDI,
	CS0,
	CS1,
	CS2,
	CS3,
	SDO,
	SCK
};

static const int pinmap[] = { 0, 5, 6, 7, 8, 10, 12 };

#define SW_SPI_CSn_OE	 0x1E	/* bits 1 to 4 */
#define SW_SPI_CS0_OE	 BIT(1)
#define SW_SPI_SDO_OE	 BIT(9)
#define SW_SPI_SCK_OE	 BIT(11)
#define SW_PIN_CTRL_MODE BIT(13)

struct mscc_bb_spi_gpio {
	void __iomem *regs;
	u32 cache_val;
};

static int mscc_bb_spi_gpio_set(struct udevice *dev, unsigned oft, int val)
{
	struct mscc_bb_spi_gpio *gpio = dev_get_priv(dev);

	if (val)
		gpio->cache_val |= BIT(pinmap[oft]);
	else
		gpio->cache_val &= ~BIT(pinmap[oft]);

	writel(gpio->cache_val, gpio->regs);

	return 0;
}

static int mscc_bb_spi_gpio_direction_output(struct udevice *dev, unsigned oft,
					     int val)
{
	if (oft == 0) {
		pr_err("SW_SPI_DSI can't be used as output\n");
		return -ENOTSUPP;
	}

	mscc_bb_spi_gpio_set(dev, oft, val);

	return 0;
}

static int mscc_bb_spi_gpio_direction_input(struct udevice *dev, unsigned oft)
{
	return 0;
}

static int mscc_bb_spi_gpio_get(struct udevice *dev, unsigned int oft)
{
	struct mscc_bb_spi_gpio *gpio = dev_get_priv(dev);
	u32 val = readl(gpio->regs);

	return !!(val & BIT(pinmap[oft]));
}

static const struct dm_gpio_ops mscc_bb_spi_gpio_ops = {
	.direction_output	= mscc_bb_spi_gpio_direction_output,
	.direction_input	= mscc_bb_spi_gpio_direction_input,
	.set_value		= mscc_bb_spi_gpio_set,
	.get_value		= mscc_bb_spi_gpio_get,
};

static int mscc_bb_spi_gpio_probe(struct udevice *dev)
{
	struct mscc_bb_spi_gpio *gpio = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	gpio->regs = dev_remap_addr(dev);
	if (!gpio->regs)
		return -EINVAL;

	uc_priv->bank_name = dev->name;
	uc_priv->gpio_count = ARRAY_SIZE(pinmap);
	/*
	 * Enable software mode to control the SPI pin, enables the
	 * output mode for most of the pin and initialize the cache
	 * value in the same time
	 */

	gpio->cache_val = SW_PIN_CTRL_MODE | SW_SPI_SCK_OE | SW_SPI_SDO_OE |
	    SW_SPI_CS0_OE;
	writel(gpio->cache_val, gpio->regs);

	return 0;
}

static const struct udevice_id mscc_bb_spi_gpio_ids[] = {
	{.compatible = "mscc,spi-bitbang-gpio"},
	{}
};

U_BOOT_DRIVER(gpio_mscc_bb_spi) = {
	.name	= "gpio-mscc-spi-bitbang",
	.id	= UCLASS_GPIO,
	.ops	= &mscc_bb_spi_gpio_ops,
	.probe	= mscc_bb_spi_gpio_probe,
	.of_match = of_match_ptr(mscc_bb_spi_gpio_ids),
	.priv_auto_alloc_size = sizeof(struct mscc_bb_spi_gpio),
};
