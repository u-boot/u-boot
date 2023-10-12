// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016
 * Author: Chen-Yu Tsai <wens@csie.org>
 *
 * Based on assembly code by Marc Zyngier <marc.zyngier@arm.com>,
 * which was based on code by Carl van Schaik <carl@ok-labs.com>.
 */
#include <config.h>
#include <common.h>
#include <asm/cache.h>

#include <asm/arch/cpu.h>
#include <asm/arch/cpucfg.h>
#include <asm/arch/prcm.h>
#include <asm/armv7.h>
#include <asm/gic.h>
#include <asm/io.h>
#include <asm/psci.h>
#include <asm/secure.h>
#include <asm/system.h>

#include <linux/bitops.h>

#define __irq		__attribute__ ((interrupt ("IRQ")))

#define	GICD_BASE	(SUNXI_GIC400_BASE + GIC_DIST_OFFSET)
#define	GICC_BASE	(SUNXI_GIC400_BASE + GIC_CPU_OFFSET_A15)

/*
 * R40 is different from other single cluster SoCs.
 *
 * The power clamps are located in the unused space after the per-core
 * reset controls for core 3. The secondary core entry address register
 * is in the SRAM controller address range.
 */
#define SUN8I_R40_PWROFF			(0x110)
#define SUN8I_R40_PWR_CLAMP(cpu)		(0x120 + (cpu) * 0x4)
#define SUN8I_R40_SRAMC_SOFT_ENTRY_REG0		(0xbc)

static void __secure cp15_write_cntp_tval(u32 tval)
{
	asm volatile ("mcr p15, 0, %0, c14, c2, 0" : : "r" (tval));
}

static void __secure cp15_write_cntp_ctl(u32 val)
{
	asm volatile ("mcr p15, 0, %0, c14, c2, 1" : : "r" (val));
}

static u32 __secure cp15_read_cntp_ctl(void)
{
	u32 val;

	asm volatile ("mrc p15, 0, %0, c14, c2, 1" : "=r" (val));

	return val;
}

#define ONE_MS (CONFIG_COUNTER_FREQUENCY / 1000)

static void __secure __mdelay(u32 ms)
{
	u32 reg = ONE_MS * ms;

	cp15_write_cntp_tval(reg);
	isb();
	cp15_write_cntp_ctl(3);

	do {
		isb();
		reg = cp15_read_cntp_ctl();
	} while (!(reg & BIT(2)));

	cp15_write_cntp_ctl(0);
	isb();
}

static void __secure clamp_release(u32 *clamp)
{
	u32 tmp = 0x1ff;
	do {
		tmp >>= 1;
		writel(tmp, clamp);
	} while (tmp);

	__mdelay(10);
}

static void __secure clamp_set(u32 *clamp)
{
	writel(0xff, clamp);
}

static void __secure sunxi_cpu_set_entry(int __always_unused cpu, void *entry)
{
	/* secondary core entry address is programmed differently on R40 */
	if (IS_ENABLED(CONFIG_MACH_SUN8I_R40)) {
		writel((u32)entry,
		       SUNXI_SRAMC_BASE + SUN8I_R40_SRAMC_SOFT_ENTRY_REG0);
	} else {
		struct sunxi_cpucfg_reg *cpucfg =
			(struct sunxi_cpucfg_reg *)SUNXI_CPUCFG_BASE;

		writel((u32)entry, &cpucfg->priv0);
	}
}

static void __secure sunxi_cpu_set_power(int cpu, bool on)
{
	u32 *clamp = NULL;
	u32 *pwroff;
	struct sunxi_cpucfg_reg *cpucfg =
		(struct sunxi_cpucfg_reg *)SUNXI_CPUCFG_BASE;

	/* sun7i (A20) is different from other single cluster SoCs */
	if (IS_ENABLED(CONFIG_MACH_SUN7I)) {
		clamp = &cpucfg->cpu1_pwr_clamp;
		pwroff = &cpucfg->cpu1_pwroff;
		cpu = 0;
	} else if (IS_ENABLED(CONFIG_MACH_SUN8I_R40)) {
		clamp = (void *)cpucfg + SUN8I_R40_PWR_CLAMP(cpu);
		pwroff = (void *)cpucfg + SUN8I_R40_PWROFF;
	} else {
		struct sunxi_prcm_reg *prcm =
			(struct sunxi_prcm_reg *)SUNXI_PRCM_BASE;

		if (IS_ENABLED(CONFIG_MACH_SUN6I) ||
		    IS_ENABLED(CONFIG_MACH_SUN8I_H3))
			clamp = &prcm->cpu_pwr_clamp[cpu];

		pwroff = &prcm->cpu_pwroff;
	}

	if (on) {
		/* Release power clamp */
		if (clamp)
			clamp_release(clamp);

		/* Clear power gating */
		clrbits_le32(pwroff, BIT(cpu));
	} else {
		/* Set power gating */
		setbits_le32(pwroff, BIT(cpu));

		/* Activate power clamp */
		if (clamp)
			clamp_set(clamp);
	}
}

static void __secure sunxi_cpu_set_reset(int cpu, bool reset)
{
	struct sunxi_cpucfg_reg *cpucfg =
		(struct sunxi_cpucfg_reg *)SUNXI_CPUCFG_BASE;

	writel(reset ? 0b00 : 0b11, &cpucfg->cpu[cpu].rst);
}

