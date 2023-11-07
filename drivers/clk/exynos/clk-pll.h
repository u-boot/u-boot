/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Exynos PLL helper functions for clock drivers.
 * Copyright (C) 2016 Samsung Electronics
 * Thomas Abraham <thomas.ab@samsung.com>
 */

#ifndef __EXYNOS_CLK_PLL_H
#define __EXYNOS_CLK_PLL_H

unsigned long pll145x_get_rate(unsigned int *con1, unsigned long fin_freq);

#endif /* __EXYNOS_CLK_PLL_H */
