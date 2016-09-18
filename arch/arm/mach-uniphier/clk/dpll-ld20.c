/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "../init.h"
#include "../sc64-regs.h"
#include "pll.h"

int uniphier_ld20_dpll_init(const struct uniphier_board_data *bd)
{
	unsigned int dpll_ssc_rate = UNIPHIER_BD_DPLL_SSC_GET_RATE(bd->flags);
	unsigned int dram_freq = bd->dram_freq;

	uniphier_ld20_sscpll_init(SC_DPLL0CTRL, dram_freq, dpll_ssc_rate, 2);
	uniphier_ld20_sscpll_init(SC_DPLL1CTRL, dram_freq, dpll_ssc_rate, 2);
	uniphier_ld20_sscpll_init(SC_DPLL2CTRL, dram_freq, dpll_ssc_rate, 2);

	return 0;
}
