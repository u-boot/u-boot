// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@foss.st.com> for STMicroelectronics.
 */

/* This file defines target specific routines,
 * and stubs that override weak functions that need to be nop-ed out.
 */


#include <common.h>
#include <dm.h>
#include <init.h>
#include <log.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return 0;
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}

int board_init(void)
{
	return 0;
}

int arm_reserve_mmu(void)
{
        return 0;
}

phys_size_t get_effective_memsize(void)
    {
    return 0x00048000 - 64;          // use the 256K+32K at 0x30000000
    }

ulong board_get_usable_ram_top(ulong total_size)
    {
    return 0x30048000 - 64;          // use the RAM at 0x30000000
    }


