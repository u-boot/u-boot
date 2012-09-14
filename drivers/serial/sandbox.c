/*
 * Copyright (c) 2011 The Chromium OS Authors.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * This provide a test serial port. It provides an emulated serial port where
 * a test program and read out the serial output and inject serial input for
 * U-Boot.
 */

#include <common.h>
#include <os.h>
#include <serial.h>
#include <linux/compiler.h>

static int sandbox_serial_init(void)
{
	os_tty_raw(0);
	return 0;
}

static void sandbox_serial_setbrg(void)
{
}

static void sandbox_serial_putc(const char ch)
{
	os_write(1, &ch, 1);
}

static void sandbox_serial_puts(const char *str)
{
	os_write(1, str, strlen(str));
}

static int sandbox_serial_getc(void)
{
	char buf;
	ssize_t count;

	count = os_read(0, &buf, 1);
	return count == 1 ? buf : 0;
}

static int sandbox_serial_tstc(void)
{
	return 0;
}

static struct serial_device sandbox_serial_drv = {
	.name	= "sandbox_serial",
	.start	= sandbox_serial_init,
	.stop	= NULL,
	.setbrg	= sandbox_serial_setbrg,
	.putc	= sandbox_serial_putc,
	.puts	= sandbox_serial_puts,
	.getc	= sandbox_serial_getc,
	.tstc	= sandbox_serial_tstc,
};

void sandbox_serial_initialize(void)
{
	serial_register(&sandbox_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &sandbox_serial_drv;
}
