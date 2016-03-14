/*
 * Copyright 2014 Broadcom Corporation.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <blk.h>
#include <errno.h>
#include <fastboot.h>
#include <fb_mmc.h>
#include <image-sparse.h>
#include <part.h>
#include <sparse_format.h>
#include <mmc.h>
#include <div64.h>

#ifndef CONFIG_FASTBOOT_GPT_NAME
#define CONFIG_FASTBOOT_GPT_NAME GPT_ENTRY_NAME
#endif

static char *response_str;

struct fb_mmc_sparse {
	struct blk_desc	*dev_desc;
};

static int part_get_info_efi_by_name_or_alias(struct blk_desc *dev_desc,
		const char *name, disk_partition_t *info)
{
	int ret;

	ret = part_get_info_efi_by_name(dev_desc, name, info);
	if (ret) {
		/* strlen("fastboot_partition_alias_") + 32(part_name) + 1 */
		char env_alias_name[25 + 32 + 1];
		char *aliased_part_name;

		/* check for alias */
		strcpy(env_alias_name, "fastboot_partition_alias_");
		strncat(env_alias_name, name, 32);
		aliased_part_name = getenv(env_alias_name);
		if (aliased_part_name != NULL)
			ret = part_get_info_efi_by_name(dev_desc,
					aliased_part_name, info);
	}
	return ret;
}


static int fb_mmc_sparse_write(struct sparse_storage *storage,
			       void *priv,
			       unsigned int offset,
			       unsigned int size,
			       char *data)
{
	struct fb_mmc_sparse *sparse = priv;
	struct blk_desc *dev_desc = sparse->dev_desc;
	int ret;

	ret = blk_dwrite(dev_desc, offset, size, data);
	if (!ret)
		return -EIO;

	return ret;
}

static void write_raw_image(struct blk_desc *dev_desc, disk_partition_t *info,
		const char *part_name, void *buffer,
		unsigned int download_bytes)
{
	lbaint_t blkcnt;
	lbaint_t blks;

	/* determine number of blocks to write */
	blkcnt = ((download_bytes + (info->blksz - 1)) & ~(info->blksz - 1));
	blkcnt = lldiv(blkcnt, info->blksz);

	if (blkcnt > info->size) {
		error("too large for partition: '%s'\n", part_name);
		fastboot_fail(response_str, "too large for partition");
		return;
	}

	puts("Flashing Raw Image\n");

	blks = blk_dwrite(dev_desc, info->start, blkcnt, buffer);
	if (blks != blkcnt) {
		error("failed writing to device %d\n", dev_desc->devnum);
		fastboot_fail(response_str, "failed writing to device");
		return;
	}

	printf("........ wrote " LBAFU " bytes to '%s'\n", blkcnt * info->blksz,
	       part_name);
	fastboot_okay(response_str, "");
}

void fb_mmc_flash_write(const char *cmd, unsigned int session_id,
			void *download_buffer, unsigned int download_bytes,
			char *response)
{
	struct blk_desc *dev_desc;
	disk_partition_t info;

	/* initialize the response buffer */
	response_str = response;

	dev_desc = blk_get_dev("mmc", CONFIG_FASTBOOT_FLASH_MMC_DEV);
	if (!dev_desc || dev_desc->type == DEV_TYPE_UNKNOWN) {
		error("invalid mmc device\n");
		fastboot_fail(response_str, "invalid mmc device");
		return;
	}

	if (strcmp(cmd, CONFIG_FASTBOOT_GPT_NAME) == 0) {
		printf("%s: updating MBR, Primary and Backup GPT(s)\n",
		       __func__);
		if (is_valid_gpt_buf(dev_desc, download_buffer)) {
			printf("%s: invalid GPT - refusing to write to flash\n",
			       __func__);
			fastboot_fail(response_str, "invalid GPT partition");
			return;
		}
		if (write_mbr_and_gpt_partitions(dev_desc, download_buffer)) {
			printf("%s: writing GPT partitions failed\n", __func__);
			fastboot_fail(response_str,
				      "writing GPT partitions failed");
			return;
		}
		printf("........ success\n");
		fastboot_okay(response_str, "");
		return;
	} else if (part_get_info_efi_by_name_or_alias(dev_desc, cmd, &info)) {
		error("cannot find partition: '%s'\n", cmd);
		fastboot_fail(response_str, "cannot find partition");
		return;
	}

	if (is_sparse_image(download_buffer)) {
		struct fb_mmc_sparse sparse_priv;
		sparse_storage_t sparse;

		sparse_priv.dev_desc = dev_desc;

		sparse.block_sz = info.blksz;
		sparse.start = info.start;
		sparse.size = info.size;
		sparse.name = cmd;
		sparse.write = fb_mmc_sparse_write;

		printf("Flashing sparse image at offset " LBAFU "\n",
		       info.start);

		store_sparse_image(&sparse, &sparse_priv, session_id,
				   download_buffer);
	} else {
		write_raw_image(dev_desc, &info, cmd, download_buffer,
				download_bytes);
	}

	fastboot_okay(response_str, "");
}

void fb_mmc_erase(const char *cmd, char *response)
{
	int ret;
	struct blk_desc *dev_desc;
	disk_partition_t info;
	lbaint_t blks, blks_start, blks_size, grp_size;
	struct mmc *mmc = find_mmc_device(CONFIG_FASTBOOT_FLASH_MMC_DEV);

	if (mmc == NULL) {
		error("invalid mmc device");
		fastboot_fail(response_str, "invalid mmc device");
		return;
	}

	/* initialize the response buffer */
	response_str = response;

	dev_desc = blk_get_dev("mmc", CONFIG_FASTBOOT_FLASH_MMC_DEV);
	if (!dev_desc || dev_desc->type == DEV_TYPE_UNKNOWN) {
		error("invalid mmc device");
		fastboot_fail(response_str, "invalid mmc device");
		return;
	}

	ret = part_get_info_efi_by_name_or_alias(dev_desc, cmd, &info);
	if (ret) {
		error("cannot find partition: '%s'", cmd);
		fastboot_fail(response_str, "cannot find partition");
		return;
	}

	/* Align blocks to erase group size to avoid erasing other partitions */
	grp_size = mmc->erase_grp_size;
	blks_start = (info.start + grp_size - 1) & ~(grp_size - 1);
	if (info.size >= grp_size)
		blks_size = (info.size - (blks_start - info.start)) &
				(~(grp_size - 1));
	else
		blks_size = 0;

	printf("Erasing blocks " LBAFU " to " LBAFU " due to alignment\n",
	       blks_start, blks_start + blks_size);

	blks = dev_desc->block_erase(dev_desc, blks_start, blks_size);
	if (blks != blks_size) {
		error("failed erasing from device %d", dev_desc->devnum);
		fastboot_fail(response_str, "failed erasing from device");
		return;
	}

	printf("........ erased " LBAFU " bytes from '%s'\n",
	       blks_size * info.blksz, cmd);
	fastboot_okay(response_str, "");
}
