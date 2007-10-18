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
 * do important init only if we don't start from RAM!
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
	mrs	r0,cpsr			/* set the cpu to SVC32 mode	    */
	bic	r0,r0,#0x1f		/* (superviser mode, M=10011)	    */
	orr	r0,r0,#0x13
	msr	cpsr,r0

	/*
	 * we do sys-critical inits only at reboot,
	 * not when booting from ram!
	 */
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	bl	cpu_init_crit		/* we do sys-critical inits	    */
#endif

#ifndef CONFIG_SKIP_RELOCATE_UBOOT
relocate:				/* relocate U-Boot to RAM	    */
	adr	r0, _start		/* r0 <- current position of code   */
	ldr	r1, _TEXT_BASE		/* test if we run from flash or RAM */
	cmp	r0, r1			/* don't reloc during debug	    */
	beq	stack_setup

	ldr	r2, _armboot_start
	ldr	r3, _bss_start
	sub	r2, r3, r2		/* r2 <- size of armboot	    */
	add	r2, r0, r2		/* r2 <- source end address	    */

copy_loop:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end addreee [r2]    */
	ble	copy_loop
#endif	/* CONFIG_SKIP_RELOCATE_UBOOT */

	/* Set up the stack						    */
stack_setup:
	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CFG_MALLOC_LEN /* malloc area			    */
	sub	r0, r0, #CFG_GBL_DATA_SIZE /* bdinfo			    */
#ifdef CONFIG_USE_IRQ
	sub	r0, r0, #(CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ)
#endif
	sub	sp, r0, #12		/* leave 3 words for abort-stack    */

clear_bss:
	ldr	r0, _bss_start		/* find start of bss segment	    */
	ldr	r1, _bss_end		/* stop here			    */
	mov	r2, #0x00000000		/* clear			    */

clbss_l:str	r2, [r0]		/* clear loop...		    */
	add	r0, r0, #4
	cmp	r0, r1
	ble	clbss_l

	ldr	pc, _start_armboot

_start_armboot: .word start_armboot


/****************************************************************************/
/*									    */
/* CPU_init_critical registers						    */
/*									    */
/* - setup important registers						    */
/* - setup memory timing						    */
/*									    */
/****************************************************************************/
/* mk@tbd: Fix this! */
#if defined(CONFIG_PXA250) || defined(CONFIG_CPU_MONAHANS)
#undef ICMR
#undef OSMR3
#undef OSCR
#undef OWER
#undef OIER
#endif
#ifdef CONFIG_PXA250
#undef RCSR
#undef CCCR
#endif

/* Interrupt-Controller base address					    */
IC_BASE:	   .word	   0x40d00000
#define ICMR	0x04

/* Reset-Controller */
RST_BASE:	.word	0x40f00030
#define RCSR	0x00

/* Operating System Timer */
OSTIMER_BASE:	.word	0x40a00000
#define OSMR3	0x0C
#define OSCR	0x10
#define OWER	0x18
#define OIER	0x1C

/* Clock Manager Registers						    */
#ifdef CONFIG_CPU_MONAHANS
# ifndef CFG_MONAHANS_RUN_MODE_OSC_RATIO
#  error "You have to define CFG_MONAHANS_RUN_MODE_OSC_RATIO!!"
# endif
# ifndef CFG_MONAHANS_TURBO_RUN_MODE_RATIO
#  define CFG_MONAHANS_TURBO_RUN_MODE_RATIO 0x1
# endif
#else /* ! CONFIG_CPU_MONAHANS */
#ifdef CFG_CPUSPEED
CC_BASE:	.word	0x41300000
#define CCCR	0x00
cpuspeed:	.word	CFG_CPUSPEED
#else
#error "You have to define CFG_CPUSPEED!!"
#endif
#endif /* CONFIG_CPU_MONAHANS */

	/* takes care the CP15 update has taken place */
	.macro CPWAIT reg
	mrc  p15,0,\reg,c2,c0,0
	mov  \reg,\reg
	sub  pc,pc,#4
	.endm

cpu_init_crit:

	/* mask all IRQs						    */
#ifndef CONFIG_CPU_MONAHANS
	ldr	r0, IC_BASE
	mov	r1, #0x00
	str	r1, [r0, #ICMR]
#else
	/* Step 1 - Enable CP6 permission */
	mrc	p15, 0, r1, c15, c1, 0	@ read CPAR
	orr	r1, r1, #0x40
		mcr	p15, 0, r1, c15, c1, 0
	CPWAIT	r1

	/* Step 2 - Mask ICMR & ICMR2 */
	mov	r1, #0
	mcr	p6, 0, r1, c1, c0, 0	@ ICMR
	mcr	p6, 0, r1, c7, c0, 0	@ ICMR2

	/* turn off all clocks but the ones we will definitly require */
	ldr	r1, =CKENA
	ldr	r2, =(CKENA_22_FFUART | CKENA_10_SRAM | CKENA_9_SMC | CKENA_8_DMC)
	str	r2, [r1]
	ldr	r1, =CKENB
	ldr	r2, =(CKENB_6_IRQ)
	str	r2, [r1]
#endif

	/* set clock speed */
#ifdef CONFIG_CPU_MONAHANS
	ldr	r0, =ACCR
	ldr	r1, =(((CFG_MONAHANS_TURBO_RUN_MODE_RATIO<<8) & ACCR_XN_MASK) | (CFG_MONAHANS_RUN_MODE_OSC_RATIO & ACCR_XL_MASK))
	str	r1, [r0]
#else /* ! CONFIG_CPU_MONAHANS */
#ifdef CFG_CPUSPEED
	ldr	r0, CC_BASE
	ldr	r1, cpuspeed
	str	r1, [r0, #CCCR]
	mov	r0, #2
	mcr	p14, 0, r0, c6, c0, 0

setspeed_done:

#endif /* CFG_CPUSPEED */
#endif /* CONFIG_CPU_MONAHANS */

	/*
	 * before relocating, we have to setup RAM timing
	 * because memory timing is board-dependend, you will
	 * find a lowlevel_init.S in your board directory.
	 */
	mov	ip,	lr
	bl	lowlevel_init
	mov	lr,	ip

	/* Memory interfaces are working. Disable MMU and enable I-cache.   */
	/* mk: hmm, this is not in the monahans docs, leave it now but
	 *     check here if it doesn't work :-) */

	ldr	r0, =0x2001		/* enable access to all coproc.	    */
	mcr	p15, 0, r0, c15, c1, 0
	CPWAIT r0

	mcr	p15, 0, r0, c7, c10, 4	/* drain the write & fill buffers   */
	CPWAIT r0

	mcr	p15, 0, r0, c7, c7, 0	/* flush Icache, Dcache and BTB	    */
	CPWAIT r0

	mcr	p15, 0, r0, c8, c7, 0	/* flush instuction and data TLBs   */
	CPWAIT r0

	/* Enable the Icache						    */
/*
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #0x1800
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT
*/
	mov	pc, lr


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
	sub	r2, r2, #(CONFIG_STACKSIZE+CFG_MALLOC_LEN)
	sub	r2, r2, #(CFG_GBL_DATA_SIZE+8)	@ set base 2 words into abort stack
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
	sub	r13, r13, #(CONFIG_STACKSIZE+CFG_MALLOC_LEN)
	sub	r13, r13, #(CFG_GBL_DATA_SIZE+8) @ reserved a couple spots in abort stack

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
/*									    */
/* Reset function: the PXA250 doesn't have a reset function, so we have to  */
/* perform a watchdog timeout for a soft reset.				    */
/*									    */
/****************************************************************************/

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
