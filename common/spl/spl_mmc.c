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
#include <asm/u-boot.h>
#include <mmc.h>
#include <version.h>
#include <image.h>

DECLARE_GLOBAL_DATA_PTR;

static int mmc_load_image_raw_sector(struct mmc *mmc, unsigned long sector)
{
	unsigned long err;
	u32 image_size_sectors;
	struct image_header *header;

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE -
						sizeof(struct image_header));

	/* read image header to find the image size & load address */
	err = mmc->block_dev.block_read(0, sector, 1, header);
	if (err == 0)
		goto end;

	if (image_get_magic(header) != IH_MAGIC)
		return -1;

	spl_parse_image_header(header);

	/* convert size to sectors - round up */
	image_size_sectors = (spl_image.size + mmc->read_bl_len - 1) /
				mmc->read_bl_len;

	/* Read the header too to avoid extra memcpy */
	err = mmc->block_dev.block_read(0, sector, image_size_sectors,
					(void *)spl_image.load_addr);

end:
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
	if (err == 0)
		printf("spl: mmc blk read err - %lu\n", err);
#endif

	return (err == 0);
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
static int mmc_load_image_raw_partition(struct mmc *mmc, int partition)
{
	disk_partition_t info;

	if (get_partition_info(&mmc->block_dev, partition, &info)) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("spl: partition error\n");
#endif
		return -1;
	}

	return mmc_load_image_raw_sector(mmc, info.start);
}
#endif

#ifdef CONFIG_SPL_OS_BOOT
static int mmc_load_image_raw_os(struct mmc *mmc)
{
	if (!mmc->block_dev.block_read(0,
				       CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
				       CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
				       (void *)CONFIG_SYS_SPL_ARGS_ADDR)) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("mmc args blk read error\n");
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
	int err;
	u32 boot_mode;

	mmc_initialize(gd->bd);
	/* We register only one device. So, the dev id is always 0 */
	mmc = find_mmc_device(0);
	if (!mmc) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		puts("spl: mmc device not found!!\n");
#endif
		hang();
	}

	err = mmc_init(mmc);
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("spl: mmc init failed: err - %d\n", err);
#endif
		hang();
	}

	boot_mode = spl_boot_mode();
	if (boot_mode == MMCSD_MODE_RAW) {
		debug("boot mode - RAW\n");
#ifdef CONFIG_SPL_OS_BOOT
		if (spl_start_uboot() || mmc_load_image_raw_os(mmc))
#endif
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
		err = mmc_load_image_raw_partition(mmc,
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION);
#else
		err = mmc_load_image_raw_sector(mmc,
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#endif
#if defined(CONFIG_SPL_FAT_SUPPORT) || defined(CONFIG_SPL_EXT_SUPPORT)
	}
	if (err || boot_mode == MMCSD_MODE_FS) {
		debug("boot mode - FS\n");
#ifdef CONFIG_SPL_FAT_SUPPORT
#ifdef CONFIG_SPL_OS_BOOT
		if (spl_start_uboot() || spl_load_image_fat_os(&mmc->block_dev,
								CONFIG_SYS_MMCSD_FS_BOOT_PARTITION))
#endif
		err = spl_load_image_fat(&mmc->block_dev,
					CONFIG_SYS_MMCSD_FS_BOOT_PARTITION,
					CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
		if(err)
#endif /* CONFIG_SPL_FAT_SUPPORT */
		{
#ifdef CONFIG_SPL_EXT_SUPPORT
#ifdef CONFIG_SPL_OS_BOOT
		if (spl_start_uboot() || spl_load_image_ext_os(&mmc->block_dev,
								CONFIG_SYS_MMCSD_FS_BOOT_PARTITION))
#endif
		err = spl_load_image_ext(&mmc->block_dev,
					CONFIG_SYS_MMCSD_FS_BOOT_PARTITION,
					CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
#endif /* CONFIG_SPL_EXT_SUPPORT */
		}
#endif /* defined(CONFIG_SPL_FAT_SUPPORT) || defined(CONFIG_SPL_EXT_SUPPORT) */
#ifdef CONFIG_SUPPORT_EMMC_BOOT
	} else if (boot_mode == MMCSD_MODE_EMMCBOOT) {
		/*
		 * We need to check what the partition is configured to.
		 * 1 and 2 match up to boot0 / boot1 and 7 is user data
		 * which is the first physical partition (0).
		 */
		int part = (mmc->part_config >> 3) & PART_ACCESS_MASK;

		if (part == 7)
			part = 0;

		if (mmc_switch_part(0, part)) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
			puts("MMC partition switch failed\n");
#endif
			hang();
		}
#ifdef CONFIG_SPL_OS_BOOT
		if (spl_start_uboot() || mmc_load_image_raw_os(mmc))
#endif
		err = mmc_load_image_raw_sector(mmc,
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#endif
	}

	switch(boot_mode){
		case MMCSD_MODE_RAW:
#if defined(CONFIG_SPL_FAT_SUPPORT) || defined(CONFIG_SPL_EXT_SUPPORT)
		case MMCSD_MODE_FS:
#endif
#ifdef CONFIG_SUPPORT_EMMC_BOOT
		case MMCSD_MODE_EMMCBOOT:
#endif
			/* Boot mode is ok. Nothing to do. */
			break;
		case MMCSD_MODE_UNDEFINED:
		default:
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
			puts("spl: wrong MMC boot mode\n");
#endif
			hang();
	}

	if (err)
		hang();
}
