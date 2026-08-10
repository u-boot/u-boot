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
#include <mmc.h>
#include <part.h>
#include <linux/err.h>

#include "qcom-priv.h"

/*
 * fw_images[] and update_info are built at runtime from the board's GPT, as the
 * updatable components and U-Boot's own partition vary per board.
 *
 * qcom_image_map[] maps partition base names to firmware names. image_index in
 * fw_images[] is assigned in order and capsules are matched by image type GUID.
 * U-Boot's own image is split into rows gated on the boot source: uefi/xbl (or
 * the legacy "aboot" alias) when flashed as XBL, boot when chainloaded from ABL.
 */

/**
 * struct qcom_image_map - maps a GPT partition to a capsule FMP image
 * @partition_base: base partition name, without the _a/_b slot suffix
 * @fw_name_base: slot-independent firmware name
 * @image_index: identifies the row, one partition is selected per index. Index 1
 *               is U-Boot's own image (the three rows below, one per boot)
 * @boot_source: if non-zero, the row is only used for this qcom_boot_source
 * @alias_base: optional second base name the row matches, slotted or bare
 * @match_nonslotted: also match the bare @partition_base, for boards without A/B
 *                    on U-Boot's partition
 * @image_type_id: fixed image type GUID, so a capsule applies to every board
 *
 * Every row matches "<partition_base>_a" and "<partition_base>_b".
 */
struct qcom_image_map {
	const char *partition_base;
	const u16 *fw_name_base;
	u8 image_index;
	enum qcom_boot_source boot_source;
	const char *alias_base;
	bool match_nonslotted;
	efi_guid_t image_type_id;
};

/*
 * image_type_id of each component.
 * The three U-Boot rows share one GUID since only one is selected per boot.
 */
#define QCOM_UBOOT_IMAGE_TYPE_GUID \
	EFI_GUID(0x400ffdcd, 0x22e0, 0x47e7, 0x9a, 0x23, 0xf1, 0x6e, 0xd9, 0x38, 0x23, 0x88)
#define QCOM_XBL_IMAGE_TYPE_GUID \
	EFI_GUID(0xdea0ba2c, 0xcbdd, 0x4805, 0xb4, 0xf9, 0xf4, 0x28, 0x25, 0x1c, 0x3e, 0x98)
#define QCOM_XBL_CONFIG_IMAGE_TYPE_GUID \
	EFI_GUID(0x5a325ae4, 0x4276, 0xb66d, 0x0a, 0xdd, 0x34, 0x94, 0xdf, 0x27, 0x70, 0x6a)
#define QCOM_UEFI_SECAPP_IMAGE_TYPE_GUID \
	EFI_GUID(0xbe8a7e08, 0x1b7a, 0x4cae, 0x99, 0x3a, 0xd5, 0xb7, 0xfb, 0x55, 0xb3, 0xc2)
#define QCOM_TZ_IMAGE_TYPE_GUID \
	EFI_GUID(0xa053aa7f, 0x40b8, 0x4b1c, 0xba, 0x08, 0x2f, 0x68, 0xac, 0x71, 0xa4, 0xf4)
#define QCOM_HYP_IMAGE_TYPE_GUID \
	EFI_GUID(0xe1a6a689, 0x0c8d, 0x4cc6, 0xb4, 0xe8, 0x55, 0xa4, 0x32, 0x0f, 0xbd, 0x8a)
#define QCOM_AOP_IMAGE_TYPE_GUID \
	EFI_GUID(0xd69e90a5, 0x4cab, 0x0071, 0xf6, 0xdf, 0xab, 0x97, 0x7f, 0x14, 0x1a, 0x7f)
#define QCOM_DEVCFG_IMAGE_TYPE_GUID \
	EFI_GUID(0xf65d4b16, 0x343d, 0x4e25, 0xaa, 0xfc, 0xbe, 0x99, 0xb6, 0x55, 0x6a, 0x6d)
#define QCOM_QUPFW_IMAGE_TYPE_GUID \
	EFI_GUID(0x21d1219f, 0x2ed1, 0x4ab4, 0x93, 0x0a, 0x41, 0xa1, 0x6a, 0xe7, 0x5f, 0x7f)
#define QCOM_XBL_RAMDUMP_IMAGE_TYPE_GUID \
	EFI_GUID(0x0382f197, 0xe41f, 0x4e84, 0xb1, 0x8b, 0x0b, 0x56, 0x4a, 0xea, 0xd8, 0x75)
