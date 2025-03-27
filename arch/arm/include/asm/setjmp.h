/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 * (C) Copyright 2016 Alexander Graf <agraf@suse.de>
 */

#ifndef _ASM_SETJMP_H_
#define _ASM_SETJMP_H_	1

#include <asm-generic/int-ll64.h>

struct jmp_buf_data {
#if defined(__aarch64__)
	u64  regs[13];
#else
	u32  regs[10];  /* r4-r9, sl, fp, sp, lr */
#endif
};

#endif /* _ASM_SETJMP_H_ */
