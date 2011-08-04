/*
 *  U-boot - x86 Startup Code
 *
 * (C) Copyright 2008-2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
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
.globl _x86boot_start
_x86boot_start:
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

	/* Load the segement registes to match the gdt loaded in start16.S */
	movl	$0x18, %eax
	movw	%ax, %fs
	movw	%ax, %ds
	movw	%ax, %gs
	movw	%ax, %es
	movw	%ax, %ss

	/* Clear the interrupt vectors */
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

	/* Set parameter to board_init_f() to boot flags */
	xorl	%eax, %eax
	movw	%bx, %ax

	/* Enter, U-boot! */
	call	board_init_f

	/* indicate (lack of) progress */
	movw	$0x85, %ax
	jmp	die

.globl relocate_code
.type relocate_code, @function
relocate_code:
	/*
	 * SDRAM has been initialised, U-Boot code has been copied into
	 * RAM, BSS has been cleared and relocation adjustments have been
	 * made. It is now time to jump into the in-RAM copy of U-Boot
	 *
	 * %eax = Address of top of stack
	 * %edx = Address of Global Data
	 * %ecx = Base address of in-RAM copy of U-Boot
	 */

	/* Setup stack in RAM */
	movl	%eax, %esp

	/* Setup call address of in-RAM copy of board_init_r() */
	movl	$board_init_r, %ebp
	addl	(GD_RELOC_OFF * 4)(%edx), %ebp

	/* Setup parameters to board_init_r() */
	movl	%edx, %eax
	movl	%ecx, %edx

	/* Jump to in-RAM copy of board_init_r() */
	call	*%ebp

die:	hlt
	jmp	die
	hlt

blank_idt_ptr:
	.word	0		/* limit */
	.long	0		/* base */
