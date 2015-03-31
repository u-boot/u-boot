/*
 * Copyright (C) 2011 by Vladimir Zapolskiy <vz@mleia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

void lpc32xx_mac_init(void)
{
	/* Enable MAC interface */
	writel(CLK_MAC_REG | CLK_MAC_SLAVE | CLK_MAC_MASTER
		| CLK_MAC_MII, &clk->macclk_ctrl);
}

void lpc32xx_mlc_nand_init(void)
{
	/* Enable NAND interface */
	writel(CLK_NAND_MLC | CLK_NAND_MLC_INT, &clk->flashclk_ctrl);
}

void lpc32xx_i2c_init(unsigned int devnum)
{
	/* Enable I2C interface */
	uint32_t ctrl = readl(&clk->i2cclk_ctrl);
	if (devnum == 1)
		ctrl |= CLK_I2C1_ENABLE;
	if (devnum == 2)
		ctrl |= CLK_I2C2_ENABLE;
	writel(ctrl, &clk->i2cclk_ctrl);
}
