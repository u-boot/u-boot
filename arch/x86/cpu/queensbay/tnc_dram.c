/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/fsp/fsp_support.h>
#include <asm/e820.h>
#include <asm/post.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	phys_size_t ram_size = 0;
	union hob_pointers hob;

	hob.raw = gd->arch.hob_list;
	while (!end_of_hob(hob)) {
		if (get_hob_type(hob) == HOB_TYPE_RES_DESC) {
			if (hob.res_desc->type == RES_SYS_MEM ||
			    hob.res_desc->type == RES_MEM_RESERVED) {
				ram_size += hob.res_desc->len;
			}
		}
		hob.raw = get_next_hob(hob);
	}

	gd->ram_size = ram_size;
	post_code(POST_DRAM);

	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = 0;
	gd->bd->bi_dram[0].size = gd->ram_size;
}

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
	return fsp_get_usable_lowmem_top(gd->arch.hob_list);
}

unsigned install_e820_map(unsigned max_entries, struct e820entry *entries)
{
	unsigned num_entries = 0;

	union hob_pointers hob;

	hob.raw = gd->arch.hob_list;

	while (!end_of_hob(hob)) {
		if (get_hob_type(hob) == HOB_TYPE_RES_DESC) {
			entries[num_entries].addr = hob.res_desc->phys_start;
			entries[num_entries].size = hob.res_desc->len;

			if (hob.res_desc->type == RES_SYS_MEM)
				entries[num_entries].type = E820_RAM;
			else if (hob.res_desc->type == RES_MEM_RESERVED)
				entries[num_entries].type = E820_RESERVED;
		}
		hob.raw = get_next_hob(hob);
		num_entries++;
	}

	return num_entries;
}
