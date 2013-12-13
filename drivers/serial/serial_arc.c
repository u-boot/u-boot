/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <common.h>
#include <serial.h>

DECLARE_GLOBAL_DATA_PTR;

struct arc_serial_regs {
	unsigned int id0;
	unsigned int id1;
	unsigned int id2;
	unsigned int id3;
	unsigned int data;
	unsigned int status;
	unsigned int baudl;
	unsigned int baudh;
};

/* Bit definitions of STATUS register */
#define UART_RXEMPTY		(1 << 5)
#define UART_OVERFLOW_ERR	(1 << 1)
#define UART_TXEMPTY		(1 << 7)

struct arc_serial_regs *regs;

static void arc_serial_setbrg(void)
{
	int arc_console_baud;

	if (!gd->baudrate)
		gd->baudrate = CONFIG_BAUDRATE;

	arc_console_baud = gd->cpu_clk / (gd->baudrate * 4) - 1;
	writel(arc_console_baud & 0xff, &regs->baudl);
	writel((arc_console_baud & 0xff00) >> 8, &regs->baudh);
}

static int arc_serial_init(void)
{
	regs = (struct arc_serial_regs *)CONFIG_ARC_UART_BASE;
	serial_setbrg();
	return 0;
}

static void arc_serial_putc(const char c)
{
	if (c == '\n')
		arc_serial_putc('\r');

	while (!(readl(&regs->status) & UART_TXEMPTY))
		;

	writel(c, &regs->data);
}

static int arc_serial_tstc(void)
{
	return !(readl(&regs->status) & UART_RXEMPTY);
}

static int arc_serial_getc(void)
{
	while (!arc_serial_tstc())
		;

	/* Check for overflow errors */
	if (readl(&regs->status) & UART_OVERFLOW_ERR)
		return 0;

	return readl(&regs->data) & 0xFF;
}

static void arc_serial_puts(const char *s)
{
	while (*s)
		arc_serial_putc(*s++);
}

static struct serial_device arc_serial_drv = {
	.name	= "arc_serial",
	.start	= arc_serial_init,
	.stop	= NULL,
	.setbrg	= arc_serial_setbrg,
	.putc	= arc_serial_putc,
	.puts	= arc_serial_puts,
	.getc	= arc_serial_getc,
	.tstc	= arc_serial_tstc,
};

void arc_serial_initialize(void)
{
	serial_register(&arc_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &arc_serial_drv;
}
