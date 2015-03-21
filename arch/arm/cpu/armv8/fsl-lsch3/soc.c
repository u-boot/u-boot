/*
 * Copyright 2015 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fsl_ifc.h>
#include <asm/arch-fsl-lsch3/soc.h>
#include <asm/io.h>

static void erratum_a008751(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A008751
	u32 __iomem *scfg = (u32 __iomem *)SCFG_BASE;

	writel(0x27672b2a, scfg + SCFG_USB3PRM1CR / 4);
#endif
}

void fsl_lsch3_early_init_f(void)
{
	erratum_a008751();
	init_early_memctl_regs();	/* tighten IFC timing */
}
