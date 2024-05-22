// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <fdtdec.h>

int dram_init(void)
{
	return fdtdec_setup_mem_size_base_lowest();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}
