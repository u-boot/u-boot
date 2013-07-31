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

	/* Round up to make sure size gives nice stack alignment */
	DEFINE(CLKCTL_CCMR, offsetof(struct clkctl, ccr));
	DEFINE(CLKCTL_CCDR, offsetof(struct clkctl, ccdr));
	DEFINE(CLKCTL_CSR, offsetof(struct clkctl, csr));
	DEFINE(CLKCTL_CCSR, offsetof(struct clkctl, ccsr));
	DEFINE(CLKCTL_CACRR, offsetof(struct clkctl, cacrr));
	DEFINE(CLKCTL_CBCDR, offsetof(struct clkctl, cbcdr));
	DEFINE(CLKCTL_CBCMR, offsetof(struct clkctl, cbcmr));
	DEFINE(CLKCTL_CSCMR1, offsetof(struct clkctl, cscmr1));
	DEFINE(CLKCTL_CSCMR2, offsetof(struct clkctl, cscmr2));
	DEFINE(CLKCTL_CSCDR1, offsetof(struct clkctl, cscdr1));
	DEFINE(CLKCTL_CS1CDR, offsetof(struct clkctl, cs1cdr));
	DEFINE(CLKCTL_CS2CDR, offsetof(struct clkctl, cs2cdr));
	DEFINE(CLKCTL_CDCDR, offsetof(struct clkctl, cdcdr));
	DEFINE(CLKCTL_CHSCCDR, offsetof(struct clkctl, chsccdr));
	DEFINE(CLKCTL_CSCDR2, offsetof(struct clkctl, cscdr2));
	DEFINE(CLKCTL_CSCDR3, offsetof(struct clkctl, cscdr3));
	DEFINE(CLKCTL_CSCDR4, offsetof(struct clkctl, cscdr4));
	DEFINE(CLKCTL_CWDR, offsetof(struct clkctl, cwdr));
	DEFINE(CLKCTL_CDHIPR, offsetof(struct clkctl, cdhipr));
	DEFINE(CLKCTL_CDCR, offsetof(struct clkctl, cdcr));
	DEFINE(CLKCTL_CTOR, offsetof(struct clkctl, ctor));
	DEFINE(CLKCTL_CLPCR, offsetof(struct clkctl, clpcr));
	DEFINE(CLKCTL_CISR, offsetof(struct clkctl, cisr));
	DEFINE(CLKCTL_CIMR, offsetof(struct clkctl, cimr));
	DEFINE(CLKCTL_CCOSR, offsetof(struct clkctl, ccosr));
	DEFINE(CLKCTL_CGPR, offsetof(struct clkctl, cgpr));
	DEFINE(CLKCTL_CCGR0, offsetof(struct clkctl, ccgr0));
	DEFINE(CLKCTL_CCGR1, offsetof(struct clkctl, ccgr1));
	DEFINE(CLKCTL_CCGR2, offsetof(struct clkctl, ccgr2));
	DEFINE(CLKCTL_CCGR3, offsetof(struct clkctl, ccgr3));
	DEFINE(CLKCTL_CCGR4, offsetof(struct clkctl, ccgr4));
	DEFINE(CLKCTL_CCGR5, offsetof(struct clkctl, ccgr5));
	DEFINE(CLKCTL_CCGR6, offsetof(struct clkctl, ccgr6));
	DEFINE(CLKCTL_CMEOR, offsetof(struct clkctl, cmeor));
#if defined(CONFIG_MX53)
	DEFINE(CLKCTL_CCGR7, offsetof(struct clkctl, ccgr7));
#endif

	/* DPLL */
	DEFINE(PLL_DP_CTL, offsetof(struct dpll, dp_ctl));
	DEFINE(PLL_DP_CONFIG, offsetof(struct dpll, dp_config));
	DEFINE(PLL_DP_OP, offsetof(struct dpll, dp_op));
	DEFINE(PLL_DP_MFD, offsetof(struct dpll, dp_mfd));
	DEFINE(PLL_DP_MFN, offsetof(struct dpll, dp_mfn));
	DEFINE(PLL_DP_HFS_OP, offsetof(struct dpll, dp_hfs_op));
	DEFINE(PLL_DP_HFS_MFD, offsetof(struct dpll, dp_hfs_mfd));
	DEFINE(PLL_DP_HFS_MFN, offsetof(struct dpll, dp_hfs_mfn));

	return 0;
}
