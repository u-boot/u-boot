/*
 * Copyright (C) 2011 Simon Guinot <sguinot@lacie.com>
 *
 * Based on Kirkwood support:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <command.h>
#include <asm/arch/cpu.h>
#include <asm/arch/kirkwood.h>
#include <asm/arch/mpp.h>
#include <asm/arch/gpio.h>

#include "net2big_v2.h"
#include "../common/common.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/* GPIO configuration */
	kw_config_gpio(NET2BIG_V2_OE_VAL_LOW, NET2BIG_V2_OE_VAL_HIGH,
			NET2BIG_V2_OE_LOW, NET2BIG_V2_OE_HIGH);

	/* Multi-Purpose Pins Functionality configuration */
	u32 kwmpp_config[] = {
		MPP0_SPI_SCn,
		MPP1_SPI_MOSI,
		MPP2_SPI_SCK,
		MPP3_SPI_MISO,
		MPP6_SYSRST_OUTn,
		MPP7_GPO,		/* Request power-off */
		MPP8_TW_SDA,
		MPP9_TW_SCK,
		MPP10_UART0_TXD,
		MPP11_UART0_RXD,
		MPP13_GPIO,		/* Rear power switch (on|auto) */
		MPP14_GPIO,		/* USB fuse alarm */
		MPP15_GPIO,		/* Rear power switch (auto|off) */
		MPP16_GPIO,		/* SATA HDD1 power */
		MPP17_GPIO,		/* SATA HDD2 power */
		MPP20_SATA1_ACTn,
		MPP21_SATA0_ACTn,
		MPP24_GPIO,		/* USB mode select */
		MPP26_GPIO,		/* USB device vbus */
		MPP28_GPIO,		/* USB enable host vbus */
		MPP29_GPIO,		/* GPIO extension ALE */
		MPP34_GPIO,		/* Rear Push button 0=on 1=off */
		MPP35_GPIO,		/* Inhibit switch power-off */
		MPP36_GPIO,		/* SATA HDD1 presence */
		MPP37_GPIO,		/* SATA HDD2 presence */
		MPP40_GPIO,		/* eSATA presence */
		MPP44_GPIO,		/* GPIO extension (data 0) */
		MPP45_GPIO,		/* GPIO extension (data 1) */
		MPP46_GPIO,		/* GPIO extension (data 2) */
		MPP47_GPIO,		/* GPIO extension (addr 0) */
		MPP48_GPIO,		/* GPIO extension (addr 1) */
		MPP49_GPIO,		/* GPIO extension (addr 2) */
		0
	};

	kirkwood_mpp_conf(kwmpp_config);

	return 0;
}

int board_init(void)
{
	/* Machine number */
	gd->bd->bi_arch_number = MACH_TYPE_NET2BIG_V2;

	/* Boot parameters address */
	gd->bd->bi_boot_params = kw_sdram_bar(0) + 0x100;

	return 0;
}

#if defined(CONFIG_MISC_INIT_R)
int misc_init_r(void)
{
#if defined(CONFIG_CMD_I2C) && defined(CONFIG_SYS_I2C_EEPROM_ADDR)
	if (!getenv("ethaddr")) {
		uchar mac[6];
		if (lacie_read_mac_address(mac) == 0)
			eth_setenv_enetaddr("ethaddr", mac);
	}
#endif
	return 0;
}
#endif

#if defined(CONFIG_CMD_NET) && defined(CONFIG_RESET_PHY_R)
/* Configure and initialize PHY */
void reset_phy(void)
{
	mv_phy_88e1116_init("egiga0");
}
#endif

#if defined(CONFIG_KIRKWOOD_GPIO)
/* Return GPIO push button status */
static int
do_read_push_button(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return !kw_gpio_get_value(NET2BIG_V2_GPIO_PUSH_BUTTON);
}

U_BOOT_CMD(button, 1, 1, do_read_push_button,
	   "Return GPIO push button status 0=off 1=on", "");
#endif
