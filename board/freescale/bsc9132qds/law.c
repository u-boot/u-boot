/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/fsl_law.h>
#include <asm/mmu.h>

struct law_entry law_table[] = {
	SET_LAW(CONFIG_SYS_FLASH_BASE_PHYS, LAW_SIZE_128M, LAW_TRGT_IF_IFC),
#ifdef CONFIG_SYS_NAND_BASE_PHYS
	SET_LAW(CONFIG_SYS_NAND_BASE_PHYS, LAW_SIZE_1M, LAW_TRGT_IF_IFC),
#endif
#ifdef CONFIG_SYS_FPGA_BASE_PHYS
	SET_LAW(CONFIG_SYS_FPGA_BASE_PHYS, LAW_SIZE_128K, LAW_TRGT_IF_IFC),
#endif
};

int num_law_entries = ARRAY_SIZE(law_table);
