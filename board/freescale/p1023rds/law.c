/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/fsl_law.h>
#include <asm/mmu.h>

struct law_entry law_table[] = {
	SET_LAW(CONFIG_SYS_NAND_BASE_PHYS, LAW_SIZE_1M, LAW_TRGT_IF_LBC),
	SET_LAW(CONFIG_SYS_QMAN_MEM_PHYS, LAW_SIZE_4M,
					LAW_TRGT_IF_DPAA_SWP_SRAM),
	/* The LAW 0xe0000000 ~ 0xefffffff for BCSR and NOR flash */
	SET_LAW(CONFIG_SYS_BCSR_BASE_PHYS, LAW_SIZE_256M, LAW_TRGT_IF_LBC),
};

int num_law_entries = ARRAY_SIZE(law_table);
