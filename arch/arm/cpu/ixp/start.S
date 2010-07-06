/* vi: set ts=8 sw=8 noet: */
/*
 *  u-boot - Startup Code for XScale IXP
 *
 * Copyright (C) 2003	Kyle Harris <kharris@nexus-tech.net>
 *
 * Based on startup code example contained in the
 * Intel IXP4xx Programmer's Guide and past u-boot Start.S
 * samples.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <version.h>
#include <asm/arch/ixp425.h>

#define MMU_Control_M  0x001    /* Enable MMU */
#define MMU_Control_A  0x002    /* Enable address alignment faults */
#define MMU_Control_C  0x004    /* Enable cache */
#define MMU_Control_W  0x008    /* Enable write-buffer */
#define MMU_Control_P  0x010    /* Compatability: 32 bit code */
#define MMU_Control_D  0x020    /* Compatability: 32 bit data */
#define MMU_Control_L  0x040    /* Compatability: */
#define MMU_Control_B  0x080    /* Enable Big-Endian */
#define MMU_Control_S  0x100    /* Enable system protection */
#define MMU_Control_R  0x200    /* Enable ROM protection */
#define MMU_Control_I  0x1000   /* Enable Instruction cache */
#define MMU_Control_X  0x2000   /* Set interrupt vectors at 0xFFFF0000 */
#define MMU_Control_Init (MMU_Control_P|MMU_Control_D|MMU_Control_L)


/*
 * Macro definitions
 */
	/* Delay a bit */
	.macro DELAY_FOR cycles, reg0
	ldr     \reg0, =\cycles
	subs    \reg0, \reg0, #1
	subne   pc,  pc, #0xc
	.endm

	/* wait for coprocessor write complete */
	.macro CPWAIT reg
	mrc  p15,0,\reg,c2,c0,0
	mov  \reg,\reg
	sub  pc,pc,#4
	.endm

.globl _start
_start: b	reset
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

	.balignl 16,0xdeadbeef


/*
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from memory!
 * - relocate armboot to ram
 * - setup stack
 * - jump to second stage
 */

_TEXT_BASE:
	.word	TEXT_BASE

.globl _armboot_start
_armboot_start:
	.word _start

/*
 * These are defined in the board-specific linker script.
 */
.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word _end

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

/****************************************************************************/
/*									    */
/* the actual reset code						    */
/*									    */
/****************************************************************************/

reset:
	/* disable mmu, set big-endian */
	mov	r0, #0xf8
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0

	/* invalidate I & D caches & BTB */
	mcr	p15, 0, r0, c7, c7, 0
	CPWAIT	r0

	/* invalidate I & Data TLB */
	mcr	p15, 0, r0, c8, c7, 0
	CPWAIT r0

	/* drain write and fill buffers */
	mcr	p15, 0, r0, c7, c10, 4
	CPWAIT	r0

	/* disable write buffer coalescing */
	mrc	p15, 0, r0, c1, c0, 1
	orr	r0, r0, #1
	mcr	p15, 0, r0, c1, c0, 1
	CPWAIT	r0

	/* set EXP CS0 to the optimum timing */
	ldr	r1, =CONFIG_SYS_EXP_CS0
	ldr     r2, =IXP425_EXP_CS0
	str     r1, [r2]

	/* make sure flash is visible at 0 */
#if 0
	ldr	r2, =IXP425_EXP_CFG0
	ldr     r1, [r2]
	orr     r1, r1, #0x80000000
	str     r1, [r2]
#endif
	mov	r1, #CONFIG_SYS_SDR_CONFIG
	ldr     r2, =IXP425_SDR_CONFIG
	str     r1, [r2]

	/* disable refresh cycles */
	mov	r1, #0
	ldr     r3, =IXP425_SDR_REFRESH
	str	r1, [r3]

	/* send nop command */
	mov	r1, #3
	ldr	r4, =IXP425_SDR_IR
	str	r1, [r4]
	DELAY_FOR 0x4000, r0

	/* set SDRAM internal refresh val */
	ldr	r1, =CONFIG_SYS_SDRAM_REFRESH_CNT
	str     r1, [r3]
	DELAY_FOR 0x4000, r0

	/* send precharge-all command to close all open banks */
	mov     r1, #2
	str     r1, [r4]
	DELAY_FOR 0x4000, r0

	/* provide 8 auto-refresh cycles */
	mov     r1, #4
	mov     r5, #8
