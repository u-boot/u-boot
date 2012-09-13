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
#include <serial.h>
#include <linux/compiler.h>

#include "ns16550.h"

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_CONS_INDEX == 1
static struct NS16550 *console =
		(struct NS16550 *) (CONFIG_SYS_EUMB_ADDR + 0x4500);
#elif CONFIG_CONS_INDEX == 2
static struct NS16550 *console =
		(struct NS16550 *) (CONFIG_SYS_EUMB_ADDR + 0x4500);
#else
#error no valid console defined
#endif

extern ulong get_bus_freq (ulong);

static int bmw_serial_init(void)
{
	int clock_divisor = gd->bus_clk / 16 / gd->baudrate;

	NS16550_init (CONFIG_CONS_INDEX - 1, clock_divisor);

	return (0);
}

static void bmw_serial_putc(const char c)
{
	if (c == '\n') {
		serial_putc ('\r');
	}
	NS16550_putc (console, c);
}

static void bmw_serial_puts(const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}


static int bmw_serial_getc(void)
{
	return NS16550_getc (console);
}

static int bmw_serial_tstc(void)
{
	return NS16550_tstc (console);
}

static void bmw_serial_setbrg(void)
{
	int clock_divisor = get_bus_freq (0) / 16 / gd->baudrate;

	NS16550_reinit (console, clock_divisor);
}

#ifdef CONFIG_SERIAL_MULTI
static struct serial_device bmw_serial_drv = {
	.name	= "bmw_serial",
	.start	= bmw_serial_init,
	.stop	= NULL,
	.setbrg	= bmw_serial_setbrg,
	.putc	= bmw_serial_putc,
	.puts	= bmw_serial_puts,
	.getc	= bmw_serial_getc,
	.tstc	= bmw_serial_tstc,
};

void bmw_serial_initialize(void)
{
	serial_register(&bmw_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &bmw_serial_drv;
}
#else
int serial_init(void)
{
	return bmw_serial_init();
}

void serial_setbrg(void)
{
	bmw_serial_setbrg();
}

void serial_putc(const char c)
{
	bmw_serial_putc(c);
}

void serial_puts(const char *s)
{
	bmw_serial_puts(s);
}

int serial_getc(void)
{
	return bmw_serial_getc();
}

int serial_tstc(void)
{
	return bmw_serial_tstc();
}
#endif
