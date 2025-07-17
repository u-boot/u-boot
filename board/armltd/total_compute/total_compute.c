// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020 Arm Limited
 * Usama Arif <usama.arif@arm.com>
 */

#include <config.h>
#include <dm.h>
#include <dm/platform_data/serial_pl01x.h>
#include <cpu_func.h>
#include <env.h>
#include <linux/sizes.h>

#include <asm/armv8/mmu.h>
#include <asm/global_data.h>
#include <asm/system.h>

/* +1 is end of list which needs to be empty */
#define TC_MEM_MAP_MAX		(1 + CONFIG_NR_DRAM_BANKS + 1)

static struct mm_region total_compute_mem_map[TC_MEM_MAP_MAX] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}
};

struct mm_region *mem_map = total_compute_mem_map;

#ifdef CONFIG_OF_HAS_PRIOR_STAGE
/*
 * Push the variable into the .data section so that it
 * does not get cleared later.
 */
unsigned long __section(".data") fw_dtb_pointer;

int board_fdt_blob_setup(void **fdtp)
{
	if (fdt_magic(fw_dtb_pointer) != FDT_MAGIC)
		return -ENXIO;

	*fdtp = (void *)fw_dtb_pointer;
	return 0;
}
#endif

int misc_init_r(void)
{
	size_t base;

#ifdef CONFIG_OF_HAS_PRIOR_STAGE
	if (!env_get("fdt_addr_r"))
		env_set_hex("fdt_addr_r", fw_dtb_pointer);
#endif
	if (!env_get("kernel_addr_r")) {
		/*
		 * The kernel has to be 2M aligned and the first 64K at the
		 * start of SDRAM is reserved for DTB.
		 */
		base = gd->ram_base + SZ_2M;
		assert(IS_ALIGNED(base, SZ_2M));

		env_set_hex("kernel_addr_r", base);
	}

	return 0;
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

void build_mem_map(void)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		/*
		 * The first node is for I/O device, start from node 1 for
		 * updating DRAM info.
		 */
		mem_map[i + 1].virt = gd->bd->bi_dram[i].start;
		mem_map[i + 1].phys = gd->bd->bi_dram[i].start;
		mem_map[i + 1].size = gd->bd->bi_dram[i].size;
		mem_map[i + 1].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				       PTE_BLOCK_INNER_SHARE;
	}
}

void enable_caches(void)
{
	build_mem_map();

	icache_enable();
	dcache_enable();
}

u64 get_page_table_size(void)
{
	return SZ_256K;
}

/* Nothing to be done here as handled by PSCI interface */
void reset_cpu(void)
{
}
