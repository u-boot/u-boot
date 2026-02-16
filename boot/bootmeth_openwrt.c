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
#include <part.h>
#include <linux/libfdt.h>
#include <linux/sizes.h>

static int openwrt_check(struct udevice *dev, struct bootflow_iter *iter)
{
	if (bootflow_iter_check_blk(iter))
		return log_msg_ret("blk", -EOPNOTSUPP);

	return 0;
}

static int openwrt_partition_name_as_filename(struct bootflow *bflow)
{
		struct blk_desc *desc = dev_get_uclass_plat(bflow->blk);
		struct disk_partition info;
		int ret;

		ret = part_get_info(desc, bflow->part, &info);
		if (ret)
			return log_msg_ret("part", ret);

		bflow->fname = strdup(info.name);
		if (!bflow->fname)
			return log_msg_ret("fn", -ENOMEM);

		return 0;
}

static int openwrt_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	const struct imagemap_ops *ops;
	struct udevice *imdev = NULL;
	char hdr[40];
	int ret;

	/* For block devices, resolve the GPT partition label */
	if (bflow->blk) {
		ret = openwrt_partition_name_as_filename(bflow);
		if (ret)
			return ret;
	}

	/* Probe for a valid FDT/FIT header via imagemap */
	ret = imagemap_create(bflow->blk ? bflow->blk :
			      dev_get_parent(bflow->dev),
			      bflow->fname, bflow->part, &imdev);
	if (ret)
		return log_msg_ret("im", ret);

	ops = imagemap_get_ops(imdev);
	ret = ops->read(imdev, 0, sizeof(hdr), hdr);
	imagemap_cleanup(imdev);

	if (ret)
		return log_msg_ret("rd", -EIO);

	if (fdt_check_header(hdr))
		return -ENOENT;

	return 0;
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
