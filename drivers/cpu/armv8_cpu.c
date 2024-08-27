// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 9elements GmbH
 */
#include <cpu.h>
#include <dm.h>
#include <acpi/acpigen.h>
#include <asm/armv8/cpu.h>
#include <dm/acpi.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/printk.h>
#include <linux/sizes.h>

static int armv8_cpu_get_desc(const struct udevice *dev, char *buf, int size)
{
	int cpuid;

	cpuid = (read_midr() & MIDR_PARTNUM_MASK) >> MIDR_PARTNUM_SHIFT;

	snprintf(buf, size, "CPU MIDR %04x", cpuid);

	return 0;
}

static int armv8_cpu_get_info(const struct udevice *dev,
			      struct cpu_info *info)
{
	info->cpu_freq = 0;
	info->features = BIT(CPU_FEAT_L1_CACHE) | BIT(CPU_FEAT_MMU);

	return 0;
}

static int armv8_cpu_get_count(const struct udevice *dev)
{
	return uclass_id_count(UCLASS_CPU);
}

#ifdef CONFIG_ACPIGEN
int armv8_cpu_fill_ssdt(const struct udevice *dev, struct acpi_ctx *ctx)
{
	uint core_id = dev_seq(dev);

	acpigen_write_processor_device(ctx, core_id);

	return 0;
}

struct acpi_ops armv8_cpu_acpi_ops = {
	.fill_ssdt	= armv8_cpu_fill_ssdt,
};
#endif

static const struct cpu_ops cpu_ops = {
	.get_count = armv8_cpu_get_count,
	.get_desc  = armv8_cpu_get_desc,
	.get_info  = armv8_cpu_get_info,
};

static const struct udevice_id cpu_ids[] = {
	{ .compatible = "arm,armv8" },
	{}
};

U_BOOT_DRIVER(arm_cpu) = {
	.name		= "arm-cpu",
	.id		= UCLASS_CPU,
	.of_match	= cpu_ids,
	.ops		= &cpu_ops,
	.flags		= DM_FLAG_PRE_RELOC,
	ACPI_OPS_PTR(&armv8_cpu_acpi_ops)
};
