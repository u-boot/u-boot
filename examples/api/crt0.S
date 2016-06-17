/*
 * (C) Copyright 2007 Semihalf
 *
 * Written by: Rafal Jaworowski <raj@semihalf.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

#elif defined(CONFIG_ARM)

	.text
	.globl _start
_start:
	ldr	ip, =search_hint
	str	sp, [ip]
	b	main


	.globl syscall
syscall:
	ldr	ip, =syscall_ptr
	ldr	pc, [ip]

#elif defined(CONFIG_MIPS)
	.text
	.globl __start
	.ent __start
__start:
	sw	$sp, search_hint
	b	main
	.end __start

	.globl syscall
	.ent syscall
syscall:
	sw	$ra, return_addr
	lw	$t9, syscall_ptr
	jalr	$t9
	nop
	lw	$ra, return_addr
	jr	$ra
	nop
	.end syscall

return_addr:
	.align 4
	.long 0
#else
#error No support for this arch!
#endif

	.globl syscall_ptr
syscall_ptr:
	.align	4
	.long	0

	.globl search_hint
search_hint:
	.long   0