#define QCOM_CPUCP_IMAGE_TYPE_GUID \
	EFI_GUID(0x1e8615bd, 0x6d8c, 0x41ad, 0xb3, 0xea, 0x50, 0xe8, 0xbf, 0x40, 0xe4, 0x3f)
#define QCOM_SHRM_IMAGE_TYPE_GUID \
	EFI_GUID(0xcb74ca22, 0x2f0d, 0x4b82, 0xa1, 0xd6, 0xc4, 0x21, 0x3f, 0x34, 0x8d, 0x73)
#define QCOM_IMAGEFV_IMAGE_TYPE_GUID \
	EFI_GUID(0x17911177, 0xc9e6, 0x4372, 0x93, 0x3c, 0x80, 0x4b, 0x67, 0x8e, 0x66, 0x6f)
#define QCOM_MULTIIMGOEM_IMAGE_TYPE_GUID \
	EFI_GUID(0xe126a436, 0x757e, 0x42d0, 0x8d, 0x19, 0x0f, 0x36, 0x2f, 0x7a, 0x62, 0xb8)
#define QCOM_RPM_IMAGE_TYPE_GUID \
	EFI_GUID(0x098df793, 0xd712, 0x413d, 0x9d, 0x4e, 0x89, 0xd7, 0x11, 0x77, 0x22, 0x28)
#define QCOM_MULTIIMGQTI_IMAGE_TYPE_GUID \
	EFI_GUID(0x846c6f05, 0xeb46, 0x4c0a, 0xa1, 0xa3, 0x36, 0x48, 0xef, 0x3f, 0x9d, 0x0e)

static const struct qcom_image_map qcom_image_map[] = {
	/*
	 * U-Boot's own image (image_index 1), only one row is selected per boot:
	 *  - boot_source gating separates the uefi/xbl rows from the boot row;
	 *  - uefi is listed before xbl, so it wins when both partitions exist;
	 *  - with no uefi, the xbl row claims the xbl partition before the
	 *    component row below, as a partition can't be two images.
	 * All rows share one image_type_id, so one capsule updates whichever
	 * partition U-Boot booted from.
	 */
	{ "uefi", u"UBOOT_UEFI_PARTITION", 1, QCOM_BOOT_SOURCE_XBL,     NULL,    false,
	  QCOM_UBOOT_IMAGE_TYPE_GUID },
	{ "xbl",  u"UBOOT_XBL_PARTITION",  1, QCOM_BOOT_SOURCE_XBL,     "aboot", false,
	  QCOM_UBOOT_IMAGE_TYPE_GUID },
	{ "boot", u"UBOOT_BOOT_PARTITION", 1, QCOM_BOOT_SOURCE_ANDROID, NULL,    true,
	  QCOM_UBOOT_IMAGE_TYPE_GUID },

	{ "xbl",         u"QCOM-XBL",          2, 0, NULL, false,
	  QCOM_XBL_IMAGE_TYPE_GUID },
	{ "xbl_config",  u"QCOM-XBL-CONFIG",   3, 0, NULL, false,
	  QCOM_XBL_CONFIG_IMAGE_TYPE_GUID },
	{ "uefisecapp",  u"QCOM-UEFI-SECAPP",  4, 0, NULL, false,
	  QCOM_UEFI_SECAPP_IMAGE_TYPE_GUID },
	{ "tz",          u"QCOM-TZ",           5, 0, NULL, false,
	  QCOM_TZ_IMAGE_TYPE_GUID },
	{ "hyp",         u"QCOM-HYP",          6, 0, NULL, false,
	  QCOM_HYP_IMAGE_TYPE_GUID },
	{ "aop",         u"QCOM-AOP",          7, 0, NULL, false,
	  QCOM_AOP_IMAGE_TYPE_GUID },
	{ "devcfg",      u"QCOM-DEVCFG",       8, 0, NULL, false,
	  QCOM_DEVCFG_IMAGE_TYPE_GUID },
	{ "qupfw",       u"QCOM-QUPFW",        9, 0, NULL, false,
	  QCOM_QUPFW_IMAGE_TYPE_GUID },
	{ "xbl_ramdump", u"QCOM-XBL-RAMDUMP", 10, 0, NULL, false,
	  QCOM_XBL_RAMDUMP_IMAGE_TYPE_GUID },
	{ "cpucp",       u"QCOM-CPUCP",       11, 0, NULL, false,
	  QCOM_CPUCP_IMAGE_TYPE_GUID },
	{ "shrm",        u"QCOM-SHRM",        12, 0, NULL, false,
	  QCOM_SHRM_IMAGE_TYPE_GUID },
	{ "imagefv",     u"QCOM-IMAGEFV",     13, 0, NULL, false,
	  QCOM_IMAGEFV_IMAGE_TYPE_GUID },
	{ "multiimgoem", u"QCOM-MULTIIMGOEM", 14, 0, NULL, false,
	  QCOM_MULTIIMGOEM_IMAGE_TYPE_GUID },
	{ "multiimgqti", u"QCOM-MULTIIMGQTI", 15, 0, NULL, false,
	  QCOM_MULTIIMGQTI_IMAGE_TYPE_GUID },
	{ "rpm",         u"QCOM-RPM",         16, 0, NULL, false,
	  QCOM_RPM_IMAGE_TYPE_GUID },
};

