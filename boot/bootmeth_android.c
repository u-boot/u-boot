// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootmeth for Android
 *
 * Copyright (C) 2024 BayLibre, SAS
 * Written by Mattijs Korpershoek <mkorpershoek@baylibre.com>
 */
#define LOG_CATEGORY UCLASS_BOOTSTD

#include <android_ab.h>
#include <android_image.h>
#if CONFIG_IS_ENABLED(AVB_VERIFY)
#include <avb_verify.h>
#endif
#include <bcb.h>
#include <blk.h>
#include <bootflow.h>
#include <bootm.h>
#include <bootmeth.h>
#include <dm.h>
#include <image.h>
#include <malloc.h>
#include <mapmem.h>
#include <part.h>
#include "bootmeth_android.h"

#define BCB_FIELD_COMMAND_SZ 32
#define BCB_PART_NAME "misc"
#define BOOT_PART_NAME "boot"
#define VENDOR_BOOT_PART_NAME "vendor_boot"

/**
 * struct android_priv - Private data
 *
 * This is read from the disk and recorded for use when the full Android
 * kernel must be loaded and booted
 *
 * @boot_mode: Requested boot mode (normal, recovery, bootloader)
 * @slot: Nul-terminated partition slot suffix read from BCB ("a\0" or "b\0")
 * @header_version: Android boot image header version
 */
struct android_priv {
	enum android_boot_mode boot_mode;
	char slot[2];
	u32 header_version;
};

static int android_check(struct udevice *dev, struct bootflow_iter *iter)
{
	/* This only works on mmc devices */
	if (bootflow_iter_check_mmc(iter))
		return log_msg_ret("mmc", -ENOTSUPP);

	/*
	 * This only works on whole devices, as multiple
	 * partitions are needed to boot Android
	 */
	if (iter->part != 0)
		return log_msg_ret("mmc part", -ENOTSUPP);

	return 0;
}

static int scan_boot_part(struct udevice *blk, struct android_priv *priv)
{
	struct blk_desc *desc = dev_get_uclass_plat(blk);
	struct disk_partition partition;
	char partname[PART_NAME_LEN];
	ulong num_blks, bufsz;
	char *buf;
	int ret;

	sprintf(partname, BOOT_PART_NAME "_%s", priv->slot);
	ret = part_get_info_by_name(desc, partname, &partition);
	if (ret < 0)
		return log_msg_ret("part info", ret);

	num_blks = DIV_ROUND_UP(sizeof(struct andr_boot_img_hdr_v0), desc->blksz);
	bufsz = num_blks * desc->blksz;
	buf = malloc(bufsz);
	if (!buf)
		return log_msg_ret("buf", -ENOMEM);

	ret = blk_read(blk, partition.start, num_blks, buf);
	if (ret != num_blks) {
		free(buf);
		return log_msg_ret("part read", -EIO);
	}

	if (!is_android_boot_image_header(buf)) {
		free(buf);
		return log_msg_ret("header", -ENOENT);
	}

	priv->header_version = ((struct andr_boot_img_hdr_v0 *)buf)->header_version;
	free(buf);

	return 0;
}

static int scan_vendor_boot_part(struct udevice *blk, struct android_priv *priv)
{
	struct blk_desc *desc = dev_get_uclass_plat(blk);
	struct disk_partition partition;
	char partname[PART_NAME_LEN];
	ulong num_blks, bufsz;
	char *buf;
	int ret;

	sprintf(partname, VENDOR_BOOT_PART_NAME "_%s", priv->slot);
	ret = part_get_info_by_name(desc, partname, &partition);
	if (ret < 0)
		return log_msg_ret("part info", ret);

	num_blks = DIV_ROUND_UP(sizeof(struct andr_vnd_boot_img_hdr), desc->blksz);
	bufsz = num_blks * desc->blksz;
	buf = malloc(bufsz);
	if (!buf)
		return log_msg_ret("buf", -ENOMEM);

	ret = blk_read(blk, partition.start, num_blks, buf);
	if (ret != num_blks) {
		free(buf);
		return log_msg_ret("part read", -EIO);
	}

	if (!is_android_vendor_boot_image_header(buf)) {
		free(buf);
		return log_msg_ret("header", -ENOENT);
	}
	free(buf);

	return 0;
}

