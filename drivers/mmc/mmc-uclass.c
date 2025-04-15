// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 * Copyright 2020 NXP
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_MMC

#include <bootdev.h>
#include <log.h>
#include <mmc.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <linux/compat.h>
#include "mmc_private.h"

static int dm_mmc_get_b_max(struct udevice *dev, void *dst, lbaint_t blkcnt)
{
	struct dm_mmc_ops *ops = mmc_get_ops(dev);
	struct mmc *mmc = mmc_get_mmc_dev(dev);

	if (ops->get_b_max)
		return ops->get_b_max(dev, dst, blkcnt);
	else
		return mmc->cfg->b_max;
}

int mmc_get_b_max(struct mmc *mmc, void *dst, lbaint_t blkcnt)
{
	return dm_mmc_get_b_max(mmc->dev, dst, blkcnt);
}

static int dm_mmc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
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

static int dm_mmc_set_ios(struct udevice *dev)
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

static int dm_mmc_wait_dat0(struct udevice *dev, int state, int timeout_us)
{
	struct dm_mmc_ops *ops = mmc_get_ops(dev);

	if (!ops->wait_dat0)
		return -ENOSYS;
	return ops->wait_dat0(dev, state, timeout_us);
}

int mmc_wait_dat0(struct mmc *mmc, int state, int timeout_us)
{
	return dm_mmc_wait_dat0(mmc->dev, state, timeout_us);
}

static int dm_mmc_get_wp(struct udevice *dev)
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

static int dm_mmc_get_cd(struct udevice *dev)
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

#if CONFIG_IS_ENABLED(MMC_SUPPORTS_TUNING)
static int dm_mmc_execute_tuning(struct udevice *dev, uint opcode)
{
	struct dm_mmc_ops *ops = mmc_get_ops(dev);

	if (!ops->execute_tuning)
		return -ENOSYS;
	return ops->execute_tuning(dev, opcode);
}

int mmc_execute_tuning(struct mmc *mmc, uint opcode)
{
	int ret;

	mmc->tuning = true;
	ret = dm_mmc_execute_tuning(mmc->dev, opcode);
	mmc->tuning = false;

	return ret;
}
#endif

#if CONFIG_IS_ENABLED(MMC_HS400_ES_SUPPORT)
static int dm_mmc_set_enhanced_strobe(struct udevice *dev)
{
	struct dm_mmc_ops *ops = mmc_get_ops(dev);

	if (ops->set_enhanced_strobe)
		return ops->set_enhanced_strobe(dev);

	return -ENOTSUPP;
}

int mmc_set_enhanced_strobe(struct mmc *mmc)
{
	return dm_mmc_set_enhanced_strobe(mmc->dev);
}
#endif

static int dm_mmc_hs400_prepare_ddr(struct udevice *dev)
{
	struct dm_mmc_ops *ops = mmc_get_ops(dev);

	if (ops->hs400_prepare_ddr)
		return ops->hs400_prepare_ddr(dev);

	return 0;
}

int mmc_hs400_prepare_ddr(struct mmc *mmc)
{
	return dm_mmc_hs400_prepare_ddr(mmc->dev);
}

static int dm_mmc_host_power_cycle(struct udevice *dev)
{
	struct dm_mmc_ops *ops = mmc_get_ops(dev);

	if (ops->host_power_cycle)
		return ops->host_power_cycle(dev);
	return 0;
}

int mmc_host_power_cycle(struct mmc *mmc)
{
	return dm_mmc_host_power_cycle(mmc->dev);
}

static int dm_mmc_deferred_probe(struct udevice *dev)
{
	struct dm_mmc_ops *ops = mmc_get_ops(dev);

	if (ops->deferred_probe)
		return ops->deferred_probe(dev);

	return 0;
}

int mmc_deferred_probe(struct mmc *mmc)
{
	return dm_mmc_deferred_probe(mmc->dev);
}

static int dm_mmc_reinit(struct udevice *dev)
{
	struct dm_mmc_ops *ops = mmc_get_ops(dev);

	if (ops->reinit)
		return ops->reinit(dev);

	return 0;
}

int mmc_reinit(struct mmc *mmc)
{
	return dm_mmc_reinit(mmc->dev);
}

