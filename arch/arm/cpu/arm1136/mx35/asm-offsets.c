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

	/* Round up to make sure size gives nice stack alignment */
	DEFINE(CLKCTL_CCMR, offsetof(struct ccm_regs, ccmr));
	DEFINE(CLKCTL_PDR0, offsetof(struct ccm_regs, pdr0));
	DEFINE(CLKCTL_PDR1, offsetof(struct ccm_regs, pdr1));
	DEFINE(CLKCTL_PDR2, offsetof(struct ccm_regs, pdr2));
	DEFINE(CLKCTL_PDR3, offsetof(struct ccm_regs, pdr3));
	DEFINE(CLKCTL_PDR4, offsetof(struct ccm_regs, pdr4));
	DEFINE(CLKCTL_RCSR, offsetof(struct ccm_regs, rcsr));
	DEFINE(CLKCTL_MPCTL, offsetof(struct ccm_regs, mpctl));
	DEFINE(CLKCTL_PPCTL, offsetof(struct ccm_regs, ppctl));
	DEFINE(CLKCTL_ACMR, offsetof(struct ccm_regs, acmr));
	DEFINE(CLKCTL_COSR, offsetof(struct ccm_regs, cosr));
	DEFINE(CLKCTL_CGR0, offsetof(struct ccm_regs, cgr0));
	DEFINE(CLKCTL_CGR1, offsetof(struct ccm_regs, cgr1));
	DEFINE(CLKCTL_CGR2, offsetof(struct ccm_regs, cgr2));

	return 0;
}
