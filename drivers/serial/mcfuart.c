/*
 * (C) Copyright 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew, Tsi-Chung.Liew@freescale.com.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

/*
 * Minimal serial functions needed to use one of the uart ports
 * as serial console interface.
 */

#include <common.h>

#include <asm/immap.h>
#include <asm/uart.h>

DECLARE_GLOBAL_DATA_PTR;

extern void uart_port_conf(void);

int serial_init(void)
{
	volatile uart_t *uart;
	u32 counter;

	uart = (volatile uart_t *)(CFG_UART_BASE);

	uart_port_conf();

	/* write to SICR: SIM2 = uart mode,dcd does not affect rx */
	uart->ucr = UART_UCR_RESET_RX;
	uart->ucr = UART_UCR_RESET_TX;
	uart->ucr = UART_UCR_RESET_ERROR;
	uart->ucr = UART_UCR_RESET_MR;
	__asm__("nop");

	uart->uimr = 0;

	/* write to CSR: RX/TX baud rate from timers */
	uart->ucsr = (UART_UCSR_RCS_SYS_CLK | UART_UCSR_TCS_SYS_CLK);

	uart->umr = (UART_UMR_BC_8 | UART_UMR_PM_NONE);
	uart->umr = UART_UMR_SB_STOP_BITS_1;

	/* Setting up BaudRate */
	counter = (u32) ((gd->bus_clk / 32) + (gd->baudrate / 2));
	counter = counter / gd->baudrate;

	/* write to CTUR: divide counter upper byte */
	uart->ubg1 = (u8) ((counter & 0xff00) >> 8);
	/* write to CTLR: divide counter lower byte */
	uart->ubg2 = (u8) (counter & 0x00ff);

	uart->ucr = (UART_UCR_RX_ENABLED | UART_UCR_TX_ENABLED);

	return (0);
}

void serial_putc(const char c)
{
	volatile uart_t *uart = (volatile uart_t *)(CFG_UART_BASE);

	if (c == '\n')
		serial_putc('\r');

	/* Wait for last character to go. */
	while (!(uart->usr & UART_USR_TXRDY)) ;

	uart->utb = c;
}

void serial_puts(const char *s)
{
	while (*s) {
		serial_putc(*s++);
	}
}

int serial_getc(void)
{
	volatile uart_t *uart = (volatile uart_t *)(CFG_UART_BASE);

	/* Wait for a character to arrive. */
	while (!(uart->usr & UART_USR_RXRDY)) ;
	return uart->urb;
}

int serial_tstc(void)
{
	volatile uart_t *uart = (volatile uart_t *)(CFG_UART_BASE);

	return (uart->usr & UART_USR_RXRDY);
}

void serial_setbrg(void)
{
	volatile uart_t *uart = (volatile uart_t *)(CFG_UART_BASE);
	u32 counter;

	counter = ((gd->bus_clk / gd->baudrate)) >> 5;
	counter++;

	/* write to CTUR: divide counter upper byte */
	uart->ubg1 = ((counter & 0xff00) >> 8);
	/* write to CTLR: divide counter lower byte */
	uart->ubg2 = (counter & 0x00ff);

	uart->ucr = UART_UCR_RESET_RX;
	uart->ucr = UART_UCR_RESET_TX;

	uart->ucr = UART_UCR_RX_ENABLED | UART_UCR_TX_ENABLED;
}
