/*
 * mux.c
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mux.h>
#include <asm/io.h>
#include <i2c.h>
#include "evm.h"

static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(pincntl70), PULLUP_EN | MODE(0x01)},	/* UART0_RXD */
	{OFFSET(pincntl71), PULLUP_EN | MODE(0x01)},	/* UART0_TXD */
	{-1},
};

static struct module_pin_mux mmc1_pin_mux[] = {
	{OFFSET(pincntl1), PULLUP_EN | MODE(0x01)},	/* SD1_CLK */
	{OFFSET(pincntl2), PULLUP_EN | MODE(0x01)},	/* SD1_CMD */
	{OFFSET(pincntl3), PULLUP_EN | MODE(0x01)},	/* SD1_DAT[0] */
	{OFFSET(pincntl4), PULLUP_EN | MODE(0x01)},	/* SD1_DAT[1] */
	{OFFSET(pincntl5), PULLUP_EN | MODE(0x01)},	/* SD1_DAT[2] */
	{OFFSET(pincntl6), PULLUP_EN | MODE(0x01)},	/* SD1_DAT[3] */
	{OFFSET(pincntl74), PULLUP_EN | MODE(0x40)},	/* SD1_POW */
	{OFFSET(pincntl75), MODE(0x40)},		/* SD1_SDWP */
	{OFFSET(pincntl80), PULLUP_EN | MODE(0x02)},	/* SD1_SDCD */
	{-1},
};

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void enable_mmc1_pin_mux(void)
{
	configure_module_pin_mux(mmc1_pin_mux);
}
