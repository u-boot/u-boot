/*
 * Copyright (C) 2016 Socionext Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "../init.h"
#include "../sc64-regs.h"
#include "pll.h"

int uniphier_ld11_dpll_init(const struct uniphier_board_data *bd)
{
	uniphier_ld20_sscpll_init(SC_DPLLCTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 2);

	return 0;
}
