/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <miiphy.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
	return 0;
}

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

#if defined(CONFIG_SYS_CONSOLE_IS_IN_ENV) && \
defined(CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE)
int overwrite_console(void)
{
	return 0;
}
#endif

int misc_init_r(void)
{
	return 0;
}


/*
 * DesignWare Ethernet initialization
 */
int cpu_eth_init(bd_t *bis)
{
#if !defined(CONFIG_SOCFPGA_VIRTUAL_TARGET) && !defined(CONFIG_SPL_BUILD)
       /* initialize and register the emac */
	return designware_initialize(CONFIG_EMAC_BASE,
				     CONFIG_PHY_INTERFACE_MODE);
#else
	return 0;
#endif
}
