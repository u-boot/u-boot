/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 * See file CREDITS for list of people who contributed to this
 * project.
 * Portions Copyright 2011-2012 NVIDIA Corporation
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

/*
 * The Tegra 2x GPIO controller has 224 GPIOs arranged in 7 banks of 4 ports,
 * each with 8 GPIOs.
 */
#define TEGRA_GPIO_PORTS	4	/* number of ports per bank */
#define TEGRA_GPIO_BANKS	7	/* number of banks */
#define MAX_NUM_GPIOS		(TEGRA_GPIO_PORTS * TEGRA_GPIO_BANKS * 8)
#define GPIO_NAME_SIZE		20	/* gpio_request max label len */

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

#define GPIO_BANK(x)		((x) >> 5)
#define GPIO_PORT(x)		(((x) >> 3) & 0x3)
#define GPIO_FULLPORT(x)	((x) >> 3)
#define GPIO_BIT(x)		((x) & 0x7)

enum gpio_pin {
	GPIO_PA0 = 0,	/* pin 0 */
	GPIO_PA1,
	GPIO_PA2,
	GPIO_PA3,
	GPIO_PA4,
	GPIO_PA5,
	GPIO_PA6,
	GPIO_PA7,
	GPIO_PB0,	/* pin 8 */
	GPIO_PB1,
	GPIO_PB2,
	GPIO_PB3,
	GPIO_PB4,
	GPIO_PB5,
	GPIO_PB6,
	GPIO_PB7,
	GPIO_PC0,	/* pin 16 */
	GPIO_PC1,
	GPIO_PC2,
	GPIO_PC3,
	GPIO_PC4,
	GPIO_PC5,
	GPIO_PC6,
	GPIO_PC7,
	GPIO_PD0,	/* pin 24 */
	GPIO_PD1,
	GPIO_PD2,
	GPIO_PD3,
	GPIO_PD4,
	GPIO_PD5,
	GPIO_PD6,
	GPIO_PD7,
	GPIO_PE0,	/* pin 32 */
	GPIO_PE1,
	GPIO_PE2,
	GPIO_PE3,
	GPIO_PE4,
	GPIO_PE5,
	GPIO_PE6,
	GPIO_PE7,
	GPIO_PF0,	/* pin 40 */
	GPIO_PF1,
	GPIO_PF2,
	GPIO_PF3,
	GPIO_PF4,
	GPIO_PF5,
	GPIO_PF6,
	GPIO_PF7,
	GPIO_PG0,	/* pin 48 */
	GPIO_PG1,
	GPIO_PG2,
	GPIO_PG3,
	GPIO_PG4,
	GPIO_PG5,
	GPIO_PG6,
	GPIO_PG7,
	GPIO_PH0,	/* pin 56 */
	GPIO_PH1,
	GPIO_PH2,
	GPIO_PH3,
	GPIO_PH4,
	GPIO_PH5,
	GPIO_PH6,
	GPIO_PH7,
	GPIO_PI0,	/* pin 64 */
	GPIO_PI1,
	GPIO_PI2,
	GPIO_PI3,
	GPIO_PI4,
	GPIO_PI5,
	GPIO_PI6,
	GPIO_PI7,
	GPIO_PJ0,	/* pin 72 */
	GPIO_PJ1,
	GPIO_PJ2,
	GPIO_PJ3,
	GPIO_PJ4,
	GPIO_PJ5,
	GPIO_PJ6,
	GPIO_PJ7,
	GPIO_PK0,	/* pin 80 */
	GPIO_PK1,
	GPIO_PK2,
	GPIO_PK3,
	GPIO_PK4,
	GPIO_PK5,
	GPIO_PK6,
	GPIO_PK7,
	GPIO_PL0,	/* pin 88 */
	GPIO_PL1,
	GPIO_PL2,
	GPIO_PL3,
	GPIO_PL4,
	GPIO_PL5,
	GPIO_PL6,
	GPIO_PL7,
	GPIO_PM0,	/* pin 96 */
	GPIO_PM1,
	GPIO_PM2,
	GPIO_PM3,
	GPIO_PM4,
	GPIO_PM5,
	GPIO_PM6,
	GPIO_PM7,
	GPIO_PN0,	/* pin 104 */
	GPIO_PN1,
	GPIO_PN2,
	GPIO_PN3,
	GPIO_PN4,
	GPIO_PN5,
	GPIO_PN6,
	GPIO_PN7,
	GPIO_PO0,	/* pin 112 */
	GPIO_PO1,
	GPIO_PO2,
	GPIO_PO3,
	GPIO_PO4,
	GPIO_PO5,
	GPIO_PO6,
	GPIO_PO7,
	GPIO_PP0,	/* pin 120 */
	GPIO_PP1,
	GPIO_PP2,
	GPIO_PP3,
	GPIO_PP4,
	GPIO_PP5,
	GPIO_PP6,
	GPIO_PP7,
	GPIO_PQ0,	/* pin 128 */
	GPIO_PQ1,
	GPIO_PQ2,
	GPIO_PQ3,
	GPIO_PQ4,
	GPIO_PQ5,
	GPIO_PQ6,
	GPIO_PQ7,
	GPIO_PR0,	/* pin 136 */
	GPIO_PR1,
	GPIO_PR2,
	GPIO_PR3,
	GPIO_PR4,
	GPIO_PR5,
	GPIO_PR6,
	GPIO_PR7,
	GPIO_PS0,	/* pin 144 */
	GPIO_PS1,
	GPIO_PS2,
	GPIO_PS3,
	GPIO_PS4,
	GPIO_PS5,
	GPIO_PS6,
	GPIO_PS7,
	GPIO_PT0,	/* pin 152 */
	GPIO_PT1,
	GPIO_PT2,
	GPIO_PT3,
	GPIO_PT4,
	GPIO_PT5,
	GPIO_PT6,
	GPIO_PT7,
	GPIO_PU0,	/* pin 160 */
	GPIO_PU1,
	GPIO_PU2,
	GPIO_PU3,
	GPIO_PU4,
	GPIO_PU5,
	GPIO_PU6,
	GPIO_PU7,
	GPIO_PV0,	/* pin 168 */
	GPIO_PV1,
	GPIO_PV2,
	GPIO_PV3,
	GPIO_PV4,
	GPIO_PV5,
	GPIO_PV6,
	GPIO_PV7,
	GPIO_PW0,	/* pin 176 */
	GPIO_PW1,
	GPIO_PW2,
	GPIO_PW3,
	GPIO_PW4,
	GPIO_PW5,
	GPIO_PW6,
	GPIO_PW7,
	GPIO_PX0,	/* pin 184 */
	GPIO_PX1,
	GPIO_PX2,
	GPIO_PX3,
	GPIO_PX4,
	GPIO_PX5,
	GPIO_PX6,
	GPIO_PX7,
	GPIO_PY0,	/* pin 192 */
	GPIO_PY1,
	GPIO_PY2,
	GPIO_PY3,
	GPIO_PY4,
	GPIO_PY5,
	GPIO_PY6,
	GPIO_PY7,
	GPIO_PZ0,	/* pin 200 */
	GPIO_PZ1,
	GPIO_PZ2,
	GPIO_PZ3,
	GPIO_PZ4,
	GPIO_PZ5,
	GPIO_PZ6,
	GPIO_PZ7,
	GPIO_PAA0,	/* pin 208 */
	GPIO_PAA1,
	GPIO_PAA2,
	GPIO_PAA3,
	GPIO_PAA4,
	GPIO_PAA5,
	GPIO_PAA6,
	GPIO_PAA7,
	GPIO_PBB0,	/* pin 216 */
	GPIO_PBB1,
	GPIO_PBB2,
	GPIO_PBB3,
	GPIO_PBB4,
	GPIO_PBB5,
	GPIO_PBB6,
	GPIO_PBB7,	/* pin 223 */
};

/*
 * Tegra2-specific GPIO API
 */

void gpio_info(void);

#define gpio_status()	gpio_info()
#endif	/* TEGRA_GPIO_H_ */
