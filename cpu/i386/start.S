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
	movl    $0x18,%eax         	/* Load our segement registes, the
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
	jmp	__show_boot_progress
.progress0:

	/* size memory */
	mov	$mem_init_ret, %ebp
	jmp     mem_init
mem_init_ret:

	/* check ammount of configured memory
	 * (we need atleast bss start+bss size+stack size) */
	movl	$_i386boot_bss_start, %ecx        /* BSS start */
	addl	$_i386boot_bss_size, %ecx         /* BSS size */
	addl	$CFG_STACK_SIZE, %ecx
	cmpl	%ecx, %eax
	jae	mem_ok

	/* indicate (lack of) progress */
	movw	$0x81, %ax
	movl	$.progress0a, %ebp
	jmp	__show_boot_progress
.progress0a:
	jmp 	die
mem_ok:

	/* indicate progress */
	movw	$0x02, %ax
	movl	$.progress1, %ebp
	jmp	__show_boot_progress
.progress1:

	/* create a stack after the bss */
	movl    $_i386boot_bss_start, %eax
	addl	$_i386boot_bss_size, %eax
	addl	$CFG_STACK_SIZE, %eax
	movl    %eax, %esp

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
	jmp	__show_boot_progress
.progress1a:
	jmp die


stack_ok:
	/* indicate progress */
	movw	$0x03, %ax
	movl	$.progress2, %ebp
	jmp	__show_boot_progress
.progress2:

	/* copy data section to ram, size must be 4-byte aligned */
	movl	$_i386boot_romdata_dest, %edi	  /* destination address */
	movl	$_i386boot_romdata_start, %esi	  /* source address */
	movl	$_i386boot_romdata_size, %ecx     /* number of bytes to copy */
	movl	%ecx, %eax
	andl	$3, %eax
	jnz	data_fail

	shrl	$2, %ecx	                  /* copy 4 byte each time */
	cld
	cmpl	$0, %ecx
	je	data_ok
data_segment:
	movsl
	loop	data_segment
	jmp	data_ok
data_fail:
	/* indicate (lack of) progress */
	movw	$0x83, %ax
	movl	$.progress2a, %ebp
	jmp	__show_boot_progress
.progress2a:
	jmp 	die

data_ok:

	/* indicate progress */
	movw	$0x04, %ax
	movl	$.progress3, %ebp
	jmp	__show_boot_progress
.progress3:

	/* clear bss section in ram, size must be 4-byte aligned  */
	movl	$_i386boot_bss_start, %eax        /* BSS start */
	movl	$_i386boot_bss_size, %ecx         /* BSS size */
	movl	%ecx, %eax
	andl	$3, %eax
	jnz	bss_fail
	shrl	$2, %ecx	                  /* clear 4 byte each time */
	cld
	cmpl	$0, %ecx
	je	bss_ok
bss:
	movl	$0, (%edi)
	add	$4, %edi
	loop	bss
	jmp 	bss_ok

bss_fail:
	/* indicate (lack of) progress */
	movw	$0x84, %ax
	movl	$.progress3a, %ebp
	jmp	__show_boot_progress
.progress3a:
	jmp 	die

bss_ok:

	wbinvd


	/* indicate progress */
	movw	$0x05, %ax
	movl	$.progress4, %ebp
	jmp	__show_boot_progress
.progress4:

	call	start_i386boot  /* Enter, U-boot! */

	/* indicate (lack of) progress */
	movw	$0x85, %ax
	movl	$.progress4a, %ebp
	jmp	__show_boot_progress
.progress4a:

die:	hlt
	jmp	die
	hlt
