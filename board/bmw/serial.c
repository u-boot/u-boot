/*
 * (C) Copyright 2000
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
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

#include <common.h>
#include "ns16550.h"

#if CONFIG_CONS_INDEX == 1
static struct NS16550 *console =
		(struct NS16550 *) (CFG_EUMB_ADDR + 0x4500);
#elif CONFIG_CONS_INDEX == 2
static struct NS16550 *console =
		(struct NS16550 *) (CFG_EUMB_ADDR + 0x4500);
#else
#error no valid console defined
#endif

extern ulong get_bus_freq (ulong);

int serial_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	int clock_divisor = gd->bus_clk / 16 / gd->baudrate;

	NS16550_init (CONFIG_CONS_INDEX - 1, clock_divisor);

	return (0);
}

void serial_putc (const char c)
{
	if (c == '\n') {
		serial_putc ('\r');
	}
	NS16550_putc (console, c);
}

void serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}


int serial_getc (void)
{
	return NS16550_getc (console);
}

int serial_tstc (void)
{
	return NS16550_tstc (console);
}

void serial_setbrg (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	int clock_divisor = get_bus_freq (0) / 16 / gd->baudrate;

	NS16550_reinit (console, clock_divisor);
}
