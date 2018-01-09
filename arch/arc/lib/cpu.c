/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arcregs.h>
#include <asm/cache.h>

DECLARE_GLOBAL_DATA_PTR;

int arch_cpu_init(void)
{
	timer_init();

	gd->cpu_clk = CONFIG_SYS_CLK_FREQ;
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	cache_init();

	return 0;
}

int arch_early_init_r(void)
{
	gd->bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_memsize = CONFIG_SYS_SDRAM_SIZE;
	return 0;
}

/* This is a dummy function on arc */
int dram_init(void)
{
	return 0;
}