int mmc_of_parse(struct udevice *dev, struct mmc_config *cfg)
{
	int val;

	val = dev_read_u32_default(dev, "bus-width", 1);

	switch (val) {
	case 0x8:
		cfg->host_caps |= MMC_MODE_8BIT;
		/* fall through */
	case 0x4:
		cfg->host_caps |= MMC_MODE_4BIT;
		/* fall through */
	case 0x1:
		cfg->host_caps |= MMC_MODE_1BIT;
		break;
	default:
		dev_err(dev, "Invalid \"bus-width\" value %u!\n", val);
		return -EINVAL;
	}

	/* f_max is obtained from the optional "max-frequency" property */
	dev_read_u32(dev, "max-frequency", &cfg->f_max);

	if (dev_read_bool(dev, "cap-sd-highspeed"))
		cfg->host_caps |= MMC_CAP(SD_HS);
	if (dev_read_bool(dev, "cap-mmc-highspeed"))
		cfg->host_caps |= MMC_CAP(MMC_HS) | MMC_CAP(MMC_HS_52);
	if (dev_read_bool(dev, "sd-uhs-sdr12"))
		cfg->host_caps |= MMC_CAP(UHS_SDR12);
	if (dev_read_bool(dev, "sd-uhs-sdr25"))
		cfg->host_caps |= MMC_CAP(UHS_SDR25);
	if (dev_read_bool(dev, "sd-uhs-sdr50"))
		cfg->host_caps |= MMC_CAP(UHS_SDR50);
	if (dev_read_bool(dev, "sd-uhs-sdr104"))
		cfg->host_caps |= MMC_CAP(UHS_SDR104);
	if (dev_read_bool(dev, "sd-uhs-ddr50"))
		cfg->host_caps |= MMC_CAP(UHS_DDR50);
	if (dev_read_bool(dev, "mmc-ddr-1_8v"))
		cfg->host_caps |= MMC_CAP(MMC_DDR_52);
	if (dev_read_bool(dev, "mmc-ddr-1_2v"))
		cfg->host_caps |= MMC_CAP(MMC_DDR_52);
	if (dev_read_bool(dev, "mmc-hs200-1_8v"))
		cfg->host_caps |= MMC_CAP(MMC_HS_200);
	if (dev_read_bool(dev, "mmc-hs200-1_2v"))
		cfg->host_caps |= MMC_CAP(MMC_HS_200);
	if (dev_read_bool(dev, "mmc-hs400-1_8v"))
		cfg->host_caps |= MMC_CAP(MMC_HS_400) | MMC_CAP(MMC_HS_200);
	if (dev_read_bool(dev, "mmc-hs400-1_2v"))
		cfg->host_caps |= MMC_CAP(MMC_HS_400) | MMC_CAP(MMC_HS_200);
	if (dev_read_bool(dev, "mmc-hs400-enhanced-strobe"))
		cfg->host_caps |= MMC_CAP(MMC_HS_400_ES);
	if (dev_read_bool(dev, "no-mmc-hs400"))
		cfg->host_caps &= ~(MMC_CAP(MMC_HS_400) |
				    MMC_CAP(MMC_HS_400_ES));

	if (dev_read_bool(dev, "non-removable")) {
		cfg->host_caps |= MMC_CAP_NONREMOVABLE;
	} else {
		if (dev_read_bool(dev, "cd-inverted"))
			cfg->host_caps |= MMC_CAP_CD_ACTIVE_HIGH;
		if (dev_read_bool(dev, "broken-cd"))
			cfg->host_caps |= MMC_CAP_NEEDS_POLL;
	}

	if (dev_read_bool(dev, "no-1-8-v")) {
		cfg->host_caps &= ~(UHS_CAPS | MMC_MODE_HS200 |
				    MMC_MODE_HS400 | MMC_MODE_HS400_ES);
	}

	return 0;
}

struct mmc *mmc_get_mmc_dev(const struct udevice *dev)
{
	struct mmc_uclass_priv *upriv;

	if (!device_active(dev))
		return NULL;
	upriv = dev_get_uclass_priv(dev);
	return upriv->mmc;
}

#if CONFIG_IS_ENABLED(BLK)
struct mmc *find_mmc_device(int dev_num)
{
	struct udevice *dev, *mmc_dev;
	int ret;

	ret = blk_find_device(UCLASS_MMC, dev_num, &dev);

	if (ret) {
#if !defined(CONFIG_XPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
		printf("MMC Device %d not found\n", dev_num);
#endif
		return NULL;
	}

	mmc_dev = dev_get_parent(dev);

	struct mmc *mmc = mmc_get_mmc_dev(mmc_dev);

	return mmc;
}

int get_mmc_num(void)
{
	return max((blk_find_max_devnum(UCLASS_MMC) + 1), 0);
}

int mmc_get_next_devnum(void)
{
	return blk_find_max_devnum(UCLASS_MMC);
}

