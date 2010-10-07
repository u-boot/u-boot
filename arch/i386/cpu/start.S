/*
 *  U-boot - i386 Startup Code
 *
 *  Copyright (c) 2002	Omicron Ceti AB, Daniel Engstr�m <denaiel@omicron.se>
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
#include <asm/global_data.h>


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
	orl	$0x60000000, %eax
	movl	%eax, %cr0
	wbinvd

	/* Tell 32-bit code it is being entered from an in-RAM copy */
	movw	$GD_FLG_WARM_BOOT, %bx
_start:
	/* This is the 32-bit cold-reset entry point */

	movl	$0x18, %eax	/* Load our segement registes, the
				 * gdt have already been loaded by start16.S */
	movw	%ax, %fs
	movw	%ax, %ds
	movw	%ax, %gs
	movw	%ax, %es
	movw	%ax, %ss

	/* Clear the interupt vectors */
	lidt	blank_idt_ptr

	/* Skip low-level initialization if not starting from cold-reset */
	movl	%ebx, %ecx
	andl	$GD_FLG_COLD_BOOT, %ecx
	jz	skip_mem_init

	/* Early platform init (setup gpio, etc ) */
	jmp	early_board_init
.globl early_board_init_ret
early_board_init_ret:

	/* size memory */
	jmp	mem_init
.globl mem_init_ret
mem_init_ret:

skip_mem_init:
	/* fetch memory size (into %eax) */
	jmp	get_mem_size
.globl get_mem_size_ret
get_mem_size_ret:

#if CONFIG_SYS_SDRAM_ECC_ENABLE
	/* Skip ECC initialization if not starting from cold-reset */
	movl	%ebx, %ecx
	andl	$GD_FLG_COLD_BOOT, %ecx
	jz	init_ecc_ret
	jmp	init_ecc

.globl init_ecc_ret
init_ecc_ret:
#endif

	/* Check we have enough memory for stack */
	movl	$CONFIG_SYS_STACK_SIZE, %ecx
	cmpl	%ecx, %eax
	jb	die
mem_ok:
	/* Set stack pointer to upper memory limit*/
	movl	%eax, %esp

	/* Test the stack */
	pushl	$0
	popl	%ecx
	cmpl	$0, %ecx
	jne	die
	push	$0x55aa55aa
	popl	%ecx
	cmpl	$0x55aa55aa, %ecx
	jne	die

	wbinvd

	/* Determine our load offset */
	call	1f
1:	popl	%ecx
	subl	$1b, %ecx

	/* Set the upper memory limit parameter */
	subl	$CONFIG_SYS_STACK_SIZE, %eax

	/* Reserve space for global data */
	subl	$(GD_SIZE * 4), %eax

	/* %eax points to the global data structure */
	movl	%esp, (GD_RAM_SIZE * 4)(%eax)
	movl	%ebx, (GD_FLAGS * 4)(%eax)
	movl	%ecx, (GD_LOAD_OFF * 4)(%eax)

	call	board_init_f	/* Enter, U-boot! */

	/* indicate (lack of) progress */
	movw	$0x85, %ax
die:	hlt
	jmp	die
	hlt

blank_idt_ptr:
	.word	0		/* limit */
	.long	0		/* base */
