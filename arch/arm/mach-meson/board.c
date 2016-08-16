/*
 * (C) Copyright 2016 Beniamino Galvani <b.galvani@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <libfdt.h>
#include <linux/err.h>
#include <asm/arch/gxbb.h>
#include <asm/arch/sm.h>
#include <asm/armv8/mmu.h>
#include <asm/unaligned.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	const fdt64_t *val;
	int offset;
	int len;

	offset = fdt_path_offset(gd->fdt_blob, "/memory");
	if (offset < 0)
		return -EINVAL;

	val = fdt_getprop(gd->fdt_blob, offset, "reg", &len);
	if (len < sizeof(*val) * 2)
		return -EINVAL;

	/* Use unaligned access since cache is still disabled */
	gd->ram_size = get_unaligned_be64(&val[1]);

	return 0;
}

void dram_init_banksize(void)
{
	/* Reserve first 16 MiB of RAM for firmware */
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE + (16 * 1024 * 1024);
	gd->bd->bi_dram[0].size = gd->ram_size - (16 * 1024 * 1024);
}

void reset_cpu(ulong addr)
{
	psci_system_reset();
}

static struct mm_region gxbb_mem_map[] = {
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

struct mm_region *mem_map = gxbb_mem_map;
