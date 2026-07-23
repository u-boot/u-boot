/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#ifndef _ASM_SETJMP_H_
#define _ASM_SETJMP_H_

/*
 * This really should be opaque, but the EFI implementation wrongly
 * assumes that a 'struct jmp_buf_data' is defined.
 */
struct jmp_buf_data {
	unsigned long s_regs[9];	/* s0 - 8 */
	unsigned long fp;
	unsigned long sp;
	unsigned long ra;
};

#endif /* _SETJMP_H_ */
