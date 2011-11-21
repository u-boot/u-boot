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
#include <i2c.h>
#include <asm/arch/cpu.h>
#include <asm/arch/kirkwood.h>
#include <asm/arch/mpp.h>
#include <asm/arch/gpio.h>
#include "net2big_v2.h"

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

int misc_init_r(void)
{
#ifdef CONFIG_CMD_I2C
	if (!getenv("ethaddr")) {
		ushort version;
		uchar mac[6];
		int ret;

		/* I2C-0 for on-board EEPROM */
		i2c_set_bus_num(0);

		/* Check layout version for EEPROM data */
		ret = i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0,
				CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
				(uchar *) &version, 2);
		if (ret != 0) {
			printf("Error: failed to read I2C EEPROM @%02x\n",
				CONFIG_SYS_I2C_EEPROM_ADDR);
			return ret;
		}
		version = be16_to_cpu(version);
		if (version < 1 || version > 3) {
			printf("Error: unknown version %d for EEPROM data\n",
				version);
			return -1;
		}

		/* Read Ethernet MAC address from EEPROM */
		ret = i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 2,
				CONFIG_SYS_I2C_EEPROM_ADDR_LEN, mac, 6);
		if (ret != 0) {
			printf("Error: failed to read I2C EEPROM @%02x\n",
				CONFIG_SYS_I2C_EEPROM_ADDR);
			return ret;
		}
		eth_setenv_enetaddr("ethaddr", mac);
	}
#endif /* CONFIG_CMD_I2C */

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

/* Return GPIO push button status */
static int
do_read_push_button(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return !kw_gpio_get_value(NET2BIG_V2_GPIO_PUSH_BUTTON);
}

U_BOOT_CMD(button, 1, 1, do_read_push_button,
	   "Return GPIO push button status 0=off 1=on", "");
