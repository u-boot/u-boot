/*
 * Copyright (C) 2012
 * David Purdy <david.c.purdy@gmail.com>
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
 * along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __POGO_E02_H
#define __POGO_E02_H

/* GPIO configuration */
#define POGO_E02_OE_LOW				(~(0))
#define POGO_E02_OE_HIGH			(~(0))
#define POGO_E02_OE_VAL_LOW			(1 << 29)
#define POGO_E02_OE_VAL_HIGH			0

/* PHY related */
#define MV88E1116_LED_FCTRL_REG			10
#define MV88E1116_CPRSP_CR3_REG			21
#define MV88E1116_MAC_CTRL_REG			21
#define MV88E1116_PGADR_REG			22
#define MV88E1116_RGMII_TXTM_CTRL		(1 << 4)
#define MV88E1116_RGMII_RXTM_CTRL		(1 << 5)

#endif /* __POGO_E02_H */
