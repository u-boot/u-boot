// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Board specific initialization for J784S4 EVM
 *
 * Copyright (C) 2023-2024 Texas Instruments Incorporated - https://www.ti.com/
 *	Hari Nagalla <hnagalla@ti.com>
 *
 */

#include <init.h>
#include <spl.h>
#include "../common/fdt_ops.h"

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	ti_set_fdt_env(NULL, NULL);
	return 0;
}
#endif

void spl_board_init(void)
{
}
