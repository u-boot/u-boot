/*
 * SoC-specific setup info
 *
 * (C) Copyright 2010,2011
 * NVIDIA Corporation <www.nvidia.com>
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

#include <config.h>
#include <version.h>
#include <linux/linkage.h>

	.align	5
ENTRY(reset_cpu)
	ldr	r1, rstctl			@ get addr for global reset
						@ reg
	ldr	r3, [r1]
	orr	r3, r3, #0x10
	str	r3, [r1]			@ force reset
	mov	r0, r0
_loop_forever:
	b	_loop_forever
rstctl:
	.word	PRM_RSTCTRL
ENDPROC(reset_cpu)
