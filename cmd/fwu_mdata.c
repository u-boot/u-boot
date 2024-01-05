/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2022, Linaro Limited
 */

#include <command.h>
#include <dm.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <log.h>
#include <stdio.h>
#include <stdlib.h>

#include <linux/types.h>

static void print_mdata(struct fwu_mdata *mdata)
{
	int i, j;
	uint8_t num_banks;
	uint16_t num_images;
	struct fwu_image_entry *img_entry;
	struct fwu_image_bank_info *img_info;

	printf("\tFWU Metadata\n");
	printf("crc32: %#x\n", mdata->crc32);
	printf("version: %#x\n", mdata->version);
	printf("active_index: %#x\n", mdata->active_index);
	printf("previous_active_index: %#x\n", mdata->previous_active_index);

	num_banks = mdata->fw_desc[0].num_banks;
	num_images = mdata->fw_desc[0].num_images;

	for (i = 0; i < 4; i++)
		printf("bank_state[%d]: %#x\n", i, mdata->bank_state[i]);

	printf("\tImage Info\n");

	for (i = 0; i < num_images; i++) {
		img_entry = &mdata->fw_desc[0].img_entry[i];
		printf("\nImage Type Guid: %pUL\n",
		       &img_entry->image_type_guid);
		printf("Location Guid: %pUL\n", &img_entry->location_guid);
		for (j = 0; j < num_banks; j++) {
			img_info = &img_entry->img_bank_info[j];
			printf("Image Guid:  %pUL\n", &img_info->image_guid);
			printf("Image Acceptance: %s\n",
			       img_info->accepted == 0x1 ? "yes" : "no");
		}
	}
}

int do_fwu_mdata_read(struct cmd_tbl *cmdtp, int flag,
		     int argc, char * const argv[])
{
	uint32_t mdata_size;
	struct fwu_mdata *mdata = NULL;
	int ret = CMD_RET_SUCCESS, res;

	res = fwu_get_mdata_size(&mdata_size);
	if (res) {
		log_err("Unable to get FWU metadata size\n");
		ret = CMD_RET_FAILURE;
		goto out;
	}

	mdata = malloc(mdata_size);
	if (!mdata) {
		log_err("Unable to allocate memory for FWU metadata\n");
		ret = CMD_RET_FAILURE;
		goto out;
	}

	res = fwu_get_mdata(mdata);
	if (res < 0) {
		log_err("Unable to get valid FWU metadata\n");
		ret = CMD_RET_FAILURE;
		goto out;
	}

	print_mdata(mdata);

out:
	free(mdata);
	return ret;
}

U_BOOT_CMD(
	fwu_mdata_read,	1,	1,	do_fwu_mdata_read,
	"Read and print FWU metadata",
	""
);
