/*
 * (C) Copyright 2015 Marek Vasut <marex@denx.de>
 *
 * DesignWare APB GPIO driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <asm/arch/gpio.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <errno.h>

DECLARE_GLOBAL_DATA_PTR;

#define GPIO_SWPORTA_DR		0x00
#define GPIO_SWPORTA_DDR	0x04
#define GPIO_INTEN		0x30
#define GPIO_INTMASK		0x34
#define GPIO_INTTYPE_LEVEL	0x38
#define GPIO_INT_POLARITY	0x3c
#define GPIO_INTSTATUS		0x40
#define GPIO_PORTA_DEBOUNCE	0x48
#define GPIO_PORTA_EOI		0x4c
#define GPIO_EXT_PORTA		0x50

struct gpio_dwapb_platdata {
	const char	*name;
	int		bank;
	int		pins;
	fdt_addr_t	base;
};

static int dwapb_gpio_direction_input(struct udevice *dev, unsigned pin)
{
	struct gpio_dwapb_platdata *plat = dev_get_platdata(dev);

	clrbits_le32(plat->base + GPIO_SWPORTA_DDR, 1 << pin);
	return 0;
}

static int dwapb_gpio_direction_output(struct udevice *dev, unsigned pin,
				     int val)
{
	struct gpio_dwapb_platdata *plat = dev_get_platdata(dev);

	setbits_le32(plat->base + GPIO_SWPORTA_DDR, 1 << pin);

	if (val)
		setbits_le32(plat->base + GPIO_SWPORTA_DR, 1 << pin);
	else
		clrbits_le32(plat->base + GPIO_SWPORTA_DR, 1 << pin);

	return 0;
}

static int dwapb_gpio_get_value(struct udevice *dev, unsigned pin)
{
	struct gpio_dwapb_platdata *plat = dev_get_platdata(dev);
	return !!(readl(plat->base + GPIO_EXT_PORTA) & (1 << pin));
}


static int dwapb_gpio_set_value(struct udevice *dev, unsigned pin, int val)
{
	struct gpio_dwapb_platdata *plat = dev_get_platdata(dev);

	if (val)
		setbits_le32(plat->base + GPIO_SWPORTA_DR, 1 << pin);
	else
		clrbits_le32(plat->base + GPIO_SWPORTA_DR, 1 << pin);

	return 0;
}

static const struct dm_gpio_ops gpio_dwapb_ops = {
	.direction_input	= dwapb_gpio_direction_input,
	.direction_output	= dwapb_gpio_direction_output,
	.get_value		= dwapb_gpio_get_value,
	.set_value		= dwapb_gpio_set_value,
};

static int gpio_dwapb_probe(struct udevice *dev)
{
	struct gpio_dev_priv *priv = dev_get_uclass_priv(dev);
	struct gpio_dwapb_platdata *plat = dev->platdata;

	if (!plat)
		return 0;

	priv->gpio_count = plat->pins;
	priv->bank_name = plat->name;

	return 0;
}

static int gpio_dwapb_bind(struct udevice *dev)
{
	struct gpio_dwapb_platdata *plat = dev_get_platdata(dev);
	const void *blob = gd->fdt_blob;
	struct udevice *subdev;
	fdt_addr_t base;
	int ret, node, bank = 0;

	/* If this is a child device, there is nothing to do here */
	if (plat)
		return 0;

	base = fdtdec_get_addr(blob, dev->of_offset, "reg");
	if (base == FDT_ADDR_T_NONE) {
		debug("Can't get the GPIO register base address\n");
		return -ENXIO;
	}

	for (node = fdt_first_subnode(blob, dev->of_offset);
	     node > 0;
	     node = fdt_next_subnode(blob, node)) {
		if (!fdtdec_get_bool(blob, node, "gpio-controller"))
			continue;

		plat = NULL;
		plat = calloc(1, sizeof(*plat));
		if (!plat)
			return -ENOMEM;

		plat->base = base;
		plat->bank = bank;
		plat->pins = fdtdec_get_int(blob, node, "snps,nr-gpios", 0);
		ret = fdt_get_string(blob, node, "bank-name", &plat->name);
		if (ret)
			goto err;

		ret = device_bind(dev, dev->driver, plat->name,
				  plat, -1, &subdev);
		if (ret)
			goto err;

		subdev->of_offset = node;
		bank++;
	}

	return 0;

err:
	free(plat);
	return ret;
}

static const struct udevice_id gpio_dwapb_ids[] = {
	{ .compatible = "snps,dw-apb-gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_dwapb) = {
	.name		= "gpio-dwapb",
	.id		= UCLASS_GPIO,
	.of_match	= gpio_dwapb_ids,
	.ops		= &gpio_dwapb_ops,
	.bind		= gpio_dwapb_bind,
	.probe		= gpio_dwapb_probe,
};
