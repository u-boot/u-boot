/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/io.h>
#include <mach/sc-regs.h>

void enable_dpll_ssc(void)
{
	u32 tmp;

	tmp = readl(SC_DPLLCTRL);
	tmp |= SC_DPLLCTRL_SSC_EN;
	writel(tmp, SC_DPLLCTRL);
}
