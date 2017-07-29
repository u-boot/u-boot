/*
 * Copyright (C) 2000-2005, DENX Software Engineering
 *		Wolfgang Denk <wd@denx.de>
 * Copyright (C) Procsys. All rights reserved.
 *		Mushtaq Khan <mushtaq_k@procsys.com>
 *			<mushtaqk_921@yahoo.co.in>
 * Copyright (C) 2008 Freescale Semiconductor, Inc.
 *		Dave Liu <daveliu@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <part.h>
#include <sata.h>

static int sata_curr_device = -1;

static int do_sata(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int rc = 0;

	if (argc == 2 && strcmp(argv[1], "stop") == 0)
		return sata_stop();

	if (argc == 2 && strcmp(argv[1], "init") == 0) {
		if (sata_curr_device != -1)
			sata_stop();

		return (sata_initialize() < 0) ?
			CMD_RET_FAILURE : CMD_RET_SUCCESS;
	}

	/* If the user has not yet run `sata init`, do it now */
	if (sata_curr_device == -1) {
		rc = sata_initialize();
		if (rc == -1)
			return CMD_RET_FAILURE;
		sata_curr_device = rc;
	}

	return blk_common_cmd(argc, argv, IF_TYPE_SATA, &sata_curr_device);
}

U_BOOT_CMD(
	sata, 5, 1, do_sata,
	"SATA sub system",
	"init - init SATA sub system\n"
	"sata stop - disable SATA sub system\n"
	"sata info - show available SATA devices\n"
	"sata device [dev] - show or set current device\n"
	"sata part [dev] - print partition table\n"
	"sata read addr blk# cnt\n"
	"sata write addr blk# cnt"
);
