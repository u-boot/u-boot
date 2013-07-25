/*
 * (C) Copyright 2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Author: Sergei Poselenov <sposelenov@emcraft.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>

#if defined(CONFIG_440EP) || \
    defined(CONFIG_440EPX)

#include <asm/processor.h>
#include <asm/ppc4xx.h>


int fpu_status(void)
{
	if (mfspr(SPRN_CCR0) & CCR0_DAPUIB)
		return 0; /* Disabled */
	else
		return 1; /* Enabled */
}


void fpu_disable(void)
{
	mtspr(SPRN_CCR0, mfspr(SPRN_CCR0) | CCR0_DAPUIB);
	mtmsr(mfmsr() & ~MSR_FP);
}


void fpu_enable(void)
{
	mtspr(SPRN_CCR0, mfspr(SPRN_CCR0) & ~CCR0_DAPUIB);
	mtmsr(mfmsr() | MSR_FP);
}

#endif
