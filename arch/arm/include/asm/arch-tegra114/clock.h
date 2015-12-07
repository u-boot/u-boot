/*
 * Copyright (c) 2010-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Tegra114 clock control functions */

#ifndef _TEGRA114_CLOCK_H_
#define _TEGRA114_CLOCK_H_

#include <asm/arch-tegra/clock.h>

/* CLK_RST_CONTROLLER_OSC_CTRL_0 */
#define OSC_FREQ_SHIFT          28
#define OSC_FREQ_MASK           (0xF << OSC_FREQ_SHIFT)

/* CLK_RST_CONTROLLER_PLLC_MISC_0 */
#define PLLC_IDDQ		(1 << 26)

#endif	/* _TEGRA114_CLOCK_H_ */
