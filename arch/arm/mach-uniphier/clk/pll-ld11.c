/*
 * Copyright (C) 2016 Socionext Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/io.h>

#include "../init.h"
#include "../sc64-regs.h"
#include "pll.h"

void uniphier_ld11_pll_init(void)
{
	uniphier_ld20_sscpll_init(SC_CPLLCTRL, 1960, 1, 2);	/* 2000MHz -> 1960MHz */
	/* do nothing for SPLL */
	uniphier_ld20_sscpll_init(SC_MPLLCTRL, 1600, 1, 2);	/* 1500MHz -> 1600MHz */
	uniphier_ld20_sscpll_init(SC_VSPLLCTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 2);

	mdelay(1);

	uniphier_ld20_sscpll_ssc_en(SC_CPLLCTRL);
	uniphier_ld20_sscpll_ssc_en(SC_MPLLCTRL);
	uniphier_ld20_sscpll_ssc_en(SC_VSPLLCTRL);
	uniphier_ld20_sscpll_ssc_en(SC_DPLLCTRL);

	uniphier_ld20_vpll27_init(SC_VPLL27FCTRL);
	uniphier_ld20_vpll27_init(SC_VPLL27ACTRL);

	writel(0, SC_CA53_GEARSET);	/* Gear0: CPLL/2 */
	writel(SC_CA_GEARUPD, SC_CA53_GEARUPD);
}
