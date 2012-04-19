/*
 * Copyright (C) 2011 by Vladimir Zapolskiy <vz@mleia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clk.h>
#include <asm/arch/uart.h>
#include <asm/io.h>

static struct clk_pm_regs    *clk  = (struct clk_pm_regs *)CLK_PM_BASE;
static struct uart_ctrl_regs *ctrl = (struct uart_ctrl_regs *)UART_CTRL_BASE;

void lpc32xx_uart_init(unsigned int uart_id)
{
	if (uart_id < 1 || uart_id > 7)
		return;

	/* Disable loopback mode, if it is set by S1L bootloader */
	clrbits_le32(&ctrl->loop,
		     UART_LOOPBACK(CONFIG_SYS_LPC32XX_UART));

	if (uart_id < 3 || uart_id > 6)
		return;

	/* Enable UART system clock */
	setbits_le32(&clk->uartclk_ctrl, CLK_UART(uart_id));

	/* Set UART into autoclock mode */
	clrsetbits_le32(&ctrl->clkmode,
			UART_CLKMODE_MASK(uart_id),
			UART_CLKMODE_AUTO(uart_id));

	/* Bypass pre-divider of UART clock */
	writel(CLK_UART_X_DIV(1) | CLK_UART_Y_DIV(1),
	       &clk->u3clk + (uart_id - 3));
}
