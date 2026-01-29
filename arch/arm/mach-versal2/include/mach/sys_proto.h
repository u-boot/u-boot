/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2021 - 2022, Xilinx, Inc.
 * Copyright (C) 2022 - 2024, Advanced Micro Devices, Inc.
 */

#include <linux/build_bug.h>
#include <asm/armv8/mmu.h>

void mem_map_fill(struct mm_region *bank_info, u32 num_banks);
void fill_bd_mem_info(void);
