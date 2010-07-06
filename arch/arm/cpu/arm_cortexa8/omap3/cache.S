/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * This file is based on and replaces the existing cache.c file
 * The copyrights for the cache.c file are:
 *
 * (C) Copyright 2008 Texas Insturments
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
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

#include <asm/arch/omap3.h>

/*
 * omap3 cache code
 */

.align 5
.global invalidate_dcache
.global l2_cache_enable
.global l2_cache_disable

/*
 *	invalidate_dcache()
 *
 *	Invalidate the whole D-cache.
 *
 *	Corrupted registers: r0-r5, r7, r9-r11
 *
 *	- mm	- mm_struct describing address space
 */
invalidate_dcache:
	stmfd	r13!, {r0 - r5, r7, r9 - r12, r14}

	mov	r7, r0				@ take a backup of device type
	cmp	r0, #0x3			@ check if the device type is
						@ GP
	moveq r12, #0x1				@ set up to invalide L2
smi:	.word 0x01600070			@ Call SMI monitor (smieq)
	cmp	r7, #0x3			@ compare again in case its
						@ lost
	beq	finished_inval			@ if GP device, inval done
						@ above

	mrc	p15, 1, r0, c0, c0, 1		@ read clidr
	ands	r3, r0, #0x7000000		@ extract loc from clidr
	mov	r3, r3, lsr #23			@ left align loc bit field
	beq	finished_inval			@ if loc is 0, then no need to
						@ clean
	mov	r10, #0				@ start clean at cache level 0
inval_loop1:
	add	r2, r10, r10, lsr #1		@ work out 3x current cache
						@ level
	mov	r1, r0, lsr r2			@ extract cache type bits from
						@ clidr
	and	r1, r1, #7			@ mask of the bits for current
						@ cache only
	cmp	r1, #2				@ see what cache we have at
						@ this level
	blt	skip_inval			@ skip if no cache, or just
						@ i-cache
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level
						@ in cssr
	mov	r2, #0				@ operand for mcr SBZ
	mcr	p15, 0, r2, c7, c5, 4		@ flush prefetch buffer to
						@ sych the new cssr&csidr,
						@ with armv7 this is 'isb',
						@ but we compile with armv5
	mrc	p15, 1, r1, c0, c0, 0		@ read the new csidr
	and	r2, r1, #7			@ extract the length of the
						@ cache lines
	add	r2, r2, #4			@ add 4 (line length offset)
	ldr	r4, =0x3ff
	ands	r4, r4, r1, lsr #3		@ find maximum number on the
						@ way size
	clz	r5, r4				@ find bit position of way
						@ size increment
	ldr	r7, =0x7fff
	ands	r7, r7, r1, lsr #13		@ extract max number of the
						@ index size
inval_loop2:
	mov	r9, r4				@ create working copy of max
						@ way size
inval_loop3:
	orr	r11, r10, r9, lsl r5		@ factor way and cache number
						@ into r11
	orr	r11, r11, r7, lsl r2		@ factor index number into r11
	mcr	p15, 0, r11, c7, c6, 2		@ invalidate by set/way
	subs	r9, r9, #1			@ decrement the way
	bge	inval_loop3
	subs	r7, r7, #1			@ decrement the index
	bge	inval_loop2
skip_inval:
	add	r10, r10, #2			@ increment cache number
	cmp	r3, r10
	bgt	inval_loop1
finished_inval:
	mov	r10, #0				@ swith back to cache level 0
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level
						@ in cssr
	mcr	p15, 0, r10, c7, c5, 4		@ flush prefetch buffer,
						@ with armv7 this is 'isb',
						@ but we compile with armv5

	ldmfd	r13!, {r0 - r5, r7, r9 - r12, pc}


l2_cache_enable:
	stmfd	r13!, {r0, r1, r2, lr}
	@ ES2 onwards we can disable/enable L2 ourselves
	bl	get_cpu_rev
	cmp	r0, #CPU_3XX_ES20
	blt	l2_cache_disable_EARLIER_THAN_ES2
	mrc	15, 0, r3, cr1, cr0, 1
	orr	r3, r3, #2
	mcr	15, 0, r3, cr1, cr0, 1
	b	l2_cache_enable_END
l2_cache_enable_EARLIER_THAN_ES2:
	@ Save r0, r12 and restore them after usage
	mov	r3, ip
	str	r3, [sp, #4]
	mov	r3, r0
	@
	@ GP Device ROM code API usage here
	@ r12 = AUXCR Write function and r0 value
	@
	mov	ip, #3
	mrc	15, 0, r0, cr1, cr0, 1
	orr	r0, r0, #2
	@ SMI instruction to call ROM Code API
	.word	0xe1600070
	mov	r0, r3
	mov	ip, r3
	str	r3, [sp, #4]
l2_cache_enable_END:
	ldmfd	r13!, {r1, r2, r3, pc}


l2_cache_disable:
	stmfd	r13!, {r0, r1, r2, lr}
	@ ES2 onwards we can disable/enable L2 ourselves
	bl	get_cpu_rev
	cmp	r0, #CPU_3XX_ES20
	blt	l2_cache_disable_EARLIER_THAN_ES2
	mrc	15, 0, r3, cr1, cr0, 1
	bic	r3, r3, #2
	mcr	15, 0, r3, cr1, cr0, 1
	b	l2_cache_disable_END
l2_cache_disable_EARLIER_THAN_ES2:
	@ Save r0, r12 and restore them after usage
	mov	r3, ip
	str	r3, [sp, #4]
	mov	r3, r0
	@
	@ GP Device ROM code API usage here
	@ r12 = AUXCR Write function and r0 value
	@
	mov	ip, #3
	mrc	15, 0, r0, cr1, cr0, 1
	bic	r0, r0, #2
	@ SMI instruction to call ROM Code API
	.word	0xe1600070
	mov	r0, r3
	mov	ip, r3
	str	r3, [sp, #4]
l2_cache_disable_END:
	ldmfd	r13!, {r1, r2, r3, pc}
