/*
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
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

#ifndef __BOARD_FREESCALE_MX51_EVK_H__
#define __BOARD_FREESCALE_MX51_EVK_H__

#ifndef __ASSEMBLY__
struct io_board_ctrl {
	u16 led_ctrl;		/* 0x00 */
	u16 resv1[0x03];
	u16 sb_stat;		/* 0x08 */
	u16 resv2[0x03];
	u16 int_stat;		/* 0x10 */
	u16 resv3[0x07];
	u16 int_rest;		/* 0x20 */
	u16 resv4[0x0B];
	u16 int_mask;		/* 0x38 */
	u16 resv5[0x03];
	u16 id1;		/* 0x40 */
	u16 resv6[0x03];
	u16 id2;		/* 0x48 */
	u16 resv7[0x03];
	u16 version;		/* 0x50 */
	u16 resv8[0x03];
	u16 id3;		/* 0x58 */
	u16 resv9[0x03];
	u16 sw_reset;		/* 0x60 */
};
#endif

#endif
