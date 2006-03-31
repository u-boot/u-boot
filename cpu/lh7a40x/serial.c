/*
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <common.h>
#include <lh7a40x.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_CONSOLE_UART1)
# define UART_CONSOLE 1
#elif defined(CONFIG_CONSOLE_UART2)
# define UART_CONSOLE 2
#elif defined(CONFIG_CONSOLE_UART3)
# define UART_CONSOLE 3
#else
# error "No console configured ... "
#endif

void serial_setbrg (void)
{
	lh7a40x_uart_t* uart = LH7A40X_UART_PTR(UART_CONSOLE);
	int i;
	unsigned int reg = 0;

	/*
	 * userguide 15.1.2.4
	 *
	 * BAUDDIV is (UART_REF_FREQ/(16 X BAUD))-1
	 *
	 *   UART_REF_FREQ = external system clock input / 2 (Hz)
	 *   BAUD is desired baudrate (bits/s)
	 *
	 *   NOTE: we add (divisor/2) to numerator to round for
	 *         more precision
	 */
	reg = (((get_PLLCLK()/2) + ((16*gd->baudrate)/2)) / (16 * gd->baudrate)) - 1;
	uart->brcon = reg;

	for (i = 0; i < 100; i++);
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 *
 */
int serial_init (void)
{
	lh7a40x_uart_t* uart = LH7A40X_UART_PTR(UART_CONSOLE);

	/* UART must be enabled before writing to any config registers */
	uart->con |= (UART_EN);

#ifdef CONFIG_CONSOLE_UART1
	/* infrared disabled */
	uart->con |= UART_SIRD;
#endif
	/* loopback disabled */
	uart->con &= ~(UART_LBE);

	/* modem lines and tx/rx polarities */
	uart->con &= ~(UART_MXP | UART_TXP | UART_RXP);

	/* FIFO enable, N81 */
	uart->fcon = (UART_WLEN_8 | UART_FEN | UART_STP2_1);

	/* set baudrate */
	serial_setbrg ();

	/* enable rx interrupt */
	uart->inten |= UART_RI;

	return (0);
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int serial_getc (void)
{
	lh7a40x_uart_t* uart = LH7A40X_UART_PTR(UART_CONSOLE);

	/* wait for character to arrive */
	while (uart->status & UART_RXFE);

	return(uart->data & 0xff);
}

#ifdef CONFIG_HWFLOW
static int hwflow = 0; /* turned off by default */
int hwflow_onoff(int on)
{
	switch(on) {
	case 0:
	default:
		break; /* return current */
	case 1:
		hwflow = 1; /* turn on */
		break;
	case -1:
		hwflow = 0; /* turn off */
		break;
	}
	return hwflow;
}
#endif

#ifdef CONFIG_MODEM_SUPPORT
static int be_quiet = 0;
void disable_putc(void)
{
	be_quiet = 1;
}

void enable_putc(void)
{
	be_quiet = 0;
}
#endif


/*
 * Output a single byte to the serial port.
 */
void serial_putc (const char c)
{
	lh7a40x_uart_t* uart = LH7A40X_UART_PTR(UART_CONSOLE);

#ifdef CONFIG_MODEM_SUPPORT
	if (be_quiet)
		return;
#endif

	/* wait for room in the tx FIFO */
	while (!(uart->status & UART_TXFE));

#ifdef CONFIG_HWFLOW
	/* Wait for CTS up */
	while(hwflow && !(uart->status & UART_CTS));
#endif

	uart->data = c;

	/* If \n, also do \r */
	if (c == '\n')
		serial_putc ('\r');
}

/*
 * Test whether a character is in the RX buffer
 */
int serial_tstc (void)
{
	lh7a40x_uart_t* uart = LH7A40X_UART_PTR(UART_CONSOLE);

	return(!(uart->status & UART_RXFE));
}

void
serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}
