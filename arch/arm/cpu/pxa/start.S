/*
 *  armboot - Startup Code for XScale
 *
 *  Copyright (C) 1998	Dan Malek <dmalek@jlc.net>
 *  Copyright (C) 1999	Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 *  Copyright (C) 2000	Wolfgang Denk <wd@denx.de>
 *  Copyright (C) 2001	Alex Zuepke <azu@sysgo.de>
 *  Copyright (C) 2002	Kyle Harris <kharris@nexus-tech.net>
 *  Copyright (C) 2003	Robert Schwebel <r.schwebel@pengutronix.de>
 *  Copyright (C) 2003	Kai-Uwe Bloem <kai-uwe.bloem@auerswald.de>
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
#include <asm/arch/pxa-regs.h>
#include <asm/arch/macro.h>

/* takes care the CP15 update has taken place */
.macro CPWAIT reg
mrc  p15,0,\reg,c2,c0,0
mov  \reg,\reg
sub  pc,pc,#4
.endm

.globl _start
_start: b	reset
#ifdef CONFIG_PRELOADER
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
#endif	/* CONFIG_PRELOADER */

	.balignl 16,0xdeadbeef


/*
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from RAM!
 * - relocate armboot to RAM
 * - setup stack
 * - jump to second stage
 */

.globl _TEXT_BASE
_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE

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
#endif /* CONFIG_USE_IRQ */

#ifndef CONFIG_PRELOADER
/* IRQ stack memory (calculated at run-time) + 8 bytes */
.globl IRQ_STACK_START_IN
IRQ_STACK_START_IN:
	.word	0x0badc0de

.globl _datarel_start
_datarel_start:
	.word __datarel_start

.globl _datarelrolocal_start
_datarelrolocal_start:
	.word __datarelrolocal_start

.globl _datarellocal_start
_datarellocal_start:
	.word __datarellocal_start

.globl _datarelro_start
_datarelro_start:
	.word __datarelro_start

.globl _got_start
_got_start:
	.word __got_start

.globl _got_end
_got_end:
	.word __got_end

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

	/*
	 * Enable MMU to use DCache as DRAM
	 */
	/* Domain access -- enable for all CPs */
	ldr	r0, =0x0000ffff
	mcr	p15, 0, r0, c3, c0, 0

	/* Point TTBR to MMU table */
	ldr	r0, =mmu_table
	adr	r2, _start
	orr	r0, r2
	mcr	p15, 0, r0, c2, c0, 0

/* !!! Hereby, check if the code is running from SRAM !!! */
/* If the code is running from SRAM, alias SRAM to 0x0 to simulate NOR. The code
 * is linked to 0x0 too, so this makes things easier. */
	cmp	r2, #0x5c000000

	ldreq	r1, [r0]
	orreq	r1, r2
	streq	r1, [r0]

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
	mov	r1, #0xa0000000
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

	/* Jump to 0x0 ( + offset) if running from SRAM */
	adr	r0, zerojmp
	bic	r0, #0x5c000000
	mov	pc, r0
zerojmp:

/* Set stackpointer in internal RAM to call board_init_f */
call_board_init_f:
	ldr	sp, =(CONFIG_SYS_INIT_SP_ADDR)
	ldr	r0,=0x00000000
	bl	board_init_f

/*------------------------------------------------------------------------------*/

/*
 * void relocate_code (addr_sp, gd, addr_moni)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 */
	.globl	relocate_code
relocate_code:
	mov	r4, r0	/* save addr_sp */
	mov	r5, r1	/* save addr of gd */
	mov	r6, r2	/* save addr of destination */
	mov	r7, r2	/* save addr of destination */

	/* Set up the stack						    */
stack_setup:
	mov	sp, r4

	adr	r0, _start
	ldr	r2, _TEXT_BASE
	ldr	r3, _bss_start
	sub	r2, r3, r2		/* r2 <- size of armboot	    */
	add	r2, r0, r2		/* r2 <- source end address	    */
	cmp	r0, r6
	beq	clear_bss

#ifndef CONFIG_SKIP_RELOCATE_UBOOT
	stmfd sp!, {r0-r12}
copy_loop:
	ldmia	r0!, {r3-r5, r7-r11}	/* copy from source address [r0]    */
	stmia	r6!, {r3-r5, r7-r11}	/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end address [r2]    */
	blo	copy_loop
	ldmfd sp!, {r0-r12}

#ifndef CONFIG_PRELOADER
	/* fix got entries */
	ldr	r1, _TEXT_BASE		/* Text base */
	mov	r0, r7			/* reloc addr */
	ldr	r2, _got_start		/* addr in Flash */
	ldr	r3, _got_end		/* addr in Flash */
	sub	r3, r3, r1
	add	r3, r3, r0
	sub	r2, r2, r1
	add	r2, r2, r0

fixloop:
	ldr	r4, [r2]
	sub	r4, r4, r1
	add	r4, r4, r0
	str	r4, [r2]
	add	r2, r2, #4
	cmp	r2, r3
	bne	fixloop
#endif
#endif	/* #ifndef CONFIG_SKIP_RELOCATE_UBOOT */

