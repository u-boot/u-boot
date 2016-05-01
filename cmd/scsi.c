/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * SCSI support.
 */
#include <common.h>
#include <command.h>
#include <scsi.h>

static int scsi_curr_dev; /* current device */

/*
 * scsi boot command intepreter. Derived from diskboot
 */
int do_scsiboot(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	return common_diskboot(cmdtp, "scsi", argc, argv);
}

/*
 * scsi command intepreter
 */
int do_scsi(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	switch (argc) {
	case 0:
	case 1:
		return CMD_RET_USAGE;
	case 2:
		if (strncmp(argv[1], "res", 3) == 0) {
			printf("\nReset SCSI\n");
			scsi_bus_reset();
			scsi_scan(1);
			return 0;
		}
		if (strncmp(argv[1], "inf", 3) == 0) {
			blk_list_devices(IF_TYPE_SCSI);
			return 0;
		}
		if (strncmp(argv[1], "dev", 3) == 0) {
			if (blk_print_device_num(IF_TYPE_SCSI, scsi_curr_dev)) {
				printf("\nno SCSI devices available\n");
				return CMD_RET_FAILURE;
			}

			return 0;
		}
		if (strncmp(argv[1], "scan", 4) == 0) {
			scsi_scan(1);
			return 0;
		}
		if (strncmp(argv[1], "part", 4) == 0) {
			if (blk_list_part(IF_TYPE_SCSI))
				printf("\nno SCSI devices available\n");
			return 0;
		}
		return CMD_RET_USAGE;
	case 3:
		if (strncmp(argv[1], "dev", 3) == 0) {
			int dev = (int)simple_strtoul(argv[2], NULL, 10);

			if (!blk_show_device(IF_TYPE_SCSI, dev)) {
				scsi_curr_dev = dev;
				printf("... is now current device\n");
			} else {
				return CMD_RET_FAILURE;
			}
			return 0;
		}
		if (strncmp(argv[1], "part", 4) == 0) {
			int dev = (int)simple_strtoul(argv[2], NULL, 10);

			if (blk_print_part_devnum(IF_TYPE_SCSI, dev)) {
				printf("\nSCSI device %d not available\n",
				       dev);
				return CMD_RET_FAILURE;
			}
			return 0;
		}
		return CMD_RET_USAGE;
	default:
		/* at least 4 args */
		if (strcmp(argv[1], "read") == 0) {
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			ulong blk  = simple_strtoul(argv[3], NULL, 16);
			ulong cnt  = simple_strtoul(argv[4], NULL, 16);
			ulong n;

			printf("\nSCSI read: device %d block # %ld, count %ld ... ",
			       scsi_curr_dev, blk, cnt);
			n = blk_read_devnum(IF_TYPE_SCSI, scsi_curr_dev, blk,
					    cnt, (ulong *)addr);
			printf("%ld blocks read: %s\n", n,
			       n == cnt ? "OK" : "ERROR");
			return 0;
		} else if (strcmp(argv[1], "write") == 0) {
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			ulong blk = simple_strtoul(argv[3], NULL, 16);
			ulong cnt = simple_strtoul(argv[4], NULL, 16);
			ulong n;

			printf("\nSCSI write: device %d block # %ld, count %ld ... ",
			       scsi_curr_dev, blk, cnt);
			n = blk_write_devnum(IF_TYPE_SCSI, scsi_curr_dev, blk,
					     cnt, (ulong *)addr);
			printf("%ld blocks written: %s\n", n,
			       n == cnt ? "OK" : "ERROR");
			return 0;
		}
	} /* switch */
	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	scsi, 5, 1, do_scsi,
	"SCSI sub-system",
	"reset - reset SCSI controller\n"
	"scsi info  - show available SCSI devices\n"
	"scsi scan  - (re-)scan SCSI bus\n"
	"scsi device [dev] - show or set current device\n"
	"scsi part [dev] - print partition table of one or all SCSI devices\n"
	"scsi read addr blk# cnt - read `cnt' blocks starting at block `blk#'\n"
	"     to memory address `addr'\n"
	"scsi write addr blk# cnt - write `cnt' blocks starting at block\n"
	"     `blk#' from memory address `addr'"
);

U_BOOT_CMD(
	scsiboot, 3, 1, do_scsiboot,
	"boot from SCSI device",
	"loadAddr dev:part"
);
