/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 * Author: Wang Dongsheng <dongsheng.wang@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <linux/linkage.h>

#include <asm/armv7.h>
#include <asm/arch-armv7/generictimer.h>
#include <asm/psci.h>

#define SCFG_CORE0_SFT_RST      0x130
#define SCFG_CORESRENCR         0x204

#define DCFG_CCSR_BRR           0x0E4
#define DCFG_CCSR_SCRATCHRW1    0x200

	.pushsection ._secure.text, "ax"

	.arch_extension sec

#define	ONE_MS		(GENERIC_TIMER_CLK / 1000)
#define	RESET_WAIT	(30 * ONE_MS)

	@ r1 = target CPU
	@ r2 = target PC
.globl	psci_cpu_on
psci_cpu_on:
	push	{lr}

	@ Clear and Get the correct CPU number
	@ r1 = 0xf01
	and	r1, r1, #0xff

	mov	r0, r1
	bl	psci_get_cpu_stack_top
	str	r2, [r0]
	dsb

	@ Get DCFG base address
	movw	r4, #(CONFIG_SYS_FSL_GUTS_ADDR & 0xffff)
	movt	r4, #(CONFIG_SYS_FSL_GUTS_ADDR >> 16)

	@ Detect target CPU state
	ldr	r2, [r4, #DCFG_CCSR_BRR]
	rev	r2, r2
	lsr	r2, r2, r1
	ands	r2, r2, #1
	beq	holdoff_release

	@ Reset target CPU
	@ Get SCFG base address
	movw	r0, #(CONFIG_SYS_FSL_SCFG_ADDR & 0xffff)
	movt	r0, #(CONFIG_SYS_FSL_SCFG_ADDR >> 16)

	@ Enable CORE Soft Reset
	movw	r5, #0
	movt	r5, #(1 << 15)
	rev	r5, r5
	str	r5, [r0, #SCFG_CORESRENCR]

	@ Get CPUx offset register
	mov	r6, #0x4
	mul	r6, r6, r1
	add	r2, r0, r6

	@ Do reset on target CPU
	movw	r5, #0
	movt	r5, #(1 << 15)
	rev	r5, r5
	str	r5, [r2, #SCFG_CORE0_SFT_RST]

	@ Wait target CPU up
	timer_wait	r2, RESET_WAIT

	@ Disable CORE soft reset
	mov	r5, #0
	str	r5, [r0, #SCFG_CORESRENCR]

holdoff_release:
	@ Release on target CPU
	ldr	r2, [r4, #DCFG_CCSR_BRR]
	mov	r6, #1
	lsl	r6, r6, r1	@ 32 bytes per CPU

	rev	r6, r6
	orr	r2, r2, r6
	str	r2, [r4, #DCFG_CCSR_BRR]

	@ Set secondary boot entry
	ldr	r6, =psci_cpu_entry
	rev	r6, r6
	str	r6, [r4, #DCFG_CCSR_SCRATCHRW1]

	isb
	dsb

	@ Return
	mov	r0, #ARM_PSCI_RET_SUCCESS

	pop	{lr}
	bx	lr

.globl	psci_cpu_off
psci_cpu_off:
	bl	psci_cpu_off_common

1:	wfi
	b	1b

.globl	psci_arch_init
psci_arch_init:
	mov	r6, lr

	bl	psci_get_cpu_id
	bl	psci_get_cpu_stack_top
	mov	sp, r0

	bx	r6

	.globl psci_text_end
psci_text_end:
	.popsection
