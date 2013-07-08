/*
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
 *
 * modified for marvell db64360 eval board by
 * Ingo Assmus <ingo.assmus@keymile.com>
 *
 * modified for cpci750 board by
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * serial.c - serial support for esd cpci750 board
 */

/* supports the MPSC */

#include <common.h>
#include <command.h>
#include <serial.h>
#include <linux/compiler.h>

#include "../../Marvell/include/memory.h"
#include "serial.h"

#include "mpsc.h"

DECLARE_GLOBAL_DATA_PTR;

static int p3mx_serial_init(void)
{
	mpsc_init (gd->baudrate);

	return (0);
}

static void p3mx_serial_putc(const char c)
{
	if (c == '\n')
		mpsc_putchar ('\r');

	mpsc_putchar (c);
}

static int p3mx_serial_getc(void)
{
	return mpsc_getchar ();
}

static int p3mx_serial_tstc(void)
{
	return mpsc_test_char ();
}

static void p3mx_serial_setbrg(void)
{
	galbrg_set_baudrate (CONFIG_MPSC_PORT, gd->baudrate);
}

static struct serial_device p3mx_serial_drv = {
	.name	= "p3mx_serial",
	.start	= p3mx_serial_init,
	.stop	= NULL,
	.setbrg	= p3mx_serial_setbrg,
	.putc	= p3mx_serial_putc,
	.puts	= default_serial_puts,
	.getc	= p3mx_serial_getc,
	.tstc	= p3mx_serial_tstc,
};

void p3mx_serial_initialize(void)
{
	serial_register(&p3mx_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &p3mx_serial_drv;
}

#if defined(CONFIG_CMD_KGDB)
void kgdb_serial_init (void)
{
}

void putDebugChar (int c)
{
	serial_putc (c);
}

void putDebugStr (const char *str)
{
	serial_puts (str);
}

int getDebugChar (void)
{
	return serial_getc ();
}

void kgdb_interruptible (int yes)
{
	return;
}
#endif
