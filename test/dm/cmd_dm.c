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

static int display_succ(struct device *in, char *buf)
{
	int len;
	int ip = 0;
	char local[16];
	struct device *pos, *n, *prev = NULL;

	printf("%s- %s @ %08x", buf, in->name, map_to_sysmem(in));
	if (in->flags & DM_FLAG_ACTIVATED)
		puts(" - activated");
	puts("\n");

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

static int dm_dump(struct device *dev)
{
	if (!dev)
		return -EINVAL;
	return display_succ(dev, "");
}

static int do_dm_dump_all(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	struct device *root;

	root = dm_root();
	printf("ROOT %08x\n", map_to_sysmem(root));
	return dm_dump(root);
}

static int do_dm_dump_uclass(cmd_tbl_t *cmdtp, int flag, int argc,
			     char * const argv[])
{
	struct uclass *uc;
	int ret;
	int id;

	for (id = 0; id < UCLASS_COUNT; id++) {
		struct device *dev;

		ret = uclass_get(id, &uc);
		if (ret)
			continue;

		printf("uclass %d: %s\n", id, uc->uc_drv->name);
		for (ret = uclass_first_device(id, &dev);
		     dev;
		     ret = uclass_next_device(&dev)) {
			printf("  %s @  %08x:\n", dev->name,
			       map_to_sysmem(dev));
		}
		puts("\n");
	}

	return 0;
}

static int do_dm_test(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	return dm_test_main();
}

static cmd_tbl_t test_commands[] = {
	U_BOOT_CMD_MKENT(tree, 0, 1, do_dm_dump_all, "", ""),
	U_BOOT_CMD_MKENT(uclass, 1, 1, do_dm_dump_uclass, "", ""),
	U_BOOT_CMD_MKENT(test, 1, 1, do_dm_test, "", ""),
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
	"tree         Dump driver model tree\n"
	"dm uclass        Dump list of instances for each uclass\n"
	"dm test         Run tests"
);
