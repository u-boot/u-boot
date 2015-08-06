/*
 *  U-Boot - x86 Startup Code
 *
 * (C) Copyright 2008-2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <asm/global_data.h>
#include <asm/post.h>
#include <asm/processor.h>
#include <asm/processor-flags.h>
#include <generated/generic-asm-offsets.h>
#include <generated/asm-offsets.h>

.section .text
.code32
.globl _start
.type _start, @function
.globl _x86boot_start
_x86boot_start:
	/*
	 * This is the fail-safe 32-bit bootstrap entry point.
	 *
	 * This code is used when booting from another boot loader like
	 * coreboot or EFI. So we repeat some of the same init found in
	 * start16.
	 */
	cli
	cld

	/* Turn off cache (this might require a 486-class CPU) */
	movl	%cr0, %eax
	orl	$(X86_CR0_NW | X86_CR0_CD), %eax
	movl	%eax, %cr0
	wbinvd

	/* Tell 32-bit code it is being entered from an in-RAM copy */
	movl	$GD_FLG_WARM_BOOT, %ebx

	/*
	 * Zero the BIST (Built-In Self Test) value since we don't have it.
	 * It must be 0 or the previous loader would have reported an error.
	 */
	movl	$0, %ebp

	jmp	1f

	/* Add a way for tools to discover the _start entry point */
	.align	4
	.long	0x12345678
_start:
	/*
	 * This is the 32-bit cold-reset entry point, coming from start16.
	 * Set %ebx to GD_FLG_COLD_BOOT to indicate this.
	 */
	movl	$GD_FLG_COLD_BOOT, %ebx

	/* Save BIST */
	movl	%eax, %ebp
1:

	/* Save table pointer */
	movl	%ecx, %esi

	/* Load the segement registers to match the GDT loaded in start16.S */
	movl	$(X86_GDT_ENTRY_32BIT_DS * X86_GDT_ENTRY_SIZE), %eax
	movw	%ax, %fs
	movw	%ax, %ds
	movw	%ax, %gs
	movw	%ax, %es
	movw	%ax, %ss

	/* Clear the interrupt vectors */
	lidt	blank_idt_ptr

	/*
	 * Critical early platform init - generally not used, we prefer init
	 * to happen later when we have a console, in case something goes
	 * wrong.
	 */
	jmp	early_board_init
.globl early_board_init_ret
early_board_init_ret:
	post_code(POST_START)

	/* Initialise Cache-As-RAM */
	jmp	car_init
.globl car_init_ret
car_init_ret:
#ifndef CONFIG_HAVE_FSP
	/*
	 * We now have CONFIG_SYS_CAR_SIZE bytes of Cache-As-RAM (or SRAM,
	 * or fully initialised SDRAM - we really don't care which)
	 * starting at CONFIG_SYS_CAR_ADDR to be used as a temporary stack
	 * and early malloc() area. The MRC requires some space at the top.
	 *
	 * Stack grows down from top of CAR. We have:
	 *
	 * top-> CONFIG_SYS_CAR_ADDR + CONFIG_SYS_CAR_SIZE
	 *	MRC area
	 *	global_data
	 *	x86 global descriptor table
	 *	early malloc area
	 *	stack
	 * bottom-> CONFIG_SYS_CAR_ADDR
	 */
	movl	$(CONFIG_SYS_CAR_ADDR + CONFIG_SYS_CAR_SIZE - 4), %esp
#ifdef CONFIG_DCACHE_RAM_MRC_VAR_SIZE
	subl	$CONFIG_DCACHE_RAM_MRC_VAR_SIZE, %esp
#endif
#else
	/*
	 * When we get here after car_init(), esp points to a temporary stack
	 * and esi holds the HOB list address returned by the FSP.
	 */
#endif

	/* Reserve space on stack for global data */
	subl	$GENERATED_GBL_DATA_SIZE, %esp

	/* Align global data to 16-byte boundary */
	andl	$0xfffffff0, %esp
	post_code(POST_START_STACK)

	/*
	 * Debug UART is available here although it may not be plumbed out
	 * to pins depending on the board. To use it:
	 *
	 * call  debug_uart_init
	 * mov   $'a', %eax
	 * call  printch
	 */

	/* Zero the global data since it won't happen later */
	xorl	%eax, %eax
	movl	$GENERATED_GBL_DATA_SIZE, %ecx
	movl	%esp, %edi
	rep	stosb

#ifdef CONFIG_HAVE_FSP
	test	%esi, %esi
	jz	skip_hob

	/* Store HOB list */
	movl	%esp, %edx
	addl	$GD_HOB_LIST, %edx
	movl	%esi, (%edx)

skip_hob:
#else
	/* Store table pointer */
	movl	%esp, %edx
	addl	$GD_TABLE, %edx
	movl	%esi, (%edx)
#endif

	/* Setup first parameter to setup_gdt, pointer to global_data */
	movl	%esp, %eax

	/* Reserve space for global descriptor table */
	subl	$X86_GDT_SIZE, %esp

	/* Align temporary global descriptor table to 16-byte boundary */
	andl	$0xfffffff0, %esp
	movl	%esp, %ecx

#if defined(CONFIG_SYS_MALLOC_F_LEN)
	/* Set up the pre-relocation malloc pool */
	subl	$CONFIG_SYS_MALLOC_F_LEN, %esp
	movl	%eax, %edx
	addl	$GD_MALLOC_BASE, %edx
	movl	%esp, (%edx)
#endif
	/* Store BIST into global_data */
	movl	%eax, %edx
	addl	$GD_BIST, %edx
	movl	%ebp, (%edx)

	/* Set second parameter to setup_gdt() */
	movl	%ecx, %edx

	/* Setup global descriptor table so gd->xyz works */
	call	setup_gdt

	/* Set parameter to board_init_f() to boot flags */
	post_code(POST_START_DONE)
	xorl	%eax, %eax

	/* Enter, U-Boot! */
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

	/* Setup first parameter to memcpy() and setup_gdt() */
	movl	%esp, %eax

	/* Setup second parameter to memcpy() */
	fs movl 0, %edx

	/* Set third parameter to memcpy() */
	movl	$GENERATED_GBL_DATA_SIZE, %ecx

	/* Copy global data from CAR to SDRAM stack */
	call	memcpy

	/* Reserve space for global descriptor table */
	subl	$X86_GDT_SIZE, %esp

	/* Align global descriptor table to 16-byte boundary */
	andl	$0xfffffff0, %esp

	/* Set second parameter to setup_gdt() */
	movl	%esp, %edx

	/* Setup global descriptor table so gd->xyz works */
	call	setup_gdt

	/* Set if we need to disable CAR */
.weak	car_uninit
	movl	$car_uninit, %eax
	cmpl	$0, %eax
	jz	1f

	call	car_uninit
1:
	/* Re-enter U-Boot by calling board_init_f_r() */
	call	board_init_f_r

die:
	hlt
	jmp	die
	hlt

blank_idt_ptr:
	.word	0		/* limit */
	.long	0		/* base */

	.p2align	2	/* force 4-byte alignment */

	/* Add a multiboot header so U-Boot can be loaded by GRUB2 */
multiboot_header:
	/* magic */
	.long	0x1badb002
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
