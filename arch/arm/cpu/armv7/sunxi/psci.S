/*
 * Copyright (C) 2013 - ARM Ltd
 * Author: Marc Zyngier <marc.zyngier@arm.com>
 *
 * Based on code by Carl van Schaik <carl@ok-labs.com>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <asm/gic.h>
#include <asm/psci.h>
#include <asm/arch/cpu.h>

/*
 * Memory layout:
 *
 * SECURE_RAM to text_end :
 *	._secure_text section
 * text_end to ALIGN_PAGE(text_end):
 *	nothing
 * ALIGN_PAGE(text_end) to ALIGN_PAGE(text_end) + 0x1000)
 *	1kB of stack per CPU (4 CPUs max).
 */

	.pushsection ._secure.text, "ax"

	.arch_extension sec

#define	ONE_MS			(CONFIG_TIMER_CLK_FREQ / 1000)
#define	TEN_MS			(10 * ONE_MS)
#define	GICD_BASE		0x1c81000
#define	GICC_BASE		0x1c82000

.macro	timer_wait	reg, ticks
	@ Program CNTP_TVAL
	movw	\reg, #(\ticks & 0xffff)
	movt	\reg, #(\ticks >> 16)
	mcr	p15, 0, \reg, c14, c2, 0
	isb
	@ Enable physical timer, mask interrupt
	mov	\reg, #3
	mcr	p15, 0, \reg, c14, c2, 1
	@ Poll physical timer until ISTATUS is on
1:	isb
	mrc	p15, 0, \reg, c14, c2, 1
	ands	\reg, \reg, #4
	bne	1b
	@ Disable timer
	mov	\reg, #0
	mcr	p15, 0, \reg, c14, c2, 1
	isb
.endm

