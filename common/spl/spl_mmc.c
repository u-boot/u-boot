/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <spl.h>
#include <linux/compiler.h>
#include <asm/u-boot.h>
#include <mmc.h>
#include <image.h>

DECLARE_GLOBAL_DATA_PTR;

static int mmc_load_image_raw_sector(struct mmc *mmc, unsigned long sector)
{
	unsigned long count;
	u32 image_size_sectors;
	struct image_header *header;

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE -
					 sizeof(struct image_header));

	/* read image header to find the image size & load address */
	count = mmc->block_dev.block_read(0, sector, 1, header);
	if (count == 0)
		goto end;

	if (image_get_magic(header) != IH_MAGIC)
		return -1;

	spl_parse_image_header(header);

	/* convert size to sectors - round up */
	image_size_sectors = (spl_image.size + mmc->read_bl_len - 1) /
			     mmc->read_bl_len;

	/* Read the header too to avoid extra memcpy */
	count = mmc->block_dev.block_read(0, sector, image_size_sectors,
					  (void *) spl_image.load_addr);

end:
	if (count == 0) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		puts("spl: mmc block read error\n");
#endif
		return -1;
	}

	return 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
static int mmc_load_image_raw_partition(struct mmc *mmc, int partition)
{
	disk_partition_t info;
	int err;

	err = get_partition_info(&mmc->block_dev, partition, &info);
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		puts("spl: partition error\n");
#endif
		return -1;
	}

	return mmc_load_image_raw_sector(mmc, info.start);
}
#endif

#ifdef CONFIG_SPL_OS_BOOT
static int mmc_load_image_raw_os(struct mmc *mmc)
{
	unsigned long count;

	count = mmc->block_dev.block_read(0,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(void *) CONFIG_SYS_SPL_ARGS_ADDR);
	if (count == 0) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		puts("spl: mmc block read error\n");
#endif
		return -1;
	}

	return mmc_load_image_raw_sector(mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
}
#endif

void spl_mmc_load_image(void)
{
	struct mmc *mmc;
	u32 boot_mode;
	int err;
	__maybe_unused int part;

	mmc_initialize(gd->bd);

	/* We register only one device. So, the dev id is always 0 */
	mmc = find_mmc_device(0);
	if (!mmc) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		puts("spl: mmc device not found\n");
#endif
		hang();
	}

	err = mmc_init(mmc);
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("spl: mmc init failed with error: %d\n", err);
#endif
		hang();
	}

	boot_mode = spl_boot_mode();
	switch (boot_mode) {
	case MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

#ifdef CONFIG_SPL_OS_BOOT
		if (!spl_start_uboot()) {
			err = mmc_load_image_raw_os(mmc);
			if (!err)
				return;
		}
#endif
#if defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION)
		err = mmc_load_image_raw_partition(mmc,
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION);
		if (!err)
			return;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR)
		err = mmc_load_image_raw_sector(mmc,
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
		if (!err)
			return;
#endif
	case MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

#ifdef CONFIG_SYS_MMCSD_FS_BOOT_PARTITION
#ifdef CONFIG_SPL_FAT_SUPPORT
#ifdef CONFIG_SPL_OS_BOOT
		if (!spl_start_uboot()) {
			err = spl_load_image_fat_os(&mmc->block_dev,
				CONFIG_SYS_MMCSD_FS_BOOT_PARTITION);
			if (!err)
				return;
		}
#endif
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
		err = spl_load_image_fat(&mmc->block_dev,
					 CONFIG_SYS_MMCSD_FS_BOOT_PARTITION,
					 CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
		if (!err)
			return;
#endif
#endif
#ifdef CONFIG_SPL_EXT_SUPPORT
#ifdef CONFIG_SPL_OS_BOOT
		if (!spl_start_uboot()) {
			err = spl_load_image_ext_os(&mmc->block_dev,
				CONFIG_SYS_MMCSD_FS_BOOT_PARTITION);
			if (!err)
				return;
		}
#endif
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
		err = spl_load_image_ext(&mmc->block_dev,
					 CONFIG_SYS_MMCSD_FS_BOOT_PARTITION,
					 CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
		if (!err)
			return;
#endif
#endif
#endif
#ifdef CONFIG_SUPPORT_EMMC_BOOT
	case MMCSD_MODE_EMMCBOOT:
		/*
		 * We need to check what the partition is configured to.
		 * 1 and 2 match up to boot0 / boot1 and 7 is user data
		 * which is the first physical partition (0).
		 */
		part = (mmc->part_config >> 3) & PART_ACCESS_MASK;

		if (part == 7)
			part = 0;

		if (mmc_switch_part(0, part)) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
			puts("spl: mmc partition switch failed\n");
#endif
			hang();
		}

#ifdef CONFIG_SPL_OS_BOOT
		if (!spl_start_uboot()) {
			err = mmc_load_image_raw_os(mmc);
			if (!err)
				return;
		}
#endif
#if defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION)
		err = mmc_load_image_raw_partition(mmc,
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION);
		if (!err)
			return;
#elif defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR)
		err = mmc_load_image_raw_sector(mmc,
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
		if (!err)
			return;
#endif
#endif
	case MMCSD_MODE_UNDEFINED:
	default:
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		if (err)
			puts("spl: mmc: no boot mode left to try\n");
		else
			puts("spl: mmc: wrong boot mode\n");
#endif
		hang();
	}
}