static int android_read_slot_from_bcb(struct bootflow *bflow, bool decrement)
{
	struct blk_desc *desc = dev_get_uclass_plat(bflow->blk);
	struct android_priv *priv = bflow->bootmeth_priv;
	struct disk_partition misc;
	char slot_suffix[3];
	int ret;

	ret = part_get_info_by_name(desc, BCB_PART_NAME, &misc);
	if (ret < 0)
		return log_msg_ret("part", ret);

	ret = ab_select_slot(desc, &misc, decrement);
	if (ret < 0)
		return log_msg_ret("slot", ret);

	priv->slot[0] = BOOT_SLOT_NAME(ret);
	priv->slot[1] = '\0';

	sprintf(slot_suffix, "_%s", priv->slot);
	ret = bootflow_cmdline_set_arg(bflow, "androidboot.slot_suffix",
				       slot_suffix, false);
	if (ret < 0)
		return log_msg_ret("cmdl", ret);

	return 0;
}

static int configure_serialno(struct bootflow *bflow)
{
	char *serialno = env_get("serial#");

	if (!serialno)
		return log_msg_ret("serial", -ENOENT);

	return bootflow_cmdline_set_arg(bflow, "androidboot.serialno", serialno, false);
}

static int android_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	struct blk_desc *desc = dev_get_uclass_plat(bflow->blk);
	struct disk_partition misc;
	struct android_priv *priv;
	char command[BCB_FIELD_COMMAND_SZ];
	int ret;

	bflow->state = BOOTFLOWST_MEDIA;

	/*
	 * bcb_find_partition_and_load() will print errors to stdout
	 * if BCB_PART_NAME is not found. To avoid that, check if the
	 * partition exists first.
	 */
	ret = part_get_info_by_name(desc, BCB_PART_NAME, &misc);
	if (ret < 0)
		return log_msg_ret("part", ret);

	ret = bcb_find_partition_and_load("mmc", desc->devnum, BCB_PART_NAME);
	if (ret < 0)
		return log_msg_ret("bcb load", ret);

	ret = bcb_get(BCB_FIELD_COMMAND, command, sizeof(command));
	if (ret < 0)
		return log_msg_ret("bcb read", ret);

	priv = malloc(sizeof(struct android_priv));
	if (!priv)
		return log_msg_ret("buf", -ENOMEM);

	if (!strcmp("bootonce-bootloader", command)) {
		priv->boot_mode = ANDROID_BOOT_MODE_BOOTLOADER;
		bflow->os_name = strdup("Android (bootloader)");
	} else if (!strcmp("boot-fastboot", command)) {
		priv->boot_mode = ANDROID_BOOT_MODE_RECOVERY;
		bflow->os_name = strdup("Android (fastbootd)");
	} else if (!strcmp("boot-recovery", command)) {
		priv->boot_mode = ANDROID_BOOT_MODE_RECOVERY;
		bflow->os_name = strdup("Android (recovery)");
	} else {
		priv->boot_mode = ANDROID_BOOT_MODE_NORMAL;
		bflow->os_name = strdup("Android");
	}
	if (!bflow->os_name)
		return log_msg_ret("os", -ENOMEM);

	if (priv->boot_mode == ANDROID_BOOT_MODE_BOOTLOADER) {
		/* Clear BCB */
		memset(command, 0, sizeof(command));
		ret = bcb_set(BCB_FIELD_COMMAND, command);
		if (ret < 0) {
			free(priv);
			return log_msg_ret("bcb set", ret);
		}
		ret = bcb_store();
		if (ret < 0) {
			free(priv);
			return log_msg_ret("bcb store", ret);
		}

		bflow->bootmeth_priv = priv;
		bflow->state = BOOTFLOWST_READY;
		return 0;
	}

	bflow->bootmeth_priv = priv;

	/* For recovery and normal boot, we need to scan the partitions */
	ret = android_read_slot_from_bcb(bflow, false);
	if (ret < 0) {
		log_err("read slot: %d", ret);
		goto free_priv;
	}

	ret = scan_boot_part(bflow->blk, priv);
	if (ret < 0) {
		log_debug("scan boot failed: err=%d\n", ret);
		goto free_priv;
	}

	if (priv->header_version != 4) {
		log_debug("only boot.img v4 is supported %u\n", priv->header_version);
		ret = -EINVAL;
		goto free_priv;
	}

	ret = scan_vendor_boot_part(bflow->blk, priv);
	if (ret < 0) {
		log_debug("scan vendor_boot failed: err=%d\n", ret);
		goto free_priv;
	}

	/* Ignoring return code: setting serial number is not mandatory for booting */
	configure_serialno(bflow);

	if (priv->boot_mode == ANDROID_BOOT_MODE_NORMAL) {
		ret = bootflow_cmdline_set_arg(bflow, "androidboot.force_normal_boot",
					       "1", false);
		if (ret < 0) {
			log_debug("normal_boot %d", ret);
			goto free_priv;
		}
	}

	bflow->state = BOOTFLOWST_READY;

	return 0;

 free_priv:
	free(priv);
	bflow->bootmeth_priv = NULL;
	return ret;
}

