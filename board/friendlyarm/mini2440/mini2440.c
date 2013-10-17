/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2009
 * Michel Pollet <buserror@gmail.com>
 *
 * (C) Copyright 2012
 * Gabriel Huau <contact@huau-gabriel.fr>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/s3c2440.h>
#include <asm/arch/iomux.h>
#include <asm/arch/gpio.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <netdev.h>
#include "mini2440.h"

DECLARE_GLOBAL_DATA_PTR;

static inline void pll_delay(unsigned long loops)
{
	__asm__ volatile ("1:\n"
	  "subs %0, %1, #1\n"
	  "bne 1b" : "=r" (loops) : "0" (loops));
}

int board_early_init_f(void)
{
	struct s3c24x0_clock_power * const clk_power =
					s3c24x0_get_base_clock_power();

	/* to reduce PLL lock time, adjust the LOCKTIME register */
	clk_power->locktime = 0xFFFFFF; /* Max PLL Lock time count */
	clk_power->clkdivn = CLKDIVN_VAL;

	/* configure UPLL */
	clk_power->upllcon = ((U_M_MDIV << 12) + (U_M_PDIV << 4) + U_M_SDIV);
	/* some delay between MPLL and UPLL */
	pll_delay(100);

	/* configure MPLL */
	clk_power->mpllcon = ((M_MDIV << 12) + (M_PDIV << 4) + M_SDIV);

	/* some delay between MPLL and UPLL */
	pll_delay(10000);

	return 0;
}

/*
 * Miscellaneous platform dependent initialisations
 */
int board_init(void)
{
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();

	/* IOMUX Port H : UART Configuration */
	gpio->gphcon = IOMUXH_nCTS0 | IOMUXH_nRTS0 | IOMUXH_TXD0 | IOMUXH_RXD0 |
		IOMUXH_TXD1 | IOMUXH_RXD1 | IOMUXH_TXD2 | IOMUXH_RXD2;

	gpio_direction_output(GPH8, 0);
	gpio_direction_output(GPH9, 0);
	gpio_direction_output(GPH10, 0);

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_BOOT_PARAM_ADDR;

	return 0;
}

int dram_init(void)
{
	struct s3c24x0_memctl *memctl = s3c24x0_get_base_memctl();

	/*
	 * Configuring bus width and timing
	 * Initialize clocks for each bank 0..5
	 * Bank 3 and 4 are used for DM9000
	 */
	writel(BANK_CONF, &memctl->bwscon);
	writel(B0_CONF, &memctl->bankcon[0]);
	writel(B1_CONF, &memctl->bankcon[1]);
	writel(B2_CONF, &memctl->bankcon[2]);
	writel(B3_CONF, &memctl->bankcon[3]);
	writel(B4_CONF, &memctl->bankcon[4]);
	writel(B5_CONF, &memctl->bankcon[5]);

	/* Bank 6 and 7 are used for DRAM */
	writel(SDRAM_64MB, &memctl->bankcon[6]);
	writel(SDRAM_64MB, &memctl->bankcon[7]);

	writel(MEM_TIMING, &memctl->refresh);
	writel(BANKSIZE_CONF, &memctl->banksize);
	writel(B6_MRSR, &memctl->mrsrb6);
	writel(B7_MRSR, &memctl->mrsrb7);

	gd->ram_size = get_ram_size((void *) CONFIG_SYS_SDRAM_BASE,
			PHYS_SDRAM_SIZE);
	return 0;
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_DRIVER_DM9000
	return dm9000_initialize(bis);
#else
	return 0;
#endif
}
