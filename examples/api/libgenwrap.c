/*
 * (C) Copyright 2007 Semihalf
 *
 * Written by: Rafal Jaworowski <raj@semihalf.com>
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
 *
 * This is is a set of wrappers/stubs that allow to use certain routines from
 * U-Boot's lib_generic in the standalone app. This way way we can re-use
 * existing code e.g. operations on strings and similar.
 *
 */

#include <common.h>
#include <linux/types.h>
#include <api_public.h>

#include "glue.h"

/*
 * printf() and vprintf() are stolen from u-boot/common/console.c
 */
void printf (const char *fmt, ...)
{
	va_list args;
	uint i;
	char printbuffer[256];

	va_start (args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vsprintf (printbuffer, fmt, args);
	va_end (args);

	/* Print the string */
	ub_puts (printbuffer);
}

void vprintf (const char *fmt, va_list args)
{
	uint i;
	char printbuffer[256];

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vsprintf (printbuffer, fmt, args);

	/* Print the string */
	ub_puts (printbuffer);
}

void putc (const char c)
{
	ub_putc(c);
}

void udelay(unsigned long usec)
{
	ub_udelay(usec);
}

void do_reset (void)
{
	ub_reset();
}

void *malloc (size_t len)
{
	return NULL;
}

void hang (void)
{
	while (1) ;
}
