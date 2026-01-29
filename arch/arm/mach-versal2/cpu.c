// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 - 2022, Xilinx, Inc.
 * Copyright (C) 2022 - 2024, Advanced Micro Devices, Inc.
 *
 * Michal Simek <michal.simek@amd.com>
 */

#include <init.h>
#include <asm/armv8/mmu.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/cache.h>
#include <dm/platdata.h>

DECLARE_GLOBAL_DATA_PTR;

#define VERSAL2_MEM_MAP_USED	5

#define DRAM_BANKS CONFIG_NR_DRAM_BANKS

/* +1 is end of list which needs to be empty */
#define VERSAL2_MEM_MAP_MAX (VERSAL2_MEM_MAP_USED + DRAM_BANKS + 1)

static struct mm_region versal2_mem_map[VERSAL2_MEM_MAP_MAX] = {
	{
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0x70000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0xf0000000UL,
		.phys = 0xf0000000UL,
		.size = 0x0fe00000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x400000000UL,
		.phys = 0x400000000UL,
		.size = 0x200000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x600000000UL,
		.phys = 0x600000000UL,
		.size = 0x800000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xe00000000UL,
		.phys = 0xe00000000UL,
		.size = 0xf200000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}
};

/**
 * mem_map_fill() - Populate global memory map with DRAM banks
 * @bank_info: Array of memory regions parsed from device tree
 * @num_banks: Number of valid DRAM banks in bank_info array
 *
 * Copies DRAM bank information into the global versal2_mem_map[] array
 * starting at index VERSAL2_MEM_MAP_USED (5), which is after the fixed
 * device mappings. This must be called early in boot before MMU
 * initialization so that get_page_table_size() can calculate the
 * required page table size based on actual memory configuration.
 */
void mem_map_fill(struct mm_region *bank_info, u32 num_banks)
{
	int banks = VERSAL2_MEM_MAP_USED;

	for (int i = 0; i < num_banks; i++) {
		if (banks > VERSAL2_MEM_MAP_MAX)
			return;

		versal2_mem_map[banks].virt = bank_info[i].phys;
		versal2_mem_map[banks].phys = bank_info[i].phys;
		versal2_mem_map[banks].size = bank_info[i].size;
		versal2_mem_map[banks].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
					      PTE_BLOCK_INNER_SHARE;
		banks = banks + 1;
	}
}

/**
 * fill_bd_mem_info() - Copy DRAM banks from mem_map to bd_info
 *
 * Transfers DRAM bank information from the global versal2_mem_map[]
 * array to bd->bi_dram[] for passing memory configuration to the
 * Linux kernel via boot parameters (ATAGS/FDT). Each bank's physical
 * address and size are copied.
 *
 * This is called during dram_init_banksize() after the memory map
 * has been populated by mem_map_fill() in dram_init(). Called after
 * dram_init() but before kernel handoff.
 */
void fill_bd_mem_info(void)
{
	struct bd_info *bd = gd->bd;
	int banks = VERSAL2_MEM_MAP_USED;

	for (int i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		if (!versal2_mem_map[banks].size)
			break;

		bd->bi_dram[i].start = versal2_mem_map[banks].phys;
		bd->bi_dram[i].size = versal2_mem_map[banks].size;
		banks++;
	}
}

struct mm_region *mem_map = versal2_mem_map;

#if CONFIG_IS_ENABLED(SYS_MEM_RSVD_FOR_MMU)
u64 get_page_table_size(void)
{
	return 0x14000;
}
#endif

U_BOOT_DRVINFO(soc_amd_versal2) = {
	.name = "soc_amd_versal2",
};
