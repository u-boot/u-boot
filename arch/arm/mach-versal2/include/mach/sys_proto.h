/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2021 - 2022, Xilinx, Inc.
 * Copyright (C) 2022 - 2024, Advanced Micro Devices, Inc.
 */

#ifndef _ASM_ARCH_SYS_PROTO_H
#define _ASM_ARCH_SYS_PROTO_H

#include <linux/build_bug.h>
#include <asm/armv8/mmu.h>

void mem_map_fill(struct mm_region *bank_info, u32 num_banks);
void fill_bd_mem_info(void);

/* Overridable PMC multiboot accessor: weak MMIO default, firmware override */
u32 versal2_pmc_multi_boot(void);
/* Direct MMIO read of the multiboot register (EL3 / no-firmware path) */
u32 versal2_multi_boot_reg(void);
/* Weak bootmode decode (MMIO default); a firmware/SCMI build may override */
u8 versal2_get_bootmode(void);
/* EL3 clock/timer register setup, called from board_early_init_r() */
void versal2_timer_setup(void);

#endif /* _ASM_ARCH_SYS_PROTO_H */