static struct efi_fw_image fw_images[ARRAY_SIZE(qcom_image_map)];

struct efi_capsule_update_info update_info = {
	.dfu_string = NULL,
	.num_images = 0,
	.images = fw_images,
};

#define QCOM_DFU_STRING_LEN	(64 * ARRAY_SIZE(qcom_image_map))

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

/*
 * Determine if an MMC device is an SD card.
 *
 * @desc	block device descriptor
 * Return:	true if the device is an SD card
 */
static bool is_sd(struct blk_desc *desc)
{
	struct mmc *mmc = find_mmc_device(desc->devnum);

	if (!mmc)
		return false;

	return IS_SD(mmc) != 0U;
}

/**
 * struct qcom_partition_info - a selected partition for one component
 * @name: partition name as found on disk, e.g. "tz_a"
 * @part_num: partition number on the block device
 * @devnum: block device number @name lives on
 * @uclass_id: uclass (UCLASS_SCSI/UCLASS_MMC) of @devnum. Used with @devnum to
 *             group partitions on different LUNs in the DFU string
 * @fw_name: slot-independent FMP name (from the matched map row)
 * @image_index: index of the matched map row
 * @image_type_id: image type GUID of the matched map row
 */
struct qcom_partition_info {
	char name[32];
	u32 part_num;
	int devnum;
	enum uclass_id uclass_id;
	const u16 *fw_name;
	u8 image_index;
	efi_guid_t image_type_id;
};

static struct qcom_partition_info qcom_partitions[ARRAY_SIZE(qcom_image_map)];

/**
 * struct qcom_slot_cand - one candidate partition for a map row's slot
 * @part_num: partition number, or -1 if no partition filled this slot
 * @active: A/B "active" flag from the partition's slot status
 * @devnum: block device number the partition was found on
 * @uclass_id: uclass of that block device
 * @name: partition name as found on disk
 */
struct qcom_slot_cand {
	int part_num;
	bool active;
	int devnum;
	enum uclass_id uclass_id;
	char name[32];
};

/**
 * struct qcom_row_cand - candidate partitions for one qcom_image_map[] row
 * @slot: indexed by enum ab_slot (SLOT_NONE for a non-slotted/alias-bare match)
 */
struct qcom_row_cand {
	struct qcom_slot_cand slot[3];
};

/*
 * Candidates for every scanned block device, one entry per qcom_image_map[] row.
 */
static struct qcom_row_cand qcom_cands[ARRAY_SIZE(qcom_image_map)];

/**
 * qcom_match_partition() - test a partition name against a map row
 * @name: partition name as found on disk
 * @map: candidate map row
 * @slot: output, which A/B slot (SLOT_NONE for a non-slotted / bare-alias match)
 *
 * Every row matches "<partition_base>_a"/"<partition_base>_b". U-Boot's rows
 * may also match a bare non-slotted name (@match_nonslotted) and a legacy alias
 * base (@alias_base, both slotted and bare).
 *
 * Return: true if @name belongs to @map
 */
static bool qcom_match_partition(const char *name,
				 const struct qcom_image_map *map,
				 enum ab_slot *slot)
{
	char cand[36];

