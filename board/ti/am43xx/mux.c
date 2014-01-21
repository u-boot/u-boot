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

static struct module_pin_mux gpio0_22_pin_mux[] = {
	{OFFSET(ddr_ba2), (MODE(9) | PULLUP_EN)},	/* GPIO0_22 */
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

	if (board_is_gpevm())
		configure_module_pin_mux(gpio0_22_pin_mux);
}

void enable_i2c0_pin_mux(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
}
