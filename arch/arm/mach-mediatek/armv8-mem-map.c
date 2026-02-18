// SPDX-License-Identifier: GPL-2.0-only

#include <asm/armv8/mmu.h>

static struct mm_region mediatek_mem_map[] = {
	{
		/* DDR */
		.virt = CONFIG_MTK_MEM_MAP_DDR_BASE_PHY,
		.phys = CONFIG_MTK_MEM_MAP_DDR_BASE_PHY,
		.size = CONFIG_MTK_MEM_MAP_DDR_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE,
	}, {
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = CONFIG_MTK_MEM_MAP_MMIO_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
	}
};

struct mm_region *mem_map = mediatek_mem_map;
