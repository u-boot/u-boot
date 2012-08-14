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
	DEFINE(CLKCTL_CGR3, offsetof(struct ccm_regs, cgr3));

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
	DEFINE(MAX_MGPCR5, offsetof(struct max_regs, mgpcr5));

	/* AHB <-> IP-Bus Interface */
	DEFINE(AIPS_MPR_0_7, offsetof(struct aips_regs, mpr_0_7));
	DEFINE(AIPS_MPR_8_15, offsetof(struct aips_regs, mpr_8_15));
	DEFINE(AIPS_PACR_0_7, offsetof(struct aips_regs, pacr_0_7));
	DEFINE(AIPS_PACR_8_15, offsetof(struct aips_regs, pacr_8_15));
	DEFINE(AIPS_PACR_16_23, offsetof(struct aips_regs, pacr_16_23));
	DEFINE(AIPS_PACR_24_31, offsetof(struct aips_regs, pacr_24_31));
	DEFINE(AIPS_OPACR_0_7, offsetof(struct aips_regs, opacr_0_7));
	DEFINE(AIPS_OPACR_8_15, offsetof(struct aips_regs, opacr_8_15));
	DEFINE(AIPS_OPACR_16_23, offsetof(struct aips_regs, opacr_16_23));
	DEFINE(AIPS_OPACR_24_31, offsetof(struct aips_regs, opacr_24_31));
	DEFINE(AIPS_OPACR_32_39, offsetof(struct aips_regs, opacr_32_39));

	return 0;
}
