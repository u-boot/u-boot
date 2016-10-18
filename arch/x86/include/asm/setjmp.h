/*
 * Written by H. Peter Anvin <hpa@zytor.com>
 * Brought in from Linux v4.4 and modified for U-Boot
 * From Linux arch/um/sys-i386/setjmp.S
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef __setjmp_h
#define __setjmp_h

struct jmp_buf_data {
	unsigned int __ebx;
	unsigned int __esp;
	unsigned int __ebp;
	unsigned int __esi;
	unsigned int __edi;
	unsigned int __eip;
};

int setjmp(struct jmp_buf_data *jmp_buf);
void longjmp(struct jmp_buf_data *jmp_buf, int val);

#endif
