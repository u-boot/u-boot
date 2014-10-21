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
#include <errno.h>
#include <asm/io.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>

/**
 * dm_display_line() - Display information about a single device
 *
 * Displays a single line of information with an option prefix
 *
 * @dev:	Device to display
 * @buf:	Prefix to display at the start of the line
 */
static void dm_display_line(struct udevice *dev, char *buf)
{
	printf("%s- %c %s @ %08lx", buf,
	       dev->flags & DM_FLAG_ACTIVATED ? '*' : ' ',
	       dev->name, (ulong)map_to_sysmem(dev));
	if (dev->req_seq != -1)
		printf(", %d", dev->req_seq);
	puts("\n");
}

static int display_succ(struct udevice *in, char *buf)
{
	int len;
	int ip = 0;
	char local[16];
	struct udevice *pos, *n, *prev = NULL;

	dm_display_line(in, buf);

	if (list_empty(&in->child_head))
		return 0;

	len = strlen(buf);
	strncpy(local, buf, sizeof(local));
	snprintf(local + len, 2, "|");
	if (len && local[len - 1] == '`')
		local[len - 1] = ' ';

	list_for_each_entry_safe(pos, n, &in->child_head, sibling_node) {
		if (ip++)
			display_succ(prev, local);
		prev = pos;
	}

	snprintf(local + len, 2, "`");
	display_succ(prev, local);

	return 0;
}

static int dm_dump(struct udevice *dev)
{
	if (!dev)
		return -EINVAL;
	return display_succ(dev, "");
}

static int do_dm_dump_all(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	struct udevice *root;

	root = dm_root();
	printf("ROOT %08lx\n", (ulong)map_to_sysmem(root));
	return dm_dump(root);
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
			dm_display_line(dev, "");
		}
		puts("\n");
	}

	return 0;
}

#ifdef CONFIG_DM_TEST
static int do_dm_test(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	return dm_test_main();
}
#define TEST_HELP "\ndm test         Run tests"
#else
#define TEST_HELP
#endif

static cmd_tbl_t test_commands[] = {
	U_BOOT_CMD_MKENT(tree, 0, 1, do_dm_dump_all, "", ""),
	U_BOOT_CMD_MKENT(uclass, 1, 1, do_dm_dump_uclass, "", ""),
#ifdef CONFIG_DM_TEST
	U_BOOT_CMD_MKENT(test, 1, 1, do_dm_test, "", ""),
#endif
};

static int do_dm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *test_cmd;
	int ret;

	if (argc != 2)
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
	dm,	2,	1,	do_dm,
	"Driver model low level access",
	"tree         Dump driver model tree ('*' = activated)\n"
	"dm uclass        Dump list of instances for each uclass"
	TEST_HELP
);
