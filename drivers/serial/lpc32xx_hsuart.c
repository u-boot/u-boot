/*
 * Copyright (C) 2011 Vladimir Zapolskiy <vz@mleia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clk.h>
#include <asm/arch/uart.h>
#include <asm/io.h>
#include <serial.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

static struct hsuart_regs *hsuart = (struct hsuart_regs *)HS_UART_BASE;

static void lpc32xx_serial_setbrg(void)
{
	u32 div;

	/* UART rate = PERIPH_CLK / ((HSU_RATE + 1) x 14) */
	div = (get_serial_clock() / 14 + gd->baudrate / 2) / gd->baudrate - 1;
	if (div > 255)
		div = 255;

	writel(div, &hsuart->rate);
}

static int lpc32xx_serial_getc(void)
{
	while (!(readl(&hsuart->level) & HSUART_LEVEL_RX))
		/* NOP */;

	return readl(&hsuart->rx) & HSUART_RX_DATA;
}

static void lpc32xx_serial_putc(const char c)
{
	writel(c, &hsuart->tx);

	/* Wait for character to be sent */
	while (readl(&hsuart->level) & HSUART_LEVEL_TX)
		/* NOP */;
}

static int lpc32xx_serial_tstc(void)
{
	if (readl(&hsuart->level) & HSUART_LEVEL_RX)
		return 1;

	return 0;
}

static int lpc32xx_serial_init(void)
{
	lpc32xx_serial_setbrg();

	/* Disable hardware RTS and CTS flow control, set up RX and TX FIFO */
	writel(HSUART_CTRL_TMO_16 | HSUART_CTRL_HSU_OFFSET(20) |
	       HSUART_CTRL_HSU_RX_TRIG_32 | HSUART_CTRL_HSU_TX_TRIG_0,
	       &hsuart->ctrl);
	return 0;
}

static struct serial_device lpc32xx_serial_drv = {
	.name	= "lpc32xx_serial",
	.start	= lpc32xx_serial_init,
	.stop	= NULL,
	.setbrg	= lpc32xx_serial_setbrg,
	.putc	= lpc32xx_serial_putc,
	.puts	= default_serial_puts,
	.getc	= lpc32xx_serial_getc,
	.tstc	= lpc32xx_serial_tstc,
};

void lpc32xx_serial_initialize(void)
{
	serial_register(&lpc32xx_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &lpc32xx_serial_drv;
}
