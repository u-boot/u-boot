/*
 * Copyright 2014 Broadcom Corporation.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <fb_mmc.h>
#include <part.h>
#include <aboot.h>
#include <sparse_format.h>

#ifndef CONFIG_FASTBOOT_GPT_NAME
#define CONFIG_FASTBOOT_GPT_NAME GPT_ENTRY_NAME
#endif

/* The 64 defined bytes plus the '\0' */
#define RESPONSE_LEN	(64 + 1)

static char *response_str;

void fastboot_fail(const char *s)
{
	strncpy(response_str, "FAIL", 4);
	strncat(response_str, s, RESPONSE_LEN - 4 - 1);
}

void fastboot_okay(const char *s)
{
	strncpy(response_str, "OKAY", 4);
	strncat(response_str, s, RESPONSE_LEN - 4 - 1);
}

static void write_raw_image(block_dev_desc_t *dev_desc, disk_partition_t *info,
		const char *part_name, void *buffer,
		unsigned int download_bytes)
{
	lbaint_t blkcnt;
	lbaint_t blks;

	/* determine number of blocks to write */
	blkcnt = ((download_bytes + (info->blksz - 1)) & ~(info->blksz - 1));
	blkcnt = blkcnt / info->blksz;

	if (blkcnt > info->size) {
		error("too large for partition: '%s'\n", part_name);
		fastboot_fail("too large for partition");
		return;
	}

	puts("Flashing Raw Image\n");

	blks = dev_desc->block_write(dev_desc->dev, info->start, blkcnt,
				     buffer);
	if (blks != blkcnt) {
		error("failed writing to device %d\n", dev_desc->dev);
		fastboot_fail("failed writing to device");
		return;
	}

	printf("........ wrote " LBAFU " bytes to '%s'\n", blkcnt * info->blksz,
	       part_name);
	fastboot_okay("");
}

void fb_mmc_flash_write(const char *cmd, void *download_buffer,
			unsigned int download_bytes, char *response)
{
	block_dev_desc_t *dev_desc;
	disk_partition_t info;

	/* initialize the response buffer */
	response_str = response;

	dev_desc = get_dev("mmc", CONFIG_FASTBOOT_FLASH_MMC_DEV);
	if (!dev_desc || dev_desc->type == DEV_TYPE_UNKNOWN) {
		error("invalid mmc device\n");
		fastboot_fail("invalid mmc device");
		return;
	}

	if (strcmp(cmd, CONFIG_FASTBOOT_GPT_NAME) == 0) {
		printf("%s: updating MBR, Primary and Backup GPT(s)\n",
		       __func__);
		if (is_valid_gpt_buf(dev_desc, download_buffer)) {
			printf("%s: invalid GPT - refusing to write to flash\n",
			       __func__);
			fastboot_fail("invalid GPT partition");
			return;
		}
		if (write_mbr_and_gpt_partitions(dev_desc, download_buffer)) {
			printf("%s: writing GPT partitions failed\n", __func__);
			fastboot_fail("writing GPT partitions failed");
			return;
		}
		printf("........ success\n");
		fastboot_okay("");
		return;
	} else if (get_partition_info_efi_by_name(dev_desc, cmd, &info)) {
		error("cannot find partition: '%s'\n", cmd);
		fastboot_fail("cannot find partition");
		return;
	}

	if (is_sparse_image(download_buffer))
		write_sparse_image(dev_desc, &info, cmd, download_buffer,
				   download_bytes);
	else
		write_raw_image(dev_desc, &info, cmd, download_buffer,
				download_bytes);
}
