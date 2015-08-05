/*
 * Copyright (C) 2015 - Chen-Yu Tsai
 * Author: Chen-Yu Tsai <wens@csie.org>
 *
 * Based on psci_sun7i.S by Marc Zyngier <marc.zyngier@arm.com>
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
#include <asm/macro.h>
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

	movw	r8, #(SUN6I_CPUCFG_BASE & 0xffff)
	movt	r8, #(SUN6I_CPUCFG_BASE >> 16)

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
	lsl	r11, r10, r9		@ r11 is now CPU mask
	ldr	r10, [r8, #0x1e4]
	bic	r10, r10, r11
	str	r10, [r8, #0x1e4]

	movw	r8, #(SUNXI_PRCM_BASE & 0xffff)
	movt	r8, #(SUNXI_PRCM_BASE >> 16)

	@ Set power gating
	ldr	r10, [r8, #0x100]
	orr	r10, r10, r11
	str	r10, [r8, #0x100]
	timer_wait r10, ONE_MS

#ifdef CONFIG_MACH_SUN6I
	@ Activate power clamp
	lsl	r12, r9, #2		@ x4
	add	r12, r12, r8
	mov	r10, #0xff
	str	r10, [r12, #0x140]
#endif

	movw	r8, #(SUN6I_CPUCFG_BASE & 0xffff)
	movt	r8, #(SUN6I_CPUCFG_BASE >> 16)

	@ Unlock CPU
	ldr	r10, [r8, #0x1e4]
	orr	r10, r10, r11
	str	r10, [r8, #0x1e4]

	@ Restore security level
out:	mcr	p15, 0, r7, c1, c1, 0

	pop	{r0-r12}
	subs    pc, lr, #4

	@ r1 = target CPU
	@ r2 = target PC
.globl	psci_cpu_on
psci_cpu_on:
	push	{lr}

	mov	r0, r1
	bl	psci_get_cpu_stack_top	@ get stack top of target CPU
	str	r2, [r0]		@ store target PC at stack top
	dsb

	movw	r0, #(SUN6I_CPUCFG_BASE & 0xffff)
	movt	r0, #(SUN6I_CPUCFG_BASE >> 16)

	@ CPU mask
	and	r1, r1, #3	@ only care about first cluster
	mov	r4, #1
	lsl	r4, r4, r1

	ldr	r6, =psci_cpu_entry
	str	r6, [r0, #0x1a4] @ PRIVATE_REG (boot vector)

	@ Assert reset on target CPU
	mov	r6, #0
	lsl	r5, r1, #6	@ 64 bytes per CPU
	add	r5, r5, #0x40	@ Offset from base
	add	r5, r5, r0	@ CPU control block
	str	r6, [r5]	@ Reset CPU

	@ l1 invalidate
	ldr	r6, [r0, #0x184] @ CPUCFG_GEN_CTRL_REG
	bic	r6, r6, r4
	str	r6, [r0, #0x184]

	@ Lock CPU (Disable external debug access)
	ldr	r6, [r0, #0x1e4] @ CPUCFG_DBG_CTL1_REG
	bic	r6, r6, r4
	str	r6, [r0, #0x1e4]

	movw	r0, #(SUNXI_PRCM_BASE & 0xffff)
	movt	r0, #(SUNXI_PRCM_BASE >> 16)

#ifdef CONFIG_MACH_SUN6I
	@ Release power clamp
	lsl	r5, r1, #2	@ 1 register per CPU
	add	r5, r5, r0	@ PRCM
	movw	r6, #0x1ff
	movt	r6, #0
1:	lsrs	r6, r6, #1
	str	r6, [r5, #0x140] @ CPUx_PWR_CLAMP
	bne	1b
#endif

	timer_wait r6, TEN_MS

	@ Clear power gating
	ldr	r6, [r0, #0x100] @ CPU_PWROFF_GATING
	bic	r6, r6, r4
	str	r6, [r0, #0x100]

	@ re-calculate CPU control register address
	movw	r0, #(SUN6I_CPUCFG_BASE & 0xffff)
	movt	r0, #(SUN6I_CPUCFG_BASE >> 16)

	@ Deassert reset on target CPU
	mov	r6, #3
	lsl	r5, r1, #6	@ 64 bytes per CPU
	add	r5, r5, #0x40	@ Offset from base
	add	r5, r5, r0	@ CPU control block
	str	r6, [r5]

	@ Unlock CPU (Enable external debug access)
	ldr	r6, [r0, #0x1e4] @ CPUCFG_DBG_CTL1_REG
	orr	r6, r6, r4
	str	r6, [r0, #0x1e4]

	mov	r0, #ARM_PSCI_RET_SUCCESS	@ Return PSCI_RET_SUCCESS
	pop	{pc}

.globl	psci_cpu_off
psci_cpu_off:
	bl	psci_cpu_off_common

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
	mov	r6, lr

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

	bl	psci_get_cpu_id		@ CPU ID => r0
	bl	psci_get_cpu_stack_top	@ stack top => r0
	mov	sp, r0

	bx	r6

	.globl psci_text_end
psci_text_end:
	.popsection
