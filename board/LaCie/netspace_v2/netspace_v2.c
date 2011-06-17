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
#include <miiphy.h>
#include <netdev.h>
#include <command.h>
#include <asm/arch/kirkwood.h>
#include <asm/arch/mpp.h>
#include <asm/arch/gpio.h>
#include "netspace_v2.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/* Gpio configuration */
	kw_config_gpio(NETSPACE_V2_OE_VAL_LOW, NETSPACE_V2_OE_VAL_HIGH,
			NETSPACE_V2_OE_LOW, NETSPACE_V2_OE_HIGH);

	/* Multi-Purpose Pins Functionality configuration */
	u32 kwmpp_config[] = {
		MPP0_SPI_SCn,
		MPP1_SPI_MOSI,
		MPP2_SPI_SCK,
		MPP3_SPI_MISO,
		MPP4_NF_IO6,
		MPP5_NF_IO7,
		MPP6_SYSRST_OUTn,
		MPP7_GPO,		/* Fan speed (bit 1) */
		MPP8_TW_SDA,
		MPP9_TW_SCK,
		MPP10_UART0_TXD,
		MPP11_UART0_RXD,
		MPP12_GPO,		/* Red led */
		MPP14_GPIO,		/* USB fuse */
		MPP16_GPIO,		/* SATA 0 power */
		MPP17_GPIO,		/* SATA 1 power */
		MPP18_NF_IO0,
		MPP19_NF_IO1,
		MPP20_SATA1_ACTn,
		MPP21_SATA0_ACTn,
		MPP22_GPIO,		/* Fan speed (bit 0) */
		MPP23_GPIO,		/* Fan power */
		MPP24_GPIO,		/* USB mode select */
		MPP25_GPIO,		/* Fan rotation fail */
		MPP26_GPIO,		/* USB vbus-in detection */
		MPP28_GPIO,		/* USB enable vbus-out */
		MPP29_GPIO,		/* Blue led (slow register) */
		MPP30_GPIO,		/* Blue led (command register) */
		MPP31_GPIO,		/* Board power off */
		MPP32_GPIO,		/* Button (0 = Released, 1 = Pushed) */
		MPP33_GPIO,		/* Fan speed (bit 2) */
		0
	};
	kirkwood_mpp_conf(kwmpp_config);

	return 0;
}

int board_init(void)
{
	/* Machine number */
	gd->bd->bi_arch_number = CONFIG_MACH_TYPE;

	/* Boot parameters address */
	gd->bd->bi_boot_params = kw_sdram_bar(0) + 0x100;

	return 0;
}

void mv_phy_88e1116_init(char *name)
{
	u16 reg;
	u16 devadr;

	if (miiphy_set_current_dev(name))
		return;

	/* command to read PHY dev address */
	if (miiphy_read(name, 0xEE, 0xEE, (u16 *) &devadr)) {
		printf("Err..(%s) could not read PHY dev address\n", __func__);
		return;
	}

	/*
	 * Enable RGMII delay on Tx and Rx for CPU port
	 * Ref: sec 4.7.2 of chip datasheet
	 */
	miiphy_write(name, devadr, MV88E1116_PGADR_REG, 2);
	miiphy_read(name, devadr, MV88E1116_MAC_CTRL_REG, &reg);
	reg |= (MV88E1116_RGMII_RXTM_CTRL | MV88E1116_RGMII_TXTM_CTRL);
	miiphy_write(name, devadr, MV88E1116_MAC_CTRL_REG, reg);
	miiphy_write(name, devadr, MV88E1116_PGADR_REG, 0);

	/* reset the phy */
	if (miiphy_read(name, devadr, MII_BMCR, &reg) != 0) {
		printf("Err..(%s) PHY status read failed\n", __func__);
		return;
	}
	if (miiphy_write(name, devadr, MII_BMCR, reg | 0x8000) != 0) {
		printf("Err..(%s) PHY reset failed\n", __func__);
		return;
	}

	debug("88E1116 Initialized on %s\n", name);
}

/* Configure and initialize PHY */
void reset_phy(void)
{
	mv_phy_88e1116_init("egiga0");
}

/* Return GPIO button status */
static int
do_read_button(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return kw_gpio_get_value(NETSPACE_V2_GPIO_BUTTON);
}

U_BOOT_CMD(button, 1, 1, do_read_button,
	   "Return GPIO button status 0=off 1=on", "");
