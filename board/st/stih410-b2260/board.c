/*
 * Board init file for STiH410-B2260
 *
 * (C) Copyright 2017 Patrice Chotard <patrice.chotard@st.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
}

int board_init(void)
{
	return 0;
}
