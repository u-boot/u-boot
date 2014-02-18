/*
 * mux.c
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mux.h>
#include "board.h"

static struct module_pin_mux rmii1_pin_mux[] = {
	{OFFSET(mii1_txen), MODE(1)},			/* RMII1_TXEN */
	{OFFSET(mii1_txd1), MODE(1)},			/* RMII1_TD1 */
	{OFFSET(mii1_txd0), MODE(1)},			/* RMII1_TD0 */
	{OFFSET(mii1_rxd1), MODE(1) | RXACTIVE},	/* RMII1_RD1 */
	{OFFSET(mii1_rxd0), MODE(1) | RXACTIVE},	/* RMII1_RD0 */
	{OFFSET(mii1_rxdv), MODE(1) | RXACTIVE},	/* RMII1_RXDV */
	{OFFSET(mii1_crs), MODE(1) | RXACTIVE},		/* RMII1_CRS_DV */
	{OFFSET(mii1_rxerr), MODE(1) | RXACTIVE},	/* RMII1_RXERR */
	{OFFSET(rmii1_refclk), MODE(0) | RXACTIVE},	/* RMII1_refclk */
	{-1},
};

static struct module_pin_mux rgmii1_pin_mux[] = {
	{OFFSET(mii1_txen), MODE(2)},			/* RGMII1_TCTL */
	{OFFSET(mii1_rxdv), MODE(2) | RXACTIVE},	/* RGMII1_RCTL */
	{OFFSET(mii1_txd3), MODE(2)},			/* RGMII1_TD3 */
	{OFFSET(mii1_txd2), MODE(2)},			/* RGMII1_TD2 */
	{OFFSET(mii1_txd1), MODE(2)},			/* RGMII1_TD1 */
	{OFFSET(mii1_txd0), MODE(2)},			/* RGMII1_TD0 */
	{OFFSET(mii1_txclk), MODE(2)},			/* RGMII1_TCLK */
	{OFFSET(mii1_rxclk), MODE(2) | RXACTIVE},	/* RGMII1_RCLK */
	{OFFSET(mii1_rxd3), MODE(2) | RXACTIVE},	/* RGMII1_RD3 */
	{OFFSET(mii1_rxd2), MODE(2) | RXACTIVE},	/* RGMII1_RD2 */
	{OFFSET(mii1_rxd1), MODE(2) | RXACTIVE},	/* RGMII1_RD1 */
	{OFFSET(mii1_rxd0), MODE(2) | RXACTIVE},	/* RGMII1_RD0 */
	{-1},
};

static struct module_pin_mux mdio_pin_mux[] = {
	{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN},/* MDIO_DATA */
	{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},	/* MDIO_CLK */
	{-1},
};

static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE | SLEWCTRL)},
	{OFFSET(uart0_txd), (MODE(0) | PULLUDDIS | PULLUP_EN | SLEWCTRL)},
	{-1},
};

static struct module_pin_mux mmc0_pin_mux[] = {
	{OFFSET(mmc0_clk), (MODE(0) | PULLUDDIS | RXACTIVE)},  /* MMC0_CLK */
	{OFFSET(mmc0_cmd), (MODE(0) | PULLUP_EN | RXACTIVE)},  /* MMC0_CMD */
	{OFFSET(mmc0_dat0), (MODE(0) | PULLUP_EN | RXACTIVE)}, /* MMC0_DAT0 */
	{OFFSET(mmc0_dat1), (MODE(0) | PULLUP_EN | RXACTIVE)}, /* MMC0_DAT1 */
	{OFFSET(mmc0_dat2), (MODE(0) | PULLUP_EN | RXACTIVE)}, /* MMC0_DAT2 */
	{OFFSET(mmc0_dat3), (MODE(0) | PULLUP_EN | RXACTIVE)}, /* MMC0_DAT3 */
	{-1},
};

static struct module_pin_mux i2c0_pin_mux[] = {
	{OFFSET(i2c0_sda), (MODE(0) | PULLUP_EN | RXACTIVE | SLEWCTRL)},
	{OFFSET(i2c0_scl), (MODE(0) | PULLUP_EN | RXACTIVE | SLEWCTRL)},
	{-1},
};

static struct module_pin_mux gpio5_7_pin_mux[] = {
	{OFFSET(spi0_cs0), (MODE(7) | PULLUP_EN)},	/* GPIO5_7 */
	{-1},
};

static struct module_pin_mux qspi_pin_mux[] = {
	{OFFSET(gpmc_csn0), (MODE(3) | PULLUP_EN | RXACTIVE)}, /* QSPI_CS0 */
	{OFFSET(gpmc_csn3), (MODE(2) | PULLUP_EN | RXACTIVE)}, /* QSPI_CLK */
	{OFFSET(gpmc_advn_ale), (MODE(3) | PULLUP_EN | RXACTIVE)}, /* QSPI_D0 */
	{OFFSET(gpmc_oen_ren), (MODE(3) | PULLUP_EN | RXACTIVE)}, /* QSPI_D1 */
	{OFFSET(gpmc_wen), (MODE(3) | PULLUP_EN | RXACTIVE)}, /* QSPI_D2 */
	{OFFSET(gpmc_be0n_cle), (MODE(3) | PULLUP_EN | RXACTIVE)}, /* QSPI_D3 */
	{-1},
};

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void enable_board_pin_mux(void)
{
	configure_module_pin_mux(mmc0_pin_mux);
	configure_module_pin_mux(i2c0_pin_mux);
	configure_module_pin_mux(mdio_pin_mux);

	if (board_is_gpevm()) {
		configure_module_pin_mux(gpio5_7_pin_mux);
		configure_module_pin_mux(rgmii1_pin_mux);
	} else if (board_is_eposevm()) {
		configure_module_pin_mux(rmii1_pin_mux);
		configure_module_pin_mux(qspi_pin_mux);
	}
}

void enable_i2c0_pin_mux(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
}