int mmc_get_blk(struct udevice *dev, struct udevice **blkp)
{
	struct udevice *blk;
	int ret;

	device_find_first_child_by_uclass(dev, UCLASS_BLK, &blk);
	ret = device_probe(blk);
	if (ret)
		return ret;
	*blkp = blk;

	return 0;
}

struct blk_desc *mmc_get_blk_desc(struct mmc *mmc)
{
	struct blk_desc *desc;
	struct udevice *dev;

	device_find_first_child_by_uclass(mmc->dev, UCLASS_BLK, &dev);
	if (!dev)
		return NULL;
	desc = dev_get_uclass_plat(dev);

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

		m->user_speed_mode = MMC_MODES_END;  /* Initialising user set speed mode */

		if (m->preinit)
			mmc_start_init(m);
	}
}

#if !defined(CONFIG_XPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
void print_mmc_devices(char separator)
{
	struct udevice *dev;
	char *mmc_type;
	bool first = true;

	for (uclass_first_device(UCLASS_MMC, &dev);
	     dev;
	     uclass_next_device(&dev), first = false) {
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

	if (!mmc_get_ops(dev))
		return -ENOSYS;

	/* Use the fixed index with aliases node's index */
	debug("%s: alias devnum=%d\n", __func__, dev_seq(dev));

	ret = blk_create_devicef(dev, "mmc_blk", "blk", UCLASS_MMC,
				 dev_seq(dev), DEFAULT_BLKSZ, 0, &bdev);
	if (ret) {
		debug("Cannot create block device\n");
		return ret;
	}
	bdesc = dev_get_uclass_plat(bdev);
	mmc->cfg = cfg;
	mmc->priv = dev;

	ret = bootdev_setup_for_sibling_blk(bdev, "mmc_bootdev");
	if (ret)
		return log_msg_ret("bootdev", ret);

	/* the following chunk was from mmc_register() */

	/* Setup dsr related values */
	mmc->dsr_imp = 0;
	mmc->dsr = 0xffffffff;
	/* Setup the universal parts of the block interface just once */
	bdesc->removable = 1;

	/* setup initial part type */
	bdesc->part_type = cfg->part_type;
	mmc->dev = dev;
	mmc->user_speed_mode = MMC_MODES_END;
	return 0;
}

int mmc_unbind(struct udevice *dev)
{
	struct udevice *bdev;
	int ret;

	device_find_first_child_by_uclass(dev, UCLASS_BLK, &bdev);
	if (bdev) {
		device_remove(bdev, DM_REMOVE_NORMAL);
		device_unbind(bdev);
	}
	ret = bootdev_unbind_dev(dev);
	if (ret)
		return log_msg_ret("bootdev", ret);

	return 0;
}

static int mmc_select_hwpart(struct udevice *bdev, int hwpart)
{
	struct udevice *mmc_dev = dev_get_parent(bdev);
	struct mmc *mmc = mmc_get_mmc_dev(mmc_dev);
	struct blk_desc *desc = dev_get_uclass_plat(bdev);
	int ret;

	if (desc->hwpart == hwpart)
		return 0;

	if (mmc->part_config == MMCPART_NOAVAILABLE)
		return -EMEDIUMTYPE;

	ret = mmc_switch_part(mmc, hwpart);
	if (!ret)
		blkcache_invalidate(desc->uclass_id, desc->devnum);

	return ret;
}

static int mmc_blk_probe(struct udevice *dev)
{
	struct udevice *mmc_dev = dev_get_parent(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(mmc_dev);
	struct mmc *mmc = upriv->mmc;
	int ret;

	ret = mmc_init(mmc);
	if (ret) {
		debug("%s: mmc_init() failed (err=%d)\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int mmc_remove(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct mmc *mmc = upriv->mmc;

	return mmc_deinit(mmc);
}

static const struct blk_ops mmc_blk_ops = {
	.read	= mmc_bread,
#if CONFIG_IS_ENABLED(MMC_WRITE)
	.write	= mmc_bwrite,
	.erase	= mmc_berase,
#endif
	.select_hwpart	= mmc_select_hwpart,
};

U_BOOT_DRIVER(mmc_blk) = {
	.name		= "mmc_blk",
	.id		= UCLASS_BLK,
	.ops		= &mmc_blk_ops,
	.probe		= mmc_blk_probe,
	.flags		= DM_FLAG_OS_PREPARE,
};
#endif /* CONFIG_BLK */

UCLASS_DRIVER(mmc) = {
	.id		= UCLASS_MMC,
	.name		= "mmc",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.per_device_auto	= sizeof(struct mmc_uclass_priv),
	.pre_remove	= mmc_remove,
};
