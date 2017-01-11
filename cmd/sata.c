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

	switch (argc) {
	case 0:
	case 1:
		return CMD_RET_USAGE;
	case 2:
		if (strncmp(argv[1], "inf", 3) == 0) {
			blk_list_devices(IF_TYPE_SATA);
			return 0;
		} else if (strncmp(argv[1], "dev", 3) == 0) {
			if (blk_print_device_num(IF_TYPE_SATA,
						 sata_curr_device)) {
				printf("\nno SATA devices available\n");
				return CMD_RET_FAILURE;
			}
			return 0;
		} else if (strncmp(argv[1], "part", 4) == 0) {
			if (blk_list_part(IF_TYPE_SATA))
				puts("\nno SATA devices available\n");
			return 0;
		}
		return CMD_RET_USAGE;
	case 3:
		if (strncmp(argv[1], "dev", 3) == 0) {
			int dev = (int)simple_strtoul(argv[2], NULL, 10);

			if (!blk_show_device(IF_TYPE_SATA, dev)) {
				sata_curr_device = dev;
				printf("... is now current device\n");
			} else {
				return CMD_RET_FAILURE;
			}
			return 0;
		} else if (strncmp(argv[1], "part", 4) == 0) {
			int dev = (int)simple_strtoul(argv[2], NULL, 10);

			if (blk_print_part_devnum(IF_TYPE_SATA, dev)) {
				printf("\nSATA device %d not available\n",
				       dev);
				return CMD_RET_FAILURE;
			}
			return rc;
		}
		return CMD_RET_USAGE;

	default: /* at least 4 args */
		if (strcmp(argv[1], "read") == 0) {
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			ulong cnt = simple_strtoul(argv[4], NULL, 16);
			ulong n;
			lbaint_t blk = simple_strtoul(argv[3], NULL, 16);

			printf("\nSATA read: device %d block # %ld, count %ld ... ",
				sata_curr_device, blk, cnt);

			n = blk_read_devnum(IF_TYPE_SATA, sata_curr_device, blk,
					    cnt, (ulong *)addr);

			printf("%ld blocks read: %s\n",
				n, (n==cnt) ? "OK" : "ERROR");
			return (n == cnt) ? 0 : 1;
		} else if (strcmp(argv[1], "write") == 0) {
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			ulong cnt = simple_strtoul(argv[4], NULL, 16);
			ulong n;

			lbaint_t blk = simple_strtoul(argv[3], NULL, 16);

			printf("\nSATA write: device %d block # %ld, count %ld ... ",
				sata_curr_device, blk, cnt);

			n = blk_write_devnum(IF_TYPE_SATA, sata_curr_device,
					     blk, cnt, (ulong *)addr);

			printf("%ld blocks written: %s\n",
				n, (n == cnt) ? "OK" : "ERROR");
			return (n == cnt) ? 0 : 1;
		} else {
			return CMD_RET_USAGE;
		}

		return rc;
	}
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
