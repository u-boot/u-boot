// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
 */

#include <asm/global_data.h>
#include <asm/io.h>
#include <config.h>
#include <bitfield.h>
#include <fdt_support.h>
#include <linux/sizes.h>

#define DDR_BASE 0xC0000000
DECLARE_GLOBAL_DATA_PTR;

static phys_size_t ddr_map_size(u32 val)
{
	u32 tmp;

	if (!(val & 0x1))
		return 0;

	tmp = bitfield_extract(val, 16, 5);
	switch (tmp) {
	case 0xd:
		return 512;
	case 0xe:
		return 1024;
	case 0xf:
		return 2048;
	case 0x10:
		return 4096;
	case 0x11:
		return 8192;
	default:
		pr_info("Invalid DRAM density %x\n", val);
		return 0;
	}
}

phys_size_t ddr_get_density(void)
{
	phys_size_t cs0_size = ddr_map_size(readl((void *)DDR_BASE + 0x200));
	phys_size_t cs1_size = ddr_map_size(readl((void *)DDR_BASE + 0x208));
	phys_size_t ddr_size = cs0_size + cs1_size;

	return ddr_size;
}

int dram_init(void)
{
	gd->ram_base = CFG_SYS_SDRAM_BASE;
	gd->ram_size = ddr_get_density() * SZ_1M;
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