	snprintf(cand, sizeof(cand), "%s_a", map->partition_base);
	if (!strcmp(name, cand)) {
		*slot = SLOT_A;
		return true;
	}
	snprintf(cand, sizeof(cand), "%s_b", map->partition_base);
	if (!strcmp(name, cand)) {
		*slot = SLOT_B;
		return true;
	}
	if (map->match_nonslotted && !strcmp(name, map->partition_base)) {
		*slot = SLOT_NONE;
		return true;
	}

	if (map->alias_base) {
		snprintf(cand, sizeof(cand), "%s_a", map->alias_base);
		if (!strcmp(name, cand)) {
			*slot = SLOT_A;
			return true;
		}
		snprintf(cand, sizeof(cand), "%s_b", map->alias_base);
		if (!strcmp(name, cand)) {
			*slot = SLOT_B;
			return true;
		}
		if (!strcmp(name, map->alias_base)) {
			*slot = SLOT_NONE;
			return true;
		}
	}

	return false;
}

/**
 * qcom_scan_device() - record candidate partitions on one block device
 * @desc: block device to scan
 * @cands: candidate table, one entry per qcom_image_map[] row
 *
 * Each partition on @desc is checked against every eligible row (those whose
 * boot_source matches qcom_boot_source) and recorded in that row's slot. One
 * partition can match several rows, e.g. "xbl_a" matches both the U-Boot xbl
 * row and the xbl component row.
 *
 * The first device wins if a partition table is exposed more than once, e.g.
 * mirrored UFS boot LUNs. The table is global so qcom_resolve_images() can apply
 * the uefi-over-xbl priority across LUNs.
 */
static void qcom_scan_device(struct blk_desc *desc, struct qcom_row_cand *cands)
{
	struct disk_partition info;
	struct part_slot_status *slot_status;
	int partnum, i;

	for (partnum = 1; !part_get_info(desc, partnum, &info); partnum++) {
		slot_status = (struct part_slot_status *)&info.type_flags;

		for (i = 0; i < ARRAY_SIZE(qcom_image_map); i++) {
			const struct qcom_image_map *m = &qcom_image_map[i];
			struct qcom_slot_cand *c;
			enum ab_slot slot;

			if (m->boot_source && m->boot_source != qcom_boot_source)
				continue;
			if (!qcom_match_partition((char *)info.name, m, &slot))
				continue;

			c = &cands[i].slot[slot];
			if (c->part_num >= 0)	/* first device wins */
				continue;

			c->part_num = partnum;
			c->active = !!slot_status->active;
			c->devnum = desc->devnum;
			c->uclass_id = desc->uclass_id;
			strlcpy(c->name, (char *)info.name, sizeof(c->name));
		}
	}
}

/**
 * qcom_pick_slot() - choose the best candidate slot for a map row
 * @rc: candidate slots for one row
 *
 * Priority: active A > active B > non-slotted > inactive A > inactive B.
 *
 * Return: the chosen enum ab_slot, or -1 if no candidate was recorded
 */
static int qcom_pick_slot(const struct qcom_row_cand *rc)
{
	if (rc->slot[SLOT_A].part_num >= 0 && rc->slot[SLOT_A].active)
		return SLOT_A;
	if (rc->slot[SLOT_B].part_num >= 0 && rc->slot[SLOT_B].active)
		return SLOT_B;
	if (rc->slot[SLOT_NONE].part_num >= 0)
		return SLOT_NONE;
	if (rc->slot[SLOT_A].part_num >= 0)
		return SLOT_A;
	if (rc->slot[SLOT_B].part_num >= 0)
		return SLOT_B;

	return -1;
}

/**
 * qcom_resolve_images() - pick one partition per map row from the candidates
 * @cands: candidate table filled by qcom_scan_device() across every device
 * @partitions: output array; room for ARRAY_SIZE(qcom_image_map) entries
 *
 * Walks qcom_image_map[] in order, so U-Boot's own image lands in partitions[0].
 * Two dedup rules:
 *
 *  1. One partition per image_index. The U-Boot rows share index 1, so the first
 *     one with a candidate wins (uefi before xbl).
 *  2. A physical partition is claimed once. With U-Boot on xbl and no uefi, the
 *     U-Boot xbl row takes the partition and the xbl component row is skipped.
 *
 * A row with no candidate is left out, so num_images varies per board.
 *
 * Return: number of partitions written to @partitions
 */
