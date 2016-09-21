/*
 * Copyright (C) 2016 Imagination Technologies
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>

#include <asm/io.h>

#include "boston-regs.h"

phys_size_t initdram(int board_type)
{
	u32 ddrconf0 = __raw_readl((uint32_t *)BOSTON_PLAT_DDRCONF0);

	return (phys_size_t)(ddrconf0 & BOSTON_PLAT_DDRCONF0_SIZE) << 30;
}

ulong board_get_usable_ram_top(ulong total_size)
{
	DECLARE_GLOBAL_DATA_PTR;

	if (gd->ram_top < CONFIG_SYS_SDRAM_BASE) {
		/* 2GB wrapped around to 0 */
		return CKSEG0ADDR(256 << 20);
	}

	return min_t(unsigned long, gd->ram_top, CKSEG0ADDR(256 << 20));
}
