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

#if defined(CONFIG_DISPLAY_CPUINFO)
/*
 * Print CPU information
 */
int print_cpuinfo(void)
{
	puts("CPU   : Altera SOCFPGA Platform\n");
	return 0;
}
#endif

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

int misc_init_r(void)
{
	return 0;
}

#if defined(CONFIG_SYS_CONSOLE_IS_IN_ENV) && defined(CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE)
int overwrite_console(void)
{
	return 0;
}
#endif

/*
 * DesignWare Ethernet initialization
 */
/* We know all the init functions have been run now */
int board_eth_init(bd_t *bis)
{
	return 0;
}
