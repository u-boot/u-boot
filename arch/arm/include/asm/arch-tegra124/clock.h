/*
 * (C) Copyright 2010-2013
 * NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

/* Tegra124 clock control definitions */

#ifndef _TEGRA124_CLOCK_H_
#define _TEGRA124_CLOCK_H_

#include <asm/arch-tegra/clock.h>

/* CLK_RST_CONTROLLER_OSC_CTRL_0 */
#define OSC_FREQ_SHIFT          28
#define OSC_FREQ_MASK           (0xF << OSC_FREQ_SHIFT)

#endif	/* _TEGRA124_CLOCK_H_ */
