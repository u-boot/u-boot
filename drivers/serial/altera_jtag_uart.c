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
 * JTAG acts as the serial port
 *-----------------------------------------------------------------*/
static nios_jtag_t *jtag = (nios_jtag_t *)CONFIG_SYS_NIOS_CONSOLE;

static void altera_jtag_serial_setbrg(void)
{
}

static int altera_jtag_serial_init(void)
{
	return 0;
}

static void altera_jtag_serial_putc(char c)
{
	while (1) {
		unsigned st = readl(&jtag->control);
		if (NIOS_JTAG_WSPACE(st))
			break;
#ifdef CONFIG_ALTERA_JTAG_UART_BYPASS
		if (!(st & NIOS_JTAG_AC)) /* no connection */
			return;
#endif
		WATCHDOG_RESET();
	}
	writel ((unsigned char)c, &jtag->data);
}

static int altera_jtag_serial_tstc(void)
{
	return ( readl (&jtag->control) & NIOS_JTAG_RRDY);
}

static int altera_jtag_serial_getc(void)
{
	int c;
	unsigned val;

	while (1) {
		WATCHDOG_RESET ();
		val = readl (&jtag->data);
		if (val & NIOS_JTAG_RVALID)
			break;
	}
	c = val & 0x0ff;
	return (c);
}

static struct serial_device altera_jtag_serial_drv = {
	.name	= "altera_jtag_uart",
	.start	= altera_jtag_serial_init,
	.stop	= NULL,
	.setbrg	= altera_jtag_serial_setbrg,
	.putc	= altera_jtag_serial_putc,
	.puts	= default_serial_puts,
	.getc	= altera_jtag_serial_getc,
	.tstc	= altera_jtag_serial_tstc,
};

void altera_jtag_serial_initialize(void)
{
	serial_register(&altera_jtag_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &altera_jtag_serial_drv;
}
