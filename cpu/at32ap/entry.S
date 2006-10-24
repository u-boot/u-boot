/*
 * Copyright (C) 2004-2006 Atmel Corporation
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
#include <asm/sysreg.h>
#include <asm/ptrace.h>

	.section .text.exception,"ax"
	.global	_evba
	.type	_evba,@function
	.align	10
_evba:
	.irp	x,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
	.align	2
	rjmp	unknown_exception
	.endr

	.global	timer_interrupt_handler
	.type	timer_interrupt_handler,@function
	.align	2
timer_interrupt_handler:
	/*
	 * Increment timer_overflow and re-write COMPARE with 0xffffffff.
	 *
	 * We're running at interrupt level 3, so we don't need to save
	 * r8-r12 or lr to the stack.
	 */
	mov	r8, lo(timer_overflow)
	orh	r8, hi(timer_overflow)
	ld.w	r9, r8[0]
	mov	r10, -1
	mtsr	SYSREG_COMPARE, r10
	sub	r9, -1
	st.w	r8[0], r9
	rete

	.type	unknown_exception, @function
unknown_exception:
	pushm	r0-r12
	sub	r8, sp, REG_R12 - REG_R0 - 4
	mov	r9, lr
	mfsr	r10, SYSREG_RAR_EX
	mfsr	r11, SYSREG_RSR_EX
	pushm	r8-r11
	mfsr	r12, SYSREG_ECR
	mov	r11, sp
	rcall	do_unknown_exception
1:	rjmp	1b
