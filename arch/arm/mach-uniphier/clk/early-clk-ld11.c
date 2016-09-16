/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/io.h>

#include "../init.h"
#include "../sc64-regs.h"

int uniphier_ld11_early_clk_init(const struct uniphier_board_data *bd)
{
	u32 tmp;

	/* deassert reset */
	tmp = readl(SC_RSTCTRL7);
	tmp |= SC_RSTCTRL7_UMC31 | SC_RSTCTRL7_UMC30;
	writel(tmp, SC_RSTCTRL7);

	/* provide clocks */
	tmp = readl(SC_CLKCTRL4);
	tmp |= SC_CLKCTRL4_PERI;
	writel(tmp, SC_CLKCTRL4);

	tmp = readl(SC_CLKCTRL7);
	tmp |= SC_CLKCTRL7_UMC31 | SC_CLKCTRL7_UMC30;
	writel(tmp, SC_CLKCTRL7);

	return 0;
}
