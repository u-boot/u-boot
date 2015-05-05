/*
 * Copyright (C) 2012 Vikram Narayananan
 * <vikram186@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>

struct bcm2835_gpios {
	struct bcm2835_gpio_regs *reg;
};

static int bcm2835_gpio_direction_input(struct udevice *dev, unsigned gpio)
{
	struct bcm2835_gpios *gpios = dev_get_priv(dev);
	unsigned val;

	val = readl(&gpios->reg->gpfsel[BCM2835_GPIO_FSEL_BANK(gpio)]);
	val &= ~(BCM2835_GPIO_FSEL_MASK << BCM2835_GPIO_FSEL_SHIFT(gpio));
	val |= (BCM2835_GPIO_INPUT << BCM2835_GPIO_FSEL_SHIFT(gpio));
	writel(val, &gpios->reg->gpfsel[BCM2835_GPIO_FSEL_BANK(gpio)]);

	return 0;
}

static int bcm2835_gpio_direction_output(struct udevice *dev, unsigned gpio,
					 int value)
{
	struct bcm2835_gpios *gpios = dev_get_priv(dev);
	unsigned val;

	gpio_set_value(gpio, value);

	val = readl(&gpios->reg->gpfsel[BCM2835_GPIO_FSEL_BANK(gpio)]);
	val &= ~(BCM2835_GPIO_FSEL_MASK << BCM2835_GPIO_FSEL_SHIFT(gpio));
	val |= (BCM2835_GPIO_OUTPUT << BCM2835_GPIO_FSEL_SHIFT(gpio));
	writel(val, &gpios->reg->gpfsel[BCM2835_GPIO_FSEL_BANK(gpio)]);

	return 0;
}

static bool bcm2835_gpio_is_output(const struct bcm2835_gpios *gpios, int gpio)
{
	u32 val;

	val = readl(&gpios->reg->gpfsel[BCM2835_GPIO_FSEL_BANK(gpio)]);
	val &= BCM2835_GPIO_FSEL_MASK << BCM2835_GPIO_FSEL_SHIFT(gpio);
	return val ? true : false;
}

static int bcm2835_get_value(const struct bcm2835_gpios *gpios, unsigned gpio)
{
	unsigned val;

	val = readl(&gpios->reg->gplev[BCM2835_GPIO_COMMON_BANK(gpio)]);

	return (val >> BCM2835_GPIO_COMMON_SHIFT(gpio)) & 0x1;
}

static int bcm2835_gpio_get_value(struct udevice *dev, unsigned gpio)
{
	const struct bcm2835_gpios *gpios = dev_get_priv(dev);

	return bcm2835_get_value(gpios, gpio);
}

static int bcm2835_gpio_set_value(struct udevice *dev, unsigned gpio,
				  int value)
{
	struct bcm2835_gpios *gpios = dev_get_priv(dev);
	u32 *output_reg = value ? gpios->reg->gpset : gpios->reg->gpclr;

	writel(1 << BCM2835_GPIO_COMMON_SHIFT(gpio),
				&output_reg[BCM2835_GPIO_COMMON_BANK(gpio)]);

	return 0;
}

static int bcm2835_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct bcm2835_gpios *gpios = dev_get_priv(dev);

	/* GPIOF_FUNC is not implemented yet */
	if (bcm2835_gpio_is_output(gpios, offset))
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}


static const struct dm_gpio_ops gpio_bcm2835_ops = {
	.direction_input	= bcm2835_gpio_direction_input,
	.direction_output	= bcm2835_gpio_direction_output,
	.get_value		= bcm2835_gpio_get_value,
	.set_value		= bcm2835_gpio_set_value,
	.get_function		= bcm2835_gpio_get_function,
};

static int bcm2835_gpio_probe(struct udevice *dev)
{
	struct bcm2835_gpios *gpios = dev_get_priv(dev);
	struct bcm2835_gpio_platdata *plat = dev_get_platdata(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->bank_name = "GPIO";
	uc_priv->gpio_count = BCM2835_GPIO_COUNT;
	gpios->reg = (struct bcm2835_gpio_regs *)plat->base;

	return 0;
}

U_BOOT_DRIVER(gpio_bcm2835) = {
	.name	= "gpio_bcm2835",
	.id	= UCLASS_GPIO,
	.ops	= &gpio_bcm2835_ops,
	.probe	= bcm2835_gpio_probe,
	.priv_auto_alloc_size = sizeof(struct bcm2835_gpios),
};
