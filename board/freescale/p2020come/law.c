/*
 * Copyright 2009 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/fsl_law.h>
#include <asm/mmu.h>

/*
 * Create a dummy LAW entry for the DDR SDRAM which will be replaced when
 * the DDR SPD setup code runs.
 *
 * This table would be empty, except that it is used before the BSS section is
 * initialized, and therefore must have at least one entry to push it into
 * the DATA section.
 */
struct law_entry law_table[] = {
	SET_LAW(CONFIG_SYS_SDRAM_BASE, LAW_SIZE_4K, LAW_TRGT_IF_DDR),
};

int num_law_entries = ARRAY_SIZE(law_table);
