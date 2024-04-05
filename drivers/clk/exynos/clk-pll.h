/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Samsung Electronics
 * Copyright (C) 2023 Linaro Ltd.
 *
 * Authors:
 *   Thomas Abraham <thomas.ab@samsung.com>
 *   Sam Protsenko <semen.protsenko@linaro.org>
 *
 * Common Clock Framework support for all PLL's in Samsung platforms.
 */

#ifndef __EXYNOS_CLK_PLL_H
#define __EXYNOS_CLK_PLL_H

#include <linux/clk-provider.h>

struct samsung_pll_clock;

enum samsung_pll_type {
	pll_0822x,
	pll_0831x,
};

void samsung_clk_register_pll(void __iomem *base, unsigned int cmu_id,
			      const struct samsung_pll_clock *clk_list,
			      unsigned int nr_clk);

#endif /* __EXYNOS_CLK_PLL_H */
