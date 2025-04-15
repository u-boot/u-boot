// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#include <dm.h>
#include <log.h>
#include <part.h>
#include <spl.h>
#include <spl_load.h>
#include <linux/compiler.h>
#include <errno.h>
#include <errno.h>
#include <mmc.h>
#include <image.h>
#include <imx_container.h>

static ulong h_spl_load_read(struct spl_load_info *load, ulong off,
			     ulong size, void *buf)
{
	struct blk_desc *bd = load->priv;
	lbaint_t sector = off >> bd->log2blksz;
	lbaint_t count = size >> bd->log2blksz;

	return blk_dread(bd, sector, count, buf) << bd->log2blksz;
}

static __maybe_unused unsigned long spl_mmc_raw_uboot_offset(int part)
{
#if IS_ENABLED(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR)
	if (part == 0)
		return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DATA_PART_OFFSET;
#endif

	return 0;
}

static __maybe_unused
int mmc_load_image_raw_sector(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev,
			      struct mmc *mmc, unsigned long sector)
{
	int ret;
	struct blk_desc *bd = mmc_get_blk_desc(mmc);
	struct spl_load_info load;

	spl_load_init(&load, h_spl_load_read, bd, bd->blksz);
	ret = spl_load(spl_image, bootdev, &load, 0, sector << bd->log2blksz);
	if (ret) {
		puts("mmc_load_image_raw_sector: mmc block read error\n");
		log_debug("(error=%d)\n", ret);
		return ret;
	}

	return 0;
}

static int spl_mmc_get_device_index(uint boot_device)
{
	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		return 0;
	case BOOT_DEVICE_MMC2:
	case BOOT_DEVICE_MMC2_2:
		return 1;
	}

	printf("spl: unsupported mmc boot device.\n");

	return -ENODEV;
}

static int spl_mmc_find_device(struct mmc **mmcp, int mmc_dev)
{
	int ret;

#if CONFIG_IS_ENABLED(DM_MMC)
	struct udevice *dev;
	struct uclass *uc;

	log_debug("Selecting MMC dev %d; seqs:\n", mmc_dev);
	if (_LOG_DEBUG) {
		uclass_id_foreach_dev(UCLASS_MMC, dev, uc)
			log_debug("%d: %s\n", dev_seq(dev), dev->name);
	}
	ret = mmc_init_device(mmc_dev);
#else
	ret = mmc_initialize(NULL);
#endif /* DM_MMC */
	if (ret) {
		printf("spl: could not initialize mmc. error: %d\n", ret);
		return ret;
	}
	*mmcp = find_mmc_device(mmc_dev);
	ret = *mmcp ? 0 : -ENODEV;
	if (ret) {
		printf("spl: could not find mmc device %d. error: %d\n",
		       mmc_dev, ret);
		return ret;
	}
#if CONFIG_IS_ENABLED(DM_MMC)
	log_debug("mmc %d: %s\n", mmc_dev, (*mmcp)->dev->name);
#endif

	return 0;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION
static int mmc_load_image_raw_partition(struct spl_image_info *spl_image,
					struct spl_boot_device *bootdev,
					struct mmc *mmc, int partition,
					unsigned long sector)
{
	struct disk_partition info;
	int ret;

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION_TYPE
	int type_part;
	/* Only support MBR so DOS_ENTRY_NUMBERS */
	for (type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
		ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
		if (ret)
			continue;
		if (info.sys_ind ==
			CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_TYPE) {
			partition = type_part;
			break;
		}
	}
#endif

	ret = part_get_info(mmc_get_blk_desc(mmc), partition, &info);
	if (ret) {
		puts("spl: partition error\n");
		return ret;
	}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
	return mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start + sector);
#else
	return mmc_load_image_raw_sector(spl_image, bootdev, mmc, info.start);
#endif
}
#endif

#if CONFIG_IS_ENABLED(FALCON_BOOT_MMCSD)
static int mmc_load_image_raw_os(struct spl_image_info *spl_image,
				 struct spl_boot_device *bootdev,
				 struct mmc *mmc)
{
	int ret;

#if defined(CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR)
	unsigned long count;

