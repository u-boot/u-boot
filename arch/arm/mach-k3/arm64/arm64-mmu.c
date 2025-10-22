// SPDX-License-Identifier:     GPL-2.0+
/*
 * K3: ARM64 MMU setup
 *
 * Copyright (C) 2018-2020 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 *	Suman Anna <s-anna@ti.com>
 * (This file is derived from arch/arm/mach-zynqmp/cpu.c)
 *
 */

#include <asm/system.h>
#include <asm/armv8/mmu.h>
#include <linux/sizes.h>
#include <mach/k3-ddr.h>

struct mm_region k3_mem_map[K3_MEM_MAP_LEN] = {
	{ /* SoC Peripherals */
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, { /* Flash Peripherals */
		.virt = 0x500000000UL,
		.phys = 0x500000000UL,
		.size = 0x380000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, [K3_MEM_MAP_FIRST_BANK_IDX] = { /* First DRAM Bank of size 2G */
		.virt = CFG_SYS_SDRAM_BASE,
		.phys = CFG_SYS_SDRAM_BASE,
		.size = SZ_2G,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, { /* List terminator */
		0,
	}
};

struct mm_region *mem_map = k3_mem_map;

u64 get_page_table_size(void)
{
	return SZ_128K;
}
