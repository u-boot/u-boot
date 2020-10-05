/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _ASM_SYSCON_H
#define _ASM_SYSCON_H

/*
 * System controllers in a RISC-V system. These should only be used for
 * identifying IPI controllers. Other devices should use DM to probe.
 */
enum {
	RISCV_NONE,
	RISCV_SYSCON_CLINT,	/* Core Local Interruptor (CLINT) */
	RISCV_SYSCON_PLIC,	/* Platform Level Interrupt Controller (PLIC) */
};

#endif /* _ASM_SYSCON_H */
