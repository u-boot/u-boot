// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <clk.h>
#include <cpu.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <asm/loongarch.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <linux/bitops.h>
#include <linux/err.h>

static int loongarch_cpu_get_desc(const struct udevice *dev, char *buf, int size)
{
	const char *cpu;

	/* We try to get CPU name from IOCSR first */
	if ((read_cpucfg(LOONGARCH_CPUCFG1) & CPUCFG1_IOCSR)) {
		int i;
		char vendor_buf[9] = { 0 };
		char name_buf[9] = { 0 };
		u64 vendor = iocsr_read64(LOONGARCH_IOCSR_VENDOR);
		u64 name = iocsr_read64(LOONGARCH_IOCSR_CPUNAME);

		if (!vendor || !name)
			goto get_desc_dt;

		for (i = 0; i < sizeof(u64); i++) {
			vendor_buf[i] = vendor & 0xff;
			name_buf[i] = name & 0xff;
			vendor >>= 8;
			name >>= 8;
		}

		snprintf(buf, size, "%s-%s", vendor_buf, name_buf);

		return 0;
	}

get_desc_dt:
	cpu = dev_read_string(dev, "compatible");
	if (!cpu || size < (strlen(cpu) + 1))
		return -ENOSPC;

	strcpy(buf, cpu);

	return 0;
}

static int loongarch_cpu_get_info(const struct udevice *dev, struct cpu_info *info)
{
	int ret;
	struct clk clk;

	/* First try getting the frequency from the assigned clock */
	ret = clk_get_by_index((struct udevice *)dev, 0, &clk);
	if (!ret) {
		ret = clk_get_rate(&clk);
		if (!IS_ERR_VALUE(ret))
			info->cpu_freq = ret;
	}

	if (!info->cpu_freq)
		dev_read_u32(dev, "clock-frequency", (u32 *)&info->cpu_freq);

	if (read_cpucfg(LOONGARCH_CPUCFG1) & CPUCFG1_PAGING)
		info->features |= BIT(CPU_FEAT_MMU);

	if (read_cpucfg(LOONGARCH_CPUCFG16) & CPUCFG16_L1_IUPRE)
		info->features |= BIT(CPU_FEAT_L1_CACHE);

	return 0;
}

static int loongarch_cpu_get_count(const struct udevice *dev)
{
	ofnode node;
	int num = 0;

	ofnode_for_each_subnode(node, dev_ofnode(dev->parent)) {
		const char *device_type;

		/* skip if core is marked as not available in the device tree */
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

static int loongarch_cpu_bind(struct udevice *dev)
{
	struct cpu_plat *plat = dev_get_parent_plat(dev);

	plat->cpu_id = dev_read_addr(dev);

	return 0;
}

static int loongarch_cpu_probe(struct udevice *dev)
{
	int ret = 0;
	struct clk clk;

	/* Get a clock if it exists */
	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return 0;

	ret = clk_enable(&clk);
	if (ret == -ENOSYS)
		return 0;
	else
		return ret;
}

static const struct cpu_ops loongarch_cpu_ops = {
	.get_desc	= loongarch_cpu_get_desc,
	.get_info	= loongarch_cpu_get_info,
	.get_count	= loongarch_cpu_get_count,
};

static const struct udevice_id loongarch_cpu_ids[] = {
	{ .compatible = "loongarch,Loongson-3A5000" }, /* From QEMU, undocumented */
	{ .compatible = "loongson,la264" },
	{ .compatible = "loongson,la364" },
	{ }
};

U_BOOT_DRIVER(loongarch_cpu) = {
	.name = "loongarch_cpu",
	.id = UCLASS_CPU,
	.of_match = loongarch_cpu_ids,
	.bind = loongarch_cpu_bind,
	.probe = loongarch_cpu_probe,
	.ops = &loongarch_cpu_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
