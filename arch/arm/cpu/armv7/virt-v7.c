/*
 * (C) Copyright 2013
 * Andre Przywara, Linaro <andre.przywara@linaro.org>
 *
 * Routines to transition ARMv7 processors from secure into non-secure state
 * and from non-secure SVC into HYP mode
 * needed to enable ARMv7 virtualization for current hypervisors
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/armv7.h>
#include <asm/gic.h>
#include <asm/io.h>

unsigned long gic_dist_addr;

static unsigned int read_cpsr(void)
{
	unsigned int reg;

	asm volatile ("mrs %0, cpsr\n" : "=r" (reg));
	return reg;
}

static unsigned int read_id_pfr1(void)
{
	unsigned int reg;

	asm("mrc p15, 0, %0, c0, c1, 1\n" : "=r"(reg));
	return reg;
}

static unsigned long get_gicd_base_address(void)
{
#ifdef CONFIG_ARM_GIC_BASE_ADDRESS
	return CONFIG_ARM_GIC_BASE_ADDRESS + GIC_DIST_OFFSET;
#else
	unsigned midr;
	unsigned periphbase;

	/* check whether we are an Cortex-A15 or A7.
	 * The actual HYP switch should work with all CPUs supporting
	 * the virtualization extension, but we need the GIC address,
	 * which we know only for sure for those two CPUs.
	 */
	asm("mrc p15, 0, %0, c0, c0, 0\n" : "=r"(midr));
	switch (midr & MIDR_PRIMARY_PART_MASK) {
	case MIDR_CORTEX_A9_R0P1:
	case MIDR_CORTEX_A15_R0P0:
	case MIDR_CORTEX_A7_R0P0:
		break;
	default:
		printf("nonsec: could not determine GIC address.\n");
		return -1;
	}

	/* get the GIC base address from the CBAR register */
	asm("mrc p15, 4, %0, c15, c0, 0\n" : "=r" (periphbase));

	/* the PERIPHBASE can be mapped above 4 GB (lower 8 bits used to
	 * encode this). Bail out here since we cannot access this without
	 * enabling paging.
	 */
	if ((periphbase & 0xff) != 0) {
		printf("nonsec: PERIPHBASE is above 4 GB, no access.\n");
		return -1;
	}

	return (periphbase & CBAR_MASK) + GIC_DIST_OFFSET;
#endif
}

static void kick_secondary_cpus_gic(unsigned long gicdaddr)
{
	/* kick all CPUs (except this one) by writing to GICD_SGIR */
	writel(1U << 24, gicdaddr + GICD_SGIR);
}

void __weak smp_kick_all_cpus(void)
{
	kick_secondary_cpus_gic(gic_dist_addr);
}

int armv7_switch_hyp(void)
{
	unsigned int reg;

	/* check whether we are in HYP mode already */
	if ((read_cpsr() & 0x1f) == 0x1a) {
		debug("CPU already in HYP mode\n");
		return 0;
	}

	/* check whether the CPU supports the virtualization extensions */
	reg = read_id_pfr1();
	if ((reg & CPUID_ARM_VIRT_MASK) != 1 << CPUID_ARM_VIRT_SHIFT) {
		printf("HYP mode: Virtualization extensions not implemented.\n");
		return -1;
	}

	/* call the HYP switching code on this CPU also */
	_switch_to_hyp();

	if ((read_cpsr() & 0x1F) != 0x1a) {
		printf("HYP mode: switch not successful.\n");
		return -1;
	}

	return 0;
}

int armv7_switch_nonsec(void)
{
	unsigned int reg;
	unsigned itlinesnr, i;

	/* check whether the CPU supports the security extensions */
	reg = read_id_pfr1();
	if ((reg & 0xF0) == 0) {
		printf("nonsec: Security extensions not implemented.\n");
		return -1;
	}

	/* the SCR register will be set directly in the monitor mode handler,
	 * according to the spec one should not tinker with it in secure state
	 * in SVC mode. Do not try to read it once in non-secure state,
	 * any access to it will trap.
	 */

	gic_dist_addr = get_gicd_base_address();
	if (gic_dist_addr == -1)
		return -1;

	/* enable the GIC distributor */
	writel(readl(gic_dist_addr + GICD_CTLR) | 0x03,
	       gic_dist_addr + GICD_CTLR);

	/* TYPER[4:0] contains an encoded number of available interrupts */
	itlinesnr = readl(gic_dist_addr + GICD_TYPER) & 0x1f;

	/* set all bits in the GIC group registers to one to allow access
	 * from non-secure state. The first 32 interrupts are private per
	 * CPU and will be set later when enabling the GIC for each core
	 */
	for (i = 1; i <= itlinesnr; i++)
		writel((unsigned)-1, gic_dist_addr + GICD_IGROUPRn + 4 * i);

	smp_set_core_boot_addr((unsigned long)_smp_pen, -1);
	smp_kick_all_cpus();

	/* call the non-sec switching code on this CPU also */
	_nonsec_init();

	return 0;
}
