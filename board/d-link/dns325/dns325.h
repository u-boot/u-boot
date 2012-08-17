/*
 * Copyright (C) 2011
 * Stefan Herbrechtsmeier <stefan@code.herbrechtsmeier.net>
 *
 * Based on Kirkwood support:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef __DNS325_H
#define __DNS325_H

/* GPIO configuration */
#define DNS325_OE_LOW			0x00000000
#define DNS325_OE_HIGH			0x00039604
#define DNS325_OE_VAL_LOW		0x38000000	/* disable leds */
#define DNS325_OE_VAL_HIGH		0x00000800	/* disable leds */

#define DNS325_GPIO_LED_POWER		26
#define DNS325_GPIO_SATA0_EN		39
#define DNS325_GPIO_SATA1_EN		40

/* PHY related */
#define MV88E1116_MAC_CTRL_REG		21
#define MV88E1116_PGADR_REG		22
#define MV88E1116_RGMII_TXTM_CTRL	(1 << 4)
#define MV88E1116_RGMII_RXTM_CTRL	(1 << 5)

#endif /* __DNS325_H */
