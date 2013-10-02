/*
 * Adapted from Linux v2.6.36 kernel: arch/powerpc/kernel/asm-offsets.c
 *
 * This program is used to generate definitions needed by
 * assembly language modules.
 *
 * We use the technique used in the OSF Mach kernel code:
 * generate asm statements containing #defines,
 * compile this file to assembler, and then extract the
 * #defines from the assembly-language output.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/imx-regs.h>

#include <linux/kbuild.h>

int main(void)
{
	DEFINE(AIPI1_PSR0, IMX_AIPI1_BASE + offsetof(struct aipi_regs, psr0));
	DEFINE(AIPI1_PSR1, IMX_AIPI1_BASE + offsetof(struct aipi_regs, psr1));
	DEFINE(AIPI2_PSR0, IMX_AIPI2_BASE + offsetof(struct aipi_regs, psr0));
	DEFINE(AIPI2_PSR1, IMX_AIPI2_BASE + offsetof(struct aipi_regs, psr1));

	DEFINE(CSCR, IMX_PLL_BASE + offsetof(struct pll_regs, cscr));
	DEFINE(MPCTL0, IMX_PLL_BASE + offsetof(struct pll_regs, mpctl0));
	DEFINE(SPCTL0, IMX_PLL_BASE + offsetof(struct pll_regs, spctl0));
	DEFINE(PCDR0, IMX_PLL_BASE + offsetof(struct pll_regs, pcdr0));
	DEFINE(PCDR1, IMX_PLL_BASE + offsetof(struct pll_regs, pcdr1));
	DEFINE(PCCR0, IMX_PLL_BASE + offsetof(struct pll_regs, pccr0));
	DEFINE(PCCR1, IMX_PLL_BASE + offsetof(struct pll_regs, pccr1));

	DEFINE(ESDCTL0_ROF, offsetof(struct esdramc_regs, esdctl0));
	DEFINE(ESDCFG0_ROF, offsetof(struct esdramc_regs, esdcfg0));
	DEFINE(ESDCTL1_ROF, offsetof(struct esdramc_regs, esdctl1));
	DEFINE(ESDCFG1_ROF, offsetof(struct esdramc_regs, esdcfg1));
	DEFINE(ESDMISC_ROF, offsetof(struct esdramc_regs, esdmisc));

	DEFINE(GPCR, IMX_SYSTEM_CTL_BASE +
		offsetof(struct system_control_regs, gpcr));
	DEFINE(FMCR, IMX_SYSTEM_CTL_BASE +
		offsetof(struct system_control_regs, fmcr));

	return 0;
}
