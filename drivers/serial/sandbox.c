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

int serial_init(void)
{
	os_tty_raw(0);
	return 0;
}

void serial_setbrg(void)
{
}

void serial_putc(const char ch)
{
	os_write(1, &ch, 1);
}

void serial_puts(const char *str)
{
	os_write(1, str, strlen(str));
}

int serial_getc(void)
{
	char buf;
	ssize_t count;

	count = os_read(0, &buf, 1);
	return count == 1 ? buf : 0;
}

int serial_tstc(void)
{
	return 0;
}
