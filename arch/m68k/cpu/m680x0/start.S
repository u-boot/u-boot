/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Startup code for m68040
 *
 * Copyright (C) 2025, Kuan-Wei Chiu <visitorckw@gmail.com>
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>

.section .text

/*
 * Vector Table
 * m68k uses the first 1KB for the exception vector table.
 */
.balign 4
.global _vectors
_vectors:
	.long	CFG_SYS_INIT_SP_ADDR	/* 0x00: Initial SP */
	.long	_start	/* 0x04: Initial PC (Reset) */
	.long	_fault	/* 0x08: Bus Error */
	.long	_fault	/* 0x0C: Address Error */
	.long	_fault	/* 0x10: Illegal Instruction */
	.long	_fault	/* 0x14: Zero Divide */
	.long	_fault	/* 0x18: CHK */
	.long	_fault	/* 0x1C: TRAPV */
	.long	_fault	/* 0x20: Privilege */
	.long	_fault	/* 0x24: Trace */
	.long	_fault	/* 0x28: Line 1010 */
	.long	_fault	/* 0x2C: Line 1111 */
	.fill	0x400 - (.-_vectors), 1, 0

/*
 * Entry Point
 */
ENTRY(_start)
	/* Disable Interrupts */
	move.w	#0x2700, %sr

	/* Setup initial stack pointer */
	move.l	#CFG_SYS_INIT_SP_ADDR, %sp

	/*
	 * Allocate Global Data (GD)
	 * board_init_f_alloc_reserve(top) returns the new top of stack in %d0
	 */
	move.l	%sp, -(%sp)
	bsr.l	board_init_f_alloc_reserve
	addq.l	#4, %sp

	/* Update Stack Pointer and set GD register */
	move.l	%d0, %sp
	move.l	%d0, %d7	/* %d7 is the gd register */

	/* Initialize Reserved Memory. */
	move.l	%d0, -(%sp)
	bsr.l	m68k_virt_init_reserve
	addq.l	#4, %sp

	/* Enter board_init_f(0) */
	clr.l	-(%sp)
	bsr.l	board_init_f
	addq.l	#4, %sp

	/* Should not return */
hang:
	bra.s	hang
ENDPROC(_start)

_fault:
	bra.s	_fault
