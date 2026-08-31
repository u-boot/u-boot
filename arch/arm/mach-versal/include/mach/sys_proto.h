/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 - 2018 Xilinx, Inc.
 */

#ifndef _ASM_ARCH_SYS_PROTO_H
#define _ASM_ARCH_SYS_PROTO_H

#include <linux/build_bug.h>

enum tcm_mode {
	TCM_LOCK = 0,
	TCM_SPLIT = 1,
};

void initialize_tcm(enum tcm_mode mode);
void tcm_init(enum tcm_mode mode);
void mem_map_fill(void);

/* Overridable PMC multiboot accessor: weak MMIO default, firmware override */
u32 versal_pmc_multi_boot(void);
/* Direct MMIO read of the multiboot register (EL3 / no-firmware path) */
u32 versal_multi_boot_reg(void);
/* Overridable bootmode decode: weak MMIO default, firmware override */
u8 versal_get_bootmode(void);
/* Direct MMIO read of the bootmode register (EL3 / no-firmware path) */
u32 versal_bootmode_reg(void);
/* EL3 clock/timer register setup, called from board_early_init_r() */
void versal_timer_setup(void);

#endif /* _ASM_ARCH_SYS_PROTO_H */
