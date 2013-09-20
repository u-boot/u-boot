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
#include <fat.h>
#include <version.h>
#include <image.h>

DECLARE_GLOBAL_DATA_PTR;

static int mmc_load_image_raw(struct mmc *mmc, unsigned long sector)
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

	return mmc_load_image_raw(mmc, CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
}
#endif

#ifdef CONFIG_SPL_FAT_SUPPORT
static int mmc_load_image_fat(struct mmc *mmc, const char *filename)
{
	int err;
	struct image_header *header;

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE -
						sizeof(struct image_header));

	err = file_fat_read(filename, header, sizeof(struct image_header));
	if (err <= 0)
		goto end;

	spl_parse_image_header(header);

	err = file_fat_read(filename, (u8 *)spl_image.load_addr, 0);

end:
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
	if (err <= 0)
		printf("spl: error reading image %s, err - %d\n",
		       filename, err);
#endif

	return (err <= 0);
}

#ifdef CONFIG_SPL_OS_BOOT
static int mmc_load_image_fat_os(struct mmc *mmc)
{
	int err;

	err = file_fat_read(CONFIG_SPL_FAT_LOAD_ARGS_NAME,
			    (void *)CONFIG_SYS_SPL_ARGS_ADDR, 0);
	if (err <= 0) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("spl: error reading image %s, err - %d\n",
		       CONFIG_SPL_FAT_LOAD_ARGS_NAME, err);
#endif
		return -1;
	}

	return mmc_load_image_fat(mmc, CONFIG_SPL_FAT_LOAD_KERNEL_NAME);
}
#endif

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
		err = mmc_load_image_raw(mmc,
					 CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#ifdef CONFIG_SPL_FAT_SUPPORT
	} else if (boot_mode == MMCSD_MODE_FAT) {
		debug("boot mode - FAT\n");

		err = fat_register_device(&mmc->block_dev,
					  CONFIG_SYS_MMC_SD_FAT_BOOT_PARTITION);
		if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
			printf("spl: fat register err - %d\n", err);
#endif
			hang();
		}

#ifdef CONFIG_SPL_OS_BOOT
		if (spl_start_uboot() || mmc_load_image_fat_os(mmc))
#endif
		err = mmc_load_image_fat(mmc, CONFIG_SPL_FAT_LOAD_PAYLOAD_NAME);
#endif
	} else {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		puts("spl: wrong MMC boot mode\n");
#endif
		hang();
	}

	if (err)
		hang();
}
