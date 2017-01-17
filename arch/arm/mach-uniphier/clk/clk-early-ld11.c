/*
 * Copyright (C) 2016-2017 Socionext Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/io.h>

#include "../init.h"
#include "../sc64-regs.h"

void uniphier_ld11_early_clk_init(void)
{
	u32 tmp;

	/* provide clocks */
	tmp = readl(SC_CLKCTRL4);
	tmp |= SC_CLKCTRL4_PERI;
	writel(tmp, SC_CLKCTRL4);
}
