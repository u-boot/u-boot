/*
 * Copyright (C) 2011 Vladimir Zapolskiy <vz@mleia.com>
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

DECLARE_GLOBAL_DATA_PTR;

static struct hsuart_regs *hsuart = (struct hsuart_regs *)HS_UART_BASE;

static void lpc32xx_hsuart_set_baudrate(void)
{
	u32 div;

	/* UART rate = PERIPH_CLK / ((HSU_RATE + 1) x 14) */
	div = (get_serial_clock() / 14 + gd->baudrate / 2) / gd->baudrate - 1;
	if (div > 255)
		div = 255;

	writel(div, &hsuart->rate);
}

static int lpc32xx_hsuart_getc(void)
{
	while (!(readl(&hsuart->level) & HSUART_LEVEL_RX))
		/* NOP */;

	return readl(&hsuart->rx) & HSUART_RX_DATA;
}

static void lpc32xx_hsuart_putc(const char c)
{
	writel(c, &hsuart->tx);

	/* Wait for character to be sent */
	while (readl(&hsuart->level) & HSUART_LEVEL_TX)
		/* NOP */;
}

static int lpc32xx_hsuart_tstc(void)
{
	if (readl(&hsuart->level) & HSUART_LEVEL_RX)
		return 1;

	return 0;
}

static void lpc32xx_hsuart_init(void)
{
	lpc32xx_hsuart_set_baudrate();

	/* Disable hardware RTS and CTS flow control, set up RX and TX FIFO */
	writel(HSUART_CTRL_TMO_16 | HSUART_CTRL_HSU_OFFSET(20) |
	       HSUART_CTRL_HSU_RX_TRIG_32 | HSUART_CTRL_HSU_TX_TRIG_0,
	       &hsuart->ctrl);
}

void serial_setbrg(void)
{
	return lpc32xx_hsuart_set_baudrate();
}

void serial_putc(const char c)
{
	lpc32xx_hsuart_putc(c);

	/* If \n, also do \r */
	if (c == '\n')
		lpc32xx_hsuart_putc('\r');
}

int serial_getc(void)
{
	return lpc32xx_hsuart_getc();
}

void serial_puts(const char *s)
{
	while (*s)
		serial_putc(*s++);
}

int serial_tstc(void)
{
	return lpc32xx_hsuart_tstc();
}

int serial_init(void)
{
	lpc32xx_hsuart_init();

	return 0;
}
