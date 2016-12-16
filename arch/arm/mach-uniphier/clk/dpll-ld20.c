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
	uniphier_ld20_sscpll_init(SC_DPLL0CTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 2);
	uniphier_ld20_sscpll_init(SC_DPLL1CTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 2);
	uniphier_ld20_sscpll_init(SC_DPLL2CTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 2);

	return 0;
}
