// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#include <handoff.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

void handoff_save_dram(struct spl_handoff *ho)
{
	int i;

	ho->ram_size = gd->ram_size;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ho->ram_bank[i].start = gd->dram[i].start;
		ho->ram_bank[i].size = gd->dram[i].size;
	}
}

void handoff_load_dram_size(struct spl_handoff *ho)
{
	gd->ram_size = ho->ram_size;
}

void handoff_load_dram_banks(struct spl_handoff *ho)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->dram[i].start = ho->ram_bank[i].start;
		gd->dram[i].size = ho->ram_bank[i].size;
	}
}
