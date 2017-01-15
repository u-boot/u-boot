/*
 * Copyright (C) 2016-2017 Socionext Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/io.h>

#include "../init.h"
#include "../sc64-regs.h"

void uniphier_ld20_dram_clk_init(void)
{
	u32 tmp;

	/* deassert reset */
	tmp = readl(SC_RSTCTRL7);
	tmp |= SC_RSTCTRL7_UMCSB | SC_RSTCTRL7_UMCA2 | SC_RSTCTRL7_UMCA1 |
		SC_RSTCTRL7_UMCA0 | SC_RSTCTRL7_UMC32 | SC_RSTCTRL7_UMC31 |
		SC_RSTCTRL7_UMC30;
	writel(tmp, SC_RSTCTRL7);

	/* provide clocks */
	tmp = readl(SC_CLKCTRL7);
	tmp |= SC_CLKCTRL7_UMCSB | SC_CLKCTRL7_UMC32 | SC_CLKCTRL7_UMC31 |
							SC_CLKCTRL7_UMC30;
	writel(tmp, SC_CLKCTRL7);
}
