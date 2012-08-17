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
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <common.h>
#include <asm/arch/imx-regs.h>

#include <linux/kbuild.h>

int main(void)
{
	/* Clock Control Module */
	DEFINE(CCM_CCTL, offsetof(struct ccm_regs, cctl));
	DEFINE(CCM_CGCR0, offsetof(struct ccm_regs, cgr0));
	DEFINE(CCM_CGCR1, offsetof(struct ccm_regs, cgr1));
	DEFINE(CCM_CGCR2, offsetof(struct ccm_regs, cgr2));
	DEFINE(CCM_PCDR2, offsetof(struct ccm_regs, pcdr[2]));
	DEFINE(CCM_MCR, offsetof(struct ccm_regs, mcr));

	/* Enhanced SDRAM Controller */
	DEFINE(ESDRAMC_ESDCTL0, offsetof(struct esdramc_regs, ctl0));
	DEFINE(ESDRAMC_ESDCFG0, offsetof(struct esdramc_regs, cfg0));
	DEFINE(ESDRAMC_ESDMISC, offsetof(struct esdramc_regs, misc));

	/* Multi-Layer AHB Crossbar Switch */
	DEFINE(MAX_MPR0, offsetof(struct max_regs, mpr0));
	DEFINE(MAX_SGPCR0, offsetof(struct max_regs, sgpcr0));
	DEFINE(MAX_MPR1, offsetof(struct max_regs, mpr1));
	DEFINE(MAX_SGPCR1, offsetof(struct max_regs, sgpcr1));
	DEFINE(MAX_MPR2, offsetof(struct max_regs, mpr2));
	DEFINE(MAX_SGPCR2, offsetof(struct max_regs, sgpcr2));
	DEFINE(MAX_MPR3, offsetof(struct max_regs, mpr3));
	DEFINE(MAX_SGPCR3, offsetof(struct max_regs, sgpcr3));
	DEFINE(MAX_MPR4, offsetof(struct max_regs, mpr4));
	DEFINE(MAX_SGPCR4, offsetof(struct max_regs, sgpcr4));
	DEFINE(MAX_MGPCR0, offsetof(struct max_regs, mgpcr0));
	DEFINE(MAX_MGPCR1, offsetof(struct max_regs, mgpcr1));
	DEFINE(MAX_MGPCR2, offsetof(struct max_regs, mgpcr2));
	DEFINE(MAX_MGPCR3, offsetof(struct max_regs, mgpcr3));
	DEFINE(MAX_MGPCR4, offsetof(struct max_regs, mgpcr4));

	/* AHB <-> IP-Bus Interface */
	DEFINE(AIPS_MPR_0_7, offsetof(struct aips_regs, mpr_0_7));
	DEFINE(AIPS_MPR_8_15, offsetof(struct aips_regs, mpr_8_15));

	return 0;
}
