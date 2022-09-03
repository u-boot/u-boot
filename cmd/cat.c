// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022
 * Roger Knecht <rknecht@pm.de>
 */

#include <common.h>
#include <command.h>
#include <fs.h>
#include <malloc.h>
#include <mapmem.h>

static int do_cat(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
	char *ifname;
	char *dev;
	char *file;
	char *buffer;
	phys_addr_t addr;
	loff_t file_size;

	if (argc < 4)
		return CMD_RET_USAGE;

	ifname = argv[1];
	dev = argv[2];
	file = argv[3];

	// check file exists
	if (fs_set_blk_dev(ifname, dev, FS_TYPE_ANY))
		return CMD_RET_FAILURE;

	if (!fs_exists(file)) {
		log_err("File does not exist: ifname=%s dev=%s file=%s\n", ifname, dev, file);
		return CMD_RET_FAILURE;
	}

	// get file size
	if (fs_set_blk_dev(ifname, dev, FS_TYPE_ANY))
		return CMD_RET_FAILURE;

	if (fs_size(file, &file_size)) {
		log_err("Cannot read file size: ifname=%s dev=%s file=%s\n", ifname, dev, file);
		return CMD_RET_FAILURE;
	}

	// allocate memory for file content
	buffer = calloc(sizeof(char), file_size + 1);
	if (!buffer) {
		log_err("Out of memory\n");
		return CMD_RET_FAILURE;
	}

	// map pointer to system memory
	addr = map_to_sysmem(buffer);

	// read file to memory
	if (fs_set_blk_dev(ifname, dev, FS_TYPE_ANY))
		return CMD_RET_FAILURE;

	if (fs_read(file, addr, 0, 0, &file_size)) {
		log_err("Cannot read file: ifname=%s dev=%s file=%s\n", ifname, dev, file);
		return CMD_RET_FAILURE;
	}

	// print file content
	buffer[file_size] = '\0';
	puts(buffer);

	free(buffer);

	return 0;
}

#ifdef CONFIG_SYS_LONGHELP
static char cat_help_text[] =
	"<interface> <dev[:part]> <file>\n"
	"  - Print file from 'dev' on 'interface' to standard output\n";
#endif

U_BOOT_CMD(cat, 4, 1, do_cat,
	   "Print file to standard output",
	   cat_help_text
);
