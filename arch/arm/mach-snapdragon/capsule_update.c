// SPDX-License-Identifier: GPL-2.0+
/*
 * Capsule update support for Qualcomm boards.
 *
 * Copyright (c) 2024 Linaro Ltd.
 * Author: Casey Connolly <casey.connolly@linaro.org>
 */

#define pr_fmt(fmt) "QCOM-FMP: " fmt

#include <dm/device.h>
#include <dm/uclass.h>
#include <efi.h>
#include <efi_loader.h>
#include <malloc.h>
#include <scsi.h>
#include <part.h>
#include <linux/err.h>

#include "qcom-priv.h"

/*
 * To handle different variants like chainloaded U-Boot here we need to
 * build the fw_images array dynamically at runtime. These are the possible
 * implementations:
 *
 * - Devices with U-Boot on the uefi_a/b partition
 * - Devices with U-Boot on the boot (a/b) partition
 * - Devices with U-Boot on the xbl (a/b) partition
 *
 * Which partition actually has U-Boot on it is determined based on the
 * qcom_boot_source variable and additional logic in find_target_partition().
 */
struct efi_fw_image fw_images[] = {
	{
		.image_index = 1,
	},
};

struct efi_capsule_update_info update_info = {
	/* Filled in by configure_dfu_string() */
	.dfu_string = NULL,
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

enum target_part_type {
	TARGET_PART_UEFI = 1,
	TARGET_PART_XBL,
	TARGET_PART_BOOT,
};

/* LSB first */
struct part_slot_status {
	u16: 2;
	u16 active : 1;
	u16: 3;
	u16 successful : 1;
	u16 unbootable : 1;
	u16 tries_remaining : 4;
};

enum ab_slot {
	SLOT_NONE,
	SLOT_A,
	SLOT_B,
};

static enum ab_slot get_part_slot(const char *partname)
{
	int len = strlen(partname);

	if (partname[len - 2] != '_')
		return SLOT_NONE;
	if (partname[len - 1] == 'a')
		return SLOT_A;
	if (partname[len - 1] == 'b')
		return SLOT_B;

	return SLOT_NONE;
}

/*
 * Determine which partition U-Boot is flashed to based on the boot source (ABL/XBL),
 * the slot status, and prioritizing the uefi partition over xbl if found.
 */
static int find_target_partition(int *devnum, enum uclass_id *uclass,
				 enum target_part_type *target_part_type)
{
	int ret;
	int partnum, uefi_partnum = -1, xbl_partnum = -1;
	struct disk_partition info;
	struct part_slot_status *slot_status;
	struct udevice *dev = NULL;
	struct blk_desc *desc = NULL, *xbl_desc = NULL;
	uchar ptn_name[32] = { 0 };
	bool have_ufs = false;

	/*
	 * Check to see if we have UFS storage, if so U-Boot MUST be on it and we can skip
	 * all non-UFS block devices
	 */
	uclass_foreach_dev_probe(UCLASS_UFS, dev) {
		have_ufs = true;
		break;
	}

	uclass_foreach_dev_probe(UCLASS_BLK, dev) {
		if (device_get_uclass_id(dev) != UCLASS_BLK)
			continue;

		/* If we have a UFS then don't look at any other block devices */
		if (have_ufs) {
			if (device_get_uclass_id(dev->parent->parent) != UCLASS_UFS)
				continue;
		/*
		 * If we don't have UFS, then U-Boot must be on the eMMC which is always the first
		 * MMC device.
		 */
		} else if (dev->parent->seq_ > 0) {
			continue;
		}

		desc = dev_get_uclass_plat(dev);
		if (!desc || desc->part_type == PART_TYPE_UNKNOWN)
			continue;
		for (partnum = 1;; partnum++) {
			ret = part_get_info(desc, partnum, &info);
			if (ret)
				break;

			slot_status = (struct part_slot_status *)&info.type_flags;

			/*
			 * Qualcomm Linux devices have a "uefi" partition, it's A/B but the
			 * flags might not be set so we assume the A partition unless the B
			 * partition is active.
			 */
			if (!strncmp(info.name, "uefi", strlen("uefi"))) {
				/*
				 * If U-Boot was chainloaded somehow we can't be flashed to
				 * the uefi partition
				 */
				if (qcom_boot_source != QCOM_BOOT_SOURCE_XBL)
					continue;

				*target_part_type = TARGET_PART_UEFI;
				/*
				 * Found an active UEFI partition, this is where U-Boot is
				 * flashed.
				 */
				if (slot_status->active)
					goto found;

				/* Prefer A slot if it's not marked active */
				if (get_part_slot(info.name) == SLOT_A) {
					/*
					 * If we found the A slot after the B slot (both
					 * inactive) then we assume U-Boot is on the A slot.
					 */
					if (uefi_partnum >= 0)
						goto found;

					/* Didn't find the B slot yet */
					uefi_partnum = partnum;
					strlcpy(ptn_name, info.name, 32);
				} else {
					/*
					 * Found inactive B slot after inactive A slot, return
					 * the A slot
					 */
					if (uefi_partnum >= 0) {
						partnum = uefi_partnum;
						goto found;
					}

					/*
					 * Didn't find the A slot yet. Record that we found the
					 * B slot
					 */
					uefi_partnum = partnum;
					strlcpy(ptn_name, info.name, 32);
				}
				/* xbl and aboot are effectively the same */
			} else if ((!strncmp(info.name, "xbl", strlen("xbl")) &&
				    strlen(info.name) == 5) ||
				    !strncmp(info.name, "aboot", strlen("aboot"))) {
				/*
				 * If U-Boot was booted via ABL, we can't be flashed to the
				 * XBL partition
				 */
				if (qcom_boot_source != QCOM_BOOT_SOURCE_XBL)
					continue;

				/*
				 * ignore xbl partition if we have uefi partitions, U-Boot will
				 * always be on the UEFI partition in this case.
				 */
				if (*target_part_type == TARGET_PART_UEFI)
					continue;

				/* Either non-A/B or find the active XBL partition */
				if (slot_status->active || !get_part_slot(info.name)) {
					/*
					 * No quick return since we might find a uefi partition
					 * later
					 */
					xbl_partnum = partnum;
					*target_part_type = TARGET_PART_XBL;
					xbl_desc = desc;
					strlcpy(ptn_name, info.name, 32);
				}

				/*
				 * No fast return since we might also have a uefi partition which
				 * will take priority.
				 */
			} else if (!strncmp(info.name, "boot", strlen("boot"))) {
				/* We can only be flashed to boot if we were chainloaded */
				if (qcom_boot_source != QCOM_BOOT_SOURCE_ANDROID)
					continue;

				/*
				 * Either non-A/B or find the active partition. We can return
				 * immediately here since we've narrowed it down to a single option
				 */
				if (slot_status->active || !get_part_slot(info.name)) {
					*target_part_type = TARGET_PART_BOOT;
					goto found;
				}
			}
		}
	}