	count = blk_dread(mmc_get_blk_desc(mmc),
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR,
		CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS,
		(void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR);
	if (count != CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS) {
		puts("mmc_load_image_raw_os: mmc block read error\n");
		return -EIO;
	}
#endif	/* CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR */

	ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR);
	if (ret)
		return ret;

	if (spl_image->os != IH_OS_LINUX && spl_image->os != IH_OS_TEE) {
		puts("Expected image is not found. Trying to start U-Boot\n");
		return -ENOENT;
	}

	return 0;
}
#else
static int mmc_load_image_raw_os(struct spl_image_info *spl_image,
				 struct spl_boot_device *bootdev,
				 struct mmc *mmc)
{
	return -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	return 1;
}
#endif

#ifdef CONFIG_SYS_MMCSD_FS_BOOT
static int spl_mmc_do_fs_boot(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev,
			      struct mmc *mmc,
			      const char *filename)
{
	int ret = -ENOSYS;

	__maybe_unused int partition = CONFIG_SYS_MMCSD_FS_BOOT_PARTITION;

#if CONFIG_SYS_MMCSD_FS_BOOT_PARTITION == -1
	{
		struct disk_partition info;
		debug("Checking for the first MBR bootable partition\n");
		for (int type_part = 1; type_part <= DOS_ENTRY_NUMBERS; type_part++) {
			ret = part_get_info(mmc_get_blk_desc(mmc), type_part, &info);
			if (ret)
				continue;
			debug("Partition %d is of type %d and bootable=%d\n", type_part, info.sys_ind, info.bootable);
			if (info.bootable != 0) {
				debug("Partition %d is bootable, using it\n", type_part);
				partition = type_part;
				break;
			}
		}
		printf("Using first bootable partition: %d\n", partition);
		if (partition == CONFIG_SYS_MMCSD_FS_BOOT_PARTITION) {
			return -ENOSYS;
		}
	}
#endif

#ifdef CONFIG_SPL_FS_FAT
	if (!spl_start_uboot()) {
		ret = spl_load_image_fat_os(spl_image, bootdev, mmc_get_blk_desc(mmc),
					    partition);
		if (!ret)
			return 0;
	}
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	ret = spl_load_image_fat(spl_image, bootdev, mmc_get_blk_desc(mmc),
				 partition,
				 filename);
	if (!ret)
		return ret;
#endif
#endif
#ifdef CONFIG_SPL_FS_EXT4
	if (!spl_start_uboot()) {
		ret = spl_load_image_ext_os(spl_image, bootdev, mmc_get_blk_desc(mmc),
					    partition);
		if (!ret)
			return 0;
	}
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
	ret = spl_load_image_ext(spl_image, bootdev, mmc_get_blk_desc(mmc),
				 partition,
				 filename);
	if (!ret)
		return 0;
#endif
#endif

#if defined(CONFIG_SPL_FS_FAT) || defined(CONFIG_SPL_FS_EXT4)
	ret = -ENOENT;
#endif

	return ret;
}
#endif

u32 __weak spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
#if defined(CONFIG_SPL_FS_FAT) || defined(CONFIG_SPL_FS_EXT4)
	return MMCSD_MODE_FS;
#elif defined(CONFIG_SUPPORT_EMMC_BOOT)
	return MMCSD_MODE_EMMCBOOT;
#else
	return MMCSD_MODE_RAW;
#endif
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION
int __weak spl_mmc_boot_partition(const u32 boot_device)
{
	return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
}
#endif

unsigned long __weak arch_spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						       unsigned long raw_sect)
{
	return raw_sect;
}

unsigned long __weak board_spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						  unsigned long raw_sect)
{
	return arch_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

unsigned long __weak spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						  unsigned long raw_sect)
{
	return board_spl_mmc_get_uboot_raw_sector(mmc, raw_sect);
}

int default_spl_mmc_emmc_boot_partition(struct mmc *mmc)
{
	int part;
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION
	part = CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION;
#else
	/*
	 * We need to check what the partition is configured to.
	 * 1 and 2 match up to boot0 / boot1 and 7 is user data
	 * which is the first physical partition (0).
	 */
	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	if (part == EMMC_BOOT_PART_USER)
		part = EMMC_HWPART_DEFAULT;
#endif
	return part;
}

