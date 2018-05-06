// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <asm/mach-types.h>
#include <common.h>
#if defined(CONFIG_FTMAC100) && !defined(CONFIG_DM_ETH)
#include <netdev.h>
#endif
#include <linux/io.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscellaneous platform dependent initializations
 */

int board_init(void)
{
	gd->bd->bi_arch_number = MACH_TYPE_AE250;
	gd->bd->bi_boot_params = PHYS_SDRAM_0 + 0x400;

	return 0;
}

int dram_init(void)
{
	unsigned long sdram_base = PHYS_SDRAM_0;
	unsigned long expected_size = PHYS_SDRAM_0_SIZE + PHYS_SDRAM_1_SIZE;
	unsigned long actual_size;

	actual_size = get_ram_size((void *)sdram_base, expected_size);
	gd->ram_size = actual_size;

	if (expected_size != actual_size) {
		printf("Warning: Only %lu of %lu MiB SDRAM is working\n",
			actual_size >> 20, expected_size >> 20);
	}

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_0;
	gd->bd->bi_dram[0].size =  PHYS_SDRAM_0_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[1].size =  PHYS_SDRAM_1_SIZE;

	return 0;
}

#if defined(CONFIG_FTMAC100) && !defined(CONFIG_DM_ETH)
int board_eth_init(bd_t *bd)
{
	return ftmac100_initialize(bd);
}
#endif

ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t *info)
{
	return 0;
}

void *board_fdt_blob_setup(void)
{
	void **ptr = (void *)CONFIG_SYS_SDRAM_BASE;
	if (fdt_magic(*ptr) == FDT_MAGIC)
			return (void *)*ptr;

	return (void *)CONFIG_SYS_FDT_BASE;
}
