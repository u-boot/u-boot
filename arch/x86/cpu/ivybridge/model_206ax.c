/*
 * From Coreboot file of same name
 *
 * Copyright (C) 2007-2009 coresystems GmbH
 * Copyright (C) 2011 The Chromium Authors
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/acpi.h>
#include <asm/cpu.h>
#include <asm/lapic.h>
#include <asm/lapic_def.h>
#include <asm/msr.h>
#include <asm/mtrr.h>
#include <asm/processor.h>
#include <asm/speedstep.h>
#include <asm/turbo.h>
#include <asm/arch/model_206ax.h>

static void enable_vmx(void)
{
	struct cpuid_result regs;
#ifdef CONFIG_ENABLE_VMX
	int enable = true;
#else
	int enable = false;
#endif
	msr_t msr;

	regs = cpuid(1);
	/* Check that the VMX is supported before reading or writing the MSR. */
	if (!((regs.ecx & CPUID_VMX) || (regs.ecx & CPUID_SMX)))
		return;

	msr = msr_read(MSR_IA32_FEATURE_CONTROL);

	if (msr.lo & (1 << 0)) {
		debug("VMX is locked, so %s will do nothing\n", __func__);
		/* VMX locked. If we set it again we get an illegal
		 * instruction
		 */
		return;
	}

	/* The IA32_FEATURE_CONTROL MSR may initialize with random values.
	 * It must be cleared regardless of VMX config setting.
	 */
	msr.hi = 0;
	msr.lo = 0;

	debug("%s VMX\n", enable ? "Enabling" : "Disabling");

	/*
	 * Even though the Intel manual says you must set the lock bit in
	 * addition to the VMX bit in order for VMX to work, it is incorrect.
	 * Thus we leave it unlocked for the OS to manage things itself.
	 * This is good for a few reasons:
	 * - No need to reflash the bios just to toggle the lock bit.
	 * - The VMX bits really really should match each other across cores,
	 *   so hard locking it on one while another has the opposite setting
	 *   can easily lead to crashes as code using VMX migrates between
	 *   them.
	 * - Vendors that want to "upsell" from a bios that disables+locks to
	 *   one that doesn't is sleazy.
	 * By leaving this to the OS (e.g. Linux), people can do exactly what
	 * they want on the fly, and do it correctly (e.g. across multiple
	 * cores).
	 */
	if (enable) {
		msr.lo |= (1 << 2);
		if (regs.ecx & CPUID_SMX)
			msr.lo |= (1 << 1);
	}

	msr_write(MSR_IA32_FEATURE_CONTROL, msr);
}

/* Convert time in seconds to POWER_LIMIT_1_TIME MSR value */
static const u8 power_limit_time_sec_to_msr[] = {
	[0]   = 0x00,
	[1]   = 0x0a,
	[2]   = 0x0b,
	[3]   = 0x4b,
	[4]   = 0x0c,
	[5]   = 0x2c,
	[6]   = 0x4c,
	[7]   = 0x6c,
	[8]   = 0x0d,
	[10]  = 0x2d,
	[12]  = 0x4d,
	[14]  = 0x6d,
	[16]  = 0x0e,
	[20]  = 0x2e,
	[24]  = 0x4e,
	[28]  = 0x6e,
	[32]  = 0x0f,
	[40]  = 0x2f,
	[48]  = 0x4f,
	[56]  = 0x6f,
	[64]  = 0x10,
	[80]  = 0x30,
	[96]  = 0x50,
	[112] = 0x70,
	[128] = 0x11,
};

/* Convert POWER_LIMIT_1_TIME MSR value to seconds */
static const u8 power_limit_time_msr_to_sec[] = {
	[0x00] = 0,
	[0x0a] = 1,
	[0x0b] = 2,
	[0x4b] = 3,
	[0x0c] = 4,
	[0x2c] = 5,
	[0x4c] = 6,
	[0x6c] = 7,
	[0x0d] = 8,
	[0x2d] = 10,
	[0x4d] = 12,
	[0x6d] = 14,
	[0x0e] = 16,
	[0x2e] = 20,
	[0x4e] = 24,
	[0x6e] = 28,
	[0x0f] = 32,
	[0x2f] = 40,
	[0x4f] = 48,
	[0x6f] = 56,
	[0x10] = 64,
	[0x30] = 80,
	[0x50] = 96,
	[0x70] = 112,
	[0x11] = 128,
};

