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

#include "../common/common.h"
#include "netdev.h"

DECLARE_GLOBAL_DATA_PTR;

/* GPIO configuration: start FAN at low speed, USB and HDD */

#define WIRELESS_SPACE_OE_LOW		0xFF006808
#define WIRELESS_SPACE_OE_HIGH		0x0000F989
#define WIRELESS_SPACE_OE_VAL_LOW	0x00010080
#define WIRELESS_SPACE_OE_VAL_HIGH	0x00000240

#define WIRELESS_SPACE_REAR_BUTTON	13
#define WIRELESS_SPACE_FRONT_BUTTON	43

const u32 kwmpp_config[] = {
	MPP0_NF_IO2,
	MPP1_NF_IO3,
	MPP2_NF_IO4,
	MPP3_NF_IO5,
	MPP4_NF_IO6,
	MPP5_NF_IO7,
	MPP6_SYSRST_OUTn,
	MPP7_GPO,		/* Fan speed (bit 1) */
	MPP8_TW_SDA,
	MPP9_TW_SCK,
	MPP10_UART0_TXD,
	MPP11_UART0_RXD,
	MPP13_GPIO,		/* Red led */
	MPP14_GPIO,		/* USB fuse */
	MPP15_SATA0_ACTn,
	MPP16_GPIO,		/* SATA 0 power */
	MPP17_GPIO,		/* SATA 1 power */
	MPP18_NF_IO0,
	MPP19_NF_IO1,
	MPP20_GE1_0,		/* Gigabit Ethernet 1 */
	MPP21_GE1_1,
	MPP22_GE1_2,
	MPP23_GE1_3,
	MPP24_GE1_4,
	MPP25_GE1_5,
	MPP26_GE1_6,
	MPP27_GE1_7,
	MPP28_GE1_8,
	MPP29_GE1_9,
	MPP30_GE1_10,
	MPP31_GE1_11,
	MPP32_GE1_12,
	MPP33_GE1_13,
	MPP34_GE1_14,
	MPP35_GE1_15,
	MPP36_GPIO,		/* Fan speed (bit 2) */
	MPP37_GPIO,		/* Fan speed (bit 0) */
	MPP38_GPIO,		/* Fan power */
	MPP39_GPIO,		/* Fan rotation fail */
	MPP40_GPIO,		/* Ethernet switch link */
	MPP41_GPIO,		/* USB enable host vbus */
	MPP42_GPIO,		/* LED clock control */
	MPP43_GPIO,		/* WPS button (0=Pushed, 1=Released) */
	MPP44_GPIO,		/* Red LED on/off */
	MPP45_GPIO,		/* Red LED timer blink (on=off=100ms) */
	MPP46_GPIO,		/* Green LED on/off */
	MPP47_GPIO,		/* LED (blue, green) SATA activity blink */
	MPP48_GPIO,		/* Blue LED on/off */
	0
};

struct mv88e61xx_config swcfg = {
	.name = "egiga0",
	.vlancfg = MV88E61XX_VLANCFG_ROUTER,
	.rgmii_delay = MV88E61XX_RGMII_DELAY_EN,
	.led_init = MV88E61XX_LED_INIT_EN,
	.mdip = MV88E61XX_MDIP_NOCHANGE,
	.portstate = MV88E61XX_PORTSTT_FORWARDING,
	.cpuport = 0x20,
	.ports_enabled = 0x3F,
};

int board_early_init_f(void)
{
	/* Gpio configuration */
	kw_config_gpio(WIRELESS_SPACE_OE_VAL_LOW, WIRELESS_SPACE_OE_VAL_HIGH,
			WIRELESS_SPACE_OE_LOW, WIRELESS_SPACE_OE_HIGH);

	/* Multi-Purpose Pins Functionality configuration */
	kirkwood_mpp_conf(kwmpp_config, NULL);

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
	/* configure switch on egiga0 */
	mv88e61xx_switch_initialize(&swcfg);
}
#endif

#if defined(CONFIG_KIRKWOOD_GPIO) && defined(CONFIG_WIRELESS_SPACE_CMD)
/* Return GPIO button status */
static int
do_ws(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (strcmp(argv[1], "button") == 0) {
		if (strcmp(argv[2], "rear") == 0)
			/* invert GPIO result for intuitive while/until use */
			return !kw_gpio_get_value(WIRELESS_SPACE_REAR_BUTTON);
		else if (strcmp(argv[2], "front") == 0)
			return kw_gpio_get_value(WIRELESS_SPACE_FRONT_BUTTON);
		else
			return -1;
	} else {
		return -1;
	}
}

U_BOOT_CMD(ws, 3, 0, do_ws,
	   "Return GPIO button status 0=off 1=on",
	   "- ws button rear|front: test buttons' states\n"
);
#endif
