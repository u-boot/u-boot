// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Renesas Electronics
 * Copyright (C) Chris Brandt
 */

#include <errno.h>
#include <init.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	gd->bd->bi_boot_params = (CFG_SYS_SDRAM_BASE + 0x100);

	return 0;
}

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}
