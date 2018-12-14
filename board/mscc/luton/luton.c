// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>
#include <asm/io.h>

#define MSCC_GPIO_ALT0		0x88
#define MSCC_GPIO_ALT1		0x8C

DECLARE_GLOBAL_DATA_PTR;

void board_debug_uart_init(void)
{
	/* too early for the pinctrl driver, so configure the UART pins here */
	setbits_le32(BASE_DEVCPU_GCB + MSCC_GPIO_ALT0, BIT(30) | BIT(31));
}

int board_early_init_r(void)
{
	/* Prepare SPI controller to be used in master mode */
	writel(0, BASE_CFG + ICPU_SW_MODE);

	/* Address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE;
	return 0;
}