int __weak spl_mmc_emmc_boot_partition(struct mmc *mmc)
{
	return default_spl_mmc_emmc_boot_partition(mmc);
}

static int spl_mmc_get_mmc_devnum(struct mmc *mmc)
{
	struct blk_desc *block_dev;
#if !CONFIG_IS_ENABLED(BLK)
	block_dev = &mmc->block_dev;
#else
	block_dev = mmc_get_blk_desc(mmc);
#endif
	return block_dev->devnum;
}

static struct mmc *mmc;

void spl_mmc_clear_cache(void)
{
	mmc = NULL;
}

int spl_mmc_load(struct spl_image_info *spl_image,
		 struct spl_boot_device *bootdev,
		 const char *filename,
		 int raw_part,
		 unsigned long raw_sect)
{
	u32 boot_mode;
	int ret = 0;
	__maybe_unused int part = 0;
	int mmc_dev;

	/* Perform peripheral init only once for an mmc device */
	mmc_dev = spl_mmc_get_device_index(bootdev->boot_device);
	log_debug("boot_device=%d, mmc_dev=%d\n", bootdev->boot_device,
		  mmc_dev);
	if (!mmc || spl_mmc_get_mmc_devnum(mmc) != mmc_dev) {
		ret = spl_mmc_find_device(&mmc, mmc_dev);
		if (ret)
			return ret;

		ret = mmc_init(mmc);
		if (ret) {
			mmc = NULL;
			printf("spl: mmc init failed with error: %d\n", ret);
			return ret;
		}
	}

	boot_mode = spl_mmc_boot_mode(mmc, bootdev->boot_device);
	ret = -EINVAL;
	switch (boot_mode) {
	case MMCSD_MODE_EMMCBOOT:
		part = spl_mmc_emmc_boot_partition(mmc);

		if (CONFIG_IS_ENABLED(MMC_TINY))
			ret = mmc_switch_part(mmc, part);
		else
			ret = blk_dselect_hwpart(mmc_get_blk_desc(mmc), part);

		if (ret) {
			puts("spl: mmc partition switch failed\n");
			return ret;
		}
		/* Fall through */
	case MMCSD_MODE_RAW:
		debug("spl: mmc boot mode: raw\n");

		if (!spl_start_uboot()) {
			ret = mmc_load_image_raw_os(spl_image, bootdev, mmc);
			if (!ret)
				return 0;
		}

		raw_sect = spl_mmc_get_uboot_raw_sector(mmc, raw_sect);

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION
		ret = mmc_load_image_raw_partition(spl_image, bootdev,
						   mmc, raw_part,
						   raw_sect);
		if (!ret)
			return 0;
#endif
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
		ret = mmc_load_image_raw_sector(spl_image, bootdev, mmc,
						raw_sect +
						spl_mmc_raw_uboot_offset(part));
		if (!ret)
			return 0;
#endif
		/* If RAW mode fails, try FS mode. */
		fallthrough;
#ifdef CONFIG_SYS_MMCSD_FS_BOOT
	case MMCSD_MODE_FS:
		debug("spl: mmc boot mode: fs\n");

		ret = spl_mmc_do_fs_boot(spl_image, bootdev, mmc, filename);
		if (!ret)
			return 0;

		break;
#endif
	default:
		puts("spl: mmc: wrong boot mode\n");
	}

	return ret;
}

int spl_mmc_load_image(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev)
{
	return spl_mmc_load(spl_image, bootdev,
#ifdef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
			    CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
#else
			    NULL,
#endif
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION
			    spl_mmc_boot_partition(bootdev->boot_device),
#else
			    0,
#endif
#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
			    CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
#else
			    0);
#endif
}

SPL_LOAD_IMAGE_METHOD("MMC1", 0, BOOT_DEVICE_MMC1, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2", 0, BOOT_DEVICE_MMC2, spl_mmc_load_image);
SPL_LOAD_IMAGE_METHOD("MMC2_2", 0, BOOT_DEVICE_MMC2_2, spl_mmc_load_image);
