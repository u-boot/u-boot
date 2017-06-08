/*
 * Copyright (c) 2017 Andy Yan
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/grf_rk3368.h>
#include <syscon.h>

DECLARE_GLOBAL_DATA_PTR;

int mach_cpu_init(void)
{
	return 0;
}

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
	gd->bd->bi_dram[0].start = 0x200000;
	gd->bd->bi_dram[0].size = 0x7fe00000;

	return 0;
}
