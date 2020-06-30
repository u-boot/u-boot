// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) Stefan Roese <sr@denx.de>
 */

#include <dm.h>
#include <ram.h>
#include <asm/global_data.h>
#include <linux/compat.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	/*
	 * No DDR init yet -> run in L2 cache
	 */
	gd->ram_size = (4 << 20);
	gd->bd->bi_dram[0].size = gd->ram_size;
	gd->bd->bi_dram[1].size = 0;

	return 0;
}

ulong board_get_usable_ram_top(ulong total_size)
{
	return gd->ram_top;
}
