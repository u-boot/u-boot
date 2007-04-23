/*
 * U-boot - segment.h
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef _BLACKFIN_SEGMENT_H
#define _BLACKFIN_SEGMENT_H

/* define constants */
typedef unsigned long mm_segment_t;	/* domain register */

#define KERNEL_CS		0x0
#define KERNEL_DS		0x0
#define __KERNEL_CS		0x0
#define __KERNEL_DS		0x0

#define USER_CS			0x1
#define USER_DS			0x1
#define __USER_CS		0x1
#define __USER_DS		0x1

#define get_ds()		(KERNEL_DS)
#define get_fs()		(__USER_DS)
#define segment_eq(a,b)		((a) == (b))
#define set_fs(val)

#endif
