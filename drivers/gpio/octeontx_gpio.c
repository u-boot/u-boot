// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 *
 * (C) Copyright 2011
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <ajay.bhargav@einfochips.com>
 *
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <errno.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/bitops.h>
#include <dt-bindings/gpio/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

/** Returns the bit value to write or read based on the offset */
#define GPIO_BIT(x)		(1ULL << ((x) & 0x3f))

#define GPIO_RX_DAT		(0x0)
#define GPIO_TX_SET		(0x8)
#define GPIO_TX_CLR		(0x10)
#define GPIO_CONST		(0x90)
#define GPIO_RX1_DAT		(0x1400)
#define GPIO_TX1_SET		(0x1408)
#define GPIO_TX1_CLR		(0x1410)

/** Returns the offset to the output register based on the offset and value */
#define GPIO_TX_REG(offset, value)					\
	((offset) >= 64 ? ((value) ? GPIO_TX1_SET : GPIO_TX1_CLR) :	\
			  ((value) ? GPIO_TX_SET : GPIO_TX_CLR))

/** Returns the offset to the input data register based on the offset */
#define GPIO_RX_DAT_REG(offset) (((offset) >= 64) ? GPIO_RX1_DAT : GPIO_RX_DAT)

/** Returns the bit configuration register based on the offset */
#define GPIO_BIT_CFG(x)		(0x400 + 8 * (x))
#define GPIO_BIT_CFG_FN(x)	(((x) >> 16) & 0x3ff)
#define GPIO_BIT_CFG_TX_OE(x)	((x) & 0x1)
#define GPIO_BIT_CFG_RX_DAT(x)	((x) & 0x1)

/** PCI ID on NCB bus */
#define PCI_DEVICE_ID_OCTEONTX_GPIO	0xa00a

union gpio_const {
	u64 u;
	struct {
		u64 gpios:8;	/** Number of GPIOs implemented */
		u64 pp:8;	/** Number of PP vectors */
		u64:48;	/* Reserved */
	} s;
};

struct octeontx_gpio {
	void __iomem *baseaddr;
};

static int octeontx_gpio_dir_input(struct udevice *dev, unsigned int offset)
{
	struct octeontx_gpio *gpio = dev_get_priv(dev);

	debug("%s(%s, %u)\n", __func__, dev->name, offset);
	clrbits_le64(gpio->baseaddr + GPIO_BIT_CFG(offset),
		     (0x3ffUL << 16) | 4UL | 2UL | 1UL);
	return 0;
}

static int octeontx_gpio_dir_output(struct udevice *dev, unsigned int offset,
				    int value)
{
	struct octeontx_gpio *gpio = dev_get_priv(dev);

	debug("%s(%s, %u, %d)\n", __func__, dev->name, offset, value);
	writeq(GPIO_BIT(offset), gpio->baseaddr + GPIO_TX_REG(offset, value));

	clrsetbits_le64(gpio->baseaddr + GPIO_BIT_CFG(offset),
			((0x3ffUL << 16) | 4UL), 1UL);
	return 0;
}

static int octeontx_gpio_get_value(struct udevice *dev,
				   unsigned int offset)
{
	struct octeontx_gpio *gpio = dev_get_priv(dev);
	u64 reg = readq(gpio->baseaddr + GPIO_RX_DAT_REG(offset));

	debug("%s(%s, %u): value: %d\n", __func__, dev->name, offset,
	      !!(reg & GPIO_BIT(offset)));

	return !!(reg & GPIO_BIT(offset));
}

static int octeontx_gpio_set_value(struct udevice *dev,
				   unsigned int offset, int value)
{
	struct octeontx_gpio *gpio = dev_get_priv(dev);

	debug("%s(%s, %u, %d)\n", __func__, dev->name, offset, value);
	writeq(GPIO_BIT(offset), gpio->baseaddr + GPIO_TX_REG(offset, value));

	return 0;
}

static int octeontx_gpio_get_function(struct udevice *dev,
				      unsigned int offset)
{
	struct octeontx_gpio *gpio = dev_get_priv(dev);
	u64 pinsel = readl(gpio->baseaddr + GPIO_BIT_CFG(offset));

	debug("%s(%s, %u): pinsel: 0x%llx\n", __func__, dev->name, offset,
	      pinsel);
	if (GPIO_BIT_CFG_FN(pinsel))
		return GPIOF_FUNC;
	else if (GPIO_BIT_CFG_TX_OE(pinsel))
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static int octeontx_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
			       struct ofnode_phandle_args *args)
{
	if (args->args_count < 1)
		return -EINVAL;

	desc->offset = args->args[0];
	desc->flags = 0;
	if (args->args_count > 1) {
		if (args->args[1] & GPIO_ACTIVE_LOW)
			desc->flags |= GPIOD_ACTIVE_LOW;
		/* In the future add tri-state flag support */
	}
	return 0;
}

static const struct dm_gpio_ops octeontx_gpio_ops = {
	.direction_input	= octeontx_gpio_dir_input,
	.direction_output	= octeontx_gpio_dir_output,
	.get_value		= octeontx_gpio_get_value,
	.set_value		= octeontx_gpio_set_value,
	.get_function		= octeontx_gpio_get_function,
	.xlate			= octeontx_gpio_xlate,
};

static int octeontx_gpio_probe(struct udevice *dev)
{
	pci_dev_t bdf = dm_pci_get_bdf(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct octeontx_gpio *priv = dev_get_priv(dev);
	union gpio_const gpio_const;
	char *end;
	const char *status;

	status = ofnode_read_string(dev->node, "status");

	if (status && !strncmp(status, "ok", 2)) {
		debug("%s(%s): GPIO device disabled in device tree\n",
		      __func__, dev->name);
		return -1;
	}

	dev->req_seq = PCI_FUNC(bdf);
	priv->baseaddr = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0,
					PCI_REGION_MEM);

	if (!priv->baseaddr) {
		debug("%s(%s): Could not get base address\n",
		      __func__, dev->name);
		return -1;
	}

	gpio_const.u = readq(priv->baseaddr + GPIO_CONST);

	debug("%s(%s): base address: %p, of_offset: %ld, pin count: %d\n",
	      __func__, dev->name, priv->baseaddr, dev->node.of_offset,
	      gpio_const.s.gpios);

	uc_priv->gpio_count = gpio_const.s.gpios;
	uc_priv->bank_name  = strdup(dev->name);
	end = strchr(uc_priv->bank_name, '@');
	end[0] = 'A' + dev->seq;
	end[1] = '\0';

	return 0;
}

static const struct udevice_id octeontx_gpio_ids[] = {
	{ .compatible = "cavium,thunder-8890-gpio" },
	{ }
};

U_BOOT_DRIVER(octeontx_gpio) = {
	.name	= "octeontx_gpio",
	.id	= UCLASS_GPIO,
	.of_match = of_match_ptr(octeontx_gpio_ids),
	.probe = octeontx_gpio_probe,
	.priv_auto_alloc_size = sizeof(struct octeontx_gpio),
	.ops	= &octeontx_gpio_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

static struct pci_device_id octeontx_gpio_supported[] = {
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_OCTEONTX_GPIO) },
	{ },
};

U_BOOT_PCI_DEVICE(octeontx_gpio, octeontx_gpio_supported);
