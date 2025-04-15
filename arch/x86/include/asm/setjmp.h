/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Written by H. Peter Anvin <hpa@zytor.com>
 * Brought in from Linux v4.4 and modified for U-Boot
 * From Linux arch/um/sys-i386/setjmp.S
 */

#ifndef _ASM_SETJMP_H_
#define _ASM_SETJMP_H_	1

#ifdef CONFIG_X86_64

struct jmp_buf_data {
	unsigned long __rip;
	unsigned long __rsp;
	unsigned long __rbp;
	unsigned long __rbx;
	unsigned long __r12;
	unsigned long __r13;
	unsigned long __r14;
	unsigned long __r15;
};

#else

struct jmp_buf_data {
	unsigned int __ebx;
	unsigned int __esp;
	unsigned int __ebp;
	unsigned int __esi;
	unsigned int __edi;
	unsigned int __eip;
};

#endif

#endif /* _ASM_SETJMP_H_ */
