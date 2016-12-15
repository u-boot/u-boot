/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm/device.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <asm/errno.h>
#include <asm/gpio.h>

#define UNIPHIER_GPIO_PORTS_PER_BANK	8

#define UNIPHIER_GPIO_REG_DATA		0	/* data */
#define UNIPHIER_GPIO_REG_DIR		4	/* direction (1:in, 0:out) */

struct uniphier_gpio_priv {
	void __iomem *base;
	char bank_name[16];
};

static void uniphier_gpio_offset_write(struct udevice *dev, unsigned offset,
				       unsigned reg, int value)
{
	struct uniphier_gpio_priv *priv = dev_get_priv(dev);
	u32 tmp;

	tmp = readl(priv->base + reg);
	if (value)
		tmp |= BIT(offset);
	else
		tmp &= ~BIT(offset);
	writel(tmp, priv->base + reg);
}

static int uniphier_gpio_offset_read(struct udevice *dev, unsigned offset,
				     unsigned reg)
{
	struct uniphier_gpio_priv *priv = dev_get_priv(dev);

	return !!(readl(priv->base + reg) & BIT(offset));
}

static int uniphier_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	uniphier_gpio_offset_write(dev, offset, UNIPHIER_GPIO_REG_DIR, 1);

	return 0;
}

static int uniphier_gpio_direction_output(struct udevice *dev, unsigned offset,
					  int value)
{
	uniphier_gpio_offset_write(dev, offset, UNIPHIER_GPIO_REG_DATA, value);
	uniphier_gpio_offset_write(dev, offset, UNIPHIER_GPIO_REG_DIR, 0);

	return 0;
}

static int uniphier_gpio_get_value(struct udevice *dev, unsigned offset)
{
	return uniphier_gpio_offset_read(dev, offset, UNIPHIER_GPIO_REG_DATA);
}

static int uniphier_gpio_set_value(struct udevice *dev, unsigned offset,
				   int value)
{
	uniphier_gpio_offset_write(dev, offset, UNIPHIER_GPIO_REG_DATA, value);

	return 0;
}

static int uniphier_gpio_get_function(struct udevice *dev, unsigned offset)
{
	return uniphier_gpio_offset_read(dev, offset, UNIPHIER_GPIO_REG_DIR) ?
						GPIOF_INPUT : GPIOF_OUTPUT;
}

static const struct dm_gpio_ops uniphier_gpio_ops = {
	.direction_input	= uniphier_gpio_direction_input,
	.direction_output	= uniphier_gpio_direction_output,
	.get_value		= uniphier_gpio_get_value,
	.set_value		= uniphier_gpio_set_value,
	.get_function		= uniphier_gpio_get_function,
};

static int uniphier_gpio_probe(struct udevice *dev)
{
	struct uniphier_gpio_priv *priv = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	fdt_addr_t addr;
	unsigned int tmp;

	addr = dev_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = devm_ioremap(dev, addr, SZ_8);
	if (!priv->base)
		return -ENOMEM;

	uc_priv->gpio_count = UNIPHIER_GPIO_PORTS_PER_BANK;

	tmp = (addr & 0xfff);

	/* Unfortunately, there is a register hole at offset 0x90-0x9f. */
	if (tmp > 0x90)
		tmp -= 0x10;

	snprintf(priv->bank_name, sizeof(priv->bank_name) - 1,
		 "port%d-", (tmp - 8) / 8);

	uc_priv->bank_name = priv->bank_name;

	return 0;
}

/* .data = the number of GPIO banks */
static const struct udevice_id uniphier_gpio_match[] = {
	{ .compatible = "socionext,uniphier-gpio" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_gpio) = {
	.name	= "uniphier_gpio",
	.id	= UCLASS_GPIO,
	.of_match = uniphier_gpio_match,
	.probe	= uniphier_gpio_probe,
	.priv_auto_alloc_size = sizeof(struct uniphier_gpio_priv),
	.ops	= &uniphier_gpio_ops,
};
