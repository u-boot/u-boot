// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2024 CS GROUP France
 *	Christophe Leroy <christophe.leroy@csgroup.eu>
 */

#include <asm/io.h>
#include <dm.h>
#include <mapmem.h>
#include <asm/gpio.h>
#include <malloc.h>

#include "../common/common.h"

struct mcr3000_spi_gpio_plat {
	ulong addr;
};

struct mcr3000_spi_gpio_data {
	void __iomem *base;
};

static int mcr3000_spi_gpio_set_value(struct udevice *dev, uint gpio, int value)
{
	struct mcr3000_spi_gpio_data *data = dev_get_priv(dev);

	if (value)
		clrsetbits_be16(data->base, 7 << 5, (gpio & 7) << 5);
	else
		clrbits_be16(data->base, 7 << 5);

	return 0;
}

static int mcr3000_spi_gpio_get_value(struct udevice *dev, uint gpio)
{
	struct mcr3000_spi_gpio_data *data = dev_get_priv(dev);

	return gpio == ((in_be16(data->base) >> 5) & 7);
}

static int mcr3000_spi_gpio_direction_input(struct udevice *dev, uint gpio)
{
	return 0;
}

static int mcr3000_spi_gpio_get_function(struct udevice *dev, uint gpio)
{
	return GPIOF_OUTPUT;
}

static int mcr3000_spi_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct mcr3000_spi_gpio_plat *plat = dev_get_plat(dev);
	fdt_addr_t addr;
	u32 reg[2];

	dev_read_u32_array(dev, "reg", reg, 2);
	addr = dev_translate_address(dev, reg);

	plat->addr = addr;

	return 0;
}

static int mcr3000_spi_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct mcr3000_spi_gpio_data *data = dev_get_priv(dev);
	struct mcr3000_spi_gpio_plat *plat = dev_get_plat(dev);
	char name[32], *str;

	data->base = map_sysmem(plat->addr, 2);

	snprintf(name, sizeof(name), "CHIPSELECT@%lx_", plat->addr);
	str = strdup(name);

	if (!str)
		return -ENOMEM;

	uc_priv->bank_name = str;
	uc_priv->gpio_count = 16;

	return 0;
}

static const struct dm_gpio_ops gpio_mcr3000_spi_ops = {
	.get_value		= mcr3000_spi_gpio_get_value,
	.set_value		= mcr3000_spi_gpio_set_value,
	.direction_input	= mcr3000_spi_gpio_direction_input,
	.direction_output	= mcr3000_spi_gpio_set_value,
	.get_function		= mcr3000_spi_gpio_get_function,
};

static const struct udevice_id mcr3000_spi_gpio_ids[] = {
	{ .compatible = "s3k,mcr3000-cpld-csspi"},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mcr3000_spi_gpio) = {
	.name	= "mcr3000_spi_chipselect",
	.id	= UCLASS_GPIO,
	.ops	= &gpio_mcr3000_spi_ops,
	.of_to_plat = mcr3000_spi_gpio_ofdata_to_platdata,
	.plat_auto = sizeof(struct mcr3000_spi_gpio_plat),
	.of_match = mcr3000_spi_gpio_ids,
	.probe	= mcr3000_spi_gpio_probe,
	.priv_auto = sizeof(struct mcr3000_spi_gpio_data),
};
