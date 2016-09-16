/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "../init.h"
#include "pll.h"

void uniphier_sld3_pll_init(void)
{
	uniphier_ld4_dpll_ssc_en();
}
