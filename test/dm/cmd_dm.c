/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <mapmem.h>
#include <errno.h>
#include <asm/io.h>
#include <dm/root.h>
#include <dm/uclass-internal.h>

static void show_devices(struct udevice *dev, int depth, int last_flag)
{
	int i, is_last;
	struct udevice *child;
	char class_name[12];

	/* print the first 11 characters to not break the tree-format. */
	strlcpy(class_name, dev->uclass->uc_drv->name, sizeof(class_name));
	printf(" %-11s [ %c ]    ", class_name,
	       dev->flags & DM_FLAG_ACTIVATED ? '+' : ' ');

	for (i = depth; i >= 0; i--) {
		is_last = (last_flag >> i) & 1;
		if (i) {
			if (is_last)
				printf("    ");
			else
				printf("|   ");
		} else {
			if (is_last)
				printf("`-- ");
			else
				printf("|-- ");
		}
	}

	printf("%s\n", dev->name);

	list_for_each_entry(child, &dev->child_head, sibling_node) {
		is_last = list_is_last(&child->sibling_node, &dev->child_head);
		show_devices(child, depth + 1, (last_flag << 1) | is_last);
	}
}

static int do_dm_dump_all(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	struct udevice *root;

	root = dm_root();
	if (root) {
		printf(" Class       Probed   Name\n");
		printf("----------------------------------------\n");
		show_devices(root, -1, 0);
	}

	return 0;
}

/**
 * dm_display_line() - Display information about a single device
 *
 * Displays a single line of information with an option prefix
 *
 * @dev:	Device to display
 */
static void dm_display_line(struct udevice *dev)
{
	printf("- %c %s @ %08lx",
	       dev->flags & DM_FLAG_ACTIVATED ? '*' : ' ',
	       dev->name, (ulong)map_to_sysmem(dev));
	if (dev->seq != -1 || dev->req_seq != -1)
		printf(", seq %d, (req %d)", dev->seq, dev->req_seq);
	puts("\n");
}

static int do_dm_dump_uclass(cmd_tbl_t *cmdtp, int flag, int argc,
			     char * const argv[])
{
	struct uclass *uc;
	int ret;
	int id;

	for (id = 0; id < UCLASS_COUNT; id++) {
		struct udevice *dev;

		ret = uclass_get(id, &uc);
		if (ret)
			continue;

		printf("uclass %d: %s\n", id, uc->uc_drv->name);
		if (list_empty(&uc->dev_head))
			continue;
		list_for_each_entry(dev, &uc->dev_head, uclass_node) {
			dm_display_line(dev);
		}
		puts("\n");
	}

	return 0;
}

static cmd_tbl_t test_commands[] = {
	U_BOOT_CMD_MKENT(tree, 0, 1, do_dm_dump_all, "", ""),
	U_BOOT_CMD_MKENT(uclass, 1, 1, do_dm_dump_uclass, "", ""),
};

static int do_dm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *test_cmd;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;
	test_cmd = find_cmd_tbl(argv[1], test_commands,
				ARRAY_SIZE(test_commands));
	argc -= 2;
	argv += 2;
	if (!test_cmd || argc > test_cmd->maxargs)
		return CMD_RET_USAGE;

	ret = test_cmd->cmd(test_cmd, flag, argc, argv);

	return cmd_process_error(test_cmd, ret);
}

U_BOOT_CMD(
	dm,	3,	1,	do_dm,
	"Driver model low level access",
	"tree         Dump driver model tree ('*' = activated)\n"
	"dm uclass        Dump list of instances for each uclass"
);
