/*
 * (C) Copyright 2000
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
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
 */

/* Simple U-Boot driver for the PrimeCell PL010/PL011 UARTs */

#include <common.h>
#include <watchdog.h>
#include <asm/io.h>
#include "serial_pl01x.h"

/*
 * Integrator AP has two UARTs, we use the first one, at 38400-8-N-1
 * Integrator CP has two UARTs, use the first one, at 38400-8-N-1
 * Versatile PB has four UARTs.
 */
#define CONSOLE_PORT CONFIG_CONS_INDEX
static volatile unsigned char *const port[] = CONFIG_PL01x_PORTS;
#define NUM_PORTS (sizeof(port)/sizeof(port[0]))

static void pl01x_putc (int portnum, char c);
static int pl01x_getc (int portnum);
static int pl01x_tstc (int portnum);
unsigned int baudrate = CONFIG_BAUDRATE;
DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_PL010_SERIAL

int serial_init (void)
{
	unsigned int divisor;

	/* First, disable everything */
	writel(0x0, port[CONSOLE_PORT] + UART_PL010_CR);

	/* Set baud rate */
	switch (baudrate) {
	case 9600:
		divisor = UART_PL010_BAUD_9600;
		break;

	case 19200:
		divisor = UART_PL010_BAUD_9600;
		break;

	case 38400:
		divisor = UART_PL010_BAUD_38400;
		break;

	case 57600:
		divisor = UART_PL010_BAUD_57600;
		break;

	case 115200:
		divisor = UART_PL010_BAUD_115200;
		break;

	default:
		divisor = UART_PL010_BAUD_38400;
	}

	writel(((divisor & 0xf00) >> 8), port[CONSOLE_PORT] + UART_PL010_LCRM);
	writel((divisor & 0xff), port[CONSOLE_PORT] + UART_PL010_LCRL);

	/* Set the UART to be 8 bits, 1 stop bit, no parity, fifo enabled */
	writel((UART_PL010_LCRH_WLEN_8 | UART_PL010_LCRH_FEN),
		port[CONSOLE_PORT] + UART_PL010_LCRH);

	/* Finally, enable the UART */
	writel((UART_PL010_CR_UARTEN), port[CONSOLE_PORT] + UART_PL010_CR);

	return 0;
}

#endif /* CONFIG_PL010_SERIAL */

#ifdef CONFIG_PL011_SERIAL

int serial_init (void)
{
	unsigned int temp;
	unsigned int divider;
	unsigned int remainder;
	unsigned int fraction;

	/* First, disable everything */
	writel(0x0, port[CONSOLE_PORT] + UART_PL011_CR);

	/*
	 * Set baud rate
	 *
	 * IBRD = UART_CLK / (16 * BAUD_RATE)
	 * FBRD = RND((64 * MOD(UART_CLK,(16 * BAUD_RATE))) / (16 * BAUD_RATE))
	 */
	temp = 16 * baudrate;
	divider = CONFIG_PL011_CLOCK / temp;
	remainder = CONFIG_PL011_CLOCK % temp;
	temp = (8 * remainder) / baudrate;
	fraction = (temp >> 1) + (temp & 1);

	writel(divider, port[CONSOLE_PORT] + UART_PL011_IBRD);
	writel(fraction, port[CONSOLE_PORT] + UART_PL011_FBRD);

	/* Set the UART to be 8 bits, 1 stop bit, no parity, fifo enabled */
	writel((UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_FEN),
		port[CONSOLE_PORT] + UART_PL011_LCRH);

	/* Finally, enable the UART */
	writel((UART_PL011_CR_UARTEN | UART_PL011_CR_TXE | UART_PL011_CR_RXE),
		port[CONSOLE_PORT] + UART_PL011_CR);

	return 0;
}

#endif /* CONFIG_PL011_SERIAL */

void serial_putc (const char c)
{
	if (c == '\n')
		pl01x_putc (CONSOLE_PORT, '\r');

	pl01x_putc (CONSOLE_PORT, c);
}

void serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}

int serial_getc (void)
{
	return pl01x_getc (CONSOLE_PORT);
}

int serial_tstc (void)
{
	return pl01x_tstc (CONSOLE_PORT);
}

void serial_setbrg (void)
{
	baudrate = gd->baudrate;
	serial_init();
}

static void pl01x_putc (int portnum, char c)
{
	/* Wait until there is space in the FIFO */
	while (readl(port[portnum] + UART_PL01x_FR) & UART_PL01x_FR_TXFF)
		WATCHDOG_RESET();

	/* Send the character */
	writel(c, port[portnum] + UART_PL01x_DR);
}

static int pl01x_getc (int portnum)
{
	unsigned int data;

	/* Wait until there is data in the FIFO */
	while (readl(port[portnum] + UART_PL01x_FR) & UART_PL01x_FR_RXFE)
		WATCHDOG_RESET();

	data = readl(port[portnum] + UART_PL01x_DR);

	/* Check for an error flag */
	if (data & 0xFFFFFF00) {
		/* Clear the error */
		writel(0xFFFFFFFF, port[portnum] + UART_PL01x_ECR);
		return -1;
	}

	return (int) data;
}

static int pl01x_tstc (int portnum)
{
	WATCHDOG_RESET();
	return !(readl(port[portnum] + UART_PL01x_FR) &
		 UART_PL01x_FR_RXFE);
}
