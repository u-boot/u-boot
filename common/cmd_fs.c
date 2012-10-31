/*
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * Inspired by cmd_ext_common.c, cmd_fat.c.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <command.h>
#include <fs.h>

int do_load_wrapper(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return do_load(cmdtp, flag, argc, argv, FS_TYPE_ANY, 0);
}

U_BOOT_CMD(
	load,	7,	0,	do_load_wrapper,
	"load binary file from a filesystem",
	"<interface> [<dev[:part]> [<addr> [<filename> [bytes [pos]]]]]\n"
	"    - Load binary file 'filename' from partition 'part' on device\n"
	"       type 'interface' instance 'dev' to address 'addr' in memory.\n"
	"      'bytes' gives the size to load in bytes.\n"
	"      If 'bytes' is 0 or omitted, the file is read until the end.\n"
	"      'pos' gives the file byte position to start reading from.\n"
	"      If 'pos' is 0 or omitted, the file is read from the start.\n"
	"      All numeric parameters are assumed to be decimal,\n"
	"      unless specified otherwise using a leading \"0x\"."
);

int do_ls_wrapper(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return do_ls(cmdtp, flag, argc, argv, FS_TYPE_ANY);
}

U_BOOT_CMD(
	ls,	4,	1,	do_ls_wrapper,
	"list files in a directory (default /)",
	"<interface> [<dev[:part]> [directory]]\n"
	"    - List files in directory 'directory' of partition 'part' on\n"
	"      device type 'interface' instance 'dev'."
);