static u32 qcom_resolve_images(struct qcom_row_cand *cands,
			       struct qcom_partition_info *partitions)
{
	u32 count = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(qcom_image_map); i++) {
		const struct qcom_image_map *m = &qcom_image_map[i];
		struct qcom_slot_cand *c;
		struct qcom_partition_info *p;
		bool dup = false;
		int sel;
		u32 j;

		if (m->boot_source && m->boot_source != qcom_boot_source)
			continue;

		sel = qcom_pick_slot(&cands[i]);
		if (sel < 0)
			continue;
		c = &cands[i].slot[sel];

		/* rule 1: one selected partition per image_index */
		for (j = 0; j < count; j++) {
			if (partitions[j].image_index == m->image_index) {
				dup = true;
				break;
			}
		}
		if (dup)
			continue;

		/* rule 2: never claim the same physical partition twice */
		for (j = 0; j < count; j++) {
			if (partitions[j].devnum == c->devnum &&
			    partitions[j].uclass_id == c->uclass_id &&
			    partitions[j].part_num == (u32)c->part_num) {
				dup = true;
				break;
			}
		}
		if (dup)
			continue;

		p = &partitions[count++];
		strlcpy(p->name, c->name, sizeof(p->name));
		p->part_num = c->part_num;
		p->devnum = c->devnum;
		p->uclass_id = c->uclass_id;
		p->fw_name = m->fw_name_base;
		p->image_index = m->image_index;
		guidcpy(&p->image_type_id, &m->image_type_id);

		log_debug("qcom capsule: %s -> image %u (part %u, dev %d)\n",
			  p->name, p->image_index, p->part_num, p->devnum);
	}

	return count;
}

/**
 * qcom_build_fw_images() - populate fw_images[] from the discovered partitions
 * @partitions: partitions from qcom_resolve_images(); entry 0 is U-Boot's own
 *              image (image_index 1), the rest are components
 * @num_partitions: number of valid entries in @partitions
 *
 * One fw_images[] entry per partition, in the same order, so fw_images[] and
 * @partitions stay index-aligned. .image_index is assigned sequentially
 * (1..@num_partitions) as UEFI requires 1 <= ImageIndex <= DescriptorCount.
 * .image_type_id is copied from the map row. As entry 0 is then non-zero,
 * efi_gen_capsule_guids() leaves the table alone.
 *
 * Return: number of images written (equal to @num_partitions)
 */
static u32 qcom_build_fw_images(const struct qcom_partition_info *partitions,
				u32 num_partitions)
{
	u32 i;

	for (i = 0; i < num_partitions; i++) {
		fw_images[i].fw_name = (u16 *)partitions[i].fw_name;
		fw_images[i].image_index = i + 1;
		guidcpy(&fw_images[i].image_type_id, &partitions[i].image_type_id);
	}

	return num_partitions;
}

/**
 * qcom_build_dfu_string() - build the dfu_alt_info string for the partitions
 * @partitions: partitions accumulated across every scanned block device
 * @num_partitions: number of valid entries in @partitions; also the number of
 *                  valid fw_images[] entries, with matching indices
 * @buf: output buffer
 * @buf_size: size of @buf
 *
 * Builds an "interface devstring=alt;alt&interface devstring=alt" string (see
 * dfu_config_interfaces()), with '&' between devices. efi_firmware_raw_set_image()
 * passes a NULL interface, which forces this multi-interface parser, so a
 * single-group string would drop every partition after the first LUN.
 *
 * Tokens are appended in @partitions order, so the DFU alt number of each
 * image is its image_index - 1, as efi_firmware_raw_set_image() expects.
 *
 * Return: true on success, false if @buf was too small
 */