clear_bss:
#ifndef CONFIG_PRELOADER
	ldr	r0, _bss_start
	ldr	r1, _bss_end
	ldr	r3, _TEXT_BASE		/* Text base */
	mov	r4, r7			/* reloc addr */
	sub	r0, r0, r3
	add	r0, r0, r4
	sub	r1, r1, r3
	add	r1, r1, r4
	mov	r2, #0x00000000		/* clear			    */

clbss_l:str	r2, [r0]		/* clear loop...		    */
	add	r0, r0, #4
	cmp	r0, r1
	bne	clbss_l
#endif

/*
 * We are done. Do not return, instead branch to second part of board
 * initialization, now running from RAM.
 */
#ifdef CONFIG_ONENAND_IPL
	ldr     pc, _start_oneboot

_start_oneboot: .word start_oneboot
#else
	ldr	r0, _TEXT_BASE
	ldr	r2, _board_init_r
	sub	r2, r2, r0
	add	r2, r2, r7	/* position from board_init_r in RAM */
	/* setup parameters for board_init_r */
	mov	r0, r5		/* gd_t */
	mov	r1, r7		/* dest_addr */
	/* jump to it ... */
	mov	lr, r2
	mov	pc, lr

_board_init_r: .word board_init_r
#endif

#else /* #if !defined(CONFIG_SYS_ARM_WITHOUT_RELOC) */

/****************************************************************************/
/*									    */
/* the actual reset code for OneNAND IPL				    */
/*									    */
/****************************************************************************/

#ifndef	CONFIG_PXA27X
#error OneNAND IPL is not supported on PXA25x and 26x due to lack of SRAM
#endif

reset:
	/* Set CPU to SVC32 mode */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0x13
	msr	cpsr,r0

	/* Point stack at the end of SRAM and leave 32 words for abort-stack */
	ldr	sp, =0x5c03ff80

	/* Start OneNAND IPL */
	ldr	pc, =start_oneboot

#endif /* #if !defined(CONFIG_ONENAND_IPL) */

#ifndef CONFIG_PRELOADER
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

#if defined(CONFIG_SYS_ARM_WITHOUT_RELOC)
	ldr	r2, _armboot_start
	sub	r2, r2, #(CONFIG_STACKSIZE+CONFIG_SYS_MALLOC_LEN)
	sub	r2, r2, #(CONFIG_SYS_GBL_DATA_SIZE+8)	@ set base 2 words into abort stack
#else
	ldr	r2, IRQ_STACK_START_IN
#endif
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
#if defined(CONFIG_SYS_ARM_WITHOUT_RELOC)
	ldr	r13, _armboot_start		@ setup our mode stack
	sub	r13, r13, #(CONFIG_STACKSIZE+CONFIG_SYS_MALLOC_LEN)
	sub	r13, r13, #(CONFIG_SYS_GBL_DATA_SIZE+8) @ reserved a couple spots in abort stack
#else
	ldr	r13, IRQ_STACK_START_IN		@ setup our mode stack
#endif

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
#endif	/* CONFIG_PRELOADER */


/****************************************************************************/
/*									    */
/* exception handlers							    */
/*									    */
/****************************************************************************/

#ifdef CONFIG_PRELOADER
	.align	5
do_hang:
	ldr	sp, _TEXT_BASE			/* use 32 words abort stack */
	bl	hang				/* hang and never return */
#else	/* !CONFIG_PRELOADER */
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

#else /* !CONFIG_USE_IRQ */

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
#endif	/* CONFIG_PRELOADER */
#endif /* CONFIG_USE_IRQ */

/****************************************************************************/
/*									    */
/* Reset function: the PXA250 doesn't have a reset function, so we have to  */
/* perform a watchdog timeout for a soft reset.				    */
/*									    */
/****************************************************************************/
/* Operating System Timer */
OSTIMER_BASE:	.word	0x40a00000
#define OSMR3	0x0C
#define OSCR	0x10
#define OWER	0x18
#define OIER	0x1C

	.align	5
.globl reset_cpu

	/* FIXME: this code is PXA250 specific. How is this handled on	    */
	/*	  other XScale processors?				    */

reset_cpu:

	/* We set OWE:WME (watchdog enable) and wait until timeout happens  */

	ldr	r0, OSTIMER_BASE
	ldr	r1, [r0, #OWER]
	orr	r1, r1, #0x0001			/* bit0: WME		    */
	str	r1, [r0, #OWER]

	/* OS timer does only wrap every 1165 seconds, so we have to set    */
	/* the match register as well.					    */

	ldr	r1, [r0, #OSCR]			/* read OS timer	    */
	add	r1, r1, #0x800			/* let OSMR3 match after    */
	add	r1, r1, #0x800			/* 4096*(1/3.6864MHz)=1ms   */
	str	r1, [r0, #OSMR3]

reset_endless:

	b	reset_endless

#ifndef CONFIG_PRELOADER
.section .mmudata, "a"
	.align	14
	.globl	mmu_table
mmu_table:
	/* 0x00000000 - 0xa0000000 : 1:1, uncached mapping */
	.set	__base, 0
	.rept	0xa00
	.word	(__base << 20) | 0xc12
	.set	__base, __base + 1
	.endr

	/* 0xa0000000 - 0xa0100000 : 1:1, cached mapping */
	.word	(0xa00 << 20) | 0x1c1e

	.set	__base, 0xa01
	.rept	0x1000 - 0xa01
	.word	(__base << 20) | 0xc12
	.set	__base, __base + 1
	.endr
#endif
