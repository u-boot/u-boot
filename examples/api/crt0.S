/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007 Semihalf
 *
 * Written by: Rafal Jaworowski <raj@semihalf.com>
 */

#if defined(CONFIG_PPC)

	.text
	.globl _start
_start:
	lis	%r11, search_hint@ha
	addi	%r11, %r11, search_hint@l
	stw	%r1, 0(%r11)
	b	main


	.globl syscall
syscall:
	lis	%r11, syscall_ptr@ha
	addi	%r11, %r11, syscall_ptr@l
	lwz	%r11, 0(%r11)
	mtctr	%r11
	bctr

#elif defined(CONFIG_ARM) && !defined(CONFIG_ARM64)

	.text
	.globl _start
_start:
	ldr	r4, =search_hint
	mov	r5, sp
	str	r5, [r4]
	b	main


	.globl syscall
syscall:
	ldr	r4, =syscall_ptr
	ldr	r4, [r4]
	bx	r4

#elif defined(CONFIG_ARM64)

	.text
	.globl _start
_start:
	ldr	x17, =search_hint
	mov	x16, sp
	str	x16, [x17]
	b	main

	.globl syscall
syscall:
	ldr	x16, =syscall_ptr
	ldr	x16, [x16]
	br	x16

#elif defined(CONFIG_MIPS)
#include <asm/asm.h>
	.text
	.globl __start
	.ent __start
__start:
	PTR_S	$sp, search_hint
	b	main
	.end __start

	.globl syscall
	.ent syscall
syscall:
	PTR_S	$ra, return_addr
	PTR_L	$t9, syscall_ptr
	jalr	$t9
	nop
	PTR_L	$ra, return_addr
	jr	$ra
	nop
	.end syscall

return_addr:
	.align 8
	.long 0
#elif defined(CONFIG_ARCH_RV32I)

	.text
	.globl _start
_start:
	la	t0, search_hint
	sw	sp, 0(t0)
	la	t0, main
	jalr	x0, t0

	.globl syscall
syscall:
	la	t0, syscall_ptr
	lw	t0, 0(t0)
	jalr	x0, t0

#elif defined(CONFIG_ARCH_RV64I)

	.text
	.globl _start
_start:
	la	t0, search_hint
	sd	sp, 0(t0)
	la	t0, main
	jalr	x0, t0

	.globl syscall
syscall:
	la	t0, syscall_ptr
	ld	t0, 0(t0)
	jalr	x0, t0

#else
#error No support for this arch!
#endif

.section .data

	.globl syscall_ptr
syscall_ptr:
	.align	8
	.long	0

	.globl search_hint
search_hint:
	.long	0
