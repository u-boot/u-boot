/*
 * (C) Copyright 2006-2010 Eukrea Electromatique <www.eukrea.com>
 * Eric Benard <eric@eukrea.com>
 * based on at91rm9200dk.c which is :
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>

#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_pio.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_common.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

int board_init(void)
{
	/* arch number of CPUAT91-Board */
	gd->bd->bi_arch_number = MACH_TYPE_CPUAT91;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

int board_early_init_f(void)
{
	at91_seriald_hw_init();
	return 0;
}


int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
			CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

#ifdef CONFIG_DRIVER_AT91EMAC
int board_eth_init(bd_t *bis)
{
	return at91emac_register(bis, (u32) ATMEL_BASE_EMAC);
}
#endif

#ifdef CONFIG_SYS_I2C_SOFT
void i2c_init_board(void)
{
	u32 pin;
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;
	at91_pio_t *pio = (at91_pio_t *) ATMEL_BASE_PIO;

	writel(1 << AT91_ID_PIOA, &pmc->pcer);
	pin = AT91_PMX_AA_TWD | AT91_PMX_AA_TWCK;
	writel(pin, &pio->pioa.idr);
	writel(pin, &pio->pioa.pudr);
	writel(pin, &pio->pioa.per);
	writel(pin, &pio->pioa.oer);
	writel(pin, &pio->pioa.sodr);
}
#endif
