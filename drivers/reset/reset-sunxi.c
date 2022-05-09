// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (C) 2018 Amarula Solutions.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <reset-uclass.h>
#include <asm/io.h>
#include <clk/sunxi.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <linux/bitops.h>
#include <linux/log2.h>

struct sunxi_reset_plat {
	void *base;
	const struct ccu_desc *desc;
};

static const struct ccu_reset *plat_to_reset(struct sunxi_reset_plat *plat,
					     unsigned long id)
{
	return	&plat->desc->resets[id];
}

static int sunxi_reset_request(struct reset_ctl *reset_ctl)
{
	struct sunxi_reset_plat *plat = dev_get_plat(reset_ctl->dev);

	debug("%s: (RST#%ld)\n", __func__, reset_ctl->id);

	if (reset_ctl->id >= plat->desc->num_resets)
		return -EINVAL;

	return 0;
}

static int sunxi_set_reset(struct reset_ctl *reset_ctl, bool on)
{
	struct sunxi_reset_plat *plat = dev_get_plat(reset_ctl->dev);
	const struct ccu_reset *reset = plat_to_reset(plat, reset_ctl->id);
	u32 reg;

	if (!(reset->flags & CCU_RST_F_IS_VALID)) {
		printf("%s: (RST#%ld) unhandled\n", __func__, reset_ctl->id);
		return 0;
	}

	debug("%s: (RST#%ld) off#0x%x, BIT(%d)\n", __func__,
	      reset_ctl->id, reset->off, ilog2(reset->bit));

	reg = readl(plat->base + reset->off);
	if (on)
		reg |= reset->bit;
	else
		reg &= ~reset->bit;

	writel(reg, plat->base + reset->off);

	return 0;
}

static int sunxi_reset_assert(struct reset_ctl *reset_ctl)
{
	return sunxi_set_reset(reset_ctl, false);
}

static int sunxi_reset_deassert(struct reset_ctl *reset_ctl)
{
	return sunxi_set_reset(reset_ctl, true);
}

struct reset_ops sunxi_reset_ops = {
	.request = sunxi_reset_request,
	.rst_assert = sunxi_reset_assert,
	.rst_deassert = sunxi_reset_deassert,
};

static int sunxi_reset_of_to_plat(struct udevice *dev)
{
	struct sunxi_reset_plat *plat = dev_get_plat(dev);

	plat->base = dev_read_addr_ptr(dev);

	return 0;
}

int sunxi_reset_bind(struct udevice *dev)
{
	struct udevice *rst_dev;
	struct sunxi_reset_plat *plat;
	int ret;

	ret = device_bind_driver_to_node(dev, "sunxi_reset", "reset",
					 dev_ofnode(dev), &rst_dev);
	if (ret) {
		debug("failed to bind sunxi_reset driver (ret=%d)\n", ret);
		return ret;
	}
	plat = malloc(sizeof(struct sunxi_reset_plat));
	plat->desc = (const struct ccu_desc *)dev_get_driver_data(dev);
	dev_set_plat(rst_dev, plat);

	return 0;
}

U_BOOT_DRIVER(sunxi_reset) = {
	.name		= "sunxi_reset",
	.id		= UCLASS_RESET,
	.ops		= &sunxi_reset_ops,
	.of_to_plat	= sunxi_reset_of_to_plat,
	.plat_auto	= sizeof(struct sunxi_reset_plat),
};
