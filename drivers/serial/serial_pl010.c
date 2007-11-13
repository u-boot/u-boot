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

/* Simple U-Boot driver for the PrimeCell PL011 UARTs on the IntegratorCP */
/* Should be fairly simple to make it work with the PL010 as well */

#include <common.h>

#ifdef CFG_PL010_SERIAL

#include "serial_pl011.h"

#define IO_WRITE(addr, val) (*(volatile unsigned int *)(addr) = (val))
#define IO_READ(addr) (*(volatile unsigned int *)(addr))

/* Integrator AP has two UARTs, we use the first one, at 38400-8-N-1 */
#define CONSOLE_PORT CONFIG_CONS_INDEX
#define baudRate CONFIG_BAUDRATE
static volatile unsigned char *const port[] = CONFIG_PL01x_PORTS;
#define NUM_PORTS (sizeof(port)/sizeof(port[0]))


static void pl010_putc (int portnum, char c);
static int pl010_getc (int portnum);
static int pl010_tstc (int portnum);


int serial_init (void)
{
	unsigned int divisor;

	/*
	 ** First, disable everything.
	 */
	IO_WRITE (port[CONSOLE_PORT] + UART_PL010_CR, 0x0);

	/*
	 ** Set baud rate
	 **
	 */
	switch (baudRate) {
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

	IO_WRITE (port[CONSOLE_PORT] + UART_PL010_LCRM,
		  ((divisor & 0xf00) >> 8));
	IO_WRITE (port[CONSOLE_PORT] + UART_PL010_LCRL, (divisor & 0xff));

	/*
	 ** Set the UART to be 8 bits, 1 stop bit, no parity, fifo enabled.
	 */
	IO_WRITE (port[CONSOLE_PORT] + UART_PL010_LCRH,
		  (UART_PL010_LCRH_WLEN_8 | UART_PL010_LCRH_FEN));

	/*
	 ** Finally, enable the UART
	 */
	IO_WRITE (port[CONSOLE_PORT] + UART_PL010_CR, (UART_PL010_CR_UARTEN));

	return (0);
}

void serial_putc (const char c)
{
	if (c == '\n')
		pl010_putc (CONSOLE_PORT, '\r');

	pl010_putc (CONSOLE_PORT, c);
}

void serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}

int serial_getc (void)
{
	return pl010_getc (CONSOLE_PORT);
}

int serial_tstc (void)
{
	return pl010_tstc (CONSOLE_PORT);
}

void serial_setbrg (void)
{
}

static void pl010_putc (int portnum, char c)
{
	/* Wait until there is space in the FIFO */
	while (IO_READ (port[portnum] + UART_PL01x_FR) & UART_PL01x_FR_TXFF);

	/* Send the character */
	IO_WRITE (port[portnum] + UART_PL01x_DR, c);
}

static int pl010_getc (int portnum)
{
	unsigned int data;

	/* Wait until there is data in the FIFO */
	while (IO_READ (port[portnum] + UART_PL01x_FR) & UART_PL01x_FR_RXFE);

	data = IO_READ (port[portnum] + UART_PL01x_DR);

	/* Check for an error flag */
	if (data & 0xFFFFFF00) {
		/* Clear the error */
		IO_WRITE (port[portnum] + UART_PL01x_ECR, 0xFFFFFFFF);
		return -1;
	}

	return (int) data;
}

static int pl010_tstc (int portnum)
{
	return !(IO_READ (port[portnum] + UART_PL01x_FR) &
		 UART_PL01x_FR_RXFE);
}

#endif
