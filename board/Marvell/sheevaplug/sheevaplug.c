// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021  Tony Dinh <mibodhi@gmail.com>
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#include <common.h>
#include <init.h>
#include <miiphy.h>
#include <net.h>
#include <asm/global_data.h>
#include <asm/mach-types.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>
#include "sheevaplug.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the  below configuration configures mainly initial LED status
	 */
	mvebu_config_gpio(SHEEVAPLUG_OE_VAL_LOW,
			  SHEEVAPLUG_OE_VAL_HIGH,
			  SHEEVAPLUG_OE_LOW, SHEEVAPLUG_OE_HIGH);

	/* Multi-Purpose Pins Functionality configuration */
	static const u32 kwmpp_config[] = {
		MPP0_NF_IO2,
		MPP1_NF_IO3,
		MPP2_NF_IO4,
		MPP3_NF_IO5,
		MPP4_NF_IO6,
		MPP5_NF_IO7,
		MPP6_SYSRST_OUTn,
		MPP7_GPO,
		MPP8_UART0_RTS,
		MPP9_UART0_CTS,
		MPP10_UART0_TXD,
		MPP11_UART0_RXD,
		MPP12_SD_CLK,
		MPP13_SD_CMD,
		MPP14_SD_D0,
		MPP15_SD_D1,
		MPP16_SD_D2,
		MPP17_SD_D3,
		MPP18_NF_IO0,
		MPP19_NF_IO1,
		MPP20_GPIO,
		MPP21_GPIO,
		MPP22_GPIO,
		MPP23_GPIO,
		MPP24_GPIO,
		MPP25_GPIO,
		MPP26_GPIO,
		MPP27_GPIO,
		MPP28_GPIO,
		MPP29_TSMP9,
		MPP30_GPIO,
		MPP31_GPIO,
		MPP32_GPIO,
		MPP33_GPIO,
		MPP34_GPIO,
		MPP35_GPIO,
		MPP36_GPIO,
		MPP37_GPIO,
		MPP38_GPIO,
		MPP39_GPIO,
		MPP40_GPIO,
		MPP41_GPIO,
		MPP42_GPIO,
		MPP43_GPIO,
		MPP44_GPIO,
		MPP45_GPIO,
		MPP46_GPIO,
		MPP47_GPIO,
		MPP48_GPIO,
		MPP49_GPIO,
		0
	};
	kirkwood_mpp_conf(kwmpp_config, NULL);
	return 0;
}

int board_init(void)
{
	/*
	 * arch number of board
	 */
	gd->bd->bi_arch_number = MACH_TYPE_SHEEVAPLUG;

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
/* Configure and enable MV88E1116 PHY */
void reset_phy(void)
{
	u16 reg;
	int phyaddr;
	char *name = "ethernet-controller@72000";
	char *eth0_path = "/ocp@f1000000/ethernet-controller@72000/ethernet0-port@0";

	if (miiphy_set_current_dev(name))
		return;

	phyaddr = fdt_get_phy_addr(eth0_path);
	if (phyaddr < 0)
		return;

	/*
	 * Enable RGMII delay on Tx and Rx for CPU port
	 * Ref: sec 4.7.2 of chip datasheet
	 */
	miiphy_write(name, phyaddr, MV88E1116_PGADR_REG, 2);
	miiphy_read(name, phyaddr, MV88E1116_MAC_CTRL_REG, &reg);
	reg |= (MV88E1116_RGMII_RXTM_CTRL | MV88E1116_RGMII_TXTM_CTRL);
	miiphy_write(name, phyaddr, MV88E1116_MAC_CTRL_REG, reg);
	miiphy_write(name, phyaddr, MV88E1116_PGADR_REG, 0);

	/* reset the phy */
	miiphy_reset(name, phyaddr);

	printf("88E1116 Initialized on %s\n", name);
}
#endif /* CONFIG_RESET_PHY_R */
