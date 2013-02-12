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

#ifndef _TEGRA114_GPIO_H_
#define _TEGRA114_GPIO_H_

/*
 * The Tegra114 GPIO controller has 246 GPIOS in 8 banks of 4 ports,
 * each with 8 GPIOs.
 */
#define TEGRA_GPIO_PORTS	4	/* number of ports per bank */
#define TEGRA_GPIO_BANKS	8	/* number of banks */

#include <asm/arch-tegra/gpio.h>
#include <asm/arch-tegra30/gpio.h>

#endif	/* _TEGRA114_GPIO_H_ */
