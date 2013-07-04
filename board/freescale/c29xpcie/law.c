/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <common.h>
#include <asm/fsl_law.h>
#include <asm/mmu.h>

struct law_entry law_table[] = {
	SET_LAW(CONFIG_SYS_FLASH_BASE_PHYS, LAW_SIZE_64M, LAW_TRGT_IF_IFC),
	SET_LAW(CONFIG_SYS_CPLD_BASE_PHYS, LAW_SIZE_4K, LAW_TRGT_IF_IFC),
	SET_LAW(CONFIG_SYS_NAND_BASE_PHYS, LAW_SIZE_16K, LAW_TRGT_IF_IFC),
	SET_LAW(CONFIG_SYS_PLATFORM_SRAM_BASE_PHYS, LAW_SIZE_512K,
					LAW_TRGT_IF_PLATFORM_SRAM),
};

int num_law_entries = ARRAY_SIZE(law_table);
