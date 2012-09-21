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
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <part.h>
#include <vsprintf.h>

#ifndef CONFIG_PARTITION_UUIDS
#error CONFIG_PARTITION_UUIDS must be enabled for CONFIG_CMD_PART to be enabled
#endif

int do_part_uuid(int argc, char * const argv[])
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

int do_part_list(int argc, char * const argv[])
{
	int ret;
	block_dev_desc_t *desc;

	if (argc != 2)
		return CMD_RET_USAGE;

	ret = get_device(argv[0], argv[1], &desc);
	if (ret < 0)
		return 1;

	print_part(desc);

	return 0;
}

int do_part(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
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
	"    - print a device's partition table"
);