static void __secure sunxi_cpu_set_locking(int cpu, bool lock)
{
	struct sunxi_cpucfg_reg *cpucfg =
		(struct sunxi_cpucfg_reg *)SUNXI_CPUCFG_BASE;

	if (lock)
		clrbits_le32(&cpucfg->dbg_ctrl1, BIT(cpu));
	else
		setbits_le32(&cpucfg->dbg_ctrl1, BIT(cpu));
}

static bool __secure sunxi_cpu_poll_wfi(int cpu)
{
	struct sunxi_cpucfg_reg *cpucfg =
		(struct sunxi_cpucfg_reg *)SUNXI_CPUCFG_BASE;

	return !!(readl(&cpucfg->cpu[cpu].status) & BIT(2));
}

static void __secure sunxi_cpu_invalidate_cache(int cpu)
{
	struct sunxi_cpucfg_reg *cpucfg =
		(struct sunxi_cpucfg_reg *)SUNXI_CPUCFG_BASE;

	clrbits_le32(&cpucfg->gen_ctrl, BIT(cpu));
}

static void __secure sunxi_cpu_power_off(u32 cpuid)
{
	u32 cpu = cpuid & 0x3;

	/* Wait for the core to enter WFI */
	while (!sunxi_cpu_poll_wfi(cpu))
		__mdelay(1);

	/* Assert reset on target CPU */
	sunxi_cpu_set_reset(cpu, true);

	/* Lock CPU (Disable external debug access) */
	sunxi_cpu_set_locking(cpu, true);

	/* Power down CPU */
	sunxi_cpu_set_power(cpuid, false);

	/* Unlock CPU (Reenable external debug access) */
	sunxi_cpu_set_locking(cpu, false);
}

static u32 __secure cp15_read_scr(void)
{
	u32 scr;

	asm volatile ("mrc p15, 0, %0, c1, c1, 0" : "=r" (scr));

	return scr;
}

static void __secure cp15_write_scr(u32 scr)
{
	asm volatile ("mcr p15, 0, %0, c1, c1, 0" : : "r" (scr));
	isb();
}

/*
 * Although this is an FIQ handler, the FIQ is processed in monitor mode,
 * which means there's no FIQ banked registers. This is the same as IRQ
 * mode, so use the IRQ attribute to ask the compiler to handler entry
 * and return.
 */
void __secure __irq psci_fiq_enter(void)
{
	u32 scr, reg, cpu;

	/* Switch to secure mode */
	scr = cp15_read_scr();
	cp15_write_scr(scr & ~BIT(0));

	/* Validate reason based on IAR and acknowledge */
	reg = readl(GICC_BASE + GICC_IAR);

	/* Skip spurious interrupts 1022 and 1023 */
	if (reg == 1023 || reg == 1022)
		goto out;

	/* End of interrupt */
	writel(reg, GICC_BASE + GICC_EOIR);
	dsb();

	/* Get CPU number */
	cpu = (reg >> 10) & 0x7;

	/* Power off the CPU */
	sunxi_cpu_power_off(cpu);

out:
	/* Restore security level */
	cp15_write_scr(scr);
}

int __secure psci_cpu_on(u32 __always_unused unused, u32 mpidr, u32 pc,
			 u32 context_id)
{
	u32 cpu = (mpidr & 0x3);

	/* store target PC and context id */
	psci_save(cpu, pc, context_id);

	/* Set secondary core power on PC */
	sunxi_cpu_set_entry(cpu, &psci_cpu_entry);

	/* Assert reset on target CPU */
	sunxi_cpu_set_reset(cpu, true);

	/* Invalidate L1 cache */
	sunxi_cpu_invalidate_cache(cpu);

	/* Lock CPU (Disable external debug access) */
	sunxi_cpu_set_locking(cpu, true);

	/* Power up target CPU */
	sunxi_cpu_set_power(cpu, true);

	/* De-assert reset on target CPU */
	sunxi_cpu_set_reset(cpu, false);

	/* Unlock CPU (Reenable external debug access) */
	sunxi_cpu_set_locking(cpu, false);

	return ARM_PSCI_RET_SUCCESS;
}

s32 __secure psci_cpu_off(void)
{
	psci_cpu_off_common();

	/* Ask CPU0 via SGI15 to pull the rug... */
	writel(BIT(16) | 15, GICD_BASE + GICD_SGIR);
	dsb();

	/* Wait to be turned off */
	while (1)
		wfi();
}

void __secure psci_arch_init(void)
{
	u32 reg;

	/* SGI15 as Group-0 */
	clrbits_le32(GICD_BASE + GICD_IGROUPRn, BIT(15));

	/* Set SGI15 priority to 0 */
	writeb(0, GICD_BASE + GICD_IPRIORITYRn + 15);

	/* Be cool with non-secure */
	writel(0xff, GICC_BASE + GICC_PMR);

	/* Switch FIQEn on */
	setbits_le32(GICC_BASE + GICC_CTLR, BIT(3));

	reg = cp15_read_scr();
	reg |= BIT(2);  /* Enable FIQ in monitor mode */
	reg &= ~BIT(0); /* Secure mode */
	cp15_write_scr(reg);
}
