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
#include "../common/board_detect.h"

#define SYS_CLK		24000000

unsigned int external_clk[ext_clk_count] = {
	[sys_clk]	=	SYS_CLK,
	[pa_clk]	=	SYS_CLK,
	[tetris_clk]	=	SYS_CLK,
	[ddr3a_clk]	=	SYS_CLK,
	[uart_clk]	=	SYS_CLK,
};

static int arm_speeds[DEVSPEED_NUMSPDS] = {
	SPD400,
	SPD600,
	SPD800,
	SPD900,
	SPD1000,
	SPD900,
	SPD800,
	SPD600,
	SPD400,
	SPD200,
};

static int dev_speeds[DEVSPEED_NUMSPDS] = {
	SPD600,
	SPD800,
	SPD900,
	SPD1000,
	SPD900,
	SPD800,
	SPD600,
	SPD400,
};

static struct pll_init_data main_pll_config[NUM_SPDS] = {
	[SPD400]	= {MAIN_PLL, 100, 3, 2},
	[SPD600]	= {MAIN_PLL, 300, 6, 2},
	[SPD800]	= {MAIN_PLL, 200, 3, 2},
	[SPD900] =	{TETRIS_PLL, 75, 1, 2},
	[SPD1000] =	{TETRIS_PLL, 250, 3, 2},
};

static struct pll_init_data tetris_pll_config[NUM_SPDS] = {
	[SPD200] =	{TETRIS_PLL, 250, 3, 10},
	[SPD400] =	{TETRIS_PLL, 100, 1, 6},
	[SPD600] =	{TETRIS_PLL, 100, 1, 4},
	[SPD800] =	{TETRIS_PLL, 400, 3, 4},
	[SPD900] =	{TETRIS_PLL, 75, 1, 2},
	[SPD1000] =	{TETRIS_PLL, 250, 3, 2},
};

static struct pll_init_data uart_pll_config = {UART_PLL, 64, 1, 4};
static struct pll_init_data nss_pll_config = {NSS_PLL, 250, 3, 2};
static struct pll_init_data ddr3_pll_config = {DDR3A_PLL, 133, 1, 16};

struct pll_init_data *get_pll_init_data(int pll)
{
	int speed;
	struct pll_init_data *data = NULL;

	switch (pll) {
	case MAIN_PLL:
		speed = get_max_dev_speed(dev_speeds);
		data = &main_pll_config[speed];
		break;
	case TETRIS_PLL:
		speed = get_max_arm_speed(arm_speeds);
		data = &tetris_pll_config[speed];
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

#if defined(CONFIG_GENERIC_MMC)
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

static void k2g_reset_mux_config(void)
{
	/* Unlock the reset mux register */
	clrbits_le32(KS2_RSTMUX8, RSTMUX_LOCK8_MASK);

	/* Configure BOOTCFG_RSTMUX8 for WDT event to cause a device reset */
	clrsetbits_le32(KS2_RSTMUX8, RSTMUX_OMODE8_MASK,
			RSTMUX_OMODE8_DEV_RESET << RSTMUX_OMODE8_SHIFT);

	/* lock the reset mux register to prevent any spurious writes. */
	setbits_le32(KS2_RSTMUX8, RSTMUX_LOCK8_MASK);
}

int board_early_init_f(void)
{
	init_plls();

	k2g_mux_config();

	k2g_reset_mux_config();

	/* deassert FLASH_HOLD */
	clrbits_le32(K2G_GPIO1_BANK2_BASE + K2G_GPIO_DIR_OFFSET,
		     BIT(9));
	setbits_le32(K2G_GPIO1_BANK2_BASE + K2G_GPIO_SETDATA_OFFSET,
		     BIT(9));

	return 0;
}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_TI_I2C_BOARD_DETECT)
	int rc;

	rc = ti_i2c_eeprom_am_get(CONFIG_EEPROM_BUS_ADDRESS,
			CONFIG_EEPROM_CHIP_ADDRESS);
	if (rc)
		printf("ti_i2c_eeprom_init failed %d\n", rc);

	board_ti_set_ethaddr(1);
#endif

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
