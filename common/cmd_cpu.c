/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <cpu.h>
#include <dm.h>

static const char *cpu_feature_name[CPU_FEAT_COUNT] = {
	"L1 cache",
	"MMU",
};

static int print_cpu_list(bool detail)
{
	struct udevice *dev;
	struct uclass *uc;
	char buf[100];
	int ret;

	ret = uclass_get(UCLASS_CPU, &uc);
	if (ret) {
		printf("Cannot find CPU uclass\n");
		return ret;
	}
	uclass_foreach_dev(dev, uc) {
		struct cpu_platdata *plat = dev_get_parent_platdata(dev);
		struct cpu_info info;
		bool first;
		int i;

		ret = cpu_get_desc(dev, buf, sizeof(buf));
		printf("%3d: %-10s %s\n", dev->seq, dev->name,
		       ret ? "<no description>" : buf);
		if (!detail)
			continue;
		ret = cpu_get_info(dev, &info);
		if (ret) {
			printf("\t(no detail available");
			if (ret != -ENOSYS)
				printf(": err=%d\n", ret);
			printf(")\n");
			continue;
		}
		printf("\tID = %d, freq = ", plat->cpu_id);
		print_freq(info.cpu_freq, "");
		first = true;
		for (i = 0; i < CPU_FEAT_COUNT; i++) {
			if (info.features & (1 << i)) {
				printf("%s%s", first ? ": " : ", ",
				       cpu_feature_name[i]);
				first = false;
			}
		}
		printf("\n");
	}

	return 0;
}

static int do_cpu_list(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	if (print_cpu_list(false))
		return CMD_RET_FAILURE;

	return 0;
}

static int do_cpu_detail(cmd_tbl_t *cmdtp, int flag, int argc,
			 char *const argv[])
{
	if (print_cpu_list(true))
		return CMD_RET_FAILURE;

	return 0;
}

static cmd_tbl_t cmd_cpu_sub[] = {
	U_BOOT_CMD_MKENT(list, 2, 1, do_cpu_list, "", ""),
	U_BOOT_CMD_MKENT(detail, 4, 0, do_cpu_detail, "", ""),
};

/*
 * Process a cpu sub-command
 */
static int do_cpu(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	cmd_tbl_t *c = NULL;

	/* Strip off leading 'cpu' command argument */
	argc--;
	argv++;

	if (argc)
		c = find_cmd_tbl(argv[0], cmd_cpu_sub, ARRAY_SIZE(cmd_cpu_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

U_BOOT_CMD(
	cpu, 2, 1, do_cpu,
	"display information about CPUs",
	"list	- list available CPUs\n"
	"cpu detail	- show CPU detail"
);
