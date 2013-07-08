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
 * Jump vector table as in table 3.1 in [1]
 *
 *************************************************************************
 */

.globl _start
_start: b	reset
#ifndef CONFIG_SPL_BUILD
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq

_undefined_instruction:
	.word undefined_instruction
_software_interrupt:
	.word software_interrupt
_prefetch_abort:
	.word prefetch_abort
_data_abort:
	.word data_abort
_not_used:
	.word not_used
_irq:
	.word irq
_fiq:
	.word fiq
_pad:
	.word 0x12345678 /* now 16*4=64 */
#else
	. = _start + 64
#endif

.global _end_vect
_end_vect:
	.balignl 16,0xdeadbeef
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

.globl _TEXT_BASE
_TEXT_BASE:
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_TEXT_BASE)
	.word	CONFIG_SPL_TEXT_BASE
#else
	.word	CONFIG_SYS_TEXT_BASE
#endif

/*
 * These are defined in the board-specific linker script.
 * Subtracting _start from them lets the linker put their
 * relative position in the executable instead of leaving
 * them null.
 */

.globl _bss_start_ofs
_bss_start_ofs:
	.word __bss_start - _start

.globl _bss_end_ofs
_bss_end_ofs:
	.word __bss_end - _start

.globl _end_ofs
_end_ofs:
	.word _end - _start

/* IRQ stack memory (calculated at run-time) + 8 bytes */
.globl IRQ_STACK_START_IN
IRQ_STACK_START_IN:
	.word	0x0badc0de

/*
 * the actual reset code
 */

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

#ifndef CONFIG_SPL_BUILD
/*
 *************************************************************************
 *
 * Interrupt handling
 *
 *************************************************************************
 */
@
@ IRQ stack frame.
@
#define S_FRAME_SIZE	72

#define S_OLD_R0	68
#define S_PSR		64
#define S_PC		60
#define S_LR		56
#define S_SP		52

#define S_IP		48
#define S_FP		44
#define S_R10		40
#define S_R9		36
#define S_R8		32
#define S_R7		28
#define S_R6		24
#define S_R5		20
#define S_R4		16
#define S_R3		12
#define S_R2		8
#define S_R1		4
#define S_R0		0

#define MODE_SVC 0x13
#define I_BIT	 0x80

/*
 * use bad_save_user_regs for abort/prefetch/undef/swi ...
 */

	.macro	bad_save_user_regs
	/* carve out a frame on current user stack */
	sub	sp, sp, #S_FRAME_SIZE
	/* Save user registers (now in svc mode) r0-r12 */
	stmia	sp, {r0 - r12}

	ldr	r2, IRQ_STACK_START_IN
	/* get values for "aborted" pc and cpsr (into parm regs) */
	ldmia	r2, {r2 - r3}
	/* grab pointer to old stack */
	add	r0, sp, #S_FRAME_SIZE

	add	r5, sp, #S_SP
	mov	r1, lr
	/* save sp_SVC, lr_SVC, pc, cpsr */
	stmia	r5, {r0 - r3}
	/* save current stack into r0 (param register) */
	mov	r0, sp
	.endm

	.macro get_bad_stack
	ldr	r13, IRQ_STACK_START_IN		@ setup our mode stack

	/* save caller lr in position 0 of saved stack */
	str	lr, [r13]
	/* get the spsr */
	mrs	lr, spsr
	/* save spsr in position 1 of saved stack */
	str	lr, [r13, #4]

	/* prepare SVC-Mode */
	mov	r13, #MODE_SVC
	@ msr	spsr_c, r13
	/* switch modes, make sure moves will execute */
	msr	spsr, r13
	/* capture return pc */
	mov	lr, pc
	/* jump to next instruction & switch modes. */
	movs	pc, lr
	.endm

	.macro get_bad_stack_swi
	/* space on current stack for scratch reg. */
	sub	r13, r13, #4
	/* save R0's value. */
	str	r0, [r13]
	ldr	r13, IRQ_STACK_START_IN		@ setup our mode stack
	/* save caller lr in position 0 of saved stack */
	str	lr, [r0]
	/* get the spsr */
	mrs	lr, spsr
	/* save spsr in position 1 of saved stack */
	str	lr, [r0, #4]
	/* restore lr */
	ldr	lr, [r0]
	/* restore r0 */
	ldr	r0, [r13]
	/* pop stack entry */
	add	r13, r13, #4
	.endm

/*
 * exception handlers
 */
	.align	5
undefined_instruction:
	get_bad_stack
	bad_save_user_regs
	bl	do_undefined_instruction

	.align	5
software_interrupt:
	get_bad_stack_swi
	bad_save_user_regs
	bl	do_software_interrupt

	.align	5
prefetch_abort:
	get_bad_stack
	bad_save_user_regs
	bl	do_prefetch_abort

	.align	5
data_abort:
	get_bad_stack
	bad_save_user_regs
	bl	do_data_abort

	.align	5
not_used:
	get_bad_stack
	bad_save_user_regs
	bl	do_not_used

	.align	5
irq:
	get_bad_stack
	bad_save_user_regs
	bl	do_irq

	.align	5
fiq:
	get_bad_stack
	bad_save_user_regs
	bl	do_fiq
#endif /* CONFIG_SPL_BUILD */
