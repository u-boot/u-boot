/*
 * K2E EVM : Board initialization
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/arch/ddr3.h>
#include <asm/arch/hardware.h>
#include <asm/ti-common/keystone_net.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned int external_clk[ext_clk_count] = {
	[sys_clk]	= 100000000,
	[alt_core_clk]	= 100000000,
	[pa_clk]	= 100000000,
	[ddr3_clk]	= 100000000,
	[mcm_clk]	= 312500000,
	[pcie_clk]	= 100000000,
	[sgmii_clk]	= 156250000,
	[xgmii_clk]	= 156250000,
	[usb_clk]	= 100000000,
};

static struct pll_init_data core_pll_config[] = {
	CORE_PLL_800,
	CORE_PLL_850,
	CORE_PLL_1000,
	CORE_PLL_1250,
	CORE_PLL_1350,
	CORE_PLL_1400,
	CORE_PLL_1500,
};

static struct pll_init_data pa_pll_config =
	PASS_PLL_1000;

#ifdef CONFIG_DRIVER_TI_KEYSTONE_NET
struct eth_priv_t eth_priv_cfg[] = {
	{
		.int_name        = "K2E_EMAC0",
		.rx_flow         = 0,
		.phy_addr        = 0,
		.slave_port      = 1,
		.sgmii_link_type = SGMII_LINK_MAC_PHY,
	},
	{
		.int_name        = "K2E_EMAC1",
		.rx_flow         = 8,
		.phy_addr        = 1,
		.slave_port      = 2,
		.sgmii_link_type = SGMII_LINK_MAC_PHY,
	},
	{
		.int_name        = "K2E_EMAC2",
		.rx_flow         = 16,
		.phy_addr        = 2,
		.slave_port      = 3,
		.sgmii_link_type = SGMII_LINK_MAC_MAC_FORCED,
	},
	{
		.int_name        = "K2E_EMAC3",
		.rx_flow         = 24,
		.phy_addr        = 3,
		.slave_port      = 4,
		.sgmii_link_type = SGMII_LINK_MAC_MAC_FORCED,
	},
	{
		.int_name        = "K2E_EMAC4",
		.rx_flow         = 32,
		.phy_addr        = 4,
		.slave_port      = 5,
		.sgmii_link_type = SGMII_LINK_MAC_MAC_FORCED,
	},
	{
		.int_name        = "K2E_EMAC5",
		.rx_flow         = 40,
		.phy_addr        = 5,
		.slave_port      = 6,
		.sgmii_link_type = SGMII_LINK_MAC_MAC_FORCED,
	},
	{
		.int_name        = "K2E_EMAC6",
		.rx_flow         = 48,
		.phy_addr        = 6,
		.slave_port      = 7,
		.sgmii_link_type = SGMII_LINK_MAC_MAC_FORCED,
	},
	{
		.int_name        = "K2E_EMAC7",
		.rx_flow         = 56,
		.phy_addr        = 7,
		.slave_port      = 8,
		.sgmii_link_type = SGMII_LINK_MAC_MAC_FORCED,
	},
};

int get_num_eth_ports(void)
{
	return sizeof(eth_priv_cfg) / sizeof(struct eth_priv_t);
}
#endif

#if defined(CONFIG_BOARD_EARLY_INIT_F)
int board_early_init_f(void)
{
	int speed;

	speed = get_max_dev_speed();
	init_pll(&core_pll_config[speed]);

	init_pll(&pa_pll_config);

	return 0;
}
#endif

#ifdef CONFIG_SPL_BUILD
static struct pll_init_data spl_pll_config[] = {
	CORE_PLL_800,
};

void spl_init_keystone_plls(void)
{
	init_plls(ARRAY_SIZE(spl_pll_config), spl_pll_config);
}
#endif
