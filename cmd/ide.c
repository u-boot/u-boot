/*
 * (C) Copyright 2000-2011
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * IDE support
 */

#include <common.h>
#include <blk.h>
#include <config.h>
#include <watchdog.h>
#include <command.h>
#include <image.h>
#include <asm/byteorder.h>
#include <asm/io.h>

#if defined(CONFIG_IDE_PCMCIA)
# include <pcmcia.h>
#endif

#include <ide.h>
#include <ata.h>

#ifdef CONFIG_LED_STATUS
# include <status_led.h>
#endif

/* Current I/O Device	*/
static int curr_device = -1;

int do_ide(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int rcode = 0;

	switch (argc) {
	case 0:
	case 1:
		return CMD_RET_USAGE;
	case 2:
		if (strncmp(argv[1], "res", 3) == 0) {
			puts("\nReset IDE: ");
			ide_init();
			return 0;
		} else if (strncmp(argv[1], "inf", 3) == 0) {
			blk_list_devices(IF_TYPE_IDE);
			return 0;

		} else if (strncmp(argv[1], "dev", 3) == 0) {
			if (blk_print_device_num(IF_TYPE_IDE, curr_device)) {
				printf("\nno IDE devices available\n");
				return CMD_RET_FAILURE;
			}

			return 0;
		} else if (strncmp(argv[1], "part", 4) == 0) {
			if (blk_list_part(IF_TYPE_IDE))
				printf("\nno IDE devices available\n");
			return 1;
		}
		return CMD_RET_USAGE;
	case 3:
		if (strncmp(argv[1], "dev", 3) == 0) {
			int dev = (int)simple_strtoul(argv[2], NULL, 10);

			if (!blk_show_device(IF_TYPE_IDE, dev)) {
				curr_device = dev;
				printf("... is now current device\n");
			} else {
				return CMD_RET_FAILURE;
			}
			return 0;
		} else if (strncmp(argv[1], "part", 4) == 0) {
			int dev = (int)simple_strtoul(argv[2], NULL, 10);

			if (blk_print_part_devnum(IF_TYPE_IDE, dev)) {
				printf("\nIDE device %d not available\n", dev);
				return CMD_RET_FAILURE;
			}
			return 1;
		}

		return CMD_RET_USAGE;
	default:
		/* at least 4 args */

		if (strcmp(argv[1], "read") == 0) {
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			ulong cnt = simple_strtoul(argv[4], NULL, 16);
			ulong n;

#ifdef CONFIG_SYS_64BIT_LBA
			lbaint_t blk = simple_strtoull(argv[3], NULL, 16);

			printf("\nIDE read: device %d block # %lld, count %ld...",
			       curr_device, blk, cnt);
#else
			lbaint_t blk = simple_strtoul(argv[3], NULL, 16);

			printf("\nIDE read: device %d block # %ld, count %ld...",
			       curr_device, blk, cnt);
#endif

			n = blk_read_devnum(IF_TYPE_IDE, curr_device, blk, cnt,
					    (ulong *)addr);

			printf("%ld blocks read: %s\n",
			       n, (n == cnt) ? "OK" : "ERROR");
			if (n == cnt)
				return 0;
			else
				return 1;
		} else if (strcmp(argv[1], "write") == 0) {
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			ulong cnt = simple_strtoul(argv[4], NULL, 16);
			ulong n;

#ifdef CONFIG_SYS_64BIT_LBA
			lbaint_t blk = simple_strtoull(argv[3], NULL, 16);

			printf("\nIDE write: device %d block # %lld, count %ld...",
			       curr_device, blk, cnt);
#else
			lbaint_t blk = simple_strtoul(argv[3], NULL, 16);

			printf("\nIDE write: device %d block # %ld, count %ld...",
			       curr_device, blk, cnt);
#endif
			n = blk_write_devnum(IF_TYPE_IDE, curr_device, blk, cnt,
					     (ulong *)addr);

			printf("%ld blocks written: %s\n", n,
			       n == cnt ? "OK" : "ERROR");
			if (n == cnt)
				return 0;
			else
				return 1;
		} else {
			return CMD_RET_USAGE;
		}

		return rcode;
	}
}

int do_diskboot(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	return common_diskboot(cmdtp, "ide", argc, argv);
}

U_BOOT_CMD(ide, 5, 1, do_ide,
	   "IDE sub-system",
	   "reset - reset IDE controller\n"
	   "ide info  - show available IDE devices\n"
	   "ide device [dev] - show or set current device\n"
	   "ide part [dev] - print partition table of one or all IDE devices\n"
	   "ide read  addr blk# cnt\n"
	   "ide write addr blk# cnt - read/write `cnt'"
	   " blocks starting at block `blk#'\n"
	   "    to/from memory address `addr'");

U_BOOT_CMD(diskboot, 3, 1, do_diskboot,
	   "boot from IDE device", "loadAddr dev:part");
