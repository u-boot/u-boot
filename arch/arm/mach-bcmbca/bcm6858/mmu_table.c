// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright 2022 Broadcom Ltd.
 */
#include <common.h>
#include <asm/armv8/mmu.h>
#include <linux/sizes.h>

static struct mm_region bcm96858_mem_map[] = {
		{
				.virt = 0x00000000UL,
				.phys = 0x00000000UL,
				.size = 1UL * SZ_1G,
				.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
						PTE_BLOCK_INNER_SHARE
		},
		{
				/* SoC peripheral */
				.virt = 0xff800000UL,
				.phys = 0xff800000UL,
				.size = 0x100000,
				.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
						PTE_BLOCK_NON_SHARE |
						PTE_BLOCK_PXN | PTE_BLOCK_UXN
		},
		{
				/* List terminator */
				0,
		}
};

struct mm_region *mem_map = bcm96858_mem_map;
