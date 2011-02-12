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
#include <asm/global_data.h>
#include <asm/processor-flags.h>


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
	orl	$(X86_CR0_NW | X86_CR0_CD), %eax
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

	/* Early platform init (setup gpio, etc ) */
	jmp	early_board_init
.globl early_board_init_ret
early_board_init_ret:

	/* Initialise Cache-As-RAM */
	jmp	car_init
.globl car_init_ret
car_init_ret:
	/*
	 * We now have CONFIG_SYS_CAR_SIZE bytes of Cache-As-RAM (or SRAM,
	 * or fully initialised SDRAM - we really don't care which)
	 * starting at CONFIG_SYS_CAR_ADDR to be used as a temporary stack
	 */
	movl	$CONFIG_SYS_INIT_SP_ADDR, %esp

	/* Skip memory initialization if not starting from cold-reset */
	movl	%ebx, %ecx
	andl	$GD_FLG_COLD_BOOT, %ecx
	jz	skip_mem_init

	/* size memory */
	call	mem_init

skip_mem_init:
	/* fetch memory size (into %eax) */
	call	get_mem_size
	movl	%eax, %esp

#if CONFIG_SYS_SDRAM_ECC_ENABLE
	/* Skip ECC initialization if not starting from cold-reset */
	movl	%ebx, %ecx
	andl	$GD_FLG_COLD_BOOT, %ecx
	jz	skip_ecc_init
	call	init_ecc

skip_init_ecc:
#endif

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

	/* Pointer to temporary global data */
	movl	$CONFIG_SYS_INIT_GD_ADDR, %edx

	/* %edx points to the global data structure */
	movl	%esp, (GD_RAM_SIZE * 4)(%edx)
	movl	%ebx, (GD_FLAGS * 4)(%edx)
	movl	%ecx, (GD_LOAD_OFF * 4)(%edx)

	call	board_init_f	/* Enter, U-boot! */

	/* indicate (lack of) progress */
	movw	$0x85, %ax
die:	hlt
	jmp	die
	hlt

blank_idt_ptr:
	.word	0		/* limit */
	.long	0		/* base */
