/*
 * K2HK EVM : Board initialization
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/emac_defs.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned int external_clk[ext_clk_count] = {
	[sys_clk]	=	122880000,
	[alt_core_clk]	=	125000000,
	[pa_clk]	=	122880000,
	[tetris_clk]	=	125000000,
	[ddr3a_clk]	=	100000000,
	[ddr3b_clk]	=	100000000,
	[mcm_clk]	=	312500000,
	[pcie_clk]	=	100000000,
	[sgmii_srio_clk] =	156250000,
	[xgmii_clk]	=	156250000,
	[usb_clk]	=	100000000,
	[rp1_clk]	=	123456789
};

static struct pll_init_data pll_config[] = {
	CORE_PLL_1228,
	PASS_PLL_983,
	TETRIS_PLL_1200,
};

#ifdef CONFIG_DRIVER_TI_KEYSTONE_NET
struct eth_priv_t eth_priv_cfg[] = {
	{
		.int_name	= "K2HK_EMAC",
		.rx_flow	= 22,
		.phy_addr	= 0,
		.slave_port	= 1,
		.sgmii_link_type = SGMII_LINK_MAC_PHY,
	},
	{
		.int_name	= "K2HK_EMAC1",
		.rx_flow	= 23,
		.phy_addr	= 1,
		.slave_port	= 2,
		.sgmii_link_type = SGMII_LINK_MAC_PHY,
	},
	{
		.int_name	= "K2HK_EMAC2",
		.rx_flow	= 24,
		.phy_addr	= 2,
		.slave_port	= 3,
		.sgmii_link_type = SGMII_LINK_MAC_MAC_FORCED,
	},
	{
		.int_name	= "K2HK_EMAC3",
		.rx_flow	= 25,
		.phy_addr	= 3,
		.slave_port	= 4,
		.sgmii_link_type = SGMII_LINK_MAC_MAC_FORCED,
	},
};

int get_num_eth_ports(void)
{
	return sizeof(eth_priv_cfg) / sizeof(struct eth_priv_t);
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	init_plls(ARRAY_SIZE(pll_config), pll_config);
	return 0;
}
#endif