static int android_read_file(struct udevice *dev, struct bootflow *bflow,
			     const char *file_path, ulong addr, ulong *sizep)
{
	/*
	 * Reading individual files is not supported since we only
	 * operate on whole mmc devices (because we require multiple partitions)
	 */
	return log_msg_ret("Unsupported", -ENOSYS);
}

/**
 * read_slotted_partition() - Read a partition by appending a slot suffix
 *
 * Most modern Android devices use Seamless Updates, where each partition
 * is duplicated. For example, the boot partition has boot_a and boot_b.
 * For more information, see:
 * https://source.android.com/docs/core/ota/ab
 * https://source.android.com/docs/core/ota/ab/ab_implement
 *
 * @blk: Block device to read
 * @name: Partition name to read
 * @slot: Nul-terminated slot suffixed to partition name ("a\0" or "b\0")
 * @addr: Address where the partition content is loaded into
 * Return: 0 if OK, negative errno on failure.
 */
static int read_slotted_partition(struct blk_desc *desc, const char *const name,
				  const char slot[2], ulong addr)
{
	struct disk_partition partition;
	char partname[PART_NAME_LEN];
	int ret;
	u32 n;

	/* Ensure name fits in partname it should be: <name>_<slot>\0 */
	if (strlen(name) > (PART_NAME_LEN - 2 - 1))
		return log_msg_ret("name too long", -EINVAL);

	sprintf(partname, "%s_%s", name, slot);
	ret = part_get_info_by_name(desc, partname, &partition);
	if (ret < 0)
		return log_msg_ret("part", ret);

	n = blk_dread(desc, partition.start, partition.size, map_sysmem(addr, 0));
	if (n < partition.size)
		return log_msg_ret("part read", -EIO);

	return 0;
}

#if CONFIG_IS_ENABLED(AVB_VERIFY)
static int avb_append_commandline_arg(struct bootflow *bflow, char *arg)
{
	char *key = strsep(&arg, "=");
	char *value = arg;
	int ret;

	ret = bootflow_cmdline_set_arg(bflow, key, value, false);
	if (ret < 0)
		return log_msg_ret("avb cmdline", ret);

	return 0;
}

static int avb_append_commandline(struct bootflow *bflow, char *cmdline)
{
	char *arg = strsep(&cmdline, " ");
	int ret;

	while (arg) {
		ret = avb_append_commandline_arg(bflow, arg);
		if (ret < 0)
			return ret;

		arg = strsep(&cmdline, " ");
	}

	return 0;
}

static int run_avb_verification(struct bootflow *bflow)
{
	struct blk_desc *desc = dev_get_uclass_plat(bflow->blk);
	struct android_priv *priv = bflow->bootmeth_priv;
	const char * const requested_partitions[] = {"boot", "vendor_boot"};
	struct AvbOps *avb_ops;
	AvbSlotVerifyResult result;
	AvbSlotVerifyData *out_data;
	enum avb_boot_state boot_state;
	char *extra_args;
	char slot_suffix[3];
	bool unlocked = false;
	int ret;

	avb_ops = avb_ops_alloc(desc->devnum);
	if (!avb_ops)
		return log_msg_ret("avb ops", -ENOMEM);

	sprintf(slot_suffix, "_%s", priv->slot);

	ret = avb_ops->read_is_device_unlocked(avb_ops, &unlocked);
	if (ret != AVB_IO_RESULT_OK)
		return log_msg_ret("avb lock", -EIO);

	result = avb_slot_verify(avb_ops,
				 requested_partitions,
				 slot_suffix,
				 unlocked,
				 AVB_HASHTREE_ERROR_MODE_RESTART_AND_INVALIDATE,
				 &out_data);

	if (result != AVB_SLOT_VERIFY_RESULT_OK) {
		printf("Verification failed, reason: %s\n",
		       str_avb_slot_error(result));
		avb_slot_verify_data_free(out_data);
		return log_msg_ret("avb verify", -EIO);
	}

	if (unlocked)
		boot_state = AVB_ORANGE;
	else
		boot_state = AVB_GREEN;

	extra_args = avb_set_state(avb_ops, boot_state);
	if (extra_args) {
		/* extra_args will be modified after this. This is fine */
		ret = avb_append_commandline_arg(bflow, extra_args);
		if (ret < 0)
			goto free_out_data;
	}

	ret = avb_append_commandline(bflow, out_data->cmdline);
	if (ret < 0)
		goto free_out_data;

	return 0;

 free_out_data:
	if (out_data)
		avb_slot_verify_data_free(out_data);

	return log_msg_ret("avb cmdline", ret);
}
#else
static int run_avb_verification(struct bootflow *bflow)
{
	int ret;

	/* When AVB is unsupported, pass ORANGE state  */
	ret = bootflow_cmdline_set_arg(bflow,
				       "androidboot.verifiedbootstate",
				       "orange", false);
	if (ret < 0)
		return log_msg_ret("avb cmdline", ret);

	return 0;
}
#endif /* AVB_VERIFY */

