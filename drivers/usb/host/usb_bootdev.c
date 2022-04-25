// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootdevice for USB
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bootdev.h>
#include <dm.h>
#include <usb.h>

static int usb_get_bootflow(struct udevice *dev, struct bootflow_iter *iter,
			    struct bootflow *bflow)
{
	struct udevice *blk;
	int ret;

	ret = bootdev_get_sibling_blk(dev, &blk);
	/*
	 * If there is no media, indicate that no more partitions should be
	 * checked
	 */
	if (ret == -EOPNOTSUPP)
		ret = -ESHUTDOWN;
	if (ret)
		return log_msg_ret("blk", ret);
	assert(blk);
	ret = bootdev_find_in_blk(dev, blk, iter, bflow);
	if (ret)
		return log_msg_ret("find", ret);

	return 0;
}

static int usb_bootdev_bind(struct udevice *dev)
{
	struct bootdev_uc_plat *ucp = dev_get_uclass_plat(dev);

	ucp->prio = BOOTDEVP_3_SCAN_SLOW;

	return 0;
}

struct bootdev_ops usb_bootdev_ops = {
	.get_bootflow	= usb_get_bootflow,
};

static const struct udevice_id usb_bootdev_ids[] = {
	{ .compatible = "u-boot,bootdev-usb" },
	{ }
};

U_BOOT_DRIVER(usb_bootdev) = {
	.name		= "usb_bootdev",
	.id		= UCLASS_BOOTDEV,
	.ops		= &usb_bootdev_ops,
	.bind		= usb_bootdev_bind,
	.of_match	= usb_bootdev_ids,
};
