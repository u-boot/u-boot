/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/reset_manager.h>
#include <asm/io.h>

#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Print Board information
 */
int checkboard(void)
{
	puts("BOARD: Altera SoCFPGA Cyclone5 Board\n");
	return 0;
}

/*
 * Initialization function which happen at early stage of c code
 */
int board_early_init_f(void)
{
	return 0;
}

/*
 * Miscellaneous platform dependent initialisations
 */
int board_init(void)
{
	/* Address of boot parameters for ATAG (if ATAG is used) */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}
