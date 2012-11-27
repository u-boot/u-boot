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
#include <asm/processor.h>
#include <asm/processor-flags.h>
#include <generated/generic-asm-offsets.h>

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

	/* Turn off cache (this might require a 486-class CPU) */
	movl	%cr0, %eax
	orl	$(X86_CR0_NW | X86_CR0_CD), %eax
	movl	%eax, %cr0
	wbinvd

_start:
	/* This is the 32-bit cold-reset entry point */

	/* Load the segement registes to match the gdt loaded in start16.S */
	movl	$(X86_GDT_ENTRY_32BIT_DS * X86_GDT_ENTRY_SIZE), %eax
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

	/* Stack grows down from top of CAR */
	movl	$(CONFIG_SYS_CAR_ADDR + CONFIG_SYS_CAR_SIZE), %esp

	/* Reserve space on stack for global data */
	subl	$GENERATED_GBL_DATA_SIZE, %esp

	/* Align global data to 16-byte boundary */
	andl	$0xfffffff0, %esp

	/* Setup first parameter to setup_gdt */
	movl	%esp, %eax

	/* Reserve space for global descriptor table */
	subl	$X86_GDT_SIZE, %esp

	/* Align temporary global descriptor table to 16-byte boundary */
	andl	$0xfffffff0, %esp

	/* Set second parameter to setup_gdt */
	movl	%esp, %edx

	/* gd->gd_addr = gd (Required to allow gd->xyz to work) */
	movl	%eax, (%eax)

	/* Setup global descriptor table so gd->xyz works */
	call	setup_gdt

	/* Set parameter to board_init_f() to boot flags */
	xorl	%eax, %eax
	movw	%bx, %ax

	/* Enter, U-boot! */
	call	board_init_f

	/* indicate (lack of) progress */
	movw	$0x85, %ax
	jmp	die

.globl board_init_f_r_trampoline
.type board_init_f_r_trampoline, @function
board_init_f_r_trampoline:
	/*
	 * SDRAM has been initialised, U-Boot code has been copied into
	 * RAM, BSS has been cleared and relocation adjustments have been
	 * made. It is now time to jump into the in-RAM copy of U-Boot
	 *
	 * %eax = Address of top of new stack
	 */

	/* Stack grows down from top of SDRAM */
	movl	%eax, %esp

	/* Reserve space on stack for global data */
	subl	$GENERATED_GBL_DATA_SIZE, %esp

	/* Align global data to 16-byte boundary */
	andl	$0xfffffff0, %esp

	/* Setup first parameter to memcpy (and setup_gdt) */
	movl	%esp, %eax

	/* Setup second parameter to memcpy */
	fs movl 0, %edx

	/* Set third parameter to memcpy */
	movl	$GENERATED_GBL_DATA_SIZE, %ecx

	/* Copy global data from CAR to SDRAM stack */
	call	memcpy

	/* Reserve space for global descriptor table */
	subl	$X86_GDT_SIZE, %esp

	/* Align global descriptor table to 16-byte boundary */
	andl	$0xfffffff0, %esp

	/* Set second parameter to setup_gdt */
	movl	%esp, %edx

	/* gd->gd_addr = gd (Required to allow gd->xyz to work) */
	movl	%eax, (%eax)

	/* Setup global descriptor table so gd->xyz works */
	call	setup_gdt

	/* Re-enter U-Boot by calling board_init_f_r */
	call	board_init_f_r

die:
	hlt
	jmp	die
	hlt

blank_idt_ptr:
	.word	0		/* limit */
	.long	0		/* base */

	.p2align	2	/* force 4-byte alignment */

multiboot_header:
	/* magic */
	.long	0x1BADB002
	/* flags */
	.long	(1 << 16)
	/* checksum */
	.long	-0x1BADB002 - (1 << 16)
	/* header addr */
	.long	multiboot_header - _x86boot_start + CONFIG_SYS_TEXT_BASE
	/* load addr */
	.long	CONFIG_SYS_TEXT_BASE
	/* load end addr */
	.long	0
	/* bss end addr */
	.long	0
	/* entry addr */
	.long	CONFIG_SYS_TEXT_BASE
