/*
 * Copyright (c) 2012, Google Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fs.h>

static int do_sandbox_load(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	return do_load(cmdtp, flag, argc, argv, FS_TYPE_SANDBOX);
}

static int do_sandbox_ls(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	return do_ls(cmdtp, flag, argc, argv, FS_TYPE_SANDBOX);
}

static int do_sandbox_save(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	return do_save(cmdtp, flag, argc, argv, FS_TYPE_SANDBOX);
}

static cmd_tbl_t cmd_sandbox_sub[] = {
	U_BOOT_CMD_MKENT(load, 7, 0, do_sandbox_load, "", ""),
	U_BOOT_CMD_MKENT(ls, 3, 0, do_sandbox_ls, "", ""),
	U_BOOT_CMD_MKENT(save, 6, 0, do_sandbox_save, "", ""),
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
	sb,	8,	1,	do_sandbox,
	"Miscellaneous sandbox commands",
	"load host <dev> <addr> <filename> [<bytes> <offset>]  - "
		"load a file from host\n"
	"sb ls host <filename>                      - list files on host\n"
	"sb save host <dev> <filename> <addr> <bytes> [<offset>] - "
		"save a file to host\n"
);
