/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * Derived from linux/arch/mips/bcm63xx/reset.c:
 *	Copyright (C) 2012 Jonas Gorski <jonas.gorski@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <reset-uclass.h>
#include <asm/io.h>

#define MAX_RESETS	32

struct bcm6345_reset_priv {
	void __iomem *regs;
};

static int bcm6345_reset_assert(struct reset_ctl *rst)
{
	struct bcm6345_reset_priv *priv = dev_get_priv(rst->dev);

	clrbits_be32(priv->regs, BIT(rst->id));
	mdelay(20);

	return 0;
}

static int bcm6345_reset_deassert(struct reset_ctl *rst)
{
	struct bcm6345_reset_priv *priv = dev_get_priv(rst->dev);

	setbits_be32(priv->regs, BIT(rst->id));
	mdelay(20);

	return 0;
}

static int bcm6345_reset_free(struct reset_ctl *rst)
{
	return 0;
}

static int bcm6345_reset_request(struct reset_ctl *rst)
{
	if (rst->id >= MAX_RESETS)
		return -EINVAL;

	return bcm6345_reset_assert(rst);
}

struct reset_ops bcm6345_reset_reset_ops = {
	.free = bcm6345_reset_free,
	.request = bcm6345_reset_request,
	.rst_assert = bcm6345_reset_assert,
	.rst_deassert = bcm6345_reset_deassert,
};

static const struct udevice_id bcm6345_reset_ids[] = {
	{ .compatible = "brcm,bcm6345-reset" },
	{ /* sentinel */ }
};

static int bcm6345_reset_probe(struct udevice *dev)
{
	struct bcm6345_reset_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;
	fdt_size_t size;

	addr = devfdt_get_addr_size_index(dev, 0, &size);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->regs = ioremap(addr, size);

	return 0;
}

U_BOOT_DRIVER(bcm6345_reset) = {
	.name = "bcm6345-reset",
	.id = UCLASS_RESET,
	.of_match = bcm6345_reset_ids,
	.ops = &bcm6345_reset_reset_ops,
	.probe = bcm6345_reset_probe,
	.priv_auto_alloc_size = sizeof(struct bcm6345_reset_priv),
};
