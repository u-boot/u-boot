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
#include <asm/arch-coreboot/sysinfo.h>
#include <asm/arch-coreboot/tables.h>

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
	return 0;
}
