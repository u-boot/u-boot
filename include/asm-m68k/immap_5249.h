/*
 * MCF5249 Internal Memory Map
 *
 * Copyright (c) 2003 Josef Baumgartner <josef.baumgartner@telex.de>
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
 */

#ifndef __IMMAP_5249__
#define __IMMAP_5249__

/* Timer module registers
 */
typedef struct timer_ctrl {
	ushort	timer_tmr;
	ushort	res1;
	ushort	timer_trr;
	ushort	res2;
	ushort	timer_tcap;
	ushort	res3;
	ushort	timer_tcn;
	ushort	res4;
	ushort	timer_ter;
	uchar	res5[14];
} timer_t;

#endif /* __IMMAP_5249__ */
