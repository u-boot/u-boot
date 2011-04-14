/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _TEGRA2_GPIO_H_
#define _TEGRA2_GPIO_H_

/*
 * The Tegra 2x GPIO controller has 222 GPIOs arranged in 8 banks of 4 ports,
 * each with 8 GPIOs.
 */
#define TEGRA_GPIO_PORTS 4   /* The number of ports per bank */
#define TEGRA_GPIO_BANKS 8   /* The number of banks */

/* GPIO Controller registers for a single bank */
struct gpio_ctlr_bank {
	uint gpio_config[TEGRA_GPIO_PORTS];
	uint gpio_dir_out[TEGRA_GPIO_PORTS];
	uint gpio_out[TEGRA_GPIO_PORTS];
	uint gpio_in[TEGRA_GPIO_PORTS];
	uint gpio_int_status[TEGRA_GPIO_PORTS];
	uint gpio_int_enable[TEGRA_GPIO_PORTS];
	uint gpio_int_level[TEGRA_GPIO_PORTS];
	uint gpio_int_clear[TEGRA_GPIO_PORTS];
};

struct gpio_ctlr {
	struct gpio_ctlr_bank gpio_bank[TEGRA_GPIO_BANKS];
};

#define GPIO_BANK(x)	((x) >> 5)
#define GPIO_PORT(x)	(((x) >> 3) & 0x3)
#define GPIO_BIT(x)	((x) & 0x7)

/*
 * GPIO_PI3 = Port I = 8, bit = 3.
 * Seaboard: used for UART/SPI selection
 * Harmony: not used
 */
#define GPIO_PI3	((8 << 3) | 3)

#endif	/* TEGRA2_GPIO_H_ */
