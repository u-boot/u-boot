/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <errno.h>
#include <asm/cpu.h>

DECLARE_GLOBAL_DATA_PTR;

int cpu_x86_bind(struct udevice *dev)
{
	struct cpu_platdata *plat = dev_get_parent_platdata(dev);

	plat->cpu_id = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
				      "intel,apic-id", -1);

	return 0;
}

int cpu_x86_get_desc(struct udevice *dev, char *buf, int size)
{
	if (size < CPU_MAX_NAME_LEN)
		return -ENOSPC;

	cpu_get_name(buf);

	return 0;
}

static int cpu_x86_get_count(struct udevice *dev)
{
	int node, cpu;
	int num = 0;

	node = fdt_path_offset(gd->fdt_blob, "/cpus");
	if (node < 0)
		return -ENOENT;

	for (cpu = fdt_first_subnode(gd->fdt_blob, node);
	     cpu >= 0;
	     cpu = fdt_next_subnode(gd->fdt_blob, cpu)) {
		const char *device_type;

		device_type = fdt_getprop(gd->fdt_blob, cpu,
					  "device_type", NULL);
		if (!device_type)
			continue;
		if (strcmp(device_type, "cpu") == 0)
			num++;
	}

	return num;
}

static const struct cpu_ops cpu_x86_ops = {
	.get_desc	= cpu_x86_get_desc,
	.get_count	= cpu_x86_get_count,
};

static const struct udevice_id cpu_x86_ids[] = {
	{ .compatible = "cpu-x86" },
	{ }
};

U_BOOT_DRIVER(cpu_x86_drv) = {
	.name		= "cpu_x86",
	.id		= UCLASS_CPU,
	.of_match	= cpu_x86_ids,
	.bind		= cpu_x86_bind,
	.ops		= &cpu_x86_ops,
};
