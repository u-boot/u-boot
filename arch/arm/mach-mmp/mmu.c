// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024
 * Duje Mihanović <duje.mihanovic@skole.hr>
 */
#include <asm/armv8/mmu.h>
#include <linux/sizes.h>

static struct mm_region pxa1908_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 2UL * SZ_1G,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	},
	{
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 2UL * SZ_1G,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_INNER_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
		0,
	}
};

struct mm_region *mem_map = pxa1908_mem_map;
