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

#ifndef _TEGRA_GPIO_H_
#define _TEGRA_GPIO_H_

#define MAX_NUM_GPIOS           (TEGRA_GPIO_PORTS * TEGRA_GPIO_BANKS * 8)
#define GPIO_NAME_SIZE		20	/* gpio_request max label len */

#define GPIO_BANK(x)		((x) >> 5)
#define GPIO_PORT(x)		(((x) >> 3) & 0x3)
#define GPIO_FULLPORT(x)	((x) >> 3)
#define GPIO_BIT(x)		((x) & 0x7)

/*
 * Tegra-specific GPIO API
 */

void gpio_info(void);

#define gpio_status()	gpio_info()
#endif	/* TEGRA_GPIO_H_ */
