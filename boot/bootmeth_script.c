// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootmethod for booting via a U-Boot script
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <common.h>
#include <blk.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <bootstd.h>
#include <dm.h>
#include <env.h>
#include <fs.h>
#include <image.h>
#include <malloc.h>
#include <mapmem.h>

#define SCRIPT_FNAME1	"boot.scr.uimg"
#define SCRIPT_FNAME2	"boot.scr"

static int script_check(struct udevice *dev, struct bootflow_iter *iter)
{
	int ret;

	/* This only works on block devices */
	ret = bootflow_iter_uses_blk_dev(iter);
	if (ret)
		return log_msg_ret("blk", ret);

	return 0;
}

static int script_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	struct blk_desc *desc = NULL;
	const char *const *prefixes;
	struct udevice *bootstd;
	const char *prefix;
	int ret, i;

	ret = uclass_first_device_err(UCLASS_BOOTSTD, &bootstd);
	if (ret)
		return log_msg_ret("std", ret);

	/* We require a partition table */
	if (!bflow->part)
		return -ENOENT;

	if (bflow->blk)
		 desc = dev_get_uclass_plat(bflow->blk);

	prefixes = bootstd_get_prefixes(bootstd);
	i = 0;
	do {
		prefix = prefixes ? prefixes[i] : NULL;

		ret = bootmeth_try_file(bflow, desc, prefix, SCRIPT_FNAME1);
		if (ret)
			ret = bootmeth_try_file(bflow, desc, prefix,
						SCRIPT_FNAME2);
	} while (ret && prefixes && prefixes[++i]);
	if (ret)
		return log_msg_ret("try", ret);

	bflow->subdir = strdup(prefix ? prefix : "");
	if (!bflow->subdir)
		return log_msg_ret("prefix", -ENOMEM);

	ret = bootmeth_alloc_file(bflow, 0x10000, 1);
	if (ret)
		return log_msg_ret("read", ret);

	return 0;
}

static int script_boot(struct udevice *dev, struct bootflow *bflow)
{
	struct blk_desc *desc = dev_get_uclass_plat(bflow->blk);
	ulong addr;
	int ret;

	ret = env_set("devtype", blk_get_devtype(bflow->blk));
	if (!ret)
		ret = env_set_hex("devnum", desc->devnum);
	if (!ret)
		ret = env_set("prefix", bflow->subdir);
	if (!ret && IS_ENABLED(CONFIG_ARCH_SUNXI) &&
	    !strcmp("mmc", blk_get_devtype(bflow->blk)))
		ret = env_set_hex("mmc_bootdev", desc->devnum);
	if (ret)
		return log_msg_ret("env", ret);

	log_debug("devtype: %s\n", env_get("devtype"));
	log_debug("devnum: %s\n", env_get("devnum"));
	log_debug("prefix: %s\n", env_get("prefix"));
	log_debug("mmc_bootdev: %s\n", env_get("mmc_bootdev"));

	addr = map_to_sysmem(bflow->buf);
	ret = image_source_script(addr, NULL);
	if (ret)
		return log_msg_ret("boot", ret);

	return 0;
}

static int script_bootmeth_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->desc = IS_ENABLED(CONFIG_BOOTSTD_FULL) ?
		"Script boot from a block device" : "script";

	return 0;
}

static struct bootmeth_ops script_bootmeth_ops = {
	.check		= script_check,
	.read_bootflow	= script_read_bootflow,
	.read_file	= bootmeth_common_read_file,
	.boot		= script_boot,
};

static const struct udevice_id script_bootmeth_ids[] = {
	{ .compatible = "u-boot,script" },
	{ }
};

U_BOOT_DRIVER(bootmeth_script) = {
	.name		= "bootmeth_script",
	.id		= UCLASS_BOOTMETH,
	.of_match	= script_bootmeth_ids,
	.ops		= &script_bootmeth_ops,
	.bind		= script_bootmeth_bind,
};