	/*
	 * Now we've exhausted all options, if we didn't find a uefi partition
	 * then we are indeed flashed to the xbl partition.
	 */
	if (*target_part_type == TARGET_PART_XBL) {
		partnum = xbl_partnum;
		desc = xbl_desc;
		goto found;
	}

	/* Found no candidate partitions */
	return -1;

found:
	if (desc) {
		*devnum = desc->devnum;
		*uclass = desc->uclass_id;
	}

	/* info won't match for XBL hence the copy. */
	log_info("Capsule update target: %s (disk %d:%d)\n",
		 *target_part_type == TARGET_PART_BOOT ? info.name : ptn_name,
		 *devnum, partnum);
	return partnum;
}

/**
 * qcom_configure_capsule_updates() - Configure the DFU string for capsule updates
 *
 * U-Boot is flashed to the boot partition on Qualcomm boards. In most cases there
 * are two boot partitions, boot_a and boot_b. As we don't currently support doing
 * full A/B updates, we only support updating the currently active boot partition.
 *
 * So we need to find the current slot suffix and the associated boot partition.
 * We do this by looking for the boot partition that has the 'active' flag set
 * in the GPT partition vendor attribute bits.
 */
void qcom_configure_capsule_updates(void)
{
	int ret = 0, partnum = -1, devnum;
	static char dfu_string[32] = { 0 };
	enum target_part_type target_part_type = 0;
	enum uclass_id dev_uclass;

	if (IS_ENABLED(CONFIG_SCSI)) {
		/* Scan for SCSI devices */
		ret = scsi_scan(false);
		if (ret) {
			debug("Failed to scan SCSI devices: %d\n", ret);
			return;
		}
	}

	partnum = find_target_partition(&devnum, &dev_uclass, &target_part_type);
	if (partnum < 0) {
		log_err("Failed to find boot partition\n");
		return;
	}

	/*
	 * Set the fw_name based on the partition type. This causes the GUID to be different
	 * so we will never accidentally flash a U-Boot image intended for XBL to the boot
	 * partition.
	 */
	switch (target_part_type) {
	case TARGET_PART_UEFI:
		fw_images[0].fw_name = u"UBOOT_UEFI_PARTITION";
		break;
	case TARGET_PART_XBL:
		fw_images[0].fw_name = u"UBOOT_XBL_PARTITION";
		break;
	case TARGET_PART_BOOT:
		fw_images[0].fw_name = u"UBOOT_BOOT_PARTITION";
		break;
	}

	switch (dev_uclass) {
	case UCLASS_SCSI:
		snprintf(dfu_string, 32, "scsi %d=u-boot.bin part %d", devnum, partnum);
		break;
	case UCLASS_MMC:
		snprintf(dfu_string, 32, "mmc 0=u-boot.bin part %d %d", devnum, partnum);
		break;
	default:
		debug("Unsupported storage uclass: %d\n", dev_uclass);
		return;
	}
	log_debug("DFU string: '%s'\n", dfu_string);

	update_info.dfu_string = dfu_string;
}
