/*
 * Embest/Timll DevKit3250 board support
 *
 * Copyright (C) 2011 Vladimir Zapolskiy <vz@mleia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/cpu.h>
#include <asm/arch/emc.h>

DECLARE_GLOBAL_DATA_PTR;

static struct emc_regs *emc = (struct emc_regs *)EMC_BASE;

int board_early_init_f(void)
{
	lpc32xx_uart_init(CONFIG_SYS_LPC32XX_UART);

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params  = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_SYS_FLASH_CFI
	/* Use 16-bit memory interface for NOR Flash */
	emc->stat[0].config	= EMC_STAT_CONFIG_PB | EMC_STAT_CONFIG_16BIT;

	/* Change the NOR timings to optimum value to get maximum bandwidth */
	emc->stat[0].waitwen	= EMC_STAT_WAITWEN(1);
	emc->stat[0].waitoen	= EMC_STAT_WAITOEN(1);
	emc->stat[0].waitrd	= EMC_STAT_WAITRD(12);
	emc->stat[0].waitpage	= EMC_STAT_WAITPAGE(12);
	emc->stat[0].waitwr	= EMC_STAT_WAITWR(5);
	emc->stat[0].waitturn	= EMC_STAT_WAITTURN(2);
#endif

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);

	return 0;
}
