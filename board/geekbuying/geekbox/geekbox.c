/*
 * Copyright (c) 2016 Andreas Färber
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = 0x80000000;
	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = 0;
	gd->bd->bi_dram[0].size = 0x80000000;

	return 0;
}
