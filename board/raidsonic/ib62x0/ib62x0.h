/*
 * Copyright (C) 2011-2012
 * Gerald Kerma <dreagle@doukki.net>
 * Simon Baatz <gmbnomis@gmail.com>
 * Luka Perkov <uboot@lukaperkov.net>
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __IB62x0_H
#define __IB62x0_H

#define IB62x0_OE_LOW		(~(1 << 22 | 1 << 24 | 1 << 25 | 1 << 27))
#define IB62x0_OE_HIGH		(~(0))
#define IB62x0_OE_VAL_LOW	0
#define IB62x0_OE_VAL_HIGH	0

/* PHY related */
#define MV88E1116_LED_FCTRL_REG		10
#define MV88E1116_CPRSP_CR3_REG		21
#define MV88E1116_MAC_CTRL_REG		21
#define MV88E1116_PGADR_REG		22
#define MV88E1116_RGMII_TXTM_CTRL	(1 << 4)
#define MV88E1116_RGMII_RXTM_CTRL	(1 << 5)

/* SATAHC related */
#define MVSATAHC_LED_CONF_REG       (MV_SATA_BASE + 0x2C)
#define MVSATAHC_LED_POLARITY_CTRL  (1 << 3)

#endif /* __IB62x0_H */
