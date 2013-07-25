/*
 * (C) Copyright 2007 Semihalf
 *
 * Written by: Rafal Jaworowski <raj@semihalf.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * This is is a set of wrappers/stubs that allow to use certain routines from
 * U-Boot's lib in the standalone app. This way way we can re-use
 * existing code e.g. operations on strings and similar.
 */

#include <common.h>
#include <linux/types.h>
#include <api_public.h>

#include "glue.h"

/*
 * printf() and vprintf() are stolen from u-boot/common/console.c
 */
int printf (const char *fmt, ...)
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
	return i;
}

int vprintf (const char *fmt, va_list args)
{
	uint i;
	char printbuffer[256];

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vsprintf (printbuffer, fmt, args);

	/* Print the string */
	ub_puts (printbuffer);
	return i;
}

void putc (const char c)
{
	ub_putc(c);
}

void __udelay(unsigned long usec)
{
	ub_udelay(usec);
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ub_reset();
	return 0;
}

void *malloc (size_t len)
{
	return NULL;
}

void hang (void)
{
	while (1) ;
}