int cpu_config_tdp_levels(void)
{
	struct cpuid_result result;
	msr_t platform_info;

	/* Minimum CPU revision */
	result = cpuid(1);
	if (result.eax < IVB_CONFIG_TDP_MIN_CPUID)
		return 0;

	/* Bits 34:33 indicate how many levels supported */
	platform_info = msr_read(MSR_PLATFORM_INFO);
	return (platform_info.hi >> 1) & 3;
}

/*
 * Configure processor power limits if possible
 * This must be done AFTER set of BIOS_RESET_CPL
 */
void set_power_limits(u8 power_limit_1_time)
{
	msr_t msr = msr_read(MSR_PLATFORM_INFO);
	msr_t limit;
	unsigned power_unit;
	unsigned tdp, min_power, max_power, max_time;
	u8 power_limit_1_val;

	if (power_limit_1_time > ARRAY_SIZE(power_limit_time_sec_to_msr))
		return;

	if (!(msr.lo & PLATFORM_INFO_SET_TDP))
		return;

	/* Get units */
	msr = msr_read(MSR_PKG_POWER_SKU_UNIT);
	power_unit = 2 << ((msr.lo & 0xf) - 1);

	/* Get power defaults for this SKU */
	msr = msr_read(MSR_PKG_POWER_SKU);
	tdp = msr.lo & 0x7fff;
	min_power = (msr.lo >> 16) & 0x7fff;
	max_power = msr.hi & 0x7fff;
	max_time = (msr.hi >> 16) & 0x7f;

	debug("CPU TDP: %u Watts\n", tdp / power_unit);

	if (power_limit_time_msr_to_sec[max_time] > power_limit_1_time)
		power_limit_1_time = power_limit_time_msr_to_sec[max_time];

	if (min_power > 0 && tdp < min_power)
		tdp = min_power;

	if (max_power > 0 && tdp > max_power)
		tdp = max_power;

	power_limit_1_val = power_limit_time_sec_to_msr[power_limit_1_time];

	/* Set long term power limit to TDP */
	limit.lo = 0;
	limit.lo |= tdp & PKG_POWER_LIMIT_MASK;
	limit.lo |= PKG_POWER_LIMIT_EN;
	limit.lo |= (power_limit_1_val & PKG_POWER_LIMIT_TIME_MASK) <<
		PKG_POWER_LIMIT_TIME_SHIFT;

	/* Set short term power limit to 1.25 * TDP */
	limit.hi = 0;
	limit.hi |= ((tdp * 125) / 100) & PKG_POWER_LIMIT_MASK;
	limit.hi |= PKG_POWER_LIMIT_EN;
	/* Power limit 2 time is only programmable on SNB EP/EX */

	msr_write(MSR_PKG_POWER_LIMIT, limit);

	/* Use nominal TDP values for CPUs with configurable TDP */
	if (cpu_config_tdp_levels()) {
		msr = msr_read(MSR_CONFIG_TDP_NOMINAL);
		limit.hi = 0;
		limit.lo = msr.lo & 0xff;
		msr_write(MSR_TURBO_ACTIVATION_RATIO, limit);
	}
}

