/*
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
 *
 * modified for marvell db64360 eval board by
 * Ingo Assmus <ingo.assmus@keymile.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * serial.c - serial support for the gal ev board
 */

/* supports both the 16650 duart and the MPSC */

#include <common.h>
#include <command.h>
#include <serial.h>
#include <linux/compiler.h>

#include "../include/memory.h"

#include "ns16550.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_MPSC
static int marvell_serial_init(void)
{
#if (defined CONFIG_SYS_INIT_CHAN1) || (defined CONFIG_SYS_INIT_CHAN2)
	int clock_divisor = 230400 / gd->baudrate;
#endif

	mpsc_init (gd->baudrate);

	/* init the DUART chans so that KGDB in the kernel can use them */
#ifdef CONFIG_SYS_INIT_CHAN1
	NS16550_reinit (COM_PORTS[0], clock_divisor);
#endif
#ifdef CONFIG_SYS_INIT_CHAN2
	NS16550_reinit (COM_PORTS[1], clock_divisor);
#endif
	return (0);
}

static void marvell_serial_putc(const char c)
{
	if (c == '\n')
		mpsc_putchar ('\r');

	mpsc_putchar (c);
}

static int marvell_serial_getc(void)
{
	return mpsc_getchar ();
}

static int marvell_serial_tstc(void)
{
	return mpsc_test_char ();
}

static void marvell_serial_setbrg(void)
{
	galbrg_set_baudrate (CONFIG_MPSC_PORT, gd->baudrate);
}

#else  /* ! CONFIG_MPSC */

static int marvell_serial_init(void)
{
	int clock_divisor = 230400 / gd->baudrate;

#ifdef CONFIG_SYS_INIT_CHAN1
	(void) NS16550_init (0, clock_divisor);
#endif
#ifdef CONFIG_SYS_INIT_CHAN2
	(void) NS16550_init (1, clock_divisor);
#endif
	return (0);
}

static void marvell_serial_putc(const char c)
{
	if (c == '\n')
		NS16550_putc (COM_PORTS[CONFIG_SYS_DUART_CHAN], '\r');

	NS16550_putc (COM_PORTS[CONFIG_SYS_DUART_CHAN], c);
}

static int marvell_serial_getc(void)
{
	return NS16550_getc (COM_PORTS[CONFIG_SYS_DUART_CHAN]);
}

static int marvell_serial_tstc(void)
{
	return NS16550_tstc (COM_PORTS[CONFIG_SYS_DUART_CHAN]);
}

static void marvell_serial_setbrg(void)
{
	int clock_divisor = 230400 / gd->baudrate;

#ifdef CONFIG_SYS_INIT_CHAN1
	NS16550_reinit (COM_PORTS[0], clock_divisor);
#endif
#ifdef CONFIG_SYS_INIT_CHAN2
	NS16550_reinit (COM_PORTS[1], clock_divisor);
#endif
}

#endif /* CONFIG_MPSC */

static struct serial_device marvell_serial_drv = {
	.name	= "marvell_serial",
	.start	= marvell_serial_init,
	.stop	= NULL,
	.setbrg	= marvell_serial_setbrg,
	.putc	= marvell_serial_putc,
	.puts	= default_serial_puts,
	.getc	= marvell_serial_getc,
	.tstc	= marvell_serial_tstc,
};

void marvell_serial_initialize(void)
{
	serial_register(&marvell_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &marvell_serial_drv;
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
