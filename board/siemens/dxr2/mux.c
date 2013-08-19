/*
 * pinmux setup for siemens dxr2 board
 *
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * u-boot:/board/ti/am335x/mux.c
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mux.h>
#include <asm/io.h>
#include <i2c.h>
#include "board.h"

static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* UART0_RXD */
	{OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},		/* UART0_TXD */
	{-1},
};

static struct module_pin_mux uart3_pin_mux[] = {
	{OFFSET(spi0_cs1), (MODE(1) | PULLUP_EN | RXACTIVE)},	/* UART3_RXD */
	{OFFSET(ecap0_in_pwm0_out), (MODE(1) | PULLUDEN)},	/* UART3_TXD */
	{-1},
};

static struct module_pin_mux i2c0_pin_mux[] = {
	{OFFSET(i2c0_sda), (MODE(0) | RXACTIVE |
			PULLUDEN | SLEWCTRL)}, /* I2C_DATA */
	{OFFSET(i2c0_scl), (MODE(0) | RXACTIVE |
			PULLUDEN | SLEWCTRL)}, /* I2C_SCLK */
	{-1},
};

static struct module_pin_mux nand_pin_mux[] = {
	{OFFSET(gpmc_ad0), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD0 */
	{OFFSET(gpmc_ad1), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD1 */
	{OFFSET(gpmc_ad2), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD2 */
	{OFFSET(gpmc_ad3), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD3 */
	{OFFSET(gpmc_ad4), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD4 */
	{OFFSET(gpmc_ad5), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD5 */
	{OFFSET(gpmc_ad6), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD6 */
	{OFFSET(gpmc_ad7), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD7 */
	{OFFSET(gpmc_wait0), (MODE(0) | RXACTIVE | PULLUP_EN)}, /* NAND WAIT */
	{OFFSET(gpmc_wpn), (MODE(7) | PULLUP_EN | RXACTIVE)},	/* NAND_WPN */
	{OFFSET(gpmc_csn0), (MODE(0) | PULLUDEN)},	/* NAND_CS0 */
	{OFFSET(gpmc_advn_ale), (MODE(0) | PULLUDEN)}, /* NAND_ADV_ALE */
	{OFFSET(gpmc_oen_ren), (MODE(0) | PULLUDEN)},	/* NAND_OE */
	{OFFSET(gpmc_wen), (MODE(0) | PULLUDEN)},	/* NAND_WEN */
	{OFFSET(gpmc_be0n_cle), (MODE(0) | PULLUDEN)},	/* NAND_BE_CLE */
	{-1},
};

static struct module_pin_mux gpios_pin_mux[] = {
	/* DFU button GPIO0_27*/
	{OFFSET(gpmc_ad11), (MODE(7) | PULLUDEN | RXACTIVE)},
	{OFFSET(gpmc_csn3), MODE(7) },			/* LED0 GPIO2_0 */
	{OFFSET(emu0), MODE(7)},			/* LED1 GPIO3_7 */
	{-1},
};

static struct module_pin_mux ethernet_pin_mux[] = {
	{OFFSET(mii1_col), (MODE(3) | RXACTIVE)},
	{OFFSET(mii1_crs), (MODE(1) | RXACTIVE)},
	{OFFSET(mii1_rxerr), (MODE(1) | RXACTIVE)},
	{OFFSET(mii1_txen), (MODE(1))},
	{OFFSET(mii1_rxdv), (MODE(3) | RXACTIVE)},
	{OFFSET(mii1_txd3), (MODE(7) | RXACTIVE)},
	{OFFSET(mii1_txd2), (MODE(7) | RXACTIVE)},
	{OFFSET(mii1_txd1), (MODE(1))},
	{OFFSET(mii1_txd0), (MODE(1))},
	{OFFSET(mii1_txclk), (MODE(1) | RXACTIVE)},
	{OFFSET(mii1_rxclk), (MODE(1) | RXACTIVE)},
	{OFFSET(mii1_rxd3), (MODE(1) | RXACTIVE)},
	{OFFSET(mii1_rxd2), (MODE(1))},
	{OFFSET(mii1_rxd1), (MODE(1) | RXACTIVE)},
	{OFFSET(mii1_rxd0), (MODE(1) | RXACTIVE)},
	{OFFSET(rmii1_refclk), (MODE(0) | RXACTIVE)},
	{OFFSET(mdio_data), (MODE(0) | RXACTIVE | PULLUP_EN)},
	{OFFSET(mdio_clk), (MODE(0) | PULLUP_EN)},
	{-1},
};

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void enable_uart3_pin_mux(void)
{
	configure_module_pin_mux(uart3_pin_mux);
}

void enable_i2c0_pin_mux(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
}

void enable_board_pin_mux(void)
{
	enable_uart3_pin_mux();
	configure_module_pin_mux(nand_pin_mux);
	configure_module_pin_mux(ethernet_pin_mux);
	configure_module_pin_mux(gpios_pin_mux);
}
