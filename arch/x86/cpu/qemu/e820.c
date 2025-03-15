// SPDX-License-Identifier: GPL-2.0+
/*
 * QEMU x86 specific E820 table generation
 *
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2019 Bin Meng <bmeng.cn@gmail.com>
 */

#include <env_internal.h>
#include <malloc.h>
#include <asm/e820.h>
#include <asm/arch/qemu.h>
#include <asm/global_data.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned int install_e820_map(unsigned int max_entries,
			      struct e820_entry *entries)
{
	u64 high_mem_size;
	struct e820_ctx ctx;

	e820_init(&ctx, entries, max_entries);

	e820_next(&ctx, E820_RAM, ISA_START_ADDRESS);
	e820_next(&ctx, E820_RESERVED, ISA_END_ADDRESS);

	/*
	 * since we use memalign(malloc) to allocate high memory for
	 * storing ACPI tables, we need to reserve them in e820 tables,
	 * otherwise kernel will reclaim them and data will be corrupted
	 */
	e820_to_addr(&ctx, E820_RAM, gd->relocaddr - TOTAL_MALLOC_LEN);
	e820_next(&ctx, E820_RESERVED, TOTAL_MALLOC_LEN);
	e820_to_addr(&ctx, E820_RAM, qemu_get_low_memory_size());
	e820_add(&ctx, E820_RESERVED, CONFIG_PCIE_ECAM_BASE,
		 CONFIG_PCIE_ECAM_SIZE);

	high_mem_size = qemu_get_high_memory_size();
	if (high_mem_size)
		e820_add(&ctx, E820_RAM, SZ_4G, high_mem_size);

	return e820_finish(&ctx);
}
