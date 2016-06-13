/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mmc.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/root.h>
#include "mmc_private.h"

#ifdef CONFIG_DM_MMC_OPS
int dm_mmc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
		    struct mmc_data *data)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	struct dm_mmc_ops *ops = mmc_get_ops(dev);
	int ret;

	mmmc_trace_before_send(mmc, cmd);
	if (ops->send_cmd)
		ret = ops->send_cmd(dev, cmd, data);
	else
		ret = -ENOSYS;
	mmmc_trace_after_send(mmc, cmd, ret);

	return ret;
}

int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	return dm_mmc_send_cmd(mmc->dev, cmd, data);
}

int dm_mmc_set_ios(struct udevice *dev)
{
	struct dm_mmc_ops *ops = mmc_get_ops(dev);

	if (!ops->set_ios)
		return -ENOSYS;
	return ops->set_ios(dev);
}

int mmc_set_ios(struct mmc *mmc)
{
	return dm_mmc_set_ios(mmc->dev);
}

int dm_mmc_get_wp(struct udevice *dev)
{
	struct dm_mmc_ops *ops = mmc_get_ops(dev);

	if (!ops->get_wp)
		return -ENOSYS;
	return ops->get_wp(dev);
}

int mmc_getwp(struct mmc *mmc)
{
	return dm_mmc_get_wp(mmc->dev);
}

int dm_mmc_get_cd(struct udevice *dev)
{
	struct dm_mmc_ops *ops = mmc_get_ops(dev);

	if (!ops->get_cd)
		return -ENOSYS;
	return ops->get_cd(dev);
}

int mmc_getcd(struct mmc *mmc)
{
	return dm_mmc_get_cd(mmc->dev);
}
#endif

struct mmc *mmc_get_mmc_dev(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv;

	if (!device_active(dev))
		return NULL;
	upriv = dev_get_uclass_priv(dev);
	return upriv->mmc;
}

#ifdef CONFIG_BLK
struct mmc *find_mmc_device(int dev_num)
{
	struct udevice *dev, *mmc_dev;
	int ret;

	ret = blk_get_device(IF_TYPE_MMC, dev_num, &dev);

	if (ret) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
		printf("MMC Device %d not found\n", dev_num);
#endif
		return NULL;
	}

	mmc_dev = dev_get_parent(dev);

	return mmc_get_mmc_dev(mmc_dev);
}

int get_mmc_num(void)
{
	return max(blk_find_max_devnum(IF_TYPE_MMC), 0);
}

int mmc_get_next_devnum(void)
{
	int ret;

	ret = get_mmc_num();
	if (ret < 0)
		return ret;

	return ret + 1;
}

struct blk_desc *mmc_get_blk_desc(struct mmc *mmc)
{
	struct blk_desc *desc;
	struct udevice *dev;

	device_find_first_child(mmc->dev, &dev);
	if (!dev)
		return NULL;
	desc = dev_get_uclass_platdata(dev);

	return desc;
}

void mmc_do_preinit(void)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_MMC, &uc);
	if (ret)
		return;
	uclass_foreach_dev(dev, uc) {
		struct mmc *m = mmc_get_mmc_dev(dev);

		if (!m)
			continue;
#ifdef CONFIG_FSL_ESDHC_ADAPTER_IDENT
		mmc_set_preinit(m, 1);
#endif
		if (m->preinit)
			mmc_start_init(m);
	}
}

#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
void print_mmc_devices(char separator)
{
	struct udevice *dev;
	char *mmc_type;
	bool first = true;

	for (uclass_first_device(UCLASS_MMC, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		struct mmc *m = mmc_get_mmc_dev(dev);

		if (!first) {
			printf("%c", separator);
			if (separator != '\n')
				puts(" ");
		}
		if (m->has_init)
			mmc_type = IS_SD(m) ? "SD" : "eMMC";
		else
			mmc_type = NULL;

		printf("%s: %d", m->cfg->name, mmc_get_blk_desc(m)->devnum);
		if (mmc_type)
			printf(" (%s)", mmc_type);
	}

	printf("\n");
}

#else
void print_mmc_devices(char separator) { }
#endif

int mmc_bind(struct udevice *dev, struct mmc *mmc, const struct mmc_config *cfg)
{
	struct blk_desc *bdesc;
	struct udevice *bdev;
	int ret;

	ret = blk_create_devicef(dev, "mmc_blk", "blk", IF_TYPE_MMC, -1, 512,
				 0, &bdev);
	if (ret) {
		debug("Cannot create block device\n");
		return ret;
	}
	bdesc = dev_get_uclass_platdata(bdev);
	mmc->cfg = cfg;
	mmc->priv = dev;

	/* the following chunk was from mmc_register() */

	/* Setup dsr related values */
	mmc->dsr_imp = 0;
	mmc->dsr = 0xffffffff;
	/* Setup the universal parts of the block interface just once */
	bdesc->removable = 1;

	/* setup initial part type */
	bdesc->part_type = cfg->part_type;
	mmc->dev = dev;

	return 0;
}

int mmc_unbind(struct udevice *dev)
{
	struct udevice *bdev;

	device_find_first_child(dev, &bdev);
	if (bdev) {
		device_remove(bdev);
		device_unbind(bdev);
	}

	return 0;
}

static int mmc_select_hwpart(struct udevice *bdev, int hwpart)
{
	struct udevice *mmc_dev = dev_get_parent(bdev);
	struct mmc *mmc = mmc_get_mmc_dev(mmc_dev);
	struct blk_desc *desc = dev_get_uclass_platdata(bdev);
	int ret;

	if (desc->hwpart == hwpart)
		return 0;

	if (mmc->part_config == MMCPART_NOAVAILABLE)
		return -EMEDIUMTYPE;

	ret = mmc_switch_part(mmc, hwpart);
	if (ret)
		return ret;

	return 0;
}

static const struct blk_ops mmc_blk_ops = {
	.read	= mmc_bread,
#ifndef CONFIG_SPL_BUILD
	.write	= mmc_bwrite,
#endif
	.select_hwpart	= mmc_select_hwpart,
};

U_BOOT_DRIVER(mmc_blk) = {
	.name		= "mmc_blk",
	.id		= UCLASS_BLK,
	.ops		= &mmc_blk_ops,
};
#endif /* CONFIG_BLK */

U_BOOT_DRIVER(mmc) = {
	.name	= "mmc",
	.id	= UCLASS_MMC,
};

UCLASS_DRIVER(mmc) = {
	.id		= UCLASS_MMC,
	.name		= "mmc",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.per_device_auto_alloc_size = sizeof(struct mmc_uclass_priv),
};
