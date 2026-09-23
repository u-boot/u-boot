// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#include <dm.h>
#include <asm/armv8/mmu.h>

static struct mm_region aspeed2700_mem_map[] = {
	{
		.virt =         0x0UL,
		.phys =         0x0UL,
		.size =  0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN,
	},
	{
		.virt =  0x40000000UL,
		.phys =  0x40000000UL,
		.size = 0x2C0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE,
	},
	{
		.virt = 0x400000000UL,
		.phys = 0x400000000UL,
		.size = 0x200000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE,
	},
	{
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = aspeed2700_mem_map;

u64 get_page_table_size(void)
{
	return 0x80000;
}
