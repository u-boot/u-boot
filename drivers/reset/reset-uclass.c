/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <reset.h>
#include <reset-uclass.h>

DECLARE_GLOBAL_DATA_PTR;

static inline struct reset_ops *reset_dev_ops(struct udevice *dev)
{
	return (struct reset_ops *)dev->driver->ops;
}

static int reset_of_xlate_default(struct reset_ctl *reset_ctl,
				  struct fdtdec_phandle_args *args)
{
	debug("%s(reset_ctl=%p)\n", __func__, reset_ctl);

	if (args->args_count != 1) {
		debug("Invaild args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	reset_ctl->id = args->args[0];

	return 0;
}

int reset_get_by_index(struct udevice *dev, int index,
		       struct reset_ctl *reset_ctl)
{
	struct fdtdec_phandle_args args;
	int ret;
	struct udevice *dev_reset;
	struct reset_ops *ops;

	debug("%s(dev=%p, index=%d, reset_ctl=%p)\n", __func__, dev, index,
	      reset_ctl);

	ret = fdtdec_parse_phandle_with_args(gd->fdt_blob, dev->of_offset,
					     "resets", "#reset-cells", 0,
					     index, &args);
	if (ret) {
		debug("%s: fdtdec_parse_phandle_with_args failed: %d\n",
		      __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_RESET, args.node,
					     &dev_reset);
	if (ret) {
		debug("%s: uclass_get_device_by_of_offset failed: %d\n",
		      __func__, ret);
		return ret;
	}
	ops = reset_dev_ops(dev_reset);

	reset_ctl->dev = dev_reset;
	if (ops->of_xlate)
		ret = ops->of_xlate(reset_ctl, &args);
	else
		ret = reset_of_xlate_default(reset_ctl, &args);
	if (ret) {
		debug("of_xlate() failed: %d\n", ret);
		return ret;
	}

	ret = ops->request(reset_ctl);
	if (ret) {
		debug("ops->request() failed: %d\n", ret);
		return ret;
	}

	return 0;
}

int reset_get_by_name(struct udevice *dev, const char *name,
		     struct reset_ctl *reset_ctl)
{
	int index;

	debug("%s(dev=%p, name=%s, reset_ctl=%p)\n", __func__, dev, name,
	      reset_ctl);

	index = fdt_find_string(gd->fdt_blob, dev->of_offset, "reset-names",
				name);
	if (index < 0) {
		debug("fdt_find_string() failed: %d\n", index);
		return index;
	}

	return reset_get_by_index(dev, index, reset_ctl);
}

int reset_free(struct reset_ctl *reset_ctl)
{
	struct reset_ops *ops = reset_dev_ops(reset_ctl->dev);

	debug("%s(reset_ctl=%p)\n", __func__, reset_ctl);

	return ops->free(reset_ctl);
}

int reset_assert(struct reset_ctl *reset_ctl)
{
	struct reset_ops *ops = reset_dev_ops(reset_ctl->dev);

	debug("%s(reset_ctl=%p)\n", __func__, reset_ctl);

	return ops->rst_assert(reset_ctl);
}

int reset_deassert(struct reset_ctl *reset_ctl)
{
	struct reset_ops *ops = reset_dev_ops(reset_ctl->dev);

	debug("%s(reset_ctl=%p)\n", __func__, reset_ctl);

	return ops->rst_deassert(reset_ctl);
}

UCLASS_DRIVER(reset) = {
	.id		= UCLASS_RESET,
	.name		= "reset",
};
