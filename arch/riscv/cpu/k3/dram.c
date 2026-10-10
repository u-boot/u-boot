// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Yixun Lan <dlan@kernel.org>
 */

#include <fdtdec.h>
#include <init.h>
#include <linux/sizes.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}