111:    str	r1, [r4]
	DELAY_FOR 0x100, r0
	subs	r5, r5, #1
	bne	111b

	/* set mode register in sdram */
	mov	r1, #CONFIG_SYS_SDR_MODE_CONFIG
	str	r1, [r4]
	DELAY_FOR 0x4000, r0

	/* send normal operation command */
	mov	r1, #6
	str	r1, [r4]
	DELAY_FOR 0x4000, r0

	/* copy */
	mov     r0, #0
	mov     r4, r0
	add     r2, r0, #CONFIG_SYS_MONITOR_LEN
	mov     r1, #0x10000000
	mov     r5, r1

    30:
	ldr     r3, [r0], #4
	str     r3, [r1], #4
	cmp     r0, r2
	bne     30b

	/* invalidate I & D caches & BTB */
	mcr	p15, 0, r0, c7, c7, 0
	CPWAIT	r0

	/* invalidate I & Data TLB */
	mcr	p15, 0, r0, c8, c7, 0
	CPWAIT r0

	/* drain write and fill buffers */
	mcr	p15, 0, r0, c7, c10, 4
	CPWAIT	r0

	/* move flash to 0x50000000 */
	ldr	r2, =IXP425_EXP_CFG0
	ldr     r1, [r2]
	bic     r1, r1, #0x80000000
	str     r1, [r2]

	nop
	nop
	nop
	nop
	nop
	nop

	/* invalidate I & Data TLB */
	mcr	p15, 0, r0, c8, c7, 0
	CPWAIT r0

	/* enable I cache */
	mrc     p15, 0, r0, c1, c0, 0
	orr     r0, r0, #MMU_Control_I
	mcr     p15, 0, r0, c1, c0, 0
	CPWAIT  r0

	mrs	r0,cpsr			/* set the cpu to SVC32 mode	    */
	bic	r0,r0,#0x1f		/* (superviser mode, M=10011)	    */
	orr	r0,r0,#0x13
	msr	cpsr,r0

#ifndef CONFIG_SKIP_RELOCATE_UBOOT
relocate:				/* relocate U-Boot to RAM	    */
	adr	r0, _start		/* r0 <- current position of code   */
	ldr	r1, _TEXT_BASE		/* test if we run from flash or RAM */
	cmp     r0, r1                  /* don't reloc during debug         */
	beq     stack_setup

	ldr	r2, _armboot_start
	ldr	r3, _bss_start
	sub	r2, r3, r2		/* r2 <- size of armboot            */
	add	r2, r0, r2		/* r2 <- source end address         */

copy_loop:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end addreee [r2]    */
	ble	copy_loop
#endif	/* CONFIG_SKIP_RELOCATE_UBOOT */

	/* Set up the stack						    */
stack_setup:
	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CONFIG_SYS_MALLOC_LEN	/* malloc area                      */
	sub	r0, r0, #CONFIG_SYS_GBL_DATA_SIZE /* bdinfo                        */
#ifdef CONFIG_USE_IRQ
	sub	r0, r0, #(CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ)
#endif
	sub	sp, r0, #12		/* leave 3 words for abort-stack    */
	bic	sp, sp, #7		/* 8-byte alignment for ABI compliance */

clear_bss:
	ldr	r0, _bss_start		/* find start of bss segment        */
	ldr	r1, _bss_end		/* stop here                        */
	mov	r2, #0x00000000		/* clear                            */

clbss_l:str	r2, [r0]		/* clear loop...                    */
	add	r0, r0, #4
	cmp	r0, r1
	ble	clbss_l

	ldr	pc, _start_armboot

_start_armboot: .word start_armboot


/****************************************************************************/
/*									    */
/* Interrupt handling							    */
/*									    */
/****************************************************************************/

