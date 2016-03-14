/*
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * made from cmd_ext2, which was:
 *
 * (C) Copyright 2004
 * esd gmbh <www.esd-electronics.com>
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 *
 * made from cmd_reiserfs by
 *
 * (C) Copyright 2003 - 2004
 * Sysgo Real-Time Solutions, AG <www.elinos.com>
 * Pavel Bartusek <pba@sysgo.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <part.h>
#include <vsprintf.h>

#ifndef CONFIG_PARTITION_UUIDS
#error CONFIG_PARTITION_UUIDS must be enabled for CONFIG_CMD_PART to be enabled
#endif

static int do_part_uuid(int argc, char * const argv[])
{
	int part;
	struct blk_desc *dev_desc;
	disk_partition_t info;

	if (argc < 2)
		return CMD_RET_USAGE;
	if (argc > 3)
		return CMD_RET_USAGE;

	part = blk_get_device_part_str(argv[0], argv[1], &dev_desc, &info, 0);
	if (part < 0)
		return 1;

	if (argc > 2)
		setenv(argv[2], info.uuid);
	else
		printf("%s\n", info.uuid);

	return 0;
}

static int do_part_list(int argc, char * const argv[])
{
	int ret;
	struct blk_desc *desc;
	char *var = NULL;
	bool bootable = false;
	int i;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (argc > 2) {
		for (i = 2; i < argc ; i++) {
			if (argv[i][0] == '-') {
				if (!strcmp(argv[i], "-bootable")) {
					bootable = true;
				} else {
					printf("Unknown option %s\n", argv[i]);
					return CMD_RET_USAGE;
				}
			} else {
				var = argv[i];
				break;
			}
		}

		/* Loops should have been exited at the last argument, which
		 * as it contained the variable */
		if (argc != i + 1)
			return CMD_RET_USAGE;
	}

	ret = blk_get_device_by_str(argv[0], argv[1], &desc);
	if (ret < 0)
		return 1;

	if (var != NULL) {
		int p;
		char str[512] = { '\0', };
		disk_partition_t info;

		for (p = 1; p < 128; p++) {
			char t[5];
			int r = part_get_info(desc, p, &info);

			if (r != 0)
				continue;

			if (bootable && !info.bootable)
				continue;

			sprintf(t, "%s%x", str[0] ? " " : "", p);
			strcat(str, t);
		}
		setenv(var, str);
		return 0;
	}

	part_print(desc);

	return 0;
}

static int do_part_start(int argc, char * const argv[])
{
	struct blk_desc *desc;
	disk_partition_t info;
	char buf[512] = { 0 };
	int part;
	int err;
	int ret;

	if (argc < 3)
		return CMD_RET_USAGE;
	if (argc > 4)
		return CMD_RET_USAGE;

	part = simple_strtoul(argv[2], NULL, 0);

	ret = blk_get_device_by_str(argv[0], argv[1], &desc);
	if (ret < 0)
		return 1;

	err = part_get_info(desc, part, &info);
	if (err)
		return 1;

	snprintf(buf, sizeof(buf), LBAF, info.start);

	if (argc > 3)
		setenv(argv[3], buf);
	else
		printf("%s\n", buf);

	return 0;
}

static int do_part_size(int argc, char * const argv[])
{
	struct blk_desc *desc;
	disk_partition_t info;
	char buf[512] = { 0 };
	int part;
	int err;
	int ret;

	if (argc < 3)
		return CMD_RET_USAGE;
	if (argc > 4)
		return CMD_RET_USAGE;

	part = simple_strtoul(argv[2], NULL, 0);

	ret = blk_get_device_by_str(argv[0], argv[1], &desc);
	if (ret < 0)
		return 1;

	err = part_get_info(desc, part, &info);
	if (err)
		return 1;

	snprintf(buf, sizeof(buf), LBAF, info.size);

	if (argc > 3)
		setenv(argv[3], buf);
	else
		printf("%s\n", buf);

	return 0;
}

static int do_part(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	if (!strcmp(argv[1], "uuid"))
		return do_part_uuid(argc - 2, argv + 2);
	else if (!strcmp(argv[1], "list"))
		return do_part_list(argc - 2, argv + 2);
	else if (!strcmp(argv[1], "start"))
		return do_part_start(argc - 2, argv + 2);
	else if (!strcmp(argv[1], "size"))
		return do_part_size(argc - 2, argv + 2);

	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	part,	CONFIG_SYS_MAXARGS,	1,	do_part,
	"disk partition related commands",
	"uuid <interface> <dev>:<part>\n"
	"    - print partition UUID\n"
	"part uuid <interface> <dev>:<part> <varname>\n"
	"    - set environment variable to partition UUID\n"
	"part list <interface> <dev>\n"
	"    - print a device's partition table\n"
	"part list <interface> <dev> [flags] <varname>\n"
	"    - set environment variable to the list of partitions\n"
	"      flags can be -bootable (list only bootable partitions)\n"
	"part start <interface> <dev> <part> <varname>\n"
	"    - set environment variable to the start of the partition (in blocks)\n"
	"part size <interface> <dev> <part> <varname>\n"
	"    - set environment variable to the size of the partition (in blocks)"
);
