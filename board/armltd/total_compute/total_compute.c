// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020 Arm Limited
 * Usama Arif <usama.arif@arm.com>
 */

#include <config.h>
#include <dm.h>
#include <dm/platform_data/serial_pl01x.h>
#include <env.h>
#include <asm/armv8/mmu.h>
#include <asm/global_data.h>
#include <asm/system.h>

static struct mm_region total_compute_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0xff80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = total_compute_mem_map;

/*
 * Push the variable into the .data section so that it
 * does not get cleared later.
 */
unsigned long __section(".data") fw_dtb_pointer;

void *board_fdt_blob_setup(int *err)
{
	*err = 0;
	if (fdt_magic(fw_dtb_pointer) != FDT_MAGIC) {
		*err = -ENXIO;
		return NULL;
	}

	return (void *)fw_dtb_pointer;
}

int board_init(void)
{
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

/* Nothing to be done here as handled by PSCI interface */
void reset_cpu(void)
{
}
