// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Stephan Gerhold <stephan@gerhold.net>
 */
#include <common.h>
#include <init.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/setup.h>
#include <asm/system.h>

DECLARE_GLOBAL_DATA_PTR;

/* Parse atags provided by Samsung bootloader to get available memory */
static ulong fw_mach __section(".data");
static ulong fw_atags __section(".data");

void save_boot_params(ulong r0, ulong r1, ulong r2, ulong r3)
{
	fw_mach = r1;
	fw_atags = r2;
	save_boot_params_ret();
}

static const struct tag *fw_atags_get(void)
{
	const struct tag *tags = (const struct tag *)fw_atags;

	if (tags->hdr.tag != ATAG_CORE) {
		log_err("Invalid atags: tag 0x%x at %p\n", tags->hdr.tag, tags);
		return NULL;
	}

	return tags;
}

int dram_init(void)
{
	const struct tag *t, *tags = fw_atags_get();

	if (!tags)
		return -EINVAL;

	for_each_tag(t, tags) {
		if (t->hdr.tag != ATAG_MEM)
			continue;

		debug("Memory: %#x-%#x (size %#x)\n", t->u.mem.start,
		      t->u.mem.start + t->u.mem.size, t->u.mem.size);
		gd->ram_size += t->u.mem.size;
	}
	return 0;
}

int dram_init_banksize(void)
{
	const struct tag *t, *tags = fw_atags_get();
	unsigned int bank = 0;

	if (!tags)
		return -EINVAL;

	for_each_tag(t, tags) {
		if (t->hdr.tag != ATAG_MEM)
			continue;

		gd->bd->bi_dram[bank].start = t->u.mem.start;
		gd->bd->bi_dram[bank].size = t->u.mem.size;
		if (++bank == CONFIG_NR_DRAM_BANKS)
			break;
	}
	return 0;
}

int board_init(void)
{
	gd->bd->bi_arch_number = fw_mach;
	gd->bd->bi_boot_params = fw_atags;
	return 0;
}
