// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Nuvoton Technology Corp.
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/gcr.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	struct npcm_gcr *gcr = (struct npcm_gcr *)NPCM_GCR_BA;

	/*
	 * Get dram size from bootblock.
	 * The value is stored in scrpad_02 register.
	 */
	gd->ram_size = readl(&gcr->scrpad_b);

	return 0;
}
