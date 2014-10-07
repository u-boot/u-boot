/*
 * Copyright (C) 2011-2014 Panasonic Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sc-regs.h>

void enable_dpll_ssc(void)
{
	u32 tmp;

	tmp = readl(SC_DPLLCTRL);
	tmp |= SC_DPLLCTRL_SSC_EN;
	writel(tmp, SC_DPLLCTRL);
}
