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
	writeb(arc_console_baud & 0xff, &regs->baudl);

#ifdef CONFIG_ARC
	/*
	 * UART ISS(Instruction Set simulator) emulation has a subtle bug:
	 * A existing value of Baudh = 0 is used as a indication to startup
	 * it's internal state machine.
	 * Thus if baudh is set to 0, 2 times, it chokes.
	 * This happens with BAUD=115200 and the formaula above
	 * Until that is fixed, when running on ISS, we will set baudh to !0
	 */
	if (gd->arch.running_on_hw)
		writeb((arc_console_baud & 0xff00) >> 8, &regs->baudh);
	else
		writeb(1, &regs->baudh);
#else
	writeb((arc_console_baud & 0xff00) >> 8, &regs->baudh);
#endif
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

	while (!(readb(&regs->status) & UART_TXEMPTY))
		;

	writeb(c, &regs->data);
}

static int arc_serial_tstc(void)
{
	return !(readb(&regs->status) & UART_RXEMPTY);
}

static int arc_serial_getc(void)
{
	while (!arc_serial_tstc())
		;

	/* Check for overflow errors */
	if (readb(&regs->status) & UART_OVERFLOW_ERR)
		return 0;

	return readb(&regs->data) & 0xFF;
}

static struct serial_device arc_serial_drv = {
	.name	= "arc_serial",
	.start	= arc_serial_init,
	.stop	= NULL,
	.setbrg	= arc_serial_setbrg,
	.putc	= arc_serial_putc,
	.puts	= default_serial_puts,
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
