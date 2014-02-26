/*
 *  armboot - Startup Code for XScale CPU-core
 *
 *  Copyright (C) 1998	Dan Malek <dmalek@jlc.net>
 *  Copyright (C) 1999	Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 *  Copyright (C) 2000	Wolfgang Denk <wd@denx.de>
 *  Copyright (C) 2001	Alex Zuepke <azu@sysgo.de>
 *  Copyright (C) 2001	Marius Groger <mag@sysgo.de>
 *  Copyright (C) 2002	Alex Zupke <azu@sysgo.de>
 *  Copyright (C) 2002	Gary Jennejohn <garyj@denx.de>
 *  Copyright (C) 2002	Kyle Harris <kharris@nexus-tech.net>
 *  Copyright (C) 2003	Kai-Uwe Bloem <kai-uwe.bloem@auerswald.de>
 *  Copyright (C) 2003	Kshitij <kshitij@ti.com>
 *  Copyright (C) 2003	Richard Woodruff <r-woodruff2@ti.com>
 *  Copyright (C) 2003	Robert Schwebel <r.schwebel@pengutronix.de>
 *  Copyright (C) 2004	Texas Instruments <r-woodruff2@ti.com>
 *  Copyright (C) 2010	Marek Vasut <marek.vasut@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <version.h>

#ifdef CONFIG_CPU_PXA25X
#if ((CONFIG_SYS_INIT_SP_ADDR) != 0xfffff800)
#error "Init SP address must be set to 0xfffff800 for PXA250"
#endif
#endif

.globl _start
_start: b	reset
#ifdef CONFIG_SPL_BUILD
	ldr	pc, _hang
	ldr	pc, _hang
	ldr	pc, _hang
	ldr	pc, _hang
	ldr	pc, _hang
	ldr	pc, _hang
	ldr	pc, _hang

_hang:
	.word	do_hang
	.word	0x12345678
	.word	0x12345678
	.word	0x12345678
	.word	0x12345678
	.word	0x12345678
	.word	0x12345678
	.word	0x12345678	/* now 16*4=64 */
#else
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq

_undefined_instruction: .word undefined_instruction
_software_interrupt:	.word software_interrupt
_prefetch_abort:	.word prefetch_abort
_data_abort:		.word data_abort
_not_used:		.word not_used
_irq:			.word irq
_fiq:			.word fiq
_pad:			.word 0x12345678 /* now 16*4=64 */
#endif	/* CONFIG_SPL_BUILD */
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

#ifdef CONFIG_USE_IRQ
/* IRQ stack memory (calculated at run-time) */
.globl IRQ_STACK_START
IRQ_STACK_START:
	.word	0x0badc0de

/* IRQ stack memory (calculated at run-time) */
.globl FIQ_STACK_START
FIQ_STACK_START:
	.word 0x0badc0de
#endif

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
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0xd3
	msr	cpsr,r0

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	bl  cpu_init_crit
#endif

#ifdef	CONFIG_CPU_PXA25X
	bl	lock_cache_for_stack
#endif

	bl	_main

/*------------------------------------------------------------------------------*/

	.globl	c_runtime_cpu_setup
c_runtime_cpu_setup:

#ifdef CONFIG_CPU_PXA25X
	/*
	 * Unlock (actually, disable) the cache now that board_init_f
	 * is done. We could do this earlier but we would need to add
	 * a new C runtime hook, whereas c_runtime_cpu_setup already
	 * exists.
	 * As this routine is just a call to cpu_init_crit, let us
	 * tail-optimize and do a simple branch here.
	 */
	b	cpu_init_crit
#else
	bx	lr
#endif

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
#if !defined(CONFIG_SKIP_LOWLEVEL_INIT) || defined(CONFIG_CPU_PXA25X)
cpu_init_crit:
	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* Invalidate I+D+BTB caches */
	mcr	p15, 0, r0, c8, c7, 0	/* Invalidate Unified TLB */

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00003300	@ clear bits 13:12, 9:8 (--VI --RS)
	bic	r0, r0, #0x00000087	@ clear bits 7, 2:0 (B--- -CAM)
	orr	r0, r0, #0x00000002	@ set bit 2 (A) Align
	mcr	p15, 0, r0, c1, c0, 0

	mov	pc, lr		/* back to my caller */
