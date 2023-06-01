// SPDX-License-Identifier: GPL-2.0+
/*
 * 'cedit' command
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <command.h>
#include <expo.h>
#include <fs.h>
#include <dm/ofnode.h>
#include <linux/sizes.h>

struct expo *cur_exp;

static int do_cedit_load(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	const char *fname;
	struct expo *exp;
	oftree tree;
	ulong size;
	void *buf;
	int ret;

	if (argc < 4)
		return CMD_RET_USAGE;
	fname = argv[3];

	ret = fs_load_alloc(argv[1], argv[2], argv[3], SZ_1M, 0, &buf, &size);
	if (ret) {
		printf("File not found\n");
		return CMD_RET_FAILURE;
	}

	tree = oftree_from_fdt(buf);
	if (!oftree_valid(tree)) {
		printf("Cannot create oftree\n");
		return CMD_RET_FAILURE;
	}

	ret = expo_build(oftree_root(tree), &exp);
	oftree_dispose(tree);
	if (ret) {
		printf("Failed to build expo: %dE\n", ret);
		return CMD_RET_FAILURE;
	}

	cur_exp = exp;

	return 0;
}

static int do_cedit_run(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	ofnode node;
	int ret;

	if (!cur_exp) {
		printf("No expo loaded\n");
		return CMD_RET_FAILURE;
	}

	node = ofnode_path("/cedit-theme");
	if (ofnode_valid(node)) {
		ret = expo_apply_theme(cur_exp, node);
		if (ret)
			return CMD_RET_FAILURE;
	} else {
		log_warning("No theme found\n");
	}
	ret = cedit_run(cur_exp);
	if (ret) {
		log_err("Failed (err=%dE)\n", ret);
		return CMD_RET_FAILURE;
	}

	return 0;
}

#ifdef CONFIG_SYS_LONGHELP
static char cedit_help_text[] =
	"load <interface> <dev[:part]> <filename>   - load config editor\n"
	"cedit run                                        - run config editor";
#endif /* CONFIG_SYS_LONGHELP */

U_BOOT_CMD_WITH_SUBCMDS(cedit, "Configuration editor", cedit_help_text,
	U_BOOT_SUBCMD_MKENT(load, 5, 1, do_cedit_load),
	U_BOOT_SUBCMD_MKENT(run, 1, 1, do_cedit_run),
);
