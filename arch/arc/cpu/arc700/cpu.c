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
#ifdef CONFIG_SYS_ICACHE_OFF
	icache_disable();
#else
	icache_enable();
	invalidate_icache_all();
#endif

	flush_dcache_all();
#ifdef CONFIG_SYS_DCACHE_OFF
	dcache_disable();
#else
	dcache_enable();
#endif
	timer_init();

/* In simulation (ISS) "CHIPID" and "ARCNUM" are all "ff" */
	if ((read_aux_reg(ARC_AUX_IDENTITY) & 0xffffff00) == 0xffffff00)
		gd->arch.running_on_hw = 0;
	else
		gd->arch.running_on_hw = 1;

	gd->cpu_clk = CONFIG_SYS_CLK_FREQ;
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

int arch_early_init_r(void)
{
	gd->bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_memsize = CONFIG_SYS_SDRAM_SIZE;
	return 0;
}
