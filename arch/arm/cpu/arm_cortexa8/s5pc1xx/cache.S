/*
 * Copyright (C) 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 *
 * based on arch/arm/cpu/arm_cortexa8/omap3/cache.S
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

#include <asm/arch/cpu.h>

.align 5
.global invalidate_dcache
.global l2_cache_enable
.global l2_cache_disable

/*
 * invalidate_dcache()
 * Invalidate the whole D-cache.
 *
 * Corrupted registers: r0-r5, r7, r9-r11
 */
invalidate_dcache:
	stmfd	r13!, {r0 - r5, r7, r9 - r12, r14}

	cmp	r0, #0xC100			@ check if the cpu is s5pc100

	beq	finished_inval			@ s5pc100 doesn't need this
						@ routine
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
	push	{r0, r1, r2, lr}
	mrc	15, 0, r3, cr1, cr0, 1
	orr	r3, r3, #2
	mcr	15, 0, r3, cr1, cr0, 1
	pop	{r1, r2, r3, pc}

l2_cache_disable:
	push	{r0, r1, r2, lr}
	mrc	15, 0, r3, cr1, cr0, 1
	bic	r3, r3, #2
	mcr	15, 0, r3, cr1, cr0, 1
	pop	{r1, r2, r3, pc}