static void configure_c_states(void)
{
	struct cpuid_result result;
	msr_t msr;

	msr = msr_read(MSR_PMG_CST_CONFIG_CTL);
	msr.lo |= (1 << 28);	/* C1 Auto Undemotion Enable */
	msr.lo |= (1 << 27);	/* C3 Auto Undemotion Enable */
	msr.lo |= (1 << 26);	/* C1 Auto Demotion Enable */
	msr.lo |= (1 << 25);	/* C3 Auto Demotion Enable */
	msr.lo &= ~(1 << 10);	/* Disable IO MWAIT redirection */
	msr.lo |= 7;		/* No package C-state limit */
	msr_write(MSR_PMG_CST_CONFIG_CTL, msr);

	msr = msr_read(MSR_PMG_IO_CAPTURE_ADR);
	msr.lo &= ~0x7ffff;
	msr.lo |= (PMB0_BASE + 4);	/* LVL_2 base address */
	msr.lo |= (2 << 16);		/* CST Range: C7 is max C-state */
	msr_write(MSR_PMG_IO_CAPTURE_ADR, msr);

	msr = msr_read(MSR_MISC_PWR_MGMT);
	msr.lo &= ~(1 << 0);	/* Enable P-state HW_ALL coordination */
	msr_write(MSR_MISC_PWR_MGMT, msr);

	msr = msr_read(MSR_POWER_CTL);
	msr.lo |= (1 << 18);	/* Enable Energy Perf Bias MSR 0x1b0 */
	msr.lo |= (1 << 1);	/* C1E Enable */
	msr.lo |= (1 << 0);	/* Bi-directional PROCHOT# */
	msr_write(MSR_POWER_CTL, msr);

	/* C3 Interrupt Response Time Limit */
	msr.hi = 0;
	msr.lo = IRTL_VALID | IRTL_1024_NS | 0x50;
	msr_write(MSR_PKGC3_IRTL, msr);

	/* C6 Interrupt Response Time Limit */
	msr.hi = 0;
	msr.lo = IRTL_VALID | IRTL_1024_NS | 0x68;
	msr_write(MSR_PKGC6_IRTL, msr);

	/* C7 Interrupt Response Time Limit */
	msr.hi = 0;
	msr.lo = IRTL_VALID | IRTL_1024_NS | 0x6D;
	msr_write(MSR_PKGC7_IRTL, msr);

	/* Primary Plane Current Limit */
	msr = msr_read(MSR_PP0_CURRENT_CONFIG);
	msr.lo &= ~0x1fff;
	msr.lo |= PP0_CURRENT_LIMIT;
	msr_write(MSR_PP0_CURRENT_CONFIG, msr);

	/* Secondary Plane Current Limit */
	msr = msr_read(MSR_PP1_CURRENT_CONFIG);
	msr.lo &= ~0x1fff;
	result = cpuid(1);
	if (result.eax >= 0x30600)
		msr.lo |= PP1_CURRENT_LIMIT_IVB;
	else
		msr.lo |= PP1_CURRENT_LIMIT_SNB;
	msr_write(MSR_PP1_CURRENT_CONFIG, msr);
}

static int configure_thermal_target(void)
{
	int tcc_offset;
	msr_t msr;
	int node;

	/* Find pointer to CPU configuration */
	node = fdtdec_next_compatible(gd->fdt_blob, 0,
				      COMPAT_INTEL_MODEL_206AX);
	if (node < 0)
		return -ENOENT;
	tcc_offset = fdtdec_get_int(gd->fdt_blob, node, "tcc-offset", 0);

	/* Set TCC activaiton offset if supported */
	msr = msr_read(MSR_PLATFORM_INFO);
	if ((msr.lo & (1 << 30)) && tcc_offset) {
		msr = msr_read(MSR_TEMPERATURE_TARGET);
		msr.lo &= ~(0xf << 24); /* Bits 27:24 */
		msr.lo |= (tcc_offset & 0xf) << 24;
		msr_write(MSR_TEMPERATURE_TARGET, msr);
	}

	return 0;
}

static void configure_misc(void)
{
	msr_t msr;

	msr = msr_read(IA32_MISC_ENABLE);
	msr.lo |= (1 << 0);	  /* Fast String enable */
	msr.lo |= (1 << 3);	  /* TM1/TM2/EMTTM enable */
	msr.lo |= (1 << 16);	  /* Enhanced SpeedStep Enable */
	msr_write(IA32_MISC_ENABLE, msr);

	/* Disable Thermal interrupts */
	msr.lo = 0;
	msr.hi = 0;
	msr_write(IA32_THERM_INTERRUPT, msr);

	/* Enable package critical interrupt only */
	msr.lo = 1 << 4;
	msr.hi = 0;
	msr_write(IA32_PACKAGE_THERM_INTERRUPT, msr);
}

static void enable_lapic_tpr(void)
{
	msr_t msr;

	msr = msr_read(MSR_PIC_MSG_CONTROL);
	msr.lo &= ~(1 << 10);	/* Enable APIC TPR updates */
	msr_write(MSR_PIC_MSG_CONTROL, msr);
}

static void configure_dca_cap(void)
{
	struct cpuid_result cpuid_regs;
	msr_t msr;

	/* Check feature flag in CPUID.(EAX=1):ECX[18]==1 */
	cpuid_regs = cpuid(1);
	if (cpuid_regs.ecx & (1 << 18)) {
		msr = msr_read(IA32_PLATFORM_DCA_CAP);
		msr.lo |= 1;
		msr_write(IA32_PLATFORM_DCA_CAP, msr);
	}
}

