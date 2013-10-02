/*
 *  armboot - Startup Code for ARM926EJS CPU-core
 *
 *  Copyright (c) 2003  Texas Instruments
 *
 *  ----- Adapted for OMAP1610 OMAP730 from ARM925t code ------
 *
 *  Copyright (c) 2001	Marius Groger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Zupke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <garyj@denx.de>
 *  Copyright (c) 2003	Richard Woodruff <r-woodruff2@ti.com>
 *  Copyright (c) 2003	Kshitij <kshitij@ti.com>
 *  Copyright (c) 2010	Albert Aribaud <albert.u.boot@aribaud.net>
 *
 * Change to support call back into iMX28 bootrom
 * Copyright (c) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <common.h>
#include <version.h>

/*
 *************************************************************************
 *
 * Jump vector table as in table 3.1 in [1]
 *
 *************************************************************************
 */


.globl _start
_start:
	b	reset
	b	undefined_instruction
	b	software_interrupt
	b	prefetch_abort
	b	data_abort
	b	not_used
	b	irq
	b	fiq

/*
 * Vector table, located at address 0x20.
 * This table allows the code running AFTER SPL, the U-Boot, to install it's
 * interrupt handlers here. The problem is that the U-Boot is loaded into RAM,
 * including it's interrupt vectoring table and the table at 0x0 is still the
 * SPLs. So if interrupt happens in U-Boot, the SPLs interrupt vectoring table
 * is still used.
 */
_vt_reset:
	.word	_reset
_vt_undefined_instruction:
	.word	_hang
_vt_software_interrupt:
	.word	_hang
_vt_prefetch_abort:
	.word	_hang
_vt_data_abort:
	.word	_hang
_vt_not_used:
	.word	_reset
_vt_irq:
	.word	_hang
_vt_fiq:
	.word	_hang

reset:
	ldr	pc, _vt_reset
undefined_instruction:
	ldr	pc, _vt_undefined_instruction
software_interrupt:
	ldr	pc, _vt_software_interrupt
prefetch_abort:
	ldr	pc, _vt_prefetch_abort
data_abort:
	ldr	pc, _vt_data_abort
not_used:
	ldr	pc, _vt_not_used
irq:
	ldr	pc, _vt_irq
fiq:
	ldr	pc, _vt_fiq

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
#ifdef CONFIG_SPL_TEXT_BASE
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

_reset:
	/*
	 * Store all registers on old stack pointer, this will allow us later to
	 * return to the BootROM and let the BootROM load U-Boot into RAM.
	 *
	 * WARNING: Register r0 and r1 are used by the BootROM to pass data
	 *          to the called code. Register r0 will contain arbitrary
	 *          data that are set in the BootStream. In case this code
	 *          was started with CALL instruction, register r1 will contain
	 *          pointer to the return value this function can then set.
	 *          The code below MUST NOT CHANGE register r0 and r1 !
	 */
	push	{r0-r12,r14}

	/* Save control register c1 */
	mrc	p15, 0, r2, c1, c0, 0
	push	{r2}

	/* Set the cpu to SVC32 mode and store old CPSR register content. */
	mrs	r2, cpsr
	push	{r2}
	bic	r2, r2, #0x1f
	orr	r2, r2, #0xd3
	msr	cpsr, r2

	bl	board_init_ll

	/* Restore BootROM's CPU mode (especially FIQ). */
	pop	{r2}
	msr	cpsr,r2

	/*
	 * Restore c1 register. Especially set exception vector location
	 * back to BootROM space which is required by bootrom for USB boot.
	 */
	pop	{r2}
	mcr	p15, 0, r2, c1, c0, 0

	pop	{r0-r12,r14}

	/*
	 * In case this code was started by the CALL instruction, the register
	 * r0 is examined by the BootROM after this code returns. The value in
	 * r0 must be set to 0 to indicate successful return.
	 */
	mov r0, #0

	bx	lr

_hang:
	ldr	sp, _TEXT_BASE			/* switch to abort stack */
1:
	bl	1b				/* hang and never return */