static int boot_android_normal(struct bootflow *bflow)
{
	struct blk_desc *desc = dev_get_uclass_plat(bflow->blk);
	struct android_priv *priv = bflow->bootmeth_priv;
	int ret;
	ulong loadaddr = env_get_hex("loadaddr", 0);
	ulong vloadaddr = env_get_hex("vendor_boot_comp_addr_r", 0);

	ret = run_avb_verification(bflow);
	if (ret < 0)
		return log_msg_ret("avb", ret);

	/* Read slot once more to decrement counter from BCB */
	ret = android_read_slot_from_bcb(bflow, true);
	if (ret < 0)
		return log_msg_ret("read slot", ret);

	ret = read_slotted_partition(desc, "boot", priv->slot, loadaddr);
	if (ret < 0)
		return log_msg_ret("read boot", ret);

	ret = read_slotted_partition(desc, "vendor_boot", priv->slot, vloadaddr);
	if (ret < 0)
		return log_msg_ret("read vendor_boot", ret);

	set_abootimg_addr(loadaddr);
	set_avendor_bootimg_addr(vloadaddr);

	ret = bootm_boot_start(loadaddr, bflow->cmdline);

	return log_msg_ret("boot", ret);
}

static int boot_android_recovery(struct bootflow *bflow)
{
	int ret;

	ret = boot_android_normal(bflow);

	return log_msg_ret("boot", ret);
}

static int boot_android_bootloader(struct bootflow *bflow)
{
	int ret;

	ret = run_command("fastboot usb 0", 0);
	do_reset(NULL, 0, 0, NULL);

	return log_msg_ret("boot", ret);
}

static int android_boot(struct udevice *dev, struct bootflow *bflow)
{
	struct android_priv *priv = bflow->bootmeth_priv;
	int ret;

	switch (priv->boot_mode) {
	case ANDROID_BOOT_MODE_NORMAL:
		ret = boot_android_normal(bflow);
		break;
	case ANDROID_BOOT_MODE_RECOVERY:
		ret = boot_android_recovery(bflow);
		break;
	case ANDROID_BOOT_MODE_BOOTLOADER:
		ret = boot_android_bootloader(bflow);
		break;
	default:
		printf("ANDROID: Unknown boot mode %d. Running fastboot...\n",
		       priv->boot_mode);
		boot_android_bootloader(bflow);
		/* Tell we failed to boot since boot mode is unknown */
		ret = -EFAULT;
	}

	return ret;
}

static int android_bootmeth_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->desc = "Android boot";
	plat->flags = BOOTMETHF_ANY_PART;

	return 0;
}

static struct bootmeth_ops android_bootmeth_ops = {
	.check		= android_check,
	.read_bootflow	= android_read_bootflow,
	.read_file	= android_read_file,
	.boot		= android_boot,
};

static const struct udevice_id android_bootmeth_ids[] = {
	{ .compatible = "u-boot,android" },
	{ }
};

U_BOOT_DRIVER(bootmeth_android) = {
	.name		= "bootmeth_android",
	.id		= UCLASS_BOOTMETH,
	.of_match	= android_bootmeth_ids,
	.ops		= &android_bootmeth_ops,
	.bind		= android_bootmeth_bind,
};
