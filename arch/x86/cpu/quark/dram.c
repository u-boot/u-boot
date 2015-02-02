/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/post.h>
#include <asm/arch/quark.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	/* hardcode the DRAM size for now */
	gd->ram_size = DRAM_MAX_SIZE;
	post_code(POST_DRAM);

	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = 0;
	gd->bd->bi_dram[0].size = gd->ram_size;
}

/*
 * This function looks for the highest region of memory lower than 4GB which
 * has enough space for U-Boot where U-Boot is aligned on a page boundary.
 * It overrides the default implementation found elsewhere which simply
 * picks the end of ram, wherever that may be. The location of the stack,
 * the relocation address, and how far U-Boot is moved by relocation are
 * set in the global data structure.
 */
ulong board_get_usable_ram_top(ulong total_size)
{
	return gd->ram_size;
}
