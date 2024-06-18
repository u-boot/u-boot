// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <clk.h>
#include <cpu.h>
#include <dm.h>
#include <errno.h>
#include <linux/bitops.h>
#include <linux/err.h>

#include <asm/arch/core.h>

static int xtensa_cpu_get_desc(const struct udevice *dev, char *buf, int size)
{
	const char *cpu = XCHAL_CORE_ID;

	if (!cpu || size < (strlen(cpu) + 1))
		return -ENOSPC;

	strcpy(buf, cpu);

	return 0;
}

static int xtensa_cpu_get_info(const struct udevice *dev, struct cpu_info *info)
{
	struct cpu_plat *plat = dev_get_parent_plat(dev);

	info->cpu_freq = plat->timebase_freq;

#if XCHAL_HAVE_PTP_MMU
		info->features |= BIT(CPU_FEAT_MMU);
#endif
#if XCHAL_ICACHE_SIZE || XCHAL_DCACHE_SIZE
		info->features |= BIT(CPU_FEAT_L1_CACHE);
#endif

	return 0;
}

static int xtensa_cpu_get_count(const struct udevice *dev)
{
	ofnode node;
	int num = 0;

	ofnode_for_each_subnode(node, dev_ofnode(dev->parent)) {
		const char *device_type;

		/* skip if hart is marked as not available in the device tree */
		if (!ofnode_is_enabled(node))
			continue;

		device_type = ofnode_read_string(node, "device_type");
		if (!device_type)
			continue;
		if (strcmp(device_type, "cpu") == 0)
			num++;
	}

	return num;
}

static int xtensa_cpu_bind(struct udevice *dev)
{
	struct cpu_plat *plat = dev_get_parent_plat(dev);

	plat->cpu_id = dev_read_addr(dev);

	return 0;
}

static int xtensa_cpu_probe(struct udevice *dev)
{
	int ret = 0;
	struct clk clk;
	struct cpu_plat *plat = dev_get_parent_plat(dev);

	asm volatile ("rsr %0, 176\n"
		      "rsr %1, 208\n"
		      : "=r"(plat->id[0]), "=r"(plat->id[1]));

	/* Get a clock if it exists */
	ret = clk_get_by_index(dev, 0, &clk);
	if (!ret) {
		ret = clk_enable(&clk);
		if (ret && (ret != -ENOSYS || ret != -ENOTSUPP))
			return ret;
		ret = clk_get_rate(&clk);
		if (!IS_ERR_VALUE(ret))
			plat->timebase_freq = ret;
	}

	return 0;
}

static const struct cpu_ops xtensa_cpu_ops = {
	.get_desc	= xtensa_cpu_get_desc,
	.get_info	= xtensa_cpu_get_info,
	.get_count	= xtensa_cpu_get_count,
};

static const struct udevice_id xtensa_cpu_ids[] = {
	{ .compatible = "cdns,xtensa-cpu" },
	{ }
};

U_BOOT_DRIVER(xtensa_cpu) = {
	.name = "xtensa_cpu",
	.id = UCLASS_CPU,
	.of_match = xtensa_cpu_ids,
	.bind = xtensa_cpu_bind,
	.probe = xtensa_cpu_probe,
	.ops = &xtensa_cpu_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
