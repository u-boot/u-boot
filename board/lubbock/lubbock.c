/*
 * (C) Copyright 2002
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <asm/arch/pxa.h>
#include <asm/arch/pxa-regs.h>
#include <asm/arch/regs-mmc.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscelaneous platform dependent initialisations
 */

int board_init (void)
{
	/* We have RAM, disable cache */
	dcache_disable();
	icache_disable();

	/* arch number of Lubbock-Board */
	gd->bd->bi_arch_number = MACH_TYPE_LUBBOCK;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0xa0000100;

	/* Configure GPIO6 and GPIO8 as OUT, AF1. */
	setbits_le32(GPDR0, (1 << 6) | (1 << 8));
	clrsetbits_le32(GAFR0_L, (3 << 12) | (3 << 16), (1 << 12) | (1 << 16));

	return 0;
}

#ifdef CONFIG_CMD_MMC
int board_mmc_init(bd_t *bis)
{
	pxa_mmc_register(0);
	return 0;
}
#endif

int board_late_init(void)
{
	setenv("stdout", "serial");
	setenv("stderr", "serial");
	return 0;
}

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

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_LAN91C96
	rc = lan91c96_initialize(0, CONFIG_LAN91C96_BASE);
#endif
	return rc;
}
#endif
