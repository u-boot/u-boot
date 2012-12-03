/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2010,2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <malloc.h>
#include <asm/e820.h>
#include <asm/u-boot-x86.h>
#include <asm/global_data.h>
#include <asm/processor.h>
#include <asm/arch/sysinfo.h>
#include <asm/arch/tables.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned install_e820_map(unsigned max_entries, struct e820entry *entries)
{
	int i;

	unsigned num_entries = min(lib_sysinfo.n_memranges, max_entries);
	if (num_entries < lib_sysinfo.n_memranges) {
		printf("Warning: Limiting e820 map to %d entries.\n",
			num_entries);
	}
	for (i = 0; i < num_entries; i++) {
		struct memrange *memrange = &lib_sysinfo.memrange[i];

		entries[i].addr = memrange->base;
		entries[i].size = memrange->size;
		entries[i].type = memrange->type;
	}
	return num_entries;
}

/*
 * This function looks for the highest region of memory lower than 4GB which
 * has enough space for U-Boot where U-Boot is aligned on a page boundary. It
 * overrides the default implementation found elsewhere which simply picks the
 * end of ram, wherever that may be. The location of the stack, the relocation
 * address, and how far U-Boot is moved by relocation are set in the global
 * data structure.
 */
int calculate_relocation_address(void)
{
	const uint64_t uboot_size = (uintptr_t)&__bss_end -
			(uintptr_t)&__text_start;
	const uint64_t total_size = uboot_size + CONFIG_SYS_MALLOC_LEN +
		CONFIG_SYS_STACK_SIZE;
	uintptr_t dest_addr = 0;
	int i;

	for (i = 0; i < lib_sysinfo.n_memranges; i++) {
		struct memrange *memrange = &lib_sysinfo.memrange[i];
		/* Force U-Boot to relocate to a page aligned address. */
		uint64_t start = roundup(memrange->base, 1 << 12);
		uint64_t end = memrange->base + memrange->size;

		/* Ignore non-memory regions. */
		if (memrange->type != CB_MEM_RAM)
			continue;

		/* Filter memory over 4GB. */
		if (end > 0xffffffffULL)
			end = 0x100000000ULL;
		/* Skip this region if it's too small. */
		if (end - start < total_size)
			continue;

		/* Use this address if it's the largest so far. */
		if (end - uboot_size > dest_addr)
			dest_addr = end;
	}

	/* If no suitable area was found, return an error. */
	if (!dest_addr)
		return 1;

	dest_addr -= uboot_size;
	dest_addr &= ~((1 << 12) - 1);
	gd->relocaddr = dest_addr;
	gd->reloc_off = dest_addr - (uintptr_t)&__text_start;
	gd->start_addr_sp = dest_addr - CONFIG_SYS_MALLOC_LEN;

	return 0;
}

int dram_init_f(void)
{
	int i;
	phys_size_t ram_size = 0;

	for (i = 0; i < lib_sysinfo.n_memranges; i++) {
		struct memrange *memrange = &lib_sysinfo.memrange[i];
		unsigned long long end = memrange->base + memrange->size;

		if (memrange->type == CB_MEM_RAM && end > ram_size)
			ram_size = end;
	}
	gd->ram_size = ram_size;
	if (ram_size == 0)
		return -1;
	return 0;
}

int dram_init(void)
{
	int i, j;

	if (CONFIG_NR_DRAM_BANKS) {
		for (i = 0, j = 0; i < lib_sysinfo.n_memranges; i++) {
			struct memrange *memrange = &lib_sysinfo.memrange[i];

			if (memrange->type == CB_MEM_RAM) {
				gd->bd->bi_dram[j].start = memrange->base;
				gd->bd->bi_dram[j].size = memrange->size;
				j++;
				if (j >= CONFIG_NR_DRAM_BANKS)
					break;
			}
		}
	}
	return 0;
}
