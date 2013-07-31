/*
 * Lowlevel setup for SMDK5250 board based on S5PC520
 *
 * Copyright (C) 2012 Samsung Electronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <version.h>
#include <asm/arch/cpu.h>

_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE

	.globl lowlevel_init
lowlevel_init:

	/* use iRAM stack in bl2 */
	ldr	sp, =CONFIG_IRAM_STACK
	stmdb	r13!, {ip,lr}

	/* check reset status */
	ldr	r0, =(EXYNOS5_POWER_BASE + INFORM1_OFFSET)
	ldr	r1, [r0]

	/* AFTR wakeup reset */
	ldr	r2, =S5P_CHECK_DIDLE
	cmp	r1, r2
	beq	exit_wakeup

	/* LPA wakeup reset */
	ldr	r2, =S5P_CHECK_LPA
	cmp	r1, r2
	beq	exit_wakeup

	/* Sleep wakeup reset */
	ldr	r2, =S5P_CHECK_SLEEP
	cmp	r1, r2
	beq	wakeup_reset

	/*
	 * If U-boot is already running in RAM, no need to relocate U-Boot.
	 * Memory controller must be configured before relocating U-Boot
	 * in ram.
	 */
	ldr	r0, =0x0ffffff		/* r0 <- Mask Bits*/
	bic	r1, pc, r0		/* pc <- current addr of code */
					/* r1 <- unmasked bits of pc */
	ldr	r2, _TEXT_BASE		/* r2 <- original base addr in ram */
	bic	r2, r2, r0		/* r2 <- unmasked bits of r2*/
	cmp	r1, r2			/* compare r1, r2 */
	beq	1f			/* r0 == r1 then skip sdram init */

	/* init system clock */
	bl	system_clock_init

	/* Memory initialize */
	bl	mem_ctrl_init

1:
	bl	arch_cpu_init
	bl	tzpc_init
	ldmia	r13!, {ip,pc}

wakeup_reset:
	bl	system_clock_init
	bl	mem_ctrl_init
	bl	arch_cpu_init
	bl	tzpc_init

exit_wakeup:
	/* Load return address and jump to kernel */
	ldr	r0, =(EXYNOS5_POWER_BASE + INFORM0_OFFSET)

	/* r1 = physical address of exynos5_cpu_resume function*/
	ldr	r1, [r0]

	/* Jump to kernel */
	mov	pc, r1
	nop
	nop
