/*
 * Copyright (C) 2017 NXP Semiconductors
 * Copyright (C) 2017 Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <nvme.h>
#include <part.h>
#include <linux/math64.h>

static int nvme_curr_device;

static int do_nvme_scan(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	int ret;

	ret = nvme_scan_namespace();
	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int do_nvme_list(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	blk_list_devices(IF_TYPE_NVME);

	return CMD_RET_SUCCESS;
}

static int do_nvme_info(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	int devnum;
	struct udevice *udev;
	int ret;

	if (argc > 1)
		devnum = (int)simple_strtoul(argv[1], NULL, 10);
	else
		devnum = nvme_curr_device;

	ret = blk_get_device(IF_TYPE_NVME, devnum, &udev);
	if (ret < 0)
		return CMD_RET_FAILURE;

	nvme_print_info(udev);

	return CMD_RET_SUCCESS;
}

static int do_nvme_device(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	if (argc > 1) {
		int devnum = (int)simple_strtoul(argv[1], NULL, 10);

		if (!blk_show_device(IF_TYPE_NVME, devnum)) {
			nvme_curr_device = devnum;
			printf("... is now current device\n");
		} else {
			return CMD_RET_FAILURE;
		}
	} else {
		blk_show_device(IF_TYPE_NVME, nvme_curr_device);
	}

	return CMD_RET_SUCCESS;
}

static int do_nvme_part(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	if (argc > 1) {
		int devnum = (int)simple_strtoul(argv[2], NULL, 10);

		if (blk_print_part_devnum(IF_TYPE_NVME, devnum)) {
			printf("\nNVMe device %d not available\n", devnum);
			return CMD_RET_FAILURE;
		}
	} else {
		blk_print_part_devnum(IF_TYPE_NVME, nvme_curr_device);
	}

	return CMD_RET_SUCCESS;
}

static int do_nvme_read(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	unsigned long time;
	if (argc != 4)
		return CMD_RET_USAGE;

	ulong addr = simple_strtoul(argv[1], NULL, 16);
	ulong cnt = simple_strtoul(argv[3], NULL, 16);
	ulong n;
	lbaint_t blk = simple_strtoul(argv[2], NULL, 16);

	printf("\nNVMe read: device %d block # " LBAFU " count %ld ... ",
	       nvme_curr_device, blk, cnt);

	time = get_timer(0);
	n = blk_read_devnum(IF_TYPE_NVME, nvme_curr_device, blk,
			    cnt, (ulong *)addr);
	time = get_timer(time);

	printf("read: %s\n", (n == cnt) ? "OK" : "ERROR");
	printf("%lu bytes read in %lu ms", cnt * 512, time);
	if (time > 0) {
		puts(" (");
		print_size(div_u64(cnt * 512, time) * 1000, "/s");
		puts(")");
	}
	puts("\n");

	return (n == cnt) ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}

static int do_nvme_write(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	unsigned long time;
	if (argc != 4)
		return CMD_RET_USAGE;

	ulong addr = simple_strtoul(argv[1], NULL, 16);
	ulong cnt = simple_strtoul(argv[3], NULL, 16);
	ulong n;
	lbaint_t blk = simple_strtoul(argv[2], NULL, 16);

	printf("\nNVMe write: device %d block # " LBAFU " count %ld ... ",
	       nvme_curr_device, blk, cnt);

	time = get_timer(0);
	n = blk_write_devnum(IF_TYPE_NVME, nvme_curr_device, blk,
			    cnt, (ulong *)addr);
	time = get_timer(time);

	printf("write: %s\n", (n == cnt) ? "OK" : "ERROR");
	printf("%lu bytes write in %lu ms", cnt * 512, time);
	if (time > 0) {
		puts(" (");
		print_size(div_u64(cnt * 512, time) * 1000, "/s");
		puts(")");
	}
	puts("\n");

	return (n == cnt) ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}

static cmd_tbl_t cmd_nvme[] = {
	U_BOOT_CMD_MKENT(scan, 1, 1, do_nvme_scan, "", ""),
	U_BOOT_CMD_MKENT(list, 1, 1, do_nvme_list, "", ""),
	U_BOOT_CMD_MKENT(info, 2, 1, do_nvme_info, "", ""),
	U_BOOT_CMD_MKENT(device, 2, 1, do_nvme_device, "", ""),
	U_BOOT_CMD_MKENT(part, 2, 1, do_nvme_part, "", ""),
	U_BOOT_CMD_MKENT(write, 4, 0, do_nvme_write, "", ""),
	U_BOOT_CMD_MKENT(read, 4, 0, do_nvme_read, "", "")
};

static int do_nvmecops(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	cmd_tbl_t *cp;

	cp = find_cmd_tbl(argv[1], cmd_nvme, ARRAY_SIZE(cmd_nvme));

	argc--;
	argv++;

	if (cp == NULL || argc > cp->maxargs)
		return CMD_RET_USAGE;

	if (flag == CMD_FLAG_REPEAT && !cp->repeatable)
		return CMD_RET_SUCCESS;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	nvme, 8, 1, do_nvmecops,
	"NVM Express sub-system",
	"\nnvme scan - scan NVMe blk devices\n"
	"nvme list - show all available NVMe blk devices\n"
	"nvme info [dev]- show current or a specific NVMe blk device\n"
	"nvme device [dev] - show or set current device\n"
	"nvme part [dev] - print partition table\n"
	"nvme read  addr blk# cnt\n"
	"nvme write addr blk# cnt"
);
