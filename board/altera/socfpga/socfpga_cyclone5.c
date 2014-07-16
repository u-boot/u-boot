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
	puts("BOARD : Altera SOCFPGA Cyclone5 Board\n");
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
	icache_enable();
	return 0;
}

/*
 * DesignWare Ethernet initialization
 */
/* We know all the init functions have been run now */
int board_eth_init(bd_t *bis)
{
	return 0;
}
