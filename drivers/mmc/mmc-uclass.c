/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mmc.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/root.h>

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
