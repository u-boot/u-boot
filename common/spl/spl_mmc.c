/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <dm.h>
#include <spl.h>
#include <linux/compiler.h>
#include <asm/u-boot.h>
#include <errno.h>
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
	debug("read sector %lx, count=%lu\n", sector, count);
	if (count == 0)
		goto end;

	if (image_get_magic(header) != IH_MAGIC) {
		puts("bad magic\n");
		return -1;
	}

	spl_parse_image_header(header);

	/* convert size to sectors - round up */
	image_size_sectors = (spl_image.size + mmc->read_bl_len - 1) /
			     mmc->read_bl_len;

	/* Read the header too to avoid extra memcpy */
	count = mmc->block_dev.block_read(0, sector, image_size_sectors,
					  (void *)(ulong)spl_image.load_addr);
	debug("read %x sectors to %x\n", image_size_sectors,
	      spl_image.load_addr);

end:
	if (count == 0) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		puts("spl: mmc block read error\n");
#endif
		return -1;
	}

	return 0;
}

#ifdef CONFIG_DM_MMC
static int spl_mmc_find_device(struct mmc **mmc)
{
	struct udevice *dev;
	int err;

	err = mmc_initialize(NULL);
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("spl: could not initialize mmc. error: %d\n", err);
#endif
		return err;
	}

	err = uclass_get_device(UCLASS_MMC, 0, &dev);
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("spl: could not find mmc device. error: %d\n", err);
#endif
		return err;
	}

	*mmc = NULL;
	*mmc = mmc_get_mmc_dev(dev);
	return *mmc != NULL ? 0 : -ENODEV;
}
#else
static int spl_mmc_find_device(struct mmc **mmc)
{
	int err;

	err = mmc_initialize(gd->bd);
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("spl: could not initialize mmc. error: %d\n", err);
#endif
		return err;
	}

	/* We register only one device. So, the dev id is always 0 */
	*mmc = find_mmc_device(0);
	if (!*mmc) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		puts("spl: mmc device not found\n");
#endif
		return -ENODEV;
	}

	return 0;
}
#endif

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

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
	return mmc_load_image_raw_sector(mmc, info.start +
					 CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#else
	return mmc_load_image_raw_sector(mmc, info.start);
#endif
}
#else
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION -1
static int mmc_load_image_raw_partition(struct mmc *mmc, int partition)
{
	return -ENOSYS;
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
#else
int spl_start_uboot(void)
{
	return 1;
}
static int mmc_load_image_raw_os(struct mmc *mmc)
{
	return -ENOSYS;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT_PARTITION
int spl_mmc_do_fs_boot(struct mmc *mmc)
{
	int err = -ENOSYS;

#ifdef CONFIG_SPL_FAT_SUPPORT
	if (!spl_start_uboot()) {
		err = spl_load_image_fat_os(&mmc->block_dev,
			CONFIG_SYS_MMCSD_FS_BOOT_PARTITION);
		if (!err)
			return err;
	}
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	err = spl_load_image_fat(&mmc->block_dev,
				 CONFIG_SYS_MMCSD_FS_BOOT_PARTITION,
				 CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
	if (!err)
		return err;
#endif
#endif
#ifdef CONFIG_SPL_EXT_SUPPORT
	if (!spl_start_uboot()) {
		err = spl_load_image_ext_os(&mmc->block_dev,
			CONFIG_SYS_MMCSD_FS_BOOT_PARTITION);
		if (!err)
			return err;
	}
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	err = spl_load_image_ext(&mmc->block_dev,
				 CONFIG_SYS_MMCSD_FS_BOOT_PARTITION,
				 CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
	if (!err)
		return err;
#endif
#endif

#if defined(CONFIG_SPL_FAT_SUPPORT) || defined(CONFIG_SPL_EXT_SUPPORT)
	err = -ENOENT;
#endif

	return err;
}
#else
int spl_mmc_do_fs_boot(struct mmc *mmc)
{
	return -ENOSYS;
}
#endif

void spl_mmc_load_image(void)
{
	struct mmc *mmc;
	u32 boot_mode;
	int err = 0;
	__maybe_unused int part;

	if (spl_mmc_find_device(&mmc))
		hang();

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

		if (!spl_start_uboot()) {
			err = mmc_load_image_raw_os(mmc);
			if (!err)
				return;
		}

		err = mmc_load_image_raw_partition(mmc,
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION);
		if (!err)
			return;
#if defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR)
		err = mmc_load_image_raw_sector(mmc,
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
		if (!err)
			return;
#endif
		break;
	case MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		err = spl_mmc_do_fs_boot(mmc);
		if (!err)
			return;

		break;
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

		if (!spl_start_uboot()) {
			err = mmc_load_image_raw_os(mmc);
			if (!err)
				return;
		}
		err = mmc_load_image_raw_partition(mmc,
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION);
		if (!err)
			return;
#if defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR)
		err = mmc_load_image_raw_sector(mmc,
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
		if (!err)
			return;
#endif
		break;
#endif
	case MMCSD_MODE_UNDEFINED:
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
	default:
		puts("spl: mmc: wrong boot mode\n");
#endif
	}

	hang();
}
