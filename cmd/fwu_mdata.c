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

static void print_mdata(struct fwu_data *data)
{
	int i, j;
	struct fwu_image_entry *img_entry;
	struct fwu_image_bank_info *img_info;

	printf("\tFWU Metadata\n");
	printf("crc32: %#x\n", data->crc32);
	printf("version: %#x\n", data->version);
	printf("active_index: %#x\n", data->active_index);
	printf("previous_active_index: %#x\n", data->previous_active_index);

	if (data->version == 2) {
		for (i = 0; i < 4; i++)
			printf("bank_state[%d]: %#x\n",
			       i, data->bank_state[i]);
	}

	printf("\tImage Info\n");
	for (i = 0; i < CONFIG_FWU_NUM_IMAGES_PER_BANK; i++) {
		img_entry = &data->fwu_images[i];
		printf("\nImage Type Guid: %pUL\n",
		       &img_entry->image_type_guid);
		printf("Location Guid: %pUL\n", &img_entry->location_guid);
		for (j = 0; j < CONFIG_FWU_NUM_BANKS; j++) {
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
	struct fwu_data *data = fwu_get_data();

	print_mdata(data);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	fwu_mdata_read,	1,	1,	do_fwu_mdata_read,
	"Read and print FWU metadata",
	""
);
