/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014 Google, Inc
 * Copyright (C) 1991, 1992, 1993  Linus Torvalds
 *
 * Parts of this copied from Linux arch/x86/boot/compressed/head_64.S
 */

#include <asm/msr-index.h>
#include <asm/processor.h>
#include <asm/processor-flags.h>

.code32
.section .text_call64, "ax"
.globl cpu_call64
cpu_call64:
	/*
	 * cpu_call64(ulong pgtable, ulong setup_base, ulong target)
	 *
	 * eax - pgtable
	 * edx - setup_base
	 * ecx - target
	 */
	cli
	pushl	$0		/* top 64-bits of target */
	push	%ecx		/* arg2 = target */
	push	%edx		/* arg1 = setup_base */
	mov	%eax, %ebx

	# disable paging
	movl	%cr0, %eax
	andl	$~X86_CR0_PG, %eax
	movl	%eax, %cr0

	/* Enable PAE mode */
	movl	%cr4, %eax
	orl	$X86_CR4_PAE, %eax
	movl	%eax, %cr4

	/* Enable the boot page tables */
	leal	(%ebx), %eax
	movl	%eax, %cr3

	/* Enable Long mode in EFER (Extended Feature Enable Register) */
	movl	$MSR_EFER, %ecx
	rdmsr
	btsl	$_EFER_LME, %eax
	wrmsr

	/*
	 * Setup for the jump to 64bit mode
	 *
	 * When the jump is performed we will be in long mode but
	 * in 32bit compatibility mode with EFER.LME = 1, CS.L = 0, CS.D = 1
	 * (and in turn EFER.LMA = 1). To jump into 64bit mode we use
	 * the new gdt/idt that has __KERNEL_CS with CS.L = 1.
	 * We place all of the values on our mini stack so lret can
	 * used to perform that far jump. See the gdt below.
	 */
	pop	%esi			/* setup_base */

	/* Enter paged protected Mode, activating Long Mode */
	movl	%cr0, %eax
	orl	$X86_CR0_PG, %eax
	movl	%eax, %cr0

	/* Jump from 32bit compatibility mode into 64bit mode. */
	ljmp	$(X86_GDT_ENTRY_64BIT_CS * X86_GDT_ENTRY_SIZE), $lret_target

.code64
lret_target:
	pop	%rax			/* target */
	jmp	*%rax			/* Jump to the 64-bit target */

.globl call64_stub_size
call64_stub_size:
	.long	. - cpu_call64

	.data
	.align	16
	.globl	gdt64
gdt64:
gdt:
	.word	gdt_end - gdt - 1
	.long	gdt			/* Fixed up by code above */
	.word	0
	.quad	0x0000000000000000	/* NULL descriptor */
	.quad	0x00af9a000000ffff	/* __KERNEL_CS */
	.quad	0x00cf92000000ffff	/* __KERNEL_DS */
	.quad	0x0080890000000000	/* TS descriptor */
	.quad   0x0000000000000000	/* TS continued */
gdt_end:
