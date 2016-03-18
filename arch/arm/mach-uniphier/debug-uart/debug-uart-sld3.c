/*
 * Copyright (C) 2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <linux/kernel.h>
#include <linux/io.h>

#include "../bcu/bcu-regs.h"
#include "../sc-regs.h"
#include "../sg-regs.h"
#include "debug-uart.h"

#define UNIPHIER_SLD3_UART_CLK		36864000

unsigned int uniphier_sld3_debug_uart_init(void)
{
	u32 tmp;

	sg_set_pinsel(64, 1, 4, 4);	/* TXD0 -> TXD0 */

	writel(0x24440000, BCSCR5);

	tmp = readl(SC_CLKCTRL);
	tmp |= SC_CLKCTRL_CEN_PERI;
	writel(tmp, SC_CLKCTRL);

	return DIV_ROUND_CLOSEST(UNIPHIER_SLD3_UART_CLK, 16 * CONFIG_BAUDRATE);
}
