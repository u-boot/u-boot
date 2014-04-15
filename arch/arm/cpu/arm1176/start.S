/*
 *  armboot - Startup Code for ARM1176 CPU-core
 *
 * Copyright (c) 2007	Samsung Electronics
 *
 * Copyright (C) 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * 2007-09-21 - Restructured codes by jsgood (jsgood.yang@samsung.com)
 * 2007-09-21 - Added MoviNAND and OneNAND boot codes by
 * jsgood (jsgood.yang@samsung.com)
 * Base codes by scsuh (sc.suh)
 */

#include <asm-offsets.h>
#include <config.h>
#include <version.h>

#ifndef CONFIG_SYS_PHY_UBOOT_BASE
#define CONFIG_SYS_PHY_UBOOT_BASE	CONFIG_SYS_UBOOT_BASE
#endif

/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from memory!
 * setup Memory and board specific bits prior to relocation.
 * relocate armboot to ram
 * setup stack
 *
 *************************************************************************
 */

	.globl reset

reset:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0, cpsr
	bic	r0, r0, #0x3f
	orr	r0, r0, #0xd3
	msr	cpsr, r0

/*
 *************************************************************************
 *
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 *
 *************************************************************************
 */
	/*
	 * we do sys-critical inits only at reboot,
	 * not when booting from ram!
	 */
cpu_init_crit:
	/*
	 * When booting from NAND - it has definitely been a reset, so, no need
	 * to flush caches and disable the MMU
	 */
#ifndef CONFIG_SPL_BUILD
	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002300	@ clear bits 13, 9:8 (--V- --RS)
	bic	r0, r0, #0x00000087	@ clear bits 7, 2:0 (B--- -CAM)
	orr	r0, r0, #0x00000002	@ set bit 2 (A) Align
	orr	r0, r0, #0x00001000	@ set bit 12 (I) I-Cache

	/* Prepare to disable the MMU */
	adr	r2, mmu_disable_phys
	sub	r2, r2, #(CONFIG_SYS_PHY_UBOOT_BASE - CONFIG_SYS_TEXT_BASE)
	b	mmu_disable

	.align 5
	/* Run in a single cache-line */
mmu_disable:
	mcr	p15, 0, r0, c1, c0, 0
	nop
	nop
	mov	pc, r2
mmu_disable_phys:

#ifdef CONFIG_DISABLE_TCM
	/*
	 * Disable the TCMs
	 */
	mrc	p15, 0, r0, c0, c0, 2	/* Return TCM details */
	cmp	r0, #0
	beq	skip_tcmdisable
	mov	r1, #0
	mov	r2, #1
	tst	r0, r2
	mcrne	p15, 0, r1, c9, c1, 1	/* Disable Instruction TCM if present*/
	tst	r0, r2, LSL #16
	mcrne	p15, 0, r1, c9, c1, 0	/* Disable Data TCM if present*/
skip_tcmdisable:
#endif
#endif

#ifdef CONFIG_PERIPORT_REMAP
	/* Peri port setup */
	ldr	r0, =CONFIG_PERIPORT_BASE
	orr	r0, r0, #CONFIG_PERIPORT_SIZE
	mcr	p15,0,r0,c15,c2,4
#endif

	/*
	 * Go setup Memory and board specific bits prior to relocation.
	 */
	bl	lowlevel_init		/* go setup pll,mux,memory */

	bl	_main

/*------------------------------------------------------------------------------*/

	.globl	c_runtime_cpu_setup
c_runtime_cpu_setup:

	mov	pc, lr
