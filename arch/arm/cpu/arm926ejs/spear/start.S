/*
 *  armboot - Startup Code for ARM926EJS CPU-core
 *
 *  Copyright (c) 2003  Texas Instruments
 *
 *  ----- Adapted for OMAP1610 OMAP730 from ARM925t code ------
 *
 *  Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <garyj@denx.de>
 *  Copyright (c) 2003	Richard Woodruff <r-woodruff2@ti.com>
 *  Copyright (c) 2003	Kshitij <kshitij@ti.com>
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

.globl _start
_start:
	b	reset
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq

_undefined_instruction:
_software_interrupt:
_prefetch_abort:
_data_abort:
_not_used:
_irq:
_fiq:
	.word infinite_loop

infinite_loop:
	b	infinite_loop

/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * Below are the critical initializations already taken place in BootROM.
 * So, these are not taken care in Xloader
 * 1. Relocation to RAM
 * 2. Initializing stacks
 *
 *************************************************************************
 */

/*
 * the actual reset code
 */

reset:
/*
 * Xloader has to return back to BootROM in a few cases.
 * eg. Ethernet boot, UART boot, USB boot
 * Saving registers for returning back
 */
	stmdb	sp!, {r0-r12,r14}
	bl	cpu_init_crit
/*
 * Clearing bss area is not done in Xloader.
 * BSS area lies in the DDR location which is not yet initialized
 * bss is assumed to be uninitialized.
 */
	bl	spl_boot
	ldmia	sp!, {r0-r12,pc}

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
cpu_init_crit:
	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	/*
	 * enable instruction cache
	 */
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #0x00001000	/* set bit 12 (I) I-Cache */
	mcr	p15, 0, r0, c1, c0, 0

	/*
	 * Go setup Memory and board specific bits prior to relocation.
	 */
	stmdb	sp!, {lr}
	bl	lowlevel_init	/* go setup pll,mux,memory */
	ldmia	sp!, {pc}