static void set_max_ratio(void)
{
	msr_t msr, perf_ctl;

	perf_ctl.hi = 0;

	/* Check for configurable TDP option */
	if (cpu_config_tdp_levels()) {
		/* Set to nominal TDP ratio */
		msr = msr_read(MSR_CONFIG_TDP_NOMINAL);
		perf_ctl.lo = (msr.lo & 0xff) << 8;
	} else {
		/* Platform Info bits 15:8 give max ratio */
		msr = msr_read(MSR_PLATFORM_INFO);
		perf_ctl.lo = msr.lo & 0xff00;
	}
	msr_write(IA32_PERF_CTL, perf_ctl);

	debug("model_x06ax: frequency set to %d\n",
	      ((perf_ctl.lo >> 8) & 0xff) * SANDYBRIDGE_BCLK);
}

static void set_energy_perf_bias(u8 policy)
{
	msr_t msr;

	/* Energy Policy is bits 3:0 */
	msr = msr_read(IA32_ENERGY_PERFORMANCE_BIAS);
	msr.lo &= ~0xf;
	msr.lo |= policy & 0xf;
	msr_write(IA32_ENERGY_PERFORMANCE_BIAS, msr);

	debug("model_x06ax: energy policy set to %u\n", policy);
}

static void configure_mca(void)
{
	msr_t msr;
	int i;

	msr.lo = 0;
	msr.hi = 0;
	/* This should only be done on a cold boot */
	for (i = 0; i < 7; i++)
		msr_write(IA32_MC0_STATUS + (i * 4), msr);
}

#if CONFIG_USBDEBUG
static unsigned ehci_debug_addr;
#endif

/*
 * Initialize any extra cores/threads in this package.
 */
static int intel_cores_init(struct x86_cpu_priv *cpu)
{
	struct cpuid_result result;
	unsigned threads_per_package, threads_per_core, i;

	/* Logical processors (threads) per core */
	result = cpuid_ext(0xb, 0);
	threads_per_core = result.ebx & 0xffff;

	/* Logical processors (threads) per package */
	result = cpuid_ext(0xb, 1);
	threads_per_package = result.ebx & 0xffff;

	debug("CPU: %u has %u cores, %u threads per core\n",
	      cpu->apic_id, threads_per_package / threads_per_core,
	      threads_per_core);

	for (i = 1; i < threads_per_package; ++i) {
		struct x86_cpu_priv *new_cpu;

		new_cpu = calloc(1, sizeof(*new_cpu));
		if (!new_cpu)
			return -ENOMEM;

		new_cpu->apic_id = cpu->apic_id + i;

		/* Update APIC ID if no hyperthreading */
		if (threads_per_core == 1)
			new_cpu->apic_id <<= 1;

		debug("CPU: %u has core %u\n", cpu->apic_id, new_cpu->apic_id);

#if CONFIG_SMP && CONFIG_MAX_CPUS > 1
		/* Start the new cpu */
		if (!start_cpu(new_cpu)) {
			/* Record the error in cpu? */
			printk(BIOS_ERR, "CPU %u would not start!\n",
			       new_cpu->apic_id);
			new_cpu->start_err = 1;
		}
#endif
	}

	return 0;
}

int model_206ax_init(struct x86_cpu_priv *cpu)
{
	int ret;

	/* Clear out pending MCEs */
	configure_mca();

#if CONFIG_USBDEBUG
	/* Is this caution really needed? */
	if (!ehci_debug_addr)
		ehci_debug_addr = get_ehci_debug();
	set_ehci_debug(0);
#endif

	/* Setup MTRRs based on physical address size */
#if 0 /* TODO: Implement this */
	struct cpuid_result cpuid_regs;

	cpuid_regs = cpuid(0x80000008);
	x86_setup_fixed_mtrrs();
	x86_setup_var_mtrrs(cpuid_regs.eax & 0xff, 2);
	x86_mtrr_check();
#endif

#if CONFIG_USBDEBUG
	set_ehci_debug(ehci_debug_addr);
#endif

	/* Enable the local cpu apics */
	enable_lapic_tpr();
	lapic_setup();

	/* Enable virtualization if enabled in CMOS */
	enable_vmx();

	/* Configure C States */
	configure_c_states();

	/* Configure Enhanced SpeedStep and Thermal Sensors */
	configure_misc();

	/* Thermal throttle activation offset */
	ret = configure_thermal_target();
	if (ret)
		return ret;

	/* Enable Direct Cache Access */
	configure_dca_cap();

	/* Set energy policy */
	set_energy_perf_bias(ENERGY_POLICY_NORMAL);

	/* Set Max Ratio */
	set_max_ratio();

	/* Enable Turbo */
	turbo_enable();

	/* Start up extra cores */
	intel_cores_init(cpu);

	return 0;
}
