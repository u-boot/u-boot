/*
 * (C) Copyright 2011 ST-Ericsson
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <linux/linkage.h>

ENTRY(lowlevel_init)
	mov	pc, lr
ENDPROC(lowlevel_init)

	.align	5
ENTRY(reset_cpu)
	ldr r0, =CFG_PRCMU_BASE
	ldr r1, =0x1
	str r1, [r0, #0x228]
_loop_forever:
	b	_loop_forever
ENDPROC(reset_cpu)
