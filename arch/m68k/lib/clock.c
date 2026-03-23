// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2026 Kernelspace
 * Angelo Dureghello <angelo@kernel-space.org>
 */

#include <config.h>
#include <asm/arch/clock.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;
/*
 * Stub to use existing nxp/fsl drivers.
 */
int mxc_get_clock(enum mxc_clock clk)
{
	if (clk == MXC_ESDHC_CLK)
		return gd->arch.sdhc_clk;

	printf("Unsupported MXC CLK: %d\n", clk);

	return 0;
}
