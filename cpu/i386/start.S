/*
 *  U-boot - i386 Startup Code
 *
 *  Copyright (c) 2002	Omicron Ceti AB, Daniel Engström <denaiel@omicron.se>
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


.section .text
.code32
.globl _start
.type _start, @function
.globl _i386boot_start
_i386boot_start:
_start:
	movl    $0x18,%eax	/* Load our segement registes, the
				 * gdt have already been loaded by start16.S */
	movw    %ax,%fs
	movw	%ax,%ds
	movw    %ax,%gs
	movw    %ax,%es
	movw    %ax,%ss

	/* We call a few functions in the board support package
	 * since we have no stack yet we'll have to use %ebp
	 * to store the return address */

	/* Early platform init (setup gpio, etc ) */
	mov     $early_board_init_ret, %ebp
	jmp	early_board_init
early_board_init_ret:

	/* The __port80 entry-point should be usabe by now */
	/* so we try to indicate progress */
	movw	$0x01, %ax
	movl	$.progress0, %ebp
	jmp	show_boot_progress_asm
.progress0:

	/* size memory */
	mov	$mem_init_ret, %ebp
	jmp     mem_init
mem_init_ret:

	/* Check we have enough memory for stack */
	movl	$CONFIG_SYS_STACK_SIZE, %ecx
	cmpl	%ecx, %eax
	jae	mem_ok

	/* indicate (lack of) progress */
	movw	$0x81, %ax
	movl	$.progress0a, %ebp
	jmp	show_boot_progress_asm
.progress0a:
	jmp	die
mem_ok:
	/* Set stack pointer to upper memory limit*/
	movl    %eax, %esp

	/* indicate progress */
	movw	$0x02, %ax
	movl	$.progress1, %ebp
	jmp	show_boot_progress_asm
.progress1:

	/* Test the stack */
	pushl	$0
	popl	%eax
	cmpl	$0, %eax
	jne	no_stack
	push	$0x55aa55aa
	popl	%ebx
	cmpl	$0x55aa55aa, %ebx
	je	stack_ok

no_stack:
	/* indicate (lack of) progress */
	movw	$0x82, %ax
	movl	$.progress1a, %ebp
	jmp	show_boot_progress_asm
.progress1a:
	jmp die


stack_ok:
	/* indicate progress */
	movw	$0x03, %ax
	movl	$.progress2, %ebp
	jmp	show_boot_progress_asm
.progress2:

	wbinvd

	/* Get upper memory limit */
	movl %esp, %ecx
	subl $CONFIG_SYS_STACK_SIZE, %ecx

	/* Create a Stack Frame */
	pushl %ebp
	movl %esp, %ebp

	/* stack_limit parameter */
	pushl	%ecx
	call	board_init_f	/* Enter, U-boot! */

	/* indicate (lack of) progress */
	movw	$0x85, %ax
	movl	$.progress4a, %ebp
	jmp	show_boot_progress_asm
.progress4a:

die:	hlt
	jmp	die
	hlt
