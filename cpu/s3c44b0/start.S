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


/*
 * the actual reset code
 */

reset:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0x13
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

/*
	now copy to sram the interrupt vector
*/
	adr	r0, real_vectors
	add	r2, r0, #1024
	ldr	r1, =0x0c000000
	add	r1, r1, #0x08
vector_copy_loop:
	ldmia	r0!, {r3-r10}
	stmia	r1!, {r3-r10}
	cmp	r0, r2
	ble	vector_copy_loop
#endif	/* CONFIG_SKIP_RELOCATE_UBOOT */

	/* Set up the stack						    */
stack_setup:
	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CFG_MALLOC_LEN	/* malloc area                      */
	sub	r0, r0, #CFG_GBL_DATA_SIZE /* bdinfo                        */
#ifdef CONFIG_USE_IRQ
	sub	r0, r0, #(CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ)
#endif
	sub	sp, r0, #12		/* leave 3 words for abort-stack    */

	ldr	pc, _start_armboot

_start_armboot:	.word start_armboot


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
	ldr 	r0, =WTCON
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
	ldr	r0, =0x34031 	/* 66MHz (Quartz=11MHz) */
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
