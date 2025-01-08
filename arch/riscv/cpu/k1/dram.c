// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
 */

#include <asm/global_data.h>
#include <config.h>
#include <fdt_support.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_base = CFG_SYS_SDRAM_BASE;
	/* TODO get ram size from ddr controller */
	gd->ram_size = SZ_4G;
	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = min_t(phys_size_t, gd->ram_size, SZ_2G);

	if (gd->ram_size > SZ_2G && CONFIG_NR_DRAM_BANKS > 1) {
		gd->bd->bi_dram[1].start = 0x100000000;
		gd->bd->bi_dram[1].size = gd->ram_size - SZ_2G;
	}

	return 0;
}

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	if (gd->ram_size > SZ_2G)
		return SZ_2G;

	return gd->ram_size;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	u64 start[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		start[i] = gd->bd->bi_dram[i].start;
		size[i] = gd->bd->bi_dram[i].size;
	}

	return fdt_fixup_memory_banks(blob, start, size, CONFIG_NR_DRAM_BANKS);
}
