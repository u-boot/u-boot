// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <asm/global_data.h>
#include <fdtdec.h>
#include <init.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

#define MPFS_TOP_OF_CACHED (SZ_2G + SZ_1G)
#define MPFS_HSS_RESERVATION (SZ_4M)

int dram_init(void) {
    return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void) {
    return fdtdec_setup_memory_banksize();
}

phys_size_t board_get_usable_ram_top(phys_size_t total_size) {
    /*
     * Ensure that if we run from 32-bit memory that all memory used by
     * U-Boot is cached addresses, but also account for the reservation at
     * the top of 32 bit cached DDR used by the HSS.
     */
    if (gd->ram_top >= MPFS_TOP_OF_CACHED - MPFS_HSS_RESERVATION)
        return MPFS_TOP_OF_CACHED - MPFS_HSS_RESERVATION - 1;
    /*
     * If we don't find a 32 bit region just return the top of memory.
     * If the address is a 32-bit region, but fits beneath the HSS'
     * reservation, ram_top is adequate also.
     */
    return gd->ram_top;
}