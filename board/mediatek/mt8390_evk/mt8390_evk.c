// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2026 BayLibre SAS
 * Author: Julien Masson <jmasson@baylibre.com>
 */

#include <linux/types.h>
#include <asm/armv8/mmu.h>

int board_init(void)
{
	return 0;
}

static struct mm_region mt8390_evk_mem_map[] = {
	{
		/* DDR */
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 0x200000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE,
	}, {
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 0x20000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		0,
	}
};

struct mm_region *mem_map = mt8390_evk_mem_map;
