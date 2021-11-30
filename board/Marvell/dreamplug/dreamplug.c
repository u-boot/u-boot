// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021  Tony Dinh <mibodhi@gmail.com>
 * (C) Copyright 2011
 * Jason Cooper <u-boot@lakedaemon.net>
 *
 * Based on work by:
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Siddarth Gore <gores@marvell.com>
 */

#include <common.h>
#include <init.h>
#include <miiphy.h>
#include <net.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>
#include <asm/global_data.h>
#include "dreamplug.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the  below configuration configures mainly initial LED status
	 */
	mvebu_config_gpio(DREAMPLUG_OE_VAL_LOW,
			  DREAMPLUG_OE_VAL_HIGH,
			  DREAMPLUG_OE_LOW, DREAMPLUG_OE_HIGH);

	/* Multi-Purpose Pins Functionality configuration */
	static const u32 kwmpp_config[] = {
		MPP0_SPI_SCn,		/* SPI Flash */
		MPP1_SPI_MOSI,
		MPP2_SPI_SCK,
		MPP3_SPI_MISO,
		MPP4_NF_IO6,
		MPP5_NF_IO7,
		MPP6_SYSRST_OUTn,
		MPP7_GPO,
		MPP8_TW_SDA,
		MPP9_TW_SCK,
		MPP10_UART0_TXD,	/* Serial */
		MPP11_UART0_RXD,
		MPP12_SD_CLK,		/* SDIO Slot */
		MPP13_SD_CMD,
		MPP14_SD_D0,
		MPP15_SD_D1,
		MPP16_SD_D2,
		MPP17_SD_D3,
		MPP18_NF_IO0,
		MPP19_NF_IO1,
		MPP20_GE1_0,		/* Gigabit Ethernet */
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
		MPP36_GPIO,		/* 7 external GPIO pins (36 - 45) */
		MPP37_GPIO,
		MPP38_GPIO,
		MPP39_GPIO,
		MPP40_TDM_SPI_SCK,
		MPP41_TDM_SPI_MISO,
		MPP42_TDM_SPI_MOSI,
		MPP43_GPIO,
		MPP44_GPIO,
		MPP45_GPIO,
		MPP46_GPIO,
		MPP47_GPIO,		/* Bluetooth LED */
		MPP48_GPIO,		/* Wifi LED */
		MPP49_GPIO,		/* Wifi AP LED */
		0
	};
	kirkwood_mpp_conf(kwmpp_config, NULL);
	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

static int fdt_get_phy_addr(const char *path)
{
	const void *fdt = gd->fdt_blob;
	const u32 *reg;
	const u32 *val;
	int node, phandle, addr;

	/* Find the node by its full path */
	node = fdt_path_offset(fdt, path);
	if (node >= 0) {
		/* Look up phy-handle */
		val = fdt_getprop(fdt, node, "phy-handle", NULL);
		if (val) {
			phandle = fdt32_to_cpu(*val);
			if (!phandle)
				return -1;
			/* Follow it to its node */
			node = fdt_node_offset_by_phandle(fdt, phandle);
			if (node) {
				/* Look up reg */
				reg = fdt_getprop(fdt, node, "reg", NULL);
				if (reg) {
					addr = fdt32_to_cpu(*reg);
					return addr;
				}
			}
		}
	}
	return -1;
}

#ifdef CONFIG_RESET_PHY_R
void mv_phy_88e1116_init(const char *name, const char *path)
{
	u16 reg;
	int phyaddr;

	if (miiphy_set_current_dev(name))
		return;

	phyaddr = fdt_get_phy_addr(path);
	if (phyaddr < 0)
		return;

	/*
	 * Enable RGMII delay on Tx and Rx for CPU port
	 * Ref: sec 4.7.2 of chip datasheet
	 */
	miiphy_write(name, phyaddr, MV88E1116_PGADR_REG, 2);
	miiphy_read(name, phyaddr, MV88E1116_MAC_CTRL2_REG, &reg);
	reg |= (MV88E1116_RGMII_RXTM_CTRL | MV88E1116_RGMII_TXTM_CTRL);
	miiphy_write(name, phyaddr, MV88E1116_MAC_CTRL2_REG, reg);
	miiphy_write(name, phyaddr, MV88E1116_PGADR_REG, 0);

	/* reset the phy */
	miiphy_reset(name, phyaddr);

	printf("88E1116 Initialized on %s\n", name);
}

void reset_phy(void)
{
	char *eth0_name = "ethernet-controller@72000";
	char *eth0_path = "/ocp@f1000000/ethernet-controller@72000/ethernet0-port@0";
	char *eth1_name = "ethernet-controller@76000";
	char *eth1_path = "/ocp@f1000000/ethernet-controller@76000/ethernet1-port@0";

	/* configure and initialize both PHY's */
	mv_phy_88e1116_init(eth0_name, eth0_path);
	mv_phy_88e1116_init(eth1_name, eth1_path);
}
#endif /* CONFIG_RESET_PHY_R */
