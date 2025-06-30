// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025 BayLibre, SAS
 */

#include <command.h>
#include <dm.h>
#include <display.h>
#include <edid.h>

static int do_read_edid(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	int ret;
	u8 edid[EDID_EXT_SIZE];

	/* Get the first display device (UCLASS_DISPLAY) */
	ret = uclass_first_device_err(UCLASS_DISPLAY, &dev);
	if (ret) {
		printf("Cannot get display device: %d\n", ret);
		return CMD_RET_FAILURE;
	}

	ret = display_read_edid(dev, edid, EDID_EXT_SIZE);
	if (ret) {
		printf("Cannot read edid: %d\n", ret);
		return CMD_RET_FAILURE;
	}

	edid_print_info((struct edid1_info *)edid);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(read_edid, 1, 0, do_read_edid,
	"Read and print EDID from display",
	""
);
