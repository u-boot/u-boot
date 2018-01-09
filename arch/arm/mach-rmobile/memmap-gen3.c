/*
 * Renesas RCar Gen3 memory map tables
 *
 * Copyright (C) 2017 Marek Vasut <marek.vasut@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <asm/armv8/mmu.h>

static struct mm_region r8a7795_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

static struct mm_region r8a7796_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0xe0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xe0000000UL,
		.phys = 0xe0000000UL,
		.size = 0xe0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

static struct mm_region r8a77970_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0xe0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xe0000000UL,
		.phys = 0xe0000000UL,
		.size = 0xe0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

static struct mm_region r8a77995_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0xe0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xe0000000UL,
		.phys = 0xe0000000UL,
		.size = 0xe0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = r8a7795_mem_map;

void rcar_gen3_memmap_fixup(void)
{
	u32 cpu_type = rmobile_get_cpu_type();

	switch (cpu_type) {
	case RMOBILE_CPU_TYPE_R8A7795:
		mem_map = r8a7795_mem_map;
		break;
	case RMOBILE_CPU_TYPE_R8A7796:
		mem_map = r8a7796_mem_map;
		break;
	case RMOBILE_CPU_TYPE_R8A77970:
		mem_map = r8a77970_mem_map;
		break;
	case RMOBILE_CPU_TYPE_R8A77995:
		mem_map = r8a77995_mem_map;
		break;
	}
}
