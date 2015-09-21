/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/io.h>
#include <mach/init.h>
#include <mach/sc-regs.h>

int ph1_sld3_enable_dpll_ssc(const struct uniphier_board_data *bd)
{
	u32 tmp;

	tmp = readl(SC_DPLLCTRL);
	tmp |= SC_DPLLCTRL_SSC_EN;
	writel(tmp, SC_DPLLCTRL);

	return 0;
}
