/*
 *  Copyright (C) 2012 Samsung Electronics
 *  Lukasz Majewski <l.majewski@samsung.com>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __FG_BATTERY_CELL_PARAMS_H_
#define __FG_BATTERY_CELL_PARAMS_H_

#if  defined(CONFIG_POWER_FG_MAX17042) && defined(CONFIG_TRATS)

/* Cell characteristics - Exynos4 TRATS development board */
/* Shall be written to addr 0x80h */
u16 cell_character0[16] = {
	0xA2A0,
	0xB6E0,
	0xB850,
	0xBAD0,
	0xBB20,
	0xBB70,
	0xBBC0,
	0xBC20,
	0xBC80,
	0xBCE0,
	0xBD80,
	0xBE20,
	0xC090,
	0xC420,
	0xC910,
	0xD070
};

/* Shall be written to addr 0x90h */
u16 cell_character1[16] = {
	0x0090,
	0x1A50,
	0x02F0,
	0x2060,
	0x2060,
	0x2E60,
	0x26A0,
	0x2DB0,
	0x2DB0,
	0x1870,
	0x2A20,
	0x16F0,
	0x08F0,
	0x0D40,
	0x08C0,
	0x08C0
};

/* Shall be written to addr 0xA0h */
u16 cell_character2[16] = {
	0x0100,
	0x0100,
	0x0100,
	0x0100,
	0x0100,
	0x0100,
	0x0100,
	0x0100,
	0x0100,
	0x0100,
	0x0100,
	0x0100,
	0x0100,
	0x0100,
	0x0100,
	0x0100
};
#endif
#endif /* __FG_BATTERY_CELL_PARAMS_H_ */
