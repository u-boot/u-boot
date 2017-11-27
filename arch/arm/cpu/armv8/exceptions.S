/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <asm/ptrace.h>
#include <asm/macro.h>
#include <linux/linkage.h>

/*
 * Exception vectors.
 */
	.align	11
	.globl	vectors
vectors:
	.align	7		/* Current EL Synchronous Thread */
	stp	x29, x30, [sp, #-16]!
	bl	_exception_entry
	bl	do_bad_sync
	b	exception_exit

	.align	7		/* Current EL IRQ Thread */
	stp	x29, x30, [sp, #-16]!
	bl	_exception_entry
	bl	do_bad_irq
	b	exception_exit

	.align	7		/* Current EL FIQ Thread */
	stp	x29, x30, [sp, #-16]!
	bl	_exception_entry
	bl	do_bad_fiq
	b	exception_exit

	.align	7		/* Current EL Error Thread */
	stp	x29, x30, [sp, #-16]!
	bl	_exception_entry
	bl	do_bad_error
	b	exception_exit

	.align	7		 /* Current EL Synchronous Handler */
	stp	x29, x30, [sp, #-16]!
	bl	_exception_entry
	bl	do_sync
	b	exception_exit

	.align	7		 /* Current EL IRQ Handler */
	stp	x29, x30, [sp, #-16]!
	bl	_exception_entry
	bl	do_irq
	b	exception_exit

	.align	7		 /* Current EL FIQ Handler */
	stp	x29, x30, [sp, #-16]!
	bl	_exception_entry
	bl	do_fiq
	b	exception_exit

	.align	7		 /* Current EL Error Handler */
	stp	x29, x30, [sp, #-16]!
	bl	_exception_entry
	bl	do_error
	b	exception_exit

/*
 * Enter Exception.
 * This will save the processor state that is ELR/X0~X30
 * to the stack frame.
 */
_exception_entry:
	stp	x27, x28, [sp, #-16]!
	stp	x25, x26, [sp, #-16]!
	stp	x23, x24, [sp, #-16]!
	stp	x21, x22, [sp, #-16]!
	stp	x19, x20, [sp, #-16]!
	stp	x17, x18, [sp, #-16]!
	stp	x15, x16, [sp, #-16]!
	stp	x13, x14, [sp, #-16]!
	stp	x11, x12, [sp, #-16]!
	stp	x9, x10, [sp, #-16]!
	stp	x7, x8, [sp, #-16]!
	stp	x5, x6, [sp, #-16]!
	stp	x3, x4, [sp, #-16]!
	stp	x1, x2, [sp, #-16]!

	/* Could be running at EL3/EL2/EL1 */
	switch_el x11, 3f, 2f, 1f
3:	mrs	x1, esr_el3
	mrs	x2, elr_el3
	b	0f
2:	mrs	x1, esr_el2
	mrs	x2, elr_el2
	b	0f
1:	mrs	x1, esr_el1
	mrs	x2, elr_el1
0:
	stp	x2, x0, [sp, #-16]!
	mov	x0, sp
	ret


exception_exit:
	ldp	x2, x0, [sp],#16
	switch_el x11, 3f, 2f, 1f
3:	msr	elr_el3, x2
	b	0f
2:	msr	elr_el2, x2
	b	0f
1:	msr	elr_el1, x2
0:
	ldp	x1, x2, [sp],#16
	ldp	x3, x4, [sp],#16
	ldp	x5, x6, [sp],#16
	ldp	x7, x8, [sp],#16
	ldp	x9, x10, [sp],#16
	ldp	x11, x12, [sp],#16
	ldp	x13, x14, [sp],#16
	ldp	x15, x16, [sp],#16
	ldp	x17, x18, [sp],#16
	ldp	x19, x20, [sp],#16
	ldp	x21, x22, [sp],#16
	ldp	x23, x24, [sp],#16
	ldp	x25, x26, [sp],#16
	ldp	x27, x28, [sp],#16
	ldp	x29, x30, [sp],#16
	eret
