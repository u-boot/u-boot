/*
 * K2G EVM : Board initialization
 *
 * (C) Copyright 2015
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#include <common.h>
#include <asm/arch/clock.h>
#include <asm/ti-common/keystone_net.h>
#include <asm/arch/psc_defs.h>
#include <asm/arch/mmc_host_def.h>
#include "mux-k2g.h"

#define SYS_CLK		24000000

unsigned int external_clk[ext_clk_count] = {
	[sys_clk]	=	SYS_CLK,
	[pa_clk]	=	SYS_CLK,
	[tetris_clk]	=	SYS_CLK,
	[ddr3a_clk]	=	SYS_CLK,
	[uart_clk]	=	SYS_CLK,
};

static struct pll_init_data main_pll_config = {MAIN_PLL, 100, 1, 4};
static struct pll_init_data tetris_pll_config = {TETRIS_PLL, 100, 1, 4};
static struct pll_init_data uart_pll_config = {UART_PLL, 64, 1, 4};
static struct pll_init_data nss_pll_config = {NSS_PLL, 250, 3, 2};
static struct pll_init_data ddr3_pll_config = {DDR3A_PLL, 250, 3, 10};

struct pll_init_data *get_pll_init_data(int pll)
{
	struct pll_init_data *data = NULL;

	switch (pll) {
	case MAIN_PLL:
		data = &main_pll_config;
		break;
	case TETRIS_PLL:
		data = &tetris_pll_config;
		break;
	case NSS_PLL:
		data = &nss_pll_config;
		break;
	case UART_PLL:
		data = &uart_pll_config;
		break;
	case DDR3_PLL:
		data = &ddr3_pll_config;
		break;
	default:
		data = NULL;
	}

	return data;
}

s16 divn_val[16] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_GENERIC_MMC)
int board_mmc_init(bd_t *bis)
{
	if (psc_enable_module(KS2_LPSC_MMC)) {
		printf("%s module enabled failed\n", __func__);
		return -1;
	}

	omap_mmc_init(0, 0, 0, -1, -1);
	omap_mmc_init(1, 0, 0, -1, -1);
	return 0;
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	init_plls();

	k2g_mux_config();

	/* deassert FLASH_HOLD */
	clrbits_le32(K2G_GPIO1_BANK2_BASE + K2G_GPIO_DIR_OFFSET,
		     BIT(9));
	setbits_le32(K2G_GPIO1_BANK2_BASE + K2G_GPIO_SETDATA_OFFSET,
		     BIT(9));

	return 0;
}
#endif

#ifdef CONFIG_SPL_BUILD
void spl_init_keystone_plls(void)
{
	init_plls();
}
#endif

#ifdef CONFIG_DRIVER_TI_KEYSTONE_NET
struct eth_priv_t eth_priv_cfg[] = {
	{
		.int_name	= "K2G_EMAC",
		.rx_flow	= 0,
		.phy_addr	= 0,
		.slave_port	= 1,
		.sgmii_link_type = SGMII_LINK_MAC_PHY,
		.phy_if          = PHY_INTERFACE_MODE_RGMII,
	},
};

int get_num_eth_ports(void)
{
	return sizeof(eth_priv_cfg) / sizeof(struct eth_priv_t);
}
#endif
