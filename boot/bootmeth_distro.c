// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootmethod for distro boot (syslinux boot from a block device)
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <common.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <bootstd.h>
#include <command.h>
#include <distro.h>
#include <dm.h>
#include <fs.h>
#include <malloc.h>
#include <mapmem.h>
#include <mmc.h>
#include <pxe_utils.h>

static int disto_getfile(struct pxe_context *ctx, const char *file_path,
			 char *file_addr, ulong *sizep)
{
	struct distro_info *info = ctx->userdata;
	ulong addr;
	int ret;

	addr = simple_strtoul(file_addr, NULL, 16);

	/* Allow up to 1GB */
	*sizep = 1 << 30;
	ret = bootmeth_read_file(info->dev, info->bflow, file_path, addr,
				 sizep);
	if (ret)
		return log_msg_ret("read", ret);

	return 0;
}

static int distro_check(struct udevice *dev, struct bootflow_iter *iter)
{
	int ret;

	/* This only works on block devices */
	ret = bootflow_iter_uses_blk_dev(iter);
	if (ret)
		return log_msg_ret("blk", ret);

	return 0;
}

static int distro_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	struct blk_desc *desc;
	const char *const *prefixes;
	struct udevice *bootstd;
	const char *prefix;
	loff_t size;
	int ret, i;

	ret = uclass_first_device_err(UCLASS_BOOTSTD, &bootstd);
	if (ret)
		return log_msg_ret("std", ret);

	/* If a block device, we require a partition table */
	if (bflow->blk && !bflow->part)
		return -ENOENT;

	prefixes = bootstd_get_prefixes(bootstd);
	i = 0;
	desc = bflow->blk ? dev_get_uclass_plat(bflow->blk) : NULL;
	do {
		prefix = prefixes ? prefixes[i] : NULL;

		ret = bootmeth_try_file(bflow, desc, prefix, DISTRO_FNAME);
	} while (ret && prefixes && prefixes[++i]);
	if (ret)
		return log_msg_ret("try", ret);
	size = bflow->size;

	ret = bootmeth_alloc_file(bflow, 0x10000, 1);
	if (ret)
		return log_msg_ret("read", ret);

	return 0;
}

static int distro_boot(struct udevice *dev, struct bootflow *bflow)
{
	struct cmd_tbl cmdtp = {};	/* dummy */
	struct pxe_context ctx;
	struct distro_info info;
	ulong addr;
	int ret;

	addr = map_to_sysmem(bflow->buf);
	info.dev = dev;
	info.bflow = bflow;
	ret = pxe_setup_ctx(&ctx, &cmdtp, disto_getfile, &info, true,
			    bflow->subdir);
	if (ret)
		return log_msg_ret("ctx", -EINVAL);

	ret = pxe_process(&ctx, addr, false);
	if (ret)
		return log_msg_ret("bread", -EINVAL);

	return 0;
}

static int distro_bootmeth_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->desc = IS_ENABLED(CONFIG_BOOTSTD_FULL) ?
		"Syslinux boot from a block device" : "syslinux";

	return 0;
}

static struct bootmeth_ops distro_bootmeth_ops = {
	.check		= distro_check,
	.read_bootflow	= distro_read_bootflow,
	.read_file	= bootmeth_common_read_file,
	.boot		= distro_boot,
};

static const struct udevice_id distro_bootmeth_ids[] = {
	{ .compatible = "u-boot,distro-syslinux" },
	{ }
};

U_BOOT_DRIVER(bootmeth_distro) = {
	.name		= "bootmeth_distro",
	.id		= UCLASS_BOOTMETH,
	.of_match	= distro_bootmeth_ids,
	.ops		= &distro_bootmeth_ops,
	.bind		= distro_bootmeth_bind,
};
