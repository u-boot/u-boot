/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * Derived from linux/arch/mips/bcm63xx/gpio.c:
 *	Copyright (C) 2008 Maxime Bizon <mbizon@freebox.fr>
 *	Copyright (C) 2008-2011 Florian Fainelli <florian@openwrt.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

struct bcm6345_gpio_priv {
	void __iomem *reg_dirout;
	void __iomem *reg_data;
};

static int bcm6345_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct bcm6345_gpio_priv *priv = dev_get_priv(dev);

	return !!(readl_be(priv->reg_data) & BIT(offset));
}

static int bcm6345_gpio_set_value(struct udevice *dev, unsigned offset,
				  int value)
{
	struct bcm6345_gpio_priv *priv = dev_get_priv(dev);

	if (value)
		setbits_be32(priv->reg_data, BIT(offset));
	else
		clrbits_be32(priv->reg_data, BIT(offset));

	return 0;
}

static int bcm6345_gpio_set_direction(void __iomem *dirout, unsigned offset,
				      bool input)
{
	if (input)
		clrbits_be32(dirout, BIT(offset));
	else
		setbits_be32(dirout, BIT(offset));

	return 0;
}

static int bcm6345_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct bcm6345_gpio_priv *priv = dev_get_priv(dev);

	return bcm6345_gpio_set_direction(priv->reg_dirout, offset, 1);
}

static int bcm6345_gpio_direction_output(struct udevice *dev, unsigned offset,
					 int value)
{
	struct bcm6345_gpio_priv *priv = dev_get_priv(dev);

	bcm6345_gpio_set_value(dev, offset, value);

	return bcm6345_gpio_set_direction(priv->reg_dirout, offset, 0);
}

static int bcm6345_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct bcm6345_gpio_priv *priv = dev_get_priv(dev);

	if (readl_be(priv->reg_dirout) & BIT(offset))
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static const struct dm_gpio_ops bcm6345_gpio_ops = {
	.direction_input = bcm6345_gpio_direction_input,
	.direction_output = bcm6345_gpio_direction_output,
	.get_value = bcm6345_gpio_get_value,
	.set_value = bcm6345_gpio_set_value,
	.get_function = bcm6345_gpio_get_function,
};

static int bcm6345_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct bcm6345_gpio_priv *priv = dev_get_priv(dev);
	fdt_addr_t data_addr, dirout_addr;
	fdt_size_t data_size, dirout_size;

	dirout_addr = devfdt_get_addr_size_index(dev, 0, &dirout_size);
	if (dirout_addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	data_addr = devfdt_get_addr_size_index(dev, 1, &data_size);
	if (data_addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->reg_data = ioremap(data_addr, data_size);
	priv->reg_dirout = ioremap(dirout_addr, dirout_size);

	uc_priv->gpio_count = fdtdec_get_uint(gd->fdt_blob, dev_of_offset(dev),
					      "ngpios", 32);
	uc_priv->bank_name = dev->name;

	return 0;
}

static const struct udevice_id bcm6345_gpio_ids[] = {
	{ .compatible = "brcm,bcm6345-gpio" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(bcm6345_gpio) = {
	.name = "bcm6345-gpio",
	.id = UCLASS_GPIO,
	.of_match = bcm6345_gpio_ids,
	.ops = &bcm6345_gpio_ops,
	.priv_auto_alloc_size = sizeof(struct bcm6345_gpio_priv),
	.probe = bcm6345_gpio_probe,
};
