/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include "../init.h"
#include "../sc64-regs.h"
#include "pll.h"

int uniphier_ld20_pll_init(const struct uniphier_board_data *bd)
{
	unsigned int dpll_ssc_rate = UNIPHIER_BD_DPLL_SSC_GET_RATE(bd->flags);

	uniphier_ld20_sscpll_init(SC_CPLLCTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 4);
	/* do nothing for SPLL */
	uniphier_ld20_sscpll_init(SC_SPLL2CTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 4);
	uniphier_ld20_sscpll_init(SC_MPLLCTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 2);
	uniphier_ld20_sscpll_init(SC_VPPLLCTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 4);
	uniphier_ld20_sscpll_init(SC_GPPLLCTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 2);

	mdelay(1);

	if (dpll_ssc_rate > 0) {
		uniphier_ld20_sscpll_ssc_en(SC_DPLL0CTRL);
		uniphier_ld20_sscpll_ssc_en(SC_DPLL1CTRL);
		uniphier_ld20_sscpll_ssc_en(SC_DPLL2CTRL);
	}

	uniphier_ld20_vpll27_init(SC_VPLL27FCTRL);
	uniphier_ld20_vpll27_init(SC_VPLL27ACTRL);

	uniphier_ld20_dspll_init(SC_VPLL8KCTRL);
	uniphier_ld20_dspll_init(SC_A2PLLCTRL);

	return 0;
}
