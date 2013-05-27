/*
 * Copyright (c) 2012, Google Inc.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <fs.h>

static int do_sandbox_load(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	return do_load(cmdtp, flag, argc, argv, FS_TYPE_SANDBOX, 16);
}

static int do_sandbox_ls(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	return do_ls(cmdtp, flag, argc, argv, FS_TYPE_SANDBOX);
}

static cmd_tbl_t cmd_sandbox_sub[] = {
	U_BOOT_CMD_MKENT(load, 3, 0, do_sandbox_load, "", ""),
	U_BOOT_CMD_MKENT(ls, 3, 0, do_sandbox_ls, "", ""),
};

static int do_sandbox(cmd_tbl_t *cmdtp, int flag, int argc,
		      char * const argv[])
{
	cmd_tbl_t *c;

	/* Skip past 'sandbox' */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], cmd_sandbox_sub,
			 ARRAY_SIZE(cmd_sandbox_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

U_BOOT_CMD(
	sb,	6,	1,	do_sandbox,
	"Miscellaneous sandbox commands",
	"load host <addr> <filename> [<bytes> <offset>]  - load a file from host\n"
	"sb ls host <filename>      - save a file to host"
);
