/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#include <common.h>
#include <watchdog.h>
#include <asm/io.h>
#include <nios2-io.h>
#include <linux/compiler.h>
#include <serial.h>

DECLARE_GLOBAL_DATA_PTR;

/*------------------------------------------------------------------
 * UART the serial port
 *-----------------------------------------------------------------*/

static nios_uart_t *uart = (nios_uart_t *) CONFIG_SYS_NIOS_CONSOLE;

#if defined(CONFIG_SYS_NIOS_FIXEDBAUD)

/*
 * Everything's already setup for fixed-baud PTF
 * assignment
 */
static void altera_serial_setbrg(void)
{
}

static int altera_serial_init(void)
{
	return 0;
}

#else

static void altera_serial_setbrg(void)
{
	unsigned div;

	div = (CONFIG_SYS_CLK_FREQ/gd->baudrate)-1;
	writel (div, &uart->divisor);
}

static int altera_serial_init(void)
{
	serial_setbrg();
	return 0;
}

#endif /* CONFIG_SYS_NIOS_FIXEDBAUD */

/*-----------------------------------------------------------------------
 * UART CONSOLE
 *---------------------------------------------------------------------*/
static void altera_serial_putc(char c)
{
	if (c == '\n')
		serial_putc ('\r');
	while ((readl (&uart->status) & NIOS_UART_TRDY) == 0)
		WATCHDOG_RESET ();
	writel ((unsigned char)c, &uart->txdata);
}

static int altera_serial_tstc(void)
{
	return (readl (&uart->status) & NIOS_UART_RRDY);
}

static int altera_serial_getc(void)
{
	while (serial_tstc () == 0)
		WATCHDOG_RESET ();
	return (readl (&uart->rxdata) & 0x00ff );
}

static struct serial_device altera_serial_drv = {
	.name	= "altera_serial",
	.start	= altera_serial_init,
	.stop	= NULL,
	.setbrg	= altera_serial_setbrg,
	.putc	= altera_serial_putc,
	.puts	= default_serial_puts,
	.getc	= altera_serial_getc,
	.tstc	= altera_serial_tstc,
};

void altera_serial_initialize(void)
{
	serial_register(&altera_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &altera_serial_drv;
}
