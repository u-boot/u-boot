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

#define	ONE_MS			(CONFIG_SYS_CLK_FREQ / 1000)
#define	TEN_MS			(10 * ONE_MS)

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

.globl	psci_arch_init
psci_arch_init:
	mrc	p15, 0, r5, c1, c1, 0	@ Read SCR
	bic	r5, r5, #1		@ Secure mode
	mcr	p15, 0, r5, c1, c1, 0	@ Write SCR
	isb

	mrc	p15, 0, r4, c0, c0, 5	@ MPIDR
	and	r4, r4, #3		@ cpu number in cluster
	mov	r5, #400		@ 1kB of stack per CPU
	mul	r4, r4, r5

	adr	r5, text_end		@ end of text
	add	r5, r5, #0x2000		@ Skip two pages
	lsr	r5, r5, #12		@ Align to start of page
	lsl	r5, r5, #12
	sub	sp, r5, r4		@ here's our stack!

	bx	lr

	@ r1 = target CPU
	@ r2 = target PC
.globl	psci_cpu_on
psci_cpu_on:
	adr	r0, _target_pc
	str	r2, [r0]
	dsb

	movw	r0, #(SUNXI_CPUCFG_BASE & 0xffff)
	movt	r0, #(SUNXI_CPUCFG_BASE >> 16)

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

text_end:
	.popsection
