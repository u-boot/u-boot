/*
 * Copyright (c) 2012, Google Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fs.h>
#include <part.h>
#include <sandboxblockdev.h>
#include <asm/errno.h>

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

static int do_sandbox_bind(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	if (argc < 2 || argc > 3)
		return CMD_RET_USAGE;
	char *ep;
	char *dev_str = argv[1];
	char *file = argc >= 3 ? argv[2] : NULL;
	int dev = simple_strtoul(dev_str, &ep, 16);
	if (*ep) {
		printf("** Bad device specification %s **\n", dev_str);
		return CMD_RET_USAGE;
	}
	return host_dev_bind(dev, file);
}

static int do_sandbox_info(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	if (argc < 1 || argc > 2)
		return CMD_RET_USAGE;
	int min_dev = 0;
	int max_dev = CONFIG_HOST_MAX_DEVICES - 1;
	if (argc >= 2) {
		char *ep;
		char *dev_str = argv[1];
		int dev = simple_strtoul(dev_str, &ep, 16);
		if (*ep) {
			printf("** Bad device specification %s **\n", dev_str);
			return CMD_RET_USAGE;
		}
		min_dev = dev;
		max_dev = dev;
	}
	int dev;
	printf("%3s %12s %s\n", "dev", "blocks", "path");
	for (dev = min_dev; dev <= max_dev; dev++) {
		block_dev_desc_t *blk_dev;
		int ret;

		printf("%3d ", dev);
		ret = host_get_dev_err(dev, &blk_dev);
		if (ret) {
			if (ret == -ENOENT)
				puts("Not bound to a backing file\n");
			else if (ret == -ENODEV)
				puts("Invalid host device number\n");

			continue;
		}
		struct host_block_dev *host_dev = blk_dev->priv;
		printf("%12lu %s\n", (unsigned long)blk_dev->lba,
		       host_dev->filename);
	}
	return 0;
}

static cmd_tbl_t cmd_sandbox_sub[] = {
	U_BOOT_CMD_MKENT(load, 7, 0, do_sandbox_load, "", ""),
	U_BOOT_CMD_MKENT(ls, 3, 0, do_sandbox_ls, "", ""),
	U_BOOT_CMD_MKENT(save, 6, 0, do_sandbox_save, "", ""),
	U_BOOT_CMD_MKENT(bind, 3, 0, do_sandbox_bind, "", ""),
	U_BOOT_CMD_MKENT(info, 3, 0, do_sandbox_info, "", ""),
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
	"load hostfs - <addr> <filename> [<bytes> <offset>]  - "
		"load a file from host\n"
	"sb ls hostfs - <filename>                    - list files on host\n"
	"sb save hostfs - <addr> <filename> <bytes> [<offset>] - "
		"save a file to host\n"
	"sb bind <dev> [<filename>] - bind \"host\" device to file\n"
	"sb info [<dev>]            - show device binding & info\n"
	"sb commands use the \"hostfs\" device. The \"host\" device is used\n"
	"with standard IO commands such as fatls or ext2load"
);
