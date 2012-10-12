/*
 * Copyright (C) 2012 Nobuhiro Iwamatsu <nobuhiro.Iwamatsu.yj@renesas.com>
 * Copyright (C) 2012 Renesas Solutions Corp.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <linux/linkage.h>

ENTRY(lowlevel_init)
	ldr		r0, =MERAM_BASE
	mov		r1, #0x0
	str		r1, [r0]

	mrc		p15, 0, r0, c0, c0, 5
	ands	r0, r0, #0xF
	beq		lowlevel_init__
	b		wait_interrupt

	.pool
	.align 4

wait_interrupt:
#ifdef ICCICR
	ldr     r1, =ICCICR
	mov     r2, #0x0
	str     r2, [r1]
	mov     r2, #0xF0
	adds    r1, r1, #4 /* ICCPMR */
	str     r2, [r1]
	ldr     r1, =ICCICR
	mov     r2, #0x1
	str     r2, [r1]
#endif

wait_loop:
	.long	0xE320F003 /* wfi */

	ldr		r2, [r1, #0xC]
	str		r2, [r1, #0x10]

	ldr		r0, =MERAM_BASE
	ldr		r2, [r0]
	cmp		r2, #0
	movne	pc, r2

	b		wait_loop

wait_loop_end:
	.pool
	.align 4

lowlevel_init__:

	mov r0, #0x200000

loop0:
	subs r0, r0, #1
	bne  loop0

	ldr sp, MERAM_STACK
	b s_init

	.pool
	.align 4

ENDPROC(lowlevel_init)
	.ltorg

MERAM_STACK:
	.word LOW_LEVEL_MERAM_STACK
