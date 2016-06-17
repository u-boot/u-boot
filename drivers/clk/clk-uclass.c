/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <dm/lists.h>
#include <dm/root.h>

DECLARE_GLOBAL_DATA_PTR;

ulong clk_get_rate(struct udevice *dev)
{
	struct clk_ops *ops = clk_get_ops(dev);

	if (!ops->get_rate)
		return -ENOSYS;

	return ops->get_rate(dev);
}

ulong clk_set_rate(struct udevice *dev, ulong rate)
{
	struct clk_ops *ops = clk_get_ops(dev);

	if (!ops->set_rate)
		return -ENOSYS;

	return ops->set_rate(dev, rate);
}

int clk_enable(struct udevice *dev, int periph)
{
	struct clk_ops *ops = clk_get_ops(dev);

	if (!ops->enable)
		return -ENOSYS;

	return ops->enable(dev, periph);
}

ulong clk_get_periph_rate(struct udevice *dev, int periph)
{
	struct clk_ops *ops = clk_get_ops(dev);

	if (!ops->get_periph_rate)
		return -ENOSYS;

	return ops->get_periph_rate(dev, periph);
}

ulong clk_set_periph_rate(struct udevice *dev, int periph, ulong rate)
{
	struct clk_ops *ops = clk_get_ops(dev);

	if (!ops->set_periph_rate)
		return -ENOSYS;

	return ops->set_periph_rate(dev, periph, rate);
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
int clk_get_by_index(struct udevice *dev, int index, struct udevice **clk_devp)
{
	int ret;
#ifdef CONFIG_SPL_BUILD
	u32 cell[2];

	if (index != 0)
		return -ENOSYS;
	assert(*clk_devp);
	ret = uclass_get_device(UCLASS_CLK, 0, clk_devp);
	if (ret)
		return ret;
	ret = fdtdec_get_int_array(gd->fdt_blob, dev->of_offset, "clocks",
				   cell, 2);
	if (ret)
		return ret;
	return cell[1];
#else
	struct fdtdec_phandle_args args;

	assert(*clk_devp);
	ret = fdtdec_parse_phandle_with_args(gd->fdt_blob, dev->of_offset,
					     "clocks", "#clock-cells", 0, index,
					     &args);
	if (ret) {
		debug("%s: fdtdec_parse_phandle_with_args failed: err=%d\n",
		      __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_CLK, args.node, clk_devp);
	if (ret) {
		debug("%s: uclass_get_device_by_of_offset failed: err=%d\n",
		      __func__, ret);
		return ret;
	}
	return args.args_count > 0 ? args.args[0] : 0;
#endif
}
#endif

UCLASS_DRIVER(clk) = {
	.id		= UCLASS_CLK,
	.name		= "clk",
};
