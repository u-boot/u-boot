/*
 * Copyright 2015 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fsl_ifc.h>
#include <asm/arch-fsl-lsch3/soc.h>

void fsl_lsch3_early_init_f(void)
{
	init_early_memctl_regs();	/* tighten IFC timing */
}
