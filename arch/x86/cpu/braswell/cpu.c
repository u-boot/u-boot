/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Derived from arch/x86/cpu/baytrail/cpu.c
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <asm/cpu.h>
#include <asm/cpu_x86.h>
#include <asm/io.h>
#include <asm/lapic.h>
#include <asm/msr.h>
#include <asm/turbo.h>

static const unsigned int braswell_bus_freq_table[] = {
	83333333,
	100000000,
	133333333,
	116666666,
	80000000,
	93333333,
	90000000,
	88900000,
	87500000
};

static unsigned int braswell_bus_freq(void)
{
	msr_t clk_info = msr_read(MSR_BSEL_CR_OVERCLOCK_CONTROL);

	if ((clk_info.lo & 0xf) < (ARRAY_SIZE(braswell_bus_freq_table)))
		return braswell_bus_freq_table[clk_info.lo & 0xf];

	return 0;
}

static unsigned long braswell_tsc_freq(void)
{
	msr_t platform_info;
	ulong bclk = braswell_bus_freq();

	if (!bclk)
		return 0;

	platform_info = msr_read(MSR_PLATFORM_INFO);

	return bclk * ((platform_info.lo >> 8) & 0xff);
}

static int braswell_get_info(struct udevice *dev, struct cpu_info *info)
{
	info->cpu_freq = braswell_tsc_freq();
	info->features = (1 << CPU_FEAT_L1_CACHE) | (1 << CPU_FEAT_MMU);

	return 0;
}

static int braswell_get_count(struct udevice *dev)
{
	int ecx = 0;

	/*
	 * Use the algorithm described in Intel 64 and IA-32 Architectures
	 * Software Developer's Manual Volume 3 (3A, 3B & 3C): System
	 * Programming Guide, Jan-2015. Section 8.9.2: Hierarchical Mapping
	 * of CPUID Extended Topology Leaf.
	 */
	while (1) {
		struct cpuid_result leaf_b;

		leaf_b = cpuid_ext(0xb, ecx);

		/*
		 * Braswell doesn't have hyperthreading so just determine the
		 * number of cores by from level type (ecx[15:8] == * 2)
		 */
		if ((leaf_b.ecx & 0xff00) == 0x0200)
			return leaf_b.ebx & 0xffff;

		ecx++;
	}

	return 0;
}

static void braswell_set_max_freq(void)
{
	msr_t perf_ctl;
	msr_t msr;

	/* Enable speed step */
	msr = msr_read(MSR_IA32_MISC_ENABLES);
	msr.lo |= (1 << 16);
	msr_write(MSR_IA32_MISC_ENABLES, msr);

	/* Enable Burst Mode */
	msr = msr_read(MSR_IA32_MISC_ENABLES);
	msr.hi = 0;
	msr_write(MSR_IA32_MISC_ENABLES, msr);

	/*
	 * Set guaranteed ratio [21:16] from IACORE_TURBO_RATIOS to
	 * bits [15:8] of the PERF_CTL
	 */
	msr = msr_read(MSR_IACORE_TURBO_RATIOS);
	perf_ctl.lo = (msr.lo & 0x3f0000) >> 8;

	/*
	 * Set guaranteed vid [22:16] from IACORE_TURBO_VIDS to
	 * bits [7:0] of the PERF_CTL
	 */
	msr = msr_read(MSR_IACORE_TURBO_VIDS);
	perf_ctl.lo |= (msr.lo & 0x7f0000) >> 16;

	perf_ctl.hi = 0;
	msr_write(MSR_IA32_PERF_CTL, perf_ctl);
}

static int braswell_probe(struct udevice *dev)
{
	debug("Init Braswell core\n");

	/*
	 * On Braswell the turbo disable bit is actually scoped at the
	 * building-block level, not package. For non-BSP cores that are
	 * within a building block, enable turbo. The cores within the BSP's
	 * building block will just see it already enabled and move on.
	 */
	if (lapicid())
		turbo_enable();

	/* Dynamic L2 shrink enable and threshold, clear SINGLE_PCTL bit 11 */
	msr_clrsetbits_64(MSR_PMG_CST_CONFIG_CONTROL, 0x3f080f, 0xe0008),
	msr_clrsetbits_64(MSR_POWER_MISC,
			  ENABLE_ULFM_AUTOCM_MASK | ENABLE_INDP_AUTOCM_MASK, 0);

	/* Disable C1E */
	msr_clrsetbits_64(MSR_POWER_CTL, 2, 0);
	msr_setbits_64(MSR_POWER_MISC, 0x44);

	/* Set this core to max frequency ratio */
	braswell_set_max_freq();

	return 0;
}

static const struct udevice_id braswell_ids[] = {
	{ .compatible = "intel,braswell-cpu" },
	{ }
};

static const struct cpu_ops braswell_ops = {
	.get_desc	= cpu_x86_get_desc,
	.get_info	= braswell_get_info,
	.get_count	= braswell_get_count,
	.get_vendor	= cpu_x86_get_vendor,
};

U_BOOT_DRIVER(cpu_x86_braswell_drv) = {
	.name		= "cpu_x86_braswell",
	.id		= UCLASS_CPU,
	.of_match	= braswell_ids,
	.bind		= cpu_x86_bind,
	.probe		= braswell_probe,
	.ops		= &braswell_ops,
};