/* IRQ stack frame							    */

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

	/* use bad_save_user_regs for abort/prefetch/undef/swi ...	    */

	.macro	bad_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			/* Calling r0-r12	    */
	add	r8, sp, #S_PC

	ldr	r2, _armboot_start
	sub	r2, r2, #(CONFIG_STACKSIZE+CONFIG_SYS_MALLOC_LEN)
	sub	r2, r2, #(CONFIG_SYS_GBL_DATA_SIZE+8)  @ set base 2 words into abort stack
	ldmia	r2, {r2 - r4}			/* get pc, cpsr, old_r0	    */
	add	r0, sp, #S_FRAME_SIZE		/* restore sp_SVC	    */

	add	r5, sp, #S_SP
	mov	r1, lr
	stmia	r5, {r0 - r4}			/* save sp_SVC, lr_SVC, pc, cpsr, old_r */
	mov	r0, sp
	.endm


	/* use irq_save_user_regs / irq_restore_user_regs for		     */
	/* IRQ/FIQ handling						     */

	.macro	irq_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			/* Calling r0-r12	     */
	add	r8, sp, #S_PC
	stmdb	r8, {sp, lr}^			/* Calling SP, LR	     */
	str	lr, [r8, #0]			/* Save calling PC	     */
	mrs	r6, spsr
	str	r6, [r8, #4]			/* Save CPSR		     */
	str	r0, [r8, #8]			/* Save OLD_R0		     */
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
	ldr	r13, _armboot_start		@ setup our mode stack
	sub	r13, r13, #(CONFIG_STACKSIZE+CONFIG_SYS_MALLOC_LEN)
	sub	r13, r13, #(CONFIG_SYS_GBL_DATA_SIZE+8) @ reserved a couple spots in abort stack

	str	lr, [r13]			@ save caller lr / spsr
	mrs	lr, spsr
	str	lr, [r13, #4]

	mov	r13, #MODE_SVC			@ prepare SVC-Mode
	msr	spsr_c, r13
	mov	lr, pc
	movs	pc, lr
	.endm

	.macro get_irq_stack			@ setup IRQ stack
	ldr	sp, IRQ_STACK_START
	.endm

	.macro get_fiq_stack			@ setup FIQ stack
	ldr	sp, FIQ_STACK_START
	.endm


/****************************************************************************/
/*									    */
/* exception handlers							    */
/*									    */
/****************************************************************************/

	.align	5
undefined_instruction:
	get_bad_stack
	bad_save_user_regs
	bl	do_undefined_instruction

	.align	5
software_interrupt:
	get_bad_stack
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
	irq_save_user_regs		/* someone ought to write a more    */
	bl	do_fiq			/* effiction fiq_save_user_regs	    */
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

/****************************************************************************/
/*                                                                          */
/* Reset function: Use Watchdog to reset                                    */
/*                                                                          */
/****************************************************************************/

	.align	5
.globl reset_cpu

reset_cpu:
	ldr	r1, =0x482e
	ldr     r2, =IXP425_OSWK
	str     r1, [r2]
	ldr	r1, =0x0fff
	ldr     r2, =IXP425_OSWT
	str     r1, [r2]
	ldr	r1, =0x5
	ldr     r2, =IXP425_OSWE
	str     r1, [r2]
	b	reset_endless


reset_endless:

	b	reset_endless

#ifdef CONFIG_USE_IRQ

.LC0:		.word	loops_per_jiffy

/*
 * 0 <= r0 <= 2000
 */
.globl __udelay
__udelay:
	mov	r2,     #0x6800
	orr	r2, r2, #0x00db
	mul	r0, r2, r0
	ldr	r2, .LC0
	ldr	r2, [r2]		@ max = 0x0fffffff
	mov	r0, r0, lsr #11		@ max = 0x00003fff
	mov	r2, r2, lsr #11		@ max = 0x0003ffff
	mul	r0, r2, r0		@ max = 2^32-1
	movs	r0, r0, lsr #6

delay_loop:
	subs	r0, r0, #1
	bne	delay_loop
	mov	pc, lr

#endif /* CONFIG_USE_IRQ */
