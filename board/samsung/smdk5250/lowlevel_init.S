/*
 * Lowlevel setup for SMDK5250 board based on S5PC520
 *
 * Copyright (C) 2012 Samsung Electronics
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
	bl	tzpc_init
	ldmia	r13!, {ip,pc}

wakeup_reset:
	bl	system_clock_init
	bl	mem_ctrl_init
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
