/*
 * Palm Tungsten|C Support
 *
 * Copyright (C) 2009-2010 Marek Vasut <marek.vasut@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <serial.h>
#include <asm/io.h>
#include <asm/arch/pxa.h>
#include <asm/arch/regs-mmc.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscelaneous platform dependent initialisations
 */

int board_init(void)
{
	/* We have RAM, disable cache */
	dcache_disable();
	icache_disable();

	/* Arch number of Palm Tungsten|C */
	gd->bd->bi_arch_number = MACH_TYPE_PALMTC;

	/* Adress of boot parameters */
	gd->bd->bi_boot_params = 0xa0000100;

	/* Set PWM for LCD */
	writel(0x5f, PWM_CTRL1);
	writel(0x3ff, PWM_PERVAL1);
	writel(892, PWM_PWDUTY1);

	return 0;
}

#ifdef CONFIG_CMD_MMC
int board_mmc_init(bd_t *bis)
{
	pxa_mmc_register(0);
	return 0;
}
#endif

int dram_init(void)
{
	pxa2xx_dram_init();
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
}
