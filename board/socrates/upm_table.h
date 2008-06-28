/*
 * (C) Copyright 2008
 * Sergei Poselenov, Emcraft Systems, sposelenov@emcraft.com.
 *
 * Copyright 2004, 2007 Freescale Semiconductor, Inc.
 * (C) Copyright 2003 Motorola Inc.
 * Xianghua Xiao, (X.Xiao@motorola.com)
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.         See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __UPM_TABLE_H
#define __UPM_TABLE_H

/* UPM Table Configuration Code for FPGA access */
static const unsigned int UPMTableA[] =
{
	0x00fcfc00,  0x00fcfc00,  0x00fcfc00,  0x00fcfc00, /* Words  0 to  3 */
	0x00fcfc00,  0x00fcfc00,  0x00fcfc00,  0x00fcfc05, /* Words  4 to  7 */
	0x00fcfc00,  0x00fcfc00,  0x00fcfc04,  0x00fcfc04, /* Words  8 to 11 */
	0x00fcfc04,  0x00fcfc04,  0x00fcfc04,  0x00fcfc04, /* Words 12 to 15 */
	0x00fcfc04,  0x00fcfc04,  0x00fcfc00,  0xfffffc00, /* Words 16 to 19 */
	0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc01, /* Words 20 to 23 */
	0x0ffffc00,  0x0ffffc00,  0x0ffffc00,  0x00f3fc04, /* Words 24 to 27 */
	0x0ffffc00,  0xfffffc01,  0xfffffc00,  0xfffffc01, /* Words 28 to 31 */
	0x0ffffc00,  0x00f3fc04,  0x00f3fc04,  0x00f3fc04, /* Words 32 to 35 */
	0x00f3fc04,  0x00f3fc04,  0x00f3fc04,  0x00f3fc04, /* Words 36 to 39 */
	0x00f3fc04,  0x0ffffc00,  0xfffffc00,  0xfffffc00, /* Words 40 to 43 */
	0xfffffc01,  0xfffffc00,  0xfffffc00,  0xfffffc01, /* Words 44 to 47 */
	0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc00, /* Words 48 to 51 */
	0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc00, /* Words 52 to 55 */
	0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc01, /* Words 56 to 59 */
	0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc01  /* Words 60 to 63 */
};

#endif
