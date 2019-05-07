/* SPDX-License-Identifier: GPL-2.0+ */
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
 */


#include <config.h>

/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * The BootROM already initialized its own stack in the [0-0xb00] reserved
 * range of the SRAM. The SPL (in _main) will update the stack pointer to
 * its own SRAM area (right before the gd section).
 *
 *************************************************************************
 */

	.globl	reset

reset:
	/*
	* SPL has to return back to BootROM in a few cases (eg. Ethernet boot,
	* UART boot, USB boot): save registers in BootROM's stack.
	*/
	stmdb	sp!, {r0-r12,r14}
	bl	cpu_init_crit
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
	 * Flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* Flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* Flush v4 TLB */

	/*
	 * Enable instruction cache
	 */
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #0x00001000	/* set bit 12 (I) I-Cache */
	mcr	p15, 0, r0, c1, c0, 0

	/*
	 * Go setup Memory and board specific bits prior to relocation.
	 */
	stmdb	sp!, {lr}
	bl	_main	/* _main will call board_init_f */
	ldmia	sp!, {pc}
