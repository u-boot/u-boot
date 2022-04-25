// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootdevice for MMC
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bootdev.h>
#include <dm.h>
#include <mmc.h>

static int mmc_get_bootflow(struct udevice *dev, struct bootflow_iter *iter,
			    struct bootflow *bflow)
{
	struct udevice *mmc_dev = dev_get_parent(dev);
	struct udevice *blk;
	int ret;

	ret = mmc_get_blk(mmc_dev, &blk);
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

static int mmc_bootdev_bind(struct udevice *dev)
{
	struct bootdev_uc_plat *ucp = dev_get_uclass_plat(dev);

	ucp->prio = BOOTDEVP_0_INTERNAL_FAST;

	return 0;
}

struct bootdev_ops mmc_bootdev_ops = {
	.get_bootflow	= mmc_get_bootflow,
};

static const struct udevice_id mmc_bootdev_ids[] = {
	{ .compatible = "u-boot,bootdev-mmc" },
	{ }
};

U_BOOT_DRIVER(mmc_bootdev) = {
	.name		= "mmc_bootdev",
	.id		= UCLASS_BOOTDEV,
	.ops		= &mmc_bootdev_ops,
	.bind		= mmc_bootdev_bind,
	.of_match	= mmc_bootdev_ids,
};