static bool qcom_build_dfu_string(struct qcom_partition_info *partitions,
				  u32 num_partitions, char *buf, size_t buf_size)
{
	int prev_devnum = -1;
	enum uclass_id prev_uclass_id = UCLASS_INVALID;
	u32 i;
	size_t len;

	buf[0] = '\0';
	len = 0;

	for (i = 0; i < num_partitions; i++) {
		struct qcom_partition_info *p = &partitions[i];
		char token[48];
		bool new_group = p->devnum != prev_devnum ||
				  p->uclass_id != prev_uclass_id;

		if (new_group) {
			char prefix[24];

			if (i != 0)
				len = strlcat(buf, "&", buf_size);

			switch (p->uclass_id) {
			case UCLASS_SCSI:
				snprintf(prefix, sizeof(prefix), "scsi %d=", p->devnum);
				break;
			case UCLASS_MMC:
				snprintf(prefix, sizeof(prefix), "mmc %d=", p->devnum);
				break;
			default:
				log_err("qcom capsule: unsupported storage uclass %d for %s\n",
					p->uclass_id, p->name);
				return false;
			}
			len = strlcat(buf, prefix, buf_size);
		} else {
			len = strlcat(buf, ";", buf_size);
		}

		if (p->uclass_id == UCLASS_MMC)
			snprintf(token, sizeof(token), "%s part %u %u",
				 p->name, p->devnum, p->part_num);
		else
			snprintf(token, sizeof(token), "%s part %u",
				 p->name, p->part_num);
		len = strlcat(buf, token, buf_size);

		if (len >= buf_size) {
			log_err("qcom capsule: dfu_alt_info string truncated\n");
			return false;
		}

		prev_devnum = p->devnum;
		prev_uclass_id = p->uclass_id;
	}

	return true;
}

/**
 * qcom_configure_capsule_updates() - Configure the DFU string and fw_images[]
 * for capsule updates
 *
 * Scans every probed block device and builds fw_images[] and the matching
 * dfu_alt_info string.
 */
void qcom_configure_capsule_updates(void)
{
	static char dfu_string[QCOM_DFU_STRING_LEN];
	struct qcom_partition_info *partitions = qcom_partitions;
	u32 num_partitions;
	struct udevice *dev;
	bool have_ufs = false;
	int i;

	memset(qcom_cands, 0, sizeof(qcom_cands));
	for (i = 0; i < ARRAY_SIZE(qcom_image_map); i++) {
		qcom_cands[i].slot[SLOT_NONE].part_num = -1;
		qcom_cands[i].slot[SLOT_A].part_num = -1;
		qcom_cands[i].slot[SLOT_B].part_num = -1;
	}
	memset(partitions, 0, sizeof(qcom_partitions));

	/*
	 * Check to see if we have UFS storage, if so firmware MUST be on it and
	 * we can skip all non-UFS block devices.
	 */
	uclass_foreach_dev_probe(UCLASS_UFS, dev) {
		have_ufs = true;
		break;
	}

	uclass_foreach_dev_probe(UCLASS_BLK, dev) {
		struct blk_desc *desc;

		if (device_get_uclass_id(dev) != UCLASS_BLK)
			continue;

		desc = dev_get_uclass_plat(dev);

		/* If we have a UFS then don't look at any other block devices */
		if (have_ufs) {
			if (device_get_uclass_id(dev->parent->parent) != UCLASS_UFS)
				continue;
		} else if (IS_ENABLED(CONFIG_MMC) && is_sd(desc)) {
			/* If we don't have UFS, then firmware is on the eMMC */
			log_debug("skipped SD-Card (devnum %d)\n", desc->devnum);
			continue;
		}

		if (!desc || desc->part_type == PART_TYPE_UNKNOWN)
			continue;

		qcom_scan_device(desc, qcom_cands);
	}

	num_partitions = qcom_resolve_images(qcom_cands, partitions);
	if (!num_partitions) {
		log_err("qcom capsule: no updatable partitions found\n");
		return;
	}
	if (partitions[0].image_index != 1)
		log_warning("qcom capsule: U-Boot's own partition not found; configuring components only\n");

	qcom_build_fw_images(partitions, num_partitions);
	if (!qcom_build_dfu_string(partitions, num_partitions, dfu_string,
				   sizeof(dfu_string)))
		return;

	log_debug("dfu_alt_info: %s\n", dfu_string);

	update_info.num_images = num_partitions;
	update_info.dfu_string = dfu_string;
}

/**
 * efi_capsule_resolve_image_index() - find the fw_images[] index for a capsule
 * @image_type: image type GUID from the capsule
 * @capsule_index: image index from the capsule
 *
 * Return: image_index of the entry matching @image_type, else @capsule_index
 */
u8 efi_capsule_resolve_image_index(const efi_guid_t *image_type,
				   u8 capsule_index)
{
	int i;

	for (i = 0; i < update_info.num_images; i++) {
		if (!guidcmp(&update_info.images[i].image_type_id, image_type))
			return update_info.images[i].image_index;
	}

	return capsule_index;
}
