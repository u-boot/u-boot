/*
 * Startup Code for S3C44B0 CPU-core
 *
 * (C) Copyright 2004
 * DAVE Srl
 *
 * http://www.dave-tech.it
 * http://www.wawnet.biz
 * mailto:info@wawnet.biz
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

#include <asm-offsets.h>
#include <config.h>
#include <version.h>

/*
 * Jump vector table
 */


.globl _start
_start:	b       reset
	add	pc, pc, #0x0c000000
	add	pc, pc, #0x0c000000
	add	pc, pc, #0x0c000000
	add	pc, pc, #0x0c000000
	add	pc, pc, #0x0c000000
	add	pc, pc, #0x0c000000
	add	pc, pc, #0x0c000000

	.balignl 16,0xdeadbeef


/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from memory!
 * relocate u-boot to ram
 * setup stack
 * jump to second stage
 *
 *************************************************************************
 */

.globl _TEXT_BASE
_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE

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
	.word __bss_end__ - _start

.globl _end_ofs
_end_ofs:
	.word _end - _start

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

	/*
	 * we do sys-critical inits only at reboot,
	 * not when booting from ram!
	 */
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	bl	cpu_init_crit
	/*
	 * before relocating, we have to setup RAM timing
	 * because memory timing is board-dependend, you will
	 * find a lowlevel_init.S in your board directory.
	 */
	bl	lowlevel_init
#endif

/* Set stackpointer in internal RAM to call board_init_f */
call_board_init_f:
	ldr	sp, =(CONFIG_SYS_INIT_SP_ADDR)
	bic	sp, sp, #7 /* 8-byte alignment for ABI compliance */
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

	/* Set up the stack						    */
stack_setup:
	mov	sp, r4

	adr	r0, _start
	cmp	r0, r6
	beq	clear_bss		/* skip relocation */
	mov	r1, r6			/* r1 <- scratch for copy_loop */
	ldr	r3, _bss_start_ofs
	add	r2, r0, r3		/* r2 <- source end address	    */

copy_loop:
	ldmia	r0!, {r9-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r9-r10}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end address [r2]    */
	blo	copy_loop

#ifndef CONFIG_SPL_BUILD
	/*
	 * fix .rel.dyn relocations
	 */
	ldr	r0, _TEXT_BASE		/* r0 <- Text base */
	sub	r9, r6, r0		/* r9 <- relocation offset */
	ldr	r10, _dynsym_start_ofs	/* r10 <- sym table ofs */
	add	r10, r10, r0		/* r10 <- sym table in FLASH */
	ldr	r2, _rel_dyn_start_ofs	/* r2 <- rel dyn start ofs */
	add	r2, r2, r0		/* r2 <- rel dyn start in FLASH */
	ldr	r3, _rel_dyn_end_ofs	/* r3 <- rel dyn end ofs */
	add	r3, r3, r0		/* r3 <- rel dyn end in FLASH */
fixloop:
	ldr	r0, [r2]		/* r0 <- location to fix up, IN FLASH! */
	add	r0, r0, r9		/* r0 <- location to fix up in RAM */
	ldr	r1, [r2, #4]
	and	r7, r1, #0xff
	cmp	r7, #23			/* relative fixup? */
	beq	fixrel
	cmp	r7, #2			/* absolute fixup? */
	beq	fixabs
	/* ignore unknown type of fixup */
	b	fixnext
fixabs:
	/* absolute fix: set location to (offset) symbol value */
	mov	r1, r1, LSR #4		/* r1 <- symbol index in .dynsym */
	add	r1, r10, r1		/* r1 <- address of symbol in table */
	ldr	r1, [r1, #4]		/* r1 <- symbol value */
	add	r1, r1, r9		/* r1 <- relocated sym addr */
	b	fixnext
fixrel:
	/* relative fix: increase location by offset */
	ldr	r1, [r0]
	add	r1, r1, r9
fixnext:
	str	r1, [r0]
	add	r2, r2, #8		/* each rel.dyn entry is 8 bytes */
	cmp	r2, r3
	blo	fixloop
#endif

clear_bss:
#ifndef CONFIG_SPL_BUILD
	ldr	r0, _bss_start_ofs
	ldr	r1, _bss_end_ofs
	mov	r4, r6			/* reloc addr */
	add	r0, r0, r4
	add	r1, r1, r4
	mov	r2, #0x00000000		/* clear			    */

clbss_l:str	r2, [r0]		/* clear loop...		    */
	add	r0, r0, #4
	cmp	r0, r1
	bne	clbss_l

	bl coloured_LED_init
	bl red_led_on
#endif

/*
 * We are done. Do not return, instead branch to second part of board
 * initialization, now running from RAM.
 */
	ldr	r0, _board_init_r_ofs
	adr	r1, _start
	add	lr, r0, r1
	add	lr, lr, r9
	/* setup parameters for board_init_r */
	mov	r0, r5		/* gd_t */
	mov	r1, r6		/* dest_addr */
	/* jump to it ... */
	mov	pc, lr

_board_init_r_ofs:
	.word board_init_r - _start

_rel_dyn_start_ofs:
	.word __rel_dyn_start - _start
_rel_dyn_end_ofs:
	.word __rel_dyn_end - _start
_dynsym_start_ofs:
	.word __dynsym_start - _start

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

#define INTCON (0x01c00000+0x200000)
#define INTMSK (0x01c00000+0x20000c)
#define LOCKTIME (0x01c00000+0x18000c)
#define PLLCON (0x01c00000+0x180000)
#define CLKCON (0x01c00000+0x180004)
#define WTCON (0x01c00000+0x130000)
cpu_init_crit:
	/* disable watch dog */
	ldr	r0, =WTCON
	ldr	r1, =0x0
	str	r1, [r0]

	/*
	 * mask all IRQs by clearing all bits in the INTMRs
	 */
	ldr	r1,=INTMSK
	ldr	r0, =0x03fffeff
	str	r0, [r1]

	ldr	r1, =INTCON
	ldr	r0, =0x05
	str	r0, [r1]

	/* Set Clock Control Register */
	ldr	r1, =LOCKTIME
	ldrb	r0, =800
	strb	r0, [r1]

	ldr	r1, =PLLCON

#if CONFIG_S3C44B0_CLOCK_SPEED==66
	ldr	r0, =0x34031	/* 66MHz (Quartz=11MHz) */
#elif CONFIG_S3C44B0_CLOCK_SPEED==75
	ldr	r0, =0x610c1 /*B2: Xtal=20mhz Fclk=75MHz  */
#else
# error CONFIG_S3C44B0_CLOCK_SPEED undefined
#endif

	str	r0, [r1]

	ldr	r1,=CLKCON
	ldr	r0, =0x7ff8
	str	r0, [r1]

	mov	pc, lr


/*************************************************/
/*	interrupt vectors	*/
/*************************************************/
real_vectors:
	b	reset
	b	undefined_instruction
	b	software_interrupt
	b	prefetch_abort
	b	data_abort
	b	not_used
	b	irq
	b	fiq

/*************************************************/

undefined_instruction:
	mov	r6, #3
	b	reset

software_interrupt:
	mov	r6, #4
	b	reset

prefetch_abort:
	mov	r6, #5
	b	reset

data_abort:
	mov	r6, #6
	b	reset

not_used:
	/* we *should* never reach this */
	mov	r6, #7
	b	reset

irq:
	mov	r6, #8
	b	reset

fiq:
	mov	r6, #9
	b	reset
