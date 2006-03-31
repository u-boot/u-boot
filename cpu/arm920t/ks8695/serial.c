/*
 * serial.c -- KS8695 serial driver
 *
 * (C) Copyright 2004, Greg Ungerer <greg.ungerer@opengear.com>
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
 */

#include <common.h>
#include <asm/arch/platform.h>

#ifndef CONFIG_SERIAL1
#error "Bad: you didn't configure serial ..."
#endif

DECLARE_GLOBAL_DATA_PTR;

/*
 *	Define the UART hardware register access structure.
 */
struct ks8695uart {
	unsigned int	RX;		/* 0x00	- Receive data (r) */
	unsigned int	TX;		/* 0x04	- Transmit data (w) */
	unsigned int	FCR;		/* 0x08	- Fifo Control (r/w) */
	unsigned int	LCR;		/* 0x0c	- Line Control (r/w) */
	unsigned int	MCR;		/* 0x10	- Modem Control (r/w) */
	unsigned int	LSR;		/* 0x14	- Line Status (r/w) */
	unsigned int	MSR;		/* 0x18	- Modem Status (r/w) */
	unsigned int	BD;		/* 0x1c	- Baud Rate (r/w) */
	unsigned int	SR;		/* 0x20	- Status (r/w) */
};

#define	KS8695_UART_ADDR	((void *) (KS8695_IO_BASE + KS8695_UART_RX_BUFFER))
#define	KS8695_UART_CLK		25000000


/*
 * Under some circumstances we want to be "quiet" and not issue any
 * serial output - though we want u-boot to otherwise work and behave
 * the same. By default be noisy.
 */
int serial_console = 1;


void serial_setbrg(void)
{
	volatile struct ks8695uart *uartp = KS8695_UART_ADDR;

	/* Set to global baud rate and 8 data bits, no parity, 1 stop bit*/
	uartp->BD = KS8695_UART_CLK / gd->baudrate;
	uartp->LCR = KS8695_UART_LINEC_WLEN8;
}

int serial_init(void)
{
	serial_console = 1;
	serial_setbrg();
	return 0;
}

void serial_raw_putc(const char c)
{
	volatile struct ks8695uart *uartp = KS8695_UART_ADDR;
	int i;

	for (i = 0; (i < 0x100000); i++) {
		if (uartp->LSR & KS8695_UART_LINES_TXFE)
			break;
	}

	uartp->TX = c;
}

void serial_putc(const char c)
{
	if (serial_console) {
		serial_raw_putc(c);
		if (c == '\n')
			serial_raw_putc('\r');
	}
}

int serial_tstc(void)
{
	volatile struct ks8695uart *uartp = KS8695_UART_ADDR;
	if (serial_console)
		return ((uartp->LSR & KS8695_UART_LINES_RXFE) ? 1 : 0);
	return 0;
}

void serial_puts(const char *s)
{
	char c;
	while ((c = *s++) != 0)
		serial_putc(c);
}

int serial_getc(void)
{
	volatile struct ks8695uart *uartp = KS8695_UART_ADDR;

	while ((uartp->LSR & KS8695_UART_LINES_RXFE) == 0)
		;
	return (uartp->RX);
}
