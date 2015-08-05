/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <debug_uart.h>
#include <efi.h>
#include <errno.h>
#include <linux/err.h>
#include <linux/types.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * This function looks for the highest region of memory lower than 4GB which
 * has enough space for U-Boot where U-Boot is aligned on a page boundary.
 * It overrides the default implementation found elsewhere which simply
 * picks the end of ram, wherever that may be. The location of the stack,
 * the relocation address, and how far U-Boot is moved by relocation are
 * set in the global data structure.
 */
ulong board_get_usable_ram_top(ulong total_size)
{
	struct efi_mem_desc *desc, *end;
	struct efi_entry_memmap *map;
	int ret, size;
	uintptr_t dest_addr = 0;
	struct efi_mem_desc *largest = NULL;

	/*
	 * Find largest area of memory below 4GB. We could
	 * call efi_build_mem_table() for a more accurate picture since it
	 * merges areas together where possible. But that function uses more
	 * pre-relocation memory, and it's not critical that we find the
	 * absolute largest region.
	 */
	ret = efi_info_get(EFIET_MEMORY_MAP, (void **)&map, &size);
	if (ret) {
		/* We should have stopped in dram_init(), something is wrong */
		debug("%s: Missing memory map\n", __func__);
		goto err;
	}

	end = (struct efi_mem_desc *)((ulong)map + size);
	desc = map->desc;
	for (; desc < end; desc = efi_get_next_mem_desc(map, desc)) {
		if (desc->type != EFI_CONVENTIONAL_MEMORY ||
		    desc->physical_start >= 1ULL << 32)
			continue;
		if (!largest || desc->num_pages > largest->num_pages)
			largest = desc;
	}

	/* If no suitable area was found, return an error. */
	assert(largest);
	if (!largest || (largest->num_pages << EFI_PAGE_SHIFT) < (2 << 20))
		goto err;

	dest_addr = largest->physical_start + (largest->num_pages <<
			EFI_PAGE_SHIFT);

	return (ulong)dest_addr;
err:
	panic("No available memory found for relocation");
	return 0;
}

int dram_init(void)
{
	struct efi_mem_desc *desc, *end;
	struct efi_entry_memmap *map;
	int size, ret;

	ret = efi_info_get(EFIET_MEMORY_MAP, (void **)&map, &size);
	if (ret) {
		printf("Cannot find EFI memory map tables, ret=%d\n", ret);

		return -ENODEV;
	}

	end = (struct efi_mem_desc *)((ulong)map + size);
	gd->ram_size = 0;
	desc = map->desc;
	for (; desc < end; desc = efi_get_next_mem_desc(map, desc)) {
		if (desc->type < EFI_MMAP_IO)
			gd->ram_size += desc->num_pages << EFI_PAGE_SHIFT;
	}

	return 0;
}

void dram_init_banksize(void)
{
	struct efi_mem_desc *desc, *end;
	struct efi_entry_memmap *map;
	int ret, size;
	int num_banks;

	ret = efi_info_get(EFIET_MEMORY_MAP, (void **)&map, &size);
	if (ret) {
		/* We should have stopped in dram_init(), something is wrong */
		debug("%s: Missing memory map\n", __func__);
		return;
	}
	end = (struct efi_mem_desc *)((ulong)map + size);
	desc = map->desc;
	for (num_banks = 0;
	     desc < end && num_banks < CONFIG_NR_DRAM_BANKS;
	     desc = efi_get_next_mem_desc(map, desc)) {
		/*
		 * We only use conventional memory below 4GB, and ignore
		 * anything less than 1MB.
		 */
		if (desc->type != EFI_CONVENTIONAL_MEMORY ||
		    desc->physical_start >= 1ULL << 32 ||
		    (desc->num_pages << EFI_PAGE_SHIFT) < 1 << 20)
			continue;
		gd->bd->bi_dram[num_banks].start = desc->physical_start;
		gd->bd->bi_dram[num_banks].size = desc->num_pages <<
			EFI_PAGE_SHIFT;
		num_banks++;
	}
}

int print_cpuinfo(void)
{
	return default_print_cpuinfo();
}

/* Find any available tables and copy them to a safe place */
int reserve_arch(void)
{
	struct efi_info_hdr *hdr;

	debug("table=%lx\n", gd->arch.table);
	if (!gd->arch.table)
		return 0;

	hdr = (struct efi_info_hdr *)gd->arch.table;

	gd->start_addr_sp -= hdr->total_size;
	memcpy((void *)gd->start_addr_sp, hdr, hdr->total_size);
	debug("Stashing EFI table at %lx to %lx, size %x\n",
	      gd->arch.table, gd->start_addr_sp, hdr->total_size);
	gd->arch.table = gd->start_addr_sp;

	return 0;
}
