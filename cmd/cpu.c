// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 * Copyright (c) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 * Copyright 2024 NXP
 */

#include <command.h>
#include <cpu.h>
#include <display_options.h>
#include <dm.h>
#include <errno.h>

static const char *cpu_feature_name[CPU_FEAT_COUNT] = {
	"L1 cache",
	"MMU",
	"Microcode",
	"Device ID",
};

static struct udevice *cpu_find_device(unsigned long cpu_id)
{
	struct udevice *dev;

	for (uclass_first_device(UCLASS_CPU, &dev); dev;
	     uclass_next_device(&dev)) {
		if (cpu_id == dev_seq(dev))
			return dev;
	}

	return NULL;
}

static int print_cpu_list(bool detail)
{
	struct udevice *dev;
	char buf[100];

	for (uclass_first_device(UCLASS_CPU, &dev);
		     dev;
		     uclass_next_device(&dev)) {
		struct cpu_plat *plat = dev_get_parent_plat(dev);
		struct cpu_info info;
		bool first = true;
		int ret, i;

		ret = cpu_get_desc(dev, buf, sizeof(buf));
		printf("%3d: %-10s %s\n", dev_seq(dev), dev->name,
		       ret ? "<no description>" : buf);
		if (!detail)
			continue;
		ret = cpu_get_info(dev, &info);
		if (ret) {
			printf("\t(no detail available");
			if (ret != -ENOSYS)
				printf(": err=%d", ret);
			printf(")\n");
			continue;
		}
		printf("\tID = %d, freq = ", plat->cpu_id);
		print_freq(info.cpu_freq, "");
		for (i = 0; i < CPU_FEAT_COUNT; i++) {
			if (info.features & (1 << i)) {
				printf("%s%s", first ? ": " : ", ",
				       cpu_feature_name[i]);
				first = false;
			}
		}
		printf("\n");
		if (info.features & (1 << CPU_FEAT_UCODE))
			printf("\tMicrocode version %#x\n",
			       plat->ucode_version);
		if (info.features & (1 << CPU_FEAT_DEVICE_ID))
			printf("\tDevice ID %#lx\n", plat->device_id);
	}

	return 0;
}

static int do_cpu_list(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	if (print_cpu_list(false))
		return CMD_RET_FAILURE;

	return 0;
}

static int do_cpu_detail(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	if (print_cpu_list(true))
		return CMD_RET_FAILURE;

	return 0;
}

static int do_cpu_release(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	struct udevice *dev;
	unsigned long cpu_id;
	unsigned long long boot_addr;

	if (argc != 3)
		return CMD_RET_USAGE;

	cpu_id = dectoul(argv[1], NULL);
	dev = cpu_find_device(cpu_id);
	if (!dev)
		return CMD_RET_FAILURE;

	boot_addr = simple_strtoull(argv[2], NULL, 16);

	if (cpu_release_core(dev, boot_addr))
		return CMD_RET_FAILURE;

	return 0;
}

U_BOOT_LONGHELP(cpu,
	"list	- list available CPUs\n"
	"cpu detail	- show CPU detail\n"
	"cpu release <core ID> <addr>	- Release CPU <core ID> at <addr>\n"
	"            <core ID>: the sequence number in list subcommand outputs");

U_BOOT_CMD_WITH_SUBCMDS(cpu, "display information about CPUs", cpu_help_text,
	U_BOOT_SUBCMD_MKENT(list, 1, 1, do_cpu_list),
	U_BOOT_SUBCMD_MKENT(detail, 1, 0, do_cpu_detail),
	U_BOOT_SUBCMD_MKENT(release, 3, 0, do_cpu_release));
