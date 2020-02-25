// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <asm/cpu_common.h>
#include <asm/cpu_x86.h>

static int apl_get_info(struct udevice *dev, struct cpu_info *info)
{
	return cpu_intel_get_info(info, INTEL_BCLK_MHZ);
}

static const struct cpu_ops cpu_x86_apl_ops = {
	.get_desc	= cpu_x86_get_desc,
	.get_info	= apl_get_info,
	.get_count	= cpu_x86_get_count,
	.get_vendor	= cpu_x86_get_vendor,
};

static const struct udevice_id cpu_x86_apl_ids[] = {
	{ .compatible = "intel,apl-cpu" },
	{ }
};

U_BOOT_DRIVER(cpu_x86_apl_drv) = {
	.name		= "cpu_x86_apl",
	.id		= UCLASS_CPU,
	.of_match	= cpu_x86_apl_ids,
	.bind		= cpu_x86_bind,
	.ops		= &cpu_x86_apl_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};
