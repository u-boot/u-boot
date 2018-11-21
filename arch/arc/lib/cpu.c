// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013-2014, 2018 Synopsys, Inc. All rights reserved.
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

#ifdef CONFIG_DISPLAY_CPUINFO
const char *decode_identity(void)
{
	int arcver = read_aux_reg(ARC_AUX_IDENTITY) & 0xff;

	switch (arcver) {
	/* ARCompact cores */
	case 0x32: return "ARC 700 v4.4-4.5";
	case 0x33: return "ARC 700 v4.6-v4.9";
	case 0x34: return "ARC 700 v4.10";
	case 0x35: return "ARC 700 v4.11";

	/* ARCv2 cores */
	case 0x41: return "ARC EM v1.1a";
	case 0x42: return "ARC EM v3.0";
	case 0x43: return "ARC EM v4.0";
	case 0x50: return "ARC HS v1.0";
	case 0x51: return "ARC EM v2.0";
	case 0x52: return "ARC EM v2.1";
	case 0x53: return "ARC HS v3.0";
	case 0x54: return "ARC HS v4.0";

	default: return "Unknown ARC core";
	}
}

__weak int print_cpuinfo(void)
{
	printf("CPU:   %s\n", decode_identity());
	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */
