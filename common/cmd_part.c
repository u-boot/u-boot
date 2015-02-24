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
	block_dev_desc_t *dev_desc;
	disk_partition_t info;

	if (argc < 2)
		return CMD_RET_USAGE;
	if (argc > 3)
		return CMD_RET_USAGE;

	part = get_device_and_partition(argv[0], argv[1], &dev_desc, &info, 0);
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
	block_dev_desc_t *desc;

	if (argc < 2 || argc > 3)
		return CMD_RET_USAGE;

	ret = get_device(argv[0], argv[1], &desc);
	if (ret < 0)
		return 1;

	if (argc == 3) {
		int p;
		char str[512] = { 0, };
	  disk_partition_t info;

		for (p = 1; p < 128; p++) {
			int r = get_partition_info(desc, p, &info);

			if (r == 0) {
				char t[5];
				sprintf(t, "%s%d", str[0] ? " " : "", p);
				strcat(str, t);
			}
		}
		setenv(argv[2], str);
		return 0;
	}

	print_part(desc);

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

	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	part,	5,	1,	do_part,
	"disk partition related commands",
	"part uuid <interface> <dev>:<part>\n"
	"    - print partition UUID\n"
	"part uuid <interface> <dev>:<part> <varname>\n"
	"    - set environment variable to partition UUID\n"
	"part list <interface> <dev>\n"
	"    - print a device's partition table\n"
	"part list <interface> <dev> <varname>\n"
	"    - set environment variable to the list of partitions"
);
