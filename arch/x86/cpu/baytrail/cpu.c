/*
 * Copyright (C) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Based on code from coreboot
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <asm/cpu.h>
#include <asm/lapic.h>
#include <asm/mp.h>
#include <asm/msr.h>
#include <asm/turbo.h>

#ifdef CONFIG_SMP
static int enable_smis(struct udevice *cpu, void *unused)
{
	return 0;
}

static struct mp_flight_record mp_steps[] = {
	MP_FR_BLOCK_APS(mp_init_cpu, NULL, mp_init_cpu, NULL),
	/* Wait for APs to finish initialization before proceeding. */
	MP_FR_BLOCK_APS(NULL, NULL, enable_smis, NULL),
};

static int detect_num_cpus(void)
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
		 * Bay Trail doesn't have hyperthreading so just determine the
		 * number of cores by from level type (ecx[15:8] == * 2)
		 */
		if ((leaf_b.ecx & 0xff00) == 0x0200)
			return leaf_b.ebx & 0xffff;
		ecx++;
	}
}

static int baytrail_init_cpus(void)
{
	struct mp_params mp_params;

	lapic_setup();

	mp_params.num_cpus = detect_num_cpus();
	mp_params.parallel_microcode_load = 0,
	mp_params.flight_plan = &mp_steps[0];
	mp_params.num_records = ARRAY_SIZE(mp_steps);
	mp_params.microcode_pointer = 0;

	if (mp_init(&mp_params)) {
		printf("Warning: MP init failure\n");
		return -EIO;
	}

	return 0;
}
#endif

int x86_init_cpus(void)
{
#ifdef CONFIG_SMP
	debug("Init additional CPUs\n");
	baytrail_init_cpus();
#endif

	return 0;
}

static void set_max_freq(void)
{
	msr_t perf_ctl;
	msr_t msr;

	/* Enable speed step */
	msr = msr_read(MSR_IA32_MISC_ENABLES);
	msr.lo |= (1 << 16);
	msr_write(MSR_IA32_MISC_ENABLES, msr);

	/*
	 * Set guaranteed ratio [21:16] from IACORE_RATIOS to bits [15:8] of
	 * the PERF_CTL
	 */
	msr = msr_read(MSR_IACORE_RATIOS);
	perf_ctl.lo = (msr.lo & 0x3f0000) >> 8;

	/*
	 * Set guaranteed vid [21:16] from IACORE_VIDS to bits [7:0] of
	 * the PERF_CTL
	 */
	msr = msr_read(MSR_IACORE_VIDS);
	perf_ctl.lo |= (msr.lo & 0x7f0000) >> 16;
	perf_ctl.hi = 0;

	msr_write(MSR_IA32_PERF_CTL, perf_ctl);
}

static int cpu_x86_baytrail_probe(struct udevice *dev)
{
	debug("Init BayTrail core\n");

	/*
	 * On BayTrail the turbo disable bit is actually scoped at the
	 * building-block level, not package. For non-BSP cores that are
	 * within a building block, enable turbo. The cores within the BSP's
	 * building block will just see it already enabled and move on.
	 */
	if (lapicid())
		turbo_enable();

	/* Dynamic L2 shrink enable and threshold */
	msr_clrsetbits_64(MSR_PMG_CST_CONFIG_CONTROL, 0x3f000f, 0xe0008),

	/* Disable C1E */
	msr_clrsetbits_64(MSR_POWER_CTL, 2, 0);
	msr_setbits_64(MSR_POWER_MISC, 0x44);

	/* Set this core to max frequency ratio */
	set_max_freq();

	return 0;
}

static unsigned bus_freq(void)
{
	msr_t clk_info = msr_read(MSR_BSEL_CR_OVERCLOCK_CONTROL);
	switch (clk_info.lo & 0x3) {
	case 0:
		return 83333333;
	case 1:
		return 100000000;
	case 2:
		return 133333333;
	case 3:
		return 116666666;
	default:
		return 0;
	}
}

static unsigned long tsc_freq(void)
{
	msr_t platform_info;
	ulong bclk = bus_freq();

	if (!bclk)
		return 0;

	platform_info = msr_read(MSR_PLATFORM_INFO);

	return bclk * ((platform_info.lo >> 8) & 0xff);
}

static int baytrail_get_info(struct udevice *dev, struct cpu_info *info)
{
	info->cpu_freq = tsc_freq();
	info->features = 1 << CPU_FEAT_L1_CACHE | 1 << CPU_FEAT_MMU;

	return 0;
}

static int cpu_x86_baytrail_bind(struct udevice *dev)
{
	struct cpu_platdata *plat = dev_get_parent_platdata(dev);

	plat->cpu_id = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
				      "intel,apic-id", -1);

	return 0;
}

static const struct cpu_ops cpu_x86_baytrail_ops = {
	.get_desc	= x86_cpu_get_desc,
	.get_info	= baytrail_get_info,
};

static const struct udevice_id cpu_x86_baytrail_ids[] = {
	{ .compatible = "intel,baytrail-cpu" },
	{ }
};

U_BOOT_DRIVER(cpu_x86_baytrail_drv) = {
	.name		= "cpu_x86_baytrail",
	.id		= UCLASS_CPU,
	.of_match	= cpu_x86_baytrail_ids,
	.bind		= cpu_x86_baytrail_bind,
	.probe		= cpu_x86_baytrail_probe,
	.ops		= &cpu_x86_baytrail_ops,
};