#endif /* !CONFIG_SKIP_LOWLEVEL_INIT || CONFIG_CPU_PXA25X */

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
 * use irq_save_user_regs / irq_restore_user_regs for IRQ/FIQ handling
 */

	.macro	bad_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE		@ carve out a frame on current user stack
	stmia	sp, {r0 - r12}			@ Save user registers (now in svc mode) r0-r12

	ldr	r2, IRQ_STACK_START_IN		@ set base 2 words into abort stack
	ldmia	r2, {r2 - r3}			@ get values for "aborted" pc and cpsr (into parm regs)
	add	r0, sp, #S_FRAME_SIZE		@ grab pointer to old stack

	add	r5, sp, #S_SP
	mov	r1, lr
	stmia	r5, {r0 - r3}			@ save sp_SVC, lr_SVC, pc, cpsr
	mov	r0, sp				@ save current stack into r0 (param register)
	.endm

	.macro	irq_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	add	r8, sp, #S_PC			@ !!!! R8 NEEDS to be saved !!!! a reserved stack spot would be good.
	stmdb	r8, {sp, lr}^			@ Calling SP, LR
	str	lr, [r8, #0]			@ Save calling PC
	mrs	r6, spsr
	str	r6, [r8, #4]			@ Save CPSR
	str	r0, [r8, #8]			@ Save OLD_R0
	mov	r0, sp
	.endm

	.macro	irq_restore_user_regs
	ldmia	sp, {r0 - lr}^			@ Calling r0 - lr
	mov	r0, r0
	ldr	lr, [sp, #S_PC]			@ Get PC
	add	sp, sp, #S_FRAME_SIZE
	subs	pc, lr, #4			@ return & move spsr_svc into cpsr
	.endm

	.macro get_bad_stack
	ldr	r13, IRQ_STACK_START_IN		@ setup our mode stack (enter in banked mode)

	str	lr, [r13]			@ save caller lr in position 0 of saved stack
	mrs	lr, spsr			@ get the spsr
	str	lr, [r13, #4]			@ save spsr in position 1 of saved stack

	mov	r13, #MODE_SVC			@ prepare SVC-Mode
	@ msr	spsr_c, r13
	msr	spsr, r13			@ switch modes, make sure moves will execute
	mov	lr, pc				@ capture return pc
	movs	pc, lr				@ jump to next instruction & switch modes.
	.endm

	.macro get_bad_stack_swi
	sub	r13, r13, #4			@ space on current stack for scratch reg.
	str	r0, [r13]			@ save R0's value.
	ldr	r0, IRQ_STACK_START_IN		@ get data regions start
	str	lr, [r0]			@ save caller lr in position 0 of saved stack
	mrs	lr, spsr			@ get the spsr
	str	lr, [r0, #4]			@ save spsr in position 1 of saved stack
	ldr	lr, [r0]			@ restore lr
	ldr	r0, [r13]			@ restore r0
	add	r13, r13, #4			@ pop stack entry
	.endm

	.macro get_irq_stack			@ setup IRQ stack
	ldr	sp, IRQ_STACK_START
	.endm

	.macro get_fiq_stack			@ setup FIQ stack
	ldr	sp, FIQ_STACK_START
	.endm
#endif	/* CONFIG_SPL_BUILD */

/*
 * exception handlers
 */
#ifdef CONFIG_SPL_BUILD
	.align	5
do_hang:
	bl	hang				/* hang and never return */
#else	/* !CONFIG_SPL_BUILD */
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

#ifdef CONFIG_USE_IRQ

	.align	5
irq:
	get_irq_stack
	irq_save_user_regs
	bl	do_irq
	irq_restore_user_regs

	.align	5
fiq:
	get_fiq_stack
	/* someone ought to write a more effiction fiq_save_user_regs */
	irq_save_user_regs
	bl	do_fiq
	irq_restore_user_regs

#else

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

#endif
	.align 5
#endif	/* CONFIG_SPL_BUILD */


/*
 * Enable MMU to use DCache as DRAM.
 *
 * This is useful on PXA25x and PXA26x in early bootstages, where there is no
 * other possible memory available to hold stack.
 */
#ifdef CONFIG_CPU_PXA25X
.macro CPWAIT reg
	mrc	p15, 0, \reg, c2, c0, 0
	mov	\reg, \reg
	sub	pc, pc, #4
.endm
lock_cache_for_stack:
	/* Domain access -- enable for all CPs */
	ldr	r0, =0x0000ffff
	mcr	p15, 0, r0, c3, c0, 0

	/* Point TTBR to MMU table */
	ldr	r0, =mmutable
	mcr	p15, 0, r0, c2, c0, 0

	/* Kick in MMU, ICache, DCache, BTB */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, #0x1b00
	bic	r0, #0x0087
	orr	r0, #0x1800
	orr	r0, #0x0005
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT	r0

	/* Unlock Icache, Dcache */
	mcr	p15, 0, r0, c9, c1, 1
	mcr	p15, 0, r0, c9, c2, 1

	/* Flush Icache, Dcache, BTB */
	mcr	p15, 0, r0, c7, c7, 0

	/* Unlock I-TLB, D-TLB */
	mcr	p15, 0, r0, c10, c4, 1
	mcr	p15, 0, r0, c10, c8, 1

	/* Flush TLB */
	mcr	p15, 0, r0, c8, c7, 0

	/* Allocate 4096 bytes of Dcache as RAM */

	/* Drain pending loads and stores */
	mcr	p15, 0, r0, c7, c10, 4

	mov	r4, #0x00
	mov	r5, #0x00
	mov	r2, #0x01
	mcr	p15, 0, r0, c9, c2, 0
	CPWAIT	r0

	/* 128 lines reserved (128 x 32bytes = 4096 bytes total) */
	mov	r0, #128
	ldr	r1, =0xfffff000

alloc:
	mcr	p15, 0, r1, c7, c2, 5
	/* Drain pending loads and stores */
	mcr	p15, 0, r0, c7, c10, 4
	strd	r4, [r1], #8
	strd	r4, [r1], #8
	strd	r4, [r1], #8
	strd	r4, [r1], #8
	subs	r0, #0x01
	bne	alloc
	/* Drain pending loads and stores */
	mcr	p15, 0, r0, c7, c10, 4
	mov	r2, #0x00
	mcr	p15, 0, r2, c9, c2, 0
	CPWAIT	r0

	mov	pc, lr

.section .mmutable, "a"
mmutable:
	.align	14
	/* 0x00000000 - 0xffe00000 : 1:1, uncached mapping */
	.set	__base, 0
	.rept	0xfff
	.word	(__base << 20) | 0xc12
	.set	__base, __base + 1
	.endr

	/* 0xfff00000 : 1:1, cached mapping */
	.word	(0xfff << 20) | 0x1c1e
#endif	/* CONFIG_CPU_PXA25X */
