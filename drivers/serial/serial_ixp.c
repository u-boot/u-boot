/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <common.h>
#include <asm/arch/ixp425.h>
#include <watchdog.h>
#include <serial.h>
#include <linux/compiler.h>

/*
 *               14.7456 MHz
 * Baud Rate = --------------
 *              16 x Divisor
 */
#define SERIAL_CLOCK 921600

DECLARE_GLOBAL_DATA_PTR;

static void ixp_serial_setbrg(void)
{
	unsigned int quot = 0;
	int uart = CONFIG_SYS_IXP425_CONSOLE;

	if ((gd->baudrate <= SERIAL_CLOCK) && (SERIAL_CLOCK % gd->baudrate == 0))
		quot = SERIAL_CLOCK / gd->baudrate;
	else
		hang ();

	IER(uart) = 0;					/* Disable for now */
	FCR(uart) = 0;					/* No fifos enabled */

	/* set baud rate */
	LCR(uart) = LCR_WLS0 | LCR_WLS1 | LCR_DLAB;
	DLL(uart) = quot & 0xff;
	DLH(uart) = quot >> 8;
	LCR(uart) = LCR_WLS0 | LCR_WLS1;
#ifdef CONFIG_SERIAL_RTS_ACTIVE
	MCR(uart) = MCR_RTS;				/* set RTS active */
#else
	MCR(uart) = 0;					/* set RTS inactive */
#endif
	IER(uart) = IER_UUE;
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 *
 */
static int ixp_serial_init(void)
{
	serial_setbrg ();

	return (0);
}


/*
 * Output a single byte to the serial port.
 */
static void ixp_serial_putc(const char c)
{
	/* wait for room in the tx FIFO on UART */
	while ((LSR(CONFIG_SYS_IXP425_CONSOLE) & LSR_TEMT) == 0)
		WATCHDOG_RESET();	/* Reset HW Watchdog, if needed */

	THR(CONFIG_SYS_IXP425_CONSOLE) = c;

	/* If \n, also do \r */
	if (c == '\n')
		serial_putc ('\r');
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
static int ixp_serial_tstc(void)
{
	return LSR(CONFIG_SYS_IXP425_CONSOLE) & LSR_DR;
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
static int ixp_serial_getc(void)
{
	while (!(LSR(CONFIG_SYS_IXP425_CONSOLE) & LSR_DR))
		WATCHDOG_RESET();	/* Reset HW Watchdog, if needed */

	return (char) RBR(CONFIG_SYS_IXP425_CONSOLE) & 0xff;
}

static struct serial_device ixp_serial_drv = {
	.name	= "ixp_serial",
	.start	= ixp_serial_init,
	.stop	= NULL,
	.setbrg	= ixp_serial_setbrg,
	.putc	= ixp_serial_putc,
	.puts	= default_serial_puts,
	.getc	= ixp_serial_getc,
	.tstc	= ixp_serial_tstc,
};

void ixp_serial_initialize(void)
{
	serial_register(&ixp_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &ixp_serial_drv;
}
