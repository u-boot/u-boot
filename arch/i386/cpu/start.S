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
	/*
	 * This is the fail safe 32-bit bootstrap entry point. The
	 * following code is not executed from a cold-reset (actually, a
	 * lot of it is, but from real-mode after cold reset. It is
	 * repeated here to put the board into a state as close to cold
	 * reset as necessary)
	 */
	cli
	cld

	/* Turn of cache (this might require a 486-class CPU) */
	movl	%cr0, %eax
	orl	$0x60000000,%eax
	movl	%eax, %cr0
	wbinvd

	/* Tell 32-bit code it is being entered from an in-RAM copy */
	movw	$0x0000, %bx
_start:
	/* This is the 32-bit cold-reset entry point */

	movl    $0x18,%eax	/* Load our segement registes, the
				 * gdt have already been loaded by start16.S */
	movw    %ax,%fs
	movw	%ax,%ds
	movw    %ax,%gs
	movw    %ax,%es
	movw    %ax,%ss

	/* Clear the interupt vectors */
	lidt	blank_idt_ptr

	/*
	 * Skip low-level board and memory initialization if not starting
	 * from cold-reset. This allows us to do a fail safe boot-strap
	 * into a new build of U-Boot from a known-good boot flash
	 */
	movw	$0x0001, %ax
	cmpw	%ax, %bx
	jne	mem_init_ret

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

	/* fetch memory size (into %eax) */
	mov	$get_mem_size_ret, %ebp
	jmp     get_mem_size
get_mem_size_ret:

	/*
	 * We are now in 'Flat Protected Mode' and we know how much memory
	 * the board has. The (temporary) Global Descriptor Table is not
	 * in a 'Safe' place (it is either in Flash which can be erased or
	 * reprogrammed or in a fail-safe boot-strap image which could be
	 * over-written).
	 *
	 * Move the final gdt to a safe place (top of RAM) and load it.
	 * This is not a trivial excercise - the lgdt instruction does not
	 * have a register operand (memory only) and we may well be
	 * running from Flash, so self modifying code will not work here.
	 * To overcome this, we copy a stub into upper memory along with
	 * the GDT.
	 */

	/* Reduce upper memory limit by (Stub + GDT Pointer + GDT) */
	subl	$(end_gdt_setup - start_gdt_setup), %eax

	/* Copy the GDT and Stub */
	movl	$start_gdt_setup, %esi
	movl	%eax, %edi
	movl	$(end_gdt_setup - start_gdt_setup), %ecx
	shrl	$2, %ecx
	cld
	rep	movsl

	/* write the lgdt 'parameter' */
	subl	$(jmp_instr - start_gdt_setup - 4), %ebp
	addl	%eax, %ebp
	movl	$(gdt_ptr - start_gdt_setup), %ebx
	addl	%eax, %ebx
	movl	%ebx, (%ebp)

	/* write the gdt address into the pointer */
	movl	$(gdt_addr - start_gdt_setup), %ebp
	addl	%eax, %ebp
	movl	$(gdt - start_gdt_setup), %ebx
	addl	%eax, %ebx
	movl	%ebx, (%ebp)

	/* Save the return address */
	movl	$load_gdt_ret, %ebp

	/* Load the new (safe) Global Descriptor Table */
	jmp	*%eax

load_gdt_ret:
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

blank_idt_ptr:
	.word	0		/* limit */
	.long	0		/* base */

.align 4
start_gdt_setup:
	lgdt	gdt_ptr
jmp_instr:
	jmp	*%ebp

.align 4
gdt_ptr:
	.word	0x30		/* limit (48 bytes = 6 GDT entries) */
gdt_addr:
	.long	gdt		/* base */

	/* The GDT table ...
	 *
	 *	 Selector	Type
	 *	 0x00		NULL
	 *	 0x08		Unused
	 *	 0x10		32bit code
	 *	 0x18		32bit data/stack
	 *	 0x20		16bit code
	 *	 0x28		16bit data/stack
	 */

.align 4
gdt:
	.word	0, 0, 0, 0	/* NULL  */
	.word	0, 0, 0, 0	/* unused */

	.word	0xFFFF		/* 4Gb - (0x100000*0x1000 = 4Gb) */
	.word	0		/* base address = 0 */
	.word	0x9B00		/* code read/exec */
	.word	0x00CF		/* granularity = 4096, 386 (+5th nibble of limit) */

	.word	0xFFFF		/* 4Gb - (0x100000*0x1000 = 4Gb) */
	.word	0x0		/* base address = 0 */
	.word	0x9300		/* data read/write */
	.word	0x00CF		/* granularity = 4096, 386 (+5th nibble of limit) */

	.word	0xFFFF		/* 64kb */
	.word	0		/* base address = 0 */
	.word	0x9b00		/* data read/write */
	.word	0x0010		/* granularity = 1  (+5th nibble of limit) */

	.word	0xFFFF		/* 64kb */
	.word	0		/* base address = 0 */
	.word	0x9300		/* data read/write */
	.word	0x0010		/* granularity = 1 (+5th nibble of limit) */
end_gdt_setup:
