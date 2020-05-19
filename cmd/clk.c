// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Xilinx, Inc.
 */
#include <common.h>
#include <command.h>
#include <clk.h>
#if defined(CONFIG_DM) && defined(CONFIG_CLK)
#include <dm.h>
#include <dm/device.h>
#include <dm/root.h>
#include <dm/device-internal.h>
#include <linux/clk-provider.h>
#endif

#if defined(CONFIG_DM) && defined(CONFIG_CLK)
static void show_clks(struct udevice *dev, int depth, int last_flag)
{
	int i, is_last;
	struct udevice *child;
	struct clk *clkp;
	u32 rate;

	clkp = dev_get_clk_ptr(dev);
	if (device_get_uclass_id(dev) == UCLASS_CLK && clkp) {
		rate = clk_get_rate(clkp);

	printf(" %-12u  %8d        ", rate, clkp->enable_count);

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
	}

	list_for_each_entry(child, &dev->child_head, sibling_node) {
		is_last = list_is_last(&child->sibling_node, &dev->child_head);
		show_clks(child, depth + 1, (last_flag << 1) | is_last);
	}
}

int __weak soc_clk_dump(void)
{
	struct udevice *root;

	root = dm_root();
	if (root) {
		printf(" Rate               Usecnt      Name\n");
		printf("------------------------------------------\n");
		show_clks(root, -1, 0);
	}

	return 0;
}
#else
int __weak soc_clk_dump(void)
{
	puts("Not implemented\n");
	return 1;
}
#endif

static int do_clk_dump(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	int ret;

	ret = soc_clk_dump();
	if (ret < 0) {
		printf("Clock dump error %d\n", ret);
		ret = CMD_RET_FAILURE;
	}

	return ret;
}

static struct cmd_tbl cmd_clk_sub[] = {
	U_BOOT_CMD_MKENT(dump, 1, 1, do_clk_dump, "", ""),
};

static int do_clk(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
	struct cmd_tbl *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* Strip off leading 'clk' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_clk_sub[0], ARRAY_SIZE(cmd_clk_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

#ifdef CONFIG_SYS_LONGHELP
static char clk_help_text[] =
	"dump - Print clock frequencies";
#endif

U_BOOT_CMD(clk, 2, 1, do_clk, "CLK sub-system", clk_help_text);
