/*
 * Copyright 2014 Broadcom Corporation.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <config.h>
#include <asm/system.h>
#include <asm/iproc-common/armpll.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * board_init - early hardware init
 */
int board_init(void)
{
	/*
	 * Address of boot parameters passed to kernel
	 * Use default offset 0x100
	 */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

/*
 * dram_init - sets u-boot's idea of sdram size
 */
int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = gd->ram_size;
}

int board_early_init_f(void)
{
	uint32_t status = 0;

	/* Setup PLL if required */
#if defined(CONFIG_ARMCLK)
	armpll_config(CONFIG_ARMCLK);
#endif

	return status;
}
