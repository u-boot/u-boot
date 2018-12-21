/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _ASM_SYSCON_H
#define _ASM_SYSCON_H

/*
 * System controllers in a RISC-V system
 *
 * So far only SiFive's Core Local Interruptor (CLINT) is defined.
 */
enum {
	RISCV_NONE,
	RISCV_SYSCON_CLINT,	/* Core Local Interruptor (CLINT) */
};

#endif /* _ASM_SYSCON_H */
