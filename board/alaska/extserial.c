/*
 * (C) Copyright 2004, Freescale, Inc
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
 * Minimal serial functions needed to use one of the PSC ports
 * as serial console interface.
 */

#include <common.h>
#include <mpc8220.h>

#if defined (CONFIG_EXTUART_CONSOLE)
#   include <ns16550.h>

#   define PADSERIAL_BAUD_115200   0x40
#   define PADSERIAL_BAUD_57600    0x20
#   define PADSERIAL_BAUD_9600     0
#   define PADCARD_FREQ            18432000

const NS16550_t com_port = (NS16550_t) CFG_NS16550_COM1;

int ext_serial_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	volatile u8 *dipswitch = (volatile u8 *) (CFG_CPLD_BASE + 0x1002);
	int baud_divisor;

	/* Find out the baud rate speed on debug card dip switches */
	if (*dipswitch & PADSERIAL_BAUD_115200)
		gd->baudrate = 115200;
	else if (*dipswitch & PADSERIAL_BAUD_57600)
		gd->baudrate = 57600;
	else
		gd->baudrate = 9600;

	/* Debug card frequency */
	baud_divisor = PADCARD_FREQ / (16 * gd->baudrate);

	NS16550_init (com_port, baud_divisor);

	return (0);
}

void ext_serial_putc (const char c)
{
	if (c == '\n')
		NS16550_putc (com_port, '\r');

	NS16550_putc (com_port, c);
}

void ext_serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}

int ext_serial_getc (void)
{
	return NS16550_getc (com_port);
}

int ext_serial_tstc (void)
{
	return NS16550_tstc (com_port);
}

void ext_serial_setbrg (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	volatile u8 *dipswitch = (volatile u8 *) (CFG_CPLD_BASE + 0x1002);
	int baud_divisor;

	/* Find out the baud rate speed on debug card dip switches */
	if (*dipswitch & PADSERIAL_BAUD_115200)
		gd->baudrate = 115200;
	else if (*dipswitch & PADSERIAL_BAUD_57600)
		gd->baudrate = 57600;
	else
		gd->baudrate = 9600;

	/* Debug card frequency */
	baud_divisor = PADCARD_FREQ / (16 * gd->baudrate);

	NS16550_reinit (com_port, baud_divisor);
}
#endif /* CONFIG_EXTUART_CONSOLE */
