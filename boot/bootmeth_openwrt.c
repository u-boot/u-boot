// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootmethod for OpenWrt
 *
 * Copyright (C) 2026 Daniel Golle <daniel@makrotopia.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <blk.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootm.h>
#include <bootmeth.h>
#include <dm.h>
#include <imagemap.h>
#include <malloc.h>
#include <mapmem.h>

static int openwrt_check(struct udevice *dev, struct bootflow_iter *iter)
{
	if (bootflow_iter_check_blk(iter))
		return log_msg_ret("blk", -EOPNOTSUPP);

	return 0;
}

static int openwrt_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	return log_msg_ret("nyi", -ENOENT);
}

static int openwrt_boot(struct udevice *dev, struct bootflow *bflow)
{
	return log_msg_ret("nyi", -ENOSYS);
}

static int openwrt_bootmeth_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->desc = "OpenWrt";
	plat->flags = BOOTMETHF_ANY_PART;

	return 0;
}

static struct bootmeth_ops openwrt_bootmeth_ops = {
	.check		= openwrt_check,
	.read_bootflow	= openwrt_read_bootflow,
	.boot		= openwrt_boot,
};

static const struct udevice_id openwrt_bootmeth_ids[] = {
	{ .compatible = "u-boot,openwrt" },
	{ }
};

U_BOOT_DRIVER(bootmeth_openwrt) = {
	.name		= "bootmeth_openwrt",
	.id		= UCLASS_BOOTMETH,
	.of_match	= openwrt_bootmeth_ids,
	.ops		= &openwrt_bootmeth_ops,
	.bind		= openwrt_bootmeth_bind,
};
