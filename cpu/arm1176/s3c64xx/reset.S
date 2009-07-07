/*
 * Copyright (c) 2009 Samsung Electronics.
 * Minkyu Kang <mk7.kang@samsung.com>
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

#include <s3c6400.h>

.globl reset_cpu
reset_cpu:
	ldr	r1, =ELFIN_CLOCK_POWER_BASE
	ldr	r2, [r1, #SYS_ID_OFFSET]
	ldr	r3, =0xffff
	and	r2, r3, r2, lsr #12
	str	r2, [r1, #SW_RST_OFFSET]
_loop_forever:
	b	_loop_forever
