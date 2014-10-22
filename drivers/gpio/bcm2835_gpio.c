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

#define GPIO_NAME_SIZE		20

struct bcm2835_gpios {
	char label[BCM2835_GPIO_COUNT][GPIO_NAME_SIZE];
	struct bcm2835_gpio_regs *reg;
};

/**
 * gpio_is_requested() - check if a GPIO has been requested
 *
 * @bank:	Bank to check
 * @offset:	GPIO offset within bank to check
 * @return true if marked as requested, false if not
 */
static inline bool gpio_is_requested(struct bcm2835_gpios *gpios, int offset)
{
	return *gpios->label[offset] != '\0';
}

static int check_requested(struct udevice *dev, unsigned offset,
			   const char *func)
{
	struct bcm2835_gpios *gpios = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv = dev->uclass_priv;

	if (!gpio_is_requested(gpios, offset)) {
		printf("omap_gpio: %s: error: gpio %s%d not requested\n",
		       func, uc_priv->bank_name, offset);
		return -EPERM;
	}

	return 0;
}

static int bcm2835_gpio_request(struct udevice *dev, unsigned offset,
				const char *label)
{
	struct bcm2835_gpios *gpios = dev_get_priv(dev);

	if (gpio_is_requested(gpios, offset))
		return -EBUSY;

	strncpy(gpios->label[offset], label, GPIO_NAME_SIZE);
	gpios->label[offset][GPIO_NAME_SIZE - 1] = '\0';

	return 0;
}

static int bcm2835_gpio_free(struct udevice *dev, unsigned offset)
{
	struct bcm2835_gpios *gpios = dev_get_priv(dev);
	int ret;

	ret = check_requested(dev, offset, __func__);
	if (ret)
		return ret;
	gpios->label[offset][0] = '\0';

	return 0;
}

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

	if (!gpio_is_requested(gpios, offset))
		return GPIOF_UNUSED;

	/* GPIOF_FUNC is not implemented yet */
	if (bcm2835_gpio_is_output(gpios, offset))
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static int bcm2835_gpio_get_state(struct udevice *dev, unsigned int offset,
				  char *buf, int bufsize)
{
	struct gpio_dev_priv *uc_priv = dev->uclass_priv;
	struct bcm2835_gpios *gpios = dev_get_priv(dev);
	const char *label;
	bool requested;
	bool is_output;
	int size;

	label = gpios->label[offset];
	is_output = bcm2835_gpio_is_output(gpios, offset);
	size = snprintf(buf, bufsize, "%s%d: ",
			uc_priv->bank_name ? uc_priv->bank_name : "", offset);
	buf += size;
	bufsize -= size;
	requested = gpio_is_requested(gpios, offset);
	snprintf(buf, bufsize, "%s: %d [%c]%s%s",
		 is_output ? "out" : " in",
		 bcm2835_get_value(gpios, offset),
		 requested ? 'x' : ' ',
		 requested ? " " : "",
		 label);

	return 0;
}

static const struct dm_gpio_ops gpio_bcm2835_ops = {
	.request		= bcm2835_gpio_request,
	.free			= bcm2835_gpio_free,
	.direction_input	= bcm2835_gpio_direction_input,
	.direction_output	= bcm2835_gpio_direction_output,
	.get_value		= bcm2835_gpio_get_value,
	.set_value		= bcm2835_gpio_set_value,
	.get_function		= bcm2835_gpio_get_function,
	.get_state		= bcm2835_gpio_get_state,
};

static int bcm2835_gpio_probe(struct udevice *dev)
{
	struct bcm2835_gpios *gpios = dev_get_priv(dev);
	struct bcm2835_gpio_platdata *plat = dev_get_platdata(dev);
	struct gpio_dev_priv *uc_priv = dev->uclass_priv;

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
