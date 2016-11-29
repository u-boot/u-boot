/*
 * crt0-efi-x86_64.S - x86_64 EFI startup code.
 * Copyright (C) 1999 Hewlett-Packard Co.
 * Contributed by David Mosberger <davidm@hpl.hp.com>.
 * Copyright (C) 2005 Intel Co.
 * Contributed by Fenghua Yu <fenghua.yu@intel.com>.
 *
 * All rights reserved.
 * SPDX-License-Identifier:	BSD-3-Clause
 */
	.text
	.align 4

	.globl _start
_start:
	subq $8, %rsp
	pushq %rcx
	pushq %rdx

0:
	lea image_base(%rip), %rdi
	lea _DYNAMIC(%rip), %rsi

	popq %rcx
	popq %rdx
	pushq %rcx
	pushq %rdx
	call _relocate

	popq %rdi
	popq %rsi

	call efi_main
	addq $8, %rsp

.exit:
	ret

	/*
	 * hand-craft a dummy .reloc section so EFI knows it's a relocatable
	 * executable:
	 */
	.data
dummy:	.long	0

#define IMAGE_REL_ABSOLUTE	0
	.section .reloc, "a"
label1:
	.long	dummy-label1				/* Page RVA */
	.long	10					/* Block Size (2*4+2) */
	.word	(IMAGE_REL_ABSOLUTE << 12) +  0		/* reloc for dummy */
