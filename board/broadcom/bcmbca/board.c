// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2022 Broadcom Ltd.
 */

#include <fdtdec.h>

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		puts("fdtdec_setup_mem_size_base() has failed\n");

	return 0;
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();
	return 0;
}

int print_cpuinfo(void)
{
	return 0;
}

__weak void reset_cpu(void)
{
}
