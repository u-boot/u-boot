/*
 * vme8349pin.h -- esd VME8349 MPC8349 I/O pin definition.
 * Copyright (c) 2009 esd gmbh.
 *
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#ifndef __VME8349PIN_H__
#define __VME8349PIN_H__

#define GPIO2_V_SCON		0x80000000 /* In:  from tsi148 1: is syscon */
#define GPIO2_VME_RESET_N	0x20000000 /* Out: to tsi148                */
#define GPIO2_TSI_PLL_RESET_N	0x08000000 /* Out: to tsi148                */
#define GPIO2_TSI_POWERUP_RESET_N 0x00800000 /* Out: to tsi148              */
#define GPIO2_L_RESET_EN_N	0x00100000 /* Out: 0:vme can assert cpu lrst*/

#endif /* of ifndef __VME8349PIN_H__ */