.globl	psci_fiq_enter
psci_fiq_enter:
	push	{r0-r12}

	@ Switch to secure
	mrc	p15, 0, r7, c1, c1, 0
	bic	r8, r7, #1
	mcr	p15, 0, r8, c1, c1, 0
	isb

	@ Validate reason based on IAR and acknowledge
	movw	r8, #(GICC_BASE & 0xffff)
	movt	r8, #(GICC_BASE >> 16)
	ldr	r9, [r8, #GICC_IAR]
	movw	r10, #0x3ff
	movt	r10, #0
	cmp	r9, r10			@ skip spurious interrupt 1023
	beq	out
	movw	r10, #0x3fe		@ ...and 1022
	cmp	r9, r10
	beq	out
	str	r9, [r8, #GICC_EOIR]	@ acknowledge the interrupt
	dsb

	@ Compute CPU number
	lsr	r9, r9, #10
	and	r9, r9, #0xf

	movw	r8, #(SUN7I_CPUCFG_BASE & 0xffff)
	movt	r8, #(SUN7I_CPUCFG_BASE >> 16)

	@ Wait for the core to enter WFI
	lsl	r11, r9, #6		@ x64
	add	r11, r11, r8

1:	ldr	r10, [r11, #0x48]
	tst	r10, #(1 << 2)
	bne	2f
	timer_wait r10, ONE_MS
	b	1b

	@ Reset CPU
2:	mov	r10, #0
	str	r10, [r11, #0x40]

	@ Lock CPU
	mov	r10, #1
	lsl	r9, r10, r9		@ r9 is now CPU mask
	ldr	r10, [r8, #0x1e4]
	bic	r10, r10, r9
	str	r10, [r8, #0x1e4]

	@ Set power gating
	ldr	r10, [r8, #0x1b4]
	orr	r10, r10, #1
	str	r10, [r8, #0x1b4]
	timer_wait r10, ONE_MS

	@ Activate power clamp
	mov	r10, #1
1:	str	r10, [r8, #0x1b0]
	lsl	r10, r10, #1
	orr	r10, r10, #1
	tst	r10, #0x100
	beq	1b

	@ Restore security level
out:	mcr	p15, 0, r7, c1, c1, 0

	pop	{r0-r12}
	subs    pc, lr, #4

	@ r1 = target CPU
	@ r2 = target PC
.globl	psci_cpu_on
psci_cpu_on:
	adr	r0, _target_pc
	str	r2, [r0]
	dsb

	movw	r0, #(SUN7I_CPUCFG_BASE & 0xffff)
	movt	r0, #(SUN7I_CPUCFG_BASE >> 16)

	@ CPU mask
	and	r1, r1, #3	@ only care about first cluster
	mov	r4, #1
	lsl	r4, r4, r1

	adr	r6, _sunxi_cpu_entry
	str	r6, [r0, #0x1a4] @ PRIVATE_REG (boot vector)

	@ Assert reset on target CPU
	mov	r6, #0
	lsl	r5, r1, #6	@ 64 bytes per CPU
	add	r5, r5, #0x40	@ Offset from base
	add	r5, r5, r0	@ CPU control block
	str	r6, [r5]	@ Reset CPU

	@ l1 invalidate
	ldr	r6, [r0, #0x184]
	bic	r6, r6, r4
	str	r6, [r0, #0x184]

	@ Lock CPU
	ldr	r6, [r0, #0x1e4]
	bic	r6, r6, r4
	str	r6, [r0, #0x1e4]

	@ Release power clamp
	movw	r6, #0x1ff
	movt	r6, #0
1:	lsrs	r6, r6, #1
	str	r6, [r0, #0x1b0]
	bne	1b

	timer_wait r1, TEN_MS

	@ Clear power gating
	ldr	r6, [r0, #0x1b4]
	bic	r6, r6, #1
	str	r6, [r0, #0x1b4]

	@ Deassert reset on target CPU
	mov	r6, #3
	str	r6, [r5]

	@ Unlock CPU
	ldr	r6, [r0, #0x1e4]
	orr	r6, r6, r4
	str	r6, [r0, #0x1e4]

	mov	r0, #ARM_PSCI_RET_SUCCESS	@ Return PSCI_RET_SUCCESS
	mov	pc, lr

_target_pc:
	.word	0

/* Imported from Linux kernel */
v7_flush_dcache_all:
	dmb					@ ensure ordering with previous memory accesses
	mrc	p15, 1, r0, c0, c0, 1		@ read clidr
	ands	r3, r0, #0x7000000		@ extract loc from clidr
	mov	r3, r3, lsr #23			@ left align loc bit field
	beq	finished			@ if loc is 0, then no need to clean
	mov	r10, #0				@ start clean at cache level 0
flush_levels:
	add	r2, r10, r10, lsr #1		@ work out 3x current cache level
	mov	r1, r0, lsr r2			@ extract cache type bits from clidr
	and	r1, r1, #7			@ mask of the bits for current cache only
	cmp	r1, #2				@ see what cache we have at this level
	blt	skip				@ skip if no cache, or just i-cache
	mrs     r9, cpsr			@ make cssr&csidr read atomic
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	isb					@ isb to sych the new cssr&csidr
	mrc	p15, 1, r1, c0, c0, 0		@ read the new csidr
	msr     cpsr_c, r9
	and	r2, r1, #7			@ extract the length of the cache lines
	add	r2, r2, #4			@ add 4 (line length offset)
	ldr	r4, =0x3ff
	ands	r4, r4, r1, lsr #3		@ find maximum number on the way size
	clz	r5, r4				@ find bit position of way size increment
	ldr	r7, =0x7fff
	ands	r7, r7, r1, lsr #13		@ extract max number of the index size
loop1:
	mov	r9, r7				@ create working copy of max index
loop2:
	orr	r11, r10, r4, lsl r5		@ factor way and cache number into r11
	orr	r11, r11, r9, lsl r2		@ factor index number into r11
	mcr	p15, 0, r11, c7, c14, 2		@ clean & invalidate by set/way
	subs	r9, r9, #1			@ decrement the index
	bge	loop2
	subs	r4, r4, #1			@ decrement the way
	bge	loop1
skip:
	add	r10, r10, #2			@ increment cache number
	cmp	r3, r10
	bgt	flush_levels
finished:
	mov	r10, #0				@ swith back to cache level 0
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	dsb	st
	isb
	bx	lr

_sunxi_cpu_entry:
	@ Set SMP bit
	mrc	p15, 0, r0, c1, c0, 1
	orr	r0, r0, #0x40
	mcr	p15, 0, r0, c1, c0, 1
	isb

	bl	_nonsec_init
	bl	psci_arch_init

	adr	r0, _target_pc
	ldr	r0, [r0]
	b	_do_nonsec_entry

.globl	psci_cpu_off
psci_cpu_off:
	mrc	p15, 0, r0, c1, c0, 0		@ SCTLR
	bic	r0, r0, #(1 << 2)		@ Clear C bit
	mcr	p15, 0, r0, c1, c0, 0		@ SCTLR
	isb
	dsb

	bl	v7_flush_dcache_all

	clrex					@ Why???

	mrc	p15, 0, r0, c1, c0, 1		@ ACTLR
	bic	r0, r0, #(1 << 6)		@ Clear SMP bit
	mcr	p15, 0, r0, c1, c0, 1		@ ACTLR
	isb
	dsb

	@ Ask CPU0 to pull the rug...
	movw	r0, #(GICD_BASE & 0xffff)
	movt	r0, #(GICD_BASE >> 16)
	movw	r1, #15				@ SGI15
	movt	r1, #1				@ Target is CPU0
	str	r1, [r0, #GICD_SGIR]
	dsb

1:	wfi
	b	1b

.globl	psci_arch_init
psci_arch_init:
	movw	r4, #(GICD_BASE & 0xffff)
	movt	r4, #(GICD_BASE >> 16)

	ldr	r5, [r4, #GICD_IGROUPRn]
	bic	r5, r5, #(1 << 15) 	@ SGI15 as Group-0
	str	r5, [r4, #GICD_IGROUPRn]

	mov	r5, #0			@ Set SGI15 priority to 0
	strb	r5, [r4, #(GICD_IPRIORITYRn + 15)]

	add	r4, r4, #0x1000		@ GICC address

	mov	r5, #0xff
	str	r5, [r4, #GICC_PMR]	@ Be cool with non-secure

	ldr	r5, [r4, #GICC_CTLR]
	orr	r5, r5, #(1 << 3)	@ Switch FIQEn on
	str	r5, [r4, #GICC_CTLR]

	mrc	p15, 0, r5, c1, c1, 0	@ Read SCR
	orr	r5, r5, #4		@ Enable FIQ in monitor mode
	bic	r5, r5, #1		@ Secure mode
	mcr	p15, 0, r5, c1, c1, 0	@ Write SCR
	isb

	mrc	p15, 0, r4, c0, c0, 5	@ MPIDR
	and	r4, r4, #3		@ cpu number in cluster
	mov	r5, #0x400		@ 1kB of stack per CPU
	mul	r4, r4, r5

	adr	r5, text_end		@ end of text
	add	r5, r5, #0x2000		@ Skip two pages
	lsr	r5, r5, #12		@ Align to start of page
	lsl	r5, r5, #12
	sub	sp, r5, r4		@ here's our stack!

	bx	lr

text_end:
	.popsection
