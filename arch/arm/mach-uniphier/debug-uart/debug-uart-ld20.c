/*
 * Copyright (C) 2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <linux/kernel.h>
#include <linux/io.h>

#include "../sc64-regs.h"
#include "../sg-regs.h"
#include "debug-uart.h"

#define UNIPHIER_LD20_UART_CLK		58820000

unsigned int uniphier_ld20_debug_uart_init(void)
{
	u32 tmp;

	sg_set_iectrl(54);		/* TXD0 */
	sg_set_iectrl(58);		/* TXD1 */
	sg_set_iectrl(90);		/* TXD2 */
	sg_set_iectrl(94);		/* TXD3 */
	sg_set_pinsel(54, 0, 8, 4);	/* TXD0 -> TXD0 */
	sg_set_pinsel(58, 1, 8, 4);	/* SPITXD1 -> TXD1 */
	sg_set_pinsel(90, 1, 8, 4);	/* PC0WE -> TXD2 */
	sg_set_pinsel(94, 1, 8, 4);	/* PCD00 -> TXD3 */

	tmp = readl(SC_CLKCTRL4);
	tmp |= SC_CLKCTRL4_PERI;
	writel(tmp, SC_CLKCTRL4);

	return DIV_ROUND_CLOSEST(UNIPHIER_LD20_UART_CLK, 16 * CONFIG_BAUDRATE);
}
