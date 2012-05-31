/*
 * (C) Copyright 2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Author: Igor Lisitsin <igor@emcraft.com>
 *
 * Copyright 2010, Stefan Roese, DENX Software Engineering, sr@denx.de
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
 */

#include <common.h>
#include <asm/ppc4xx.h>
#include <ns16550.h>
#include <asm/io.h>
#include <serial.h>

/*
 * UART test
 *
 * The controllers are configured to loopback mode and several
 * characters are transmitted.
 */

#include <post.h>

#if CONFIG_POST & CONFIG_SYS_POST_UART

/*
 * This table defines the UART's that should be tested and can
 * be overridden in the board config file
 */
#ifndef CONFIG_SYS_POST_UART_TABLE
#define CONFIG_SYS_POST_UART_TABLE	{ CONFIG_SYS_NS16550_COM1, \
			CONFIG_SYS_NS16550_COM2, CONFIG_SYS_NS16550_COM3, \
			CONFIG_SYS_NS16550_COM4	}
#endif

DECLARE_GLOBAL_DATA_PTR;

static int test_ctlr (struct NS16550 *com_port, int index)
{
	int res = -1;
	char test_str[] = "*** UART Test String ***\r\n";
	int i;
	int divisor;

	divisor = (get_serial_clock() + (gd->baudrate * (16 / 2))) /
		(16 * gd->baudrate);
	NS16550_init(com_port, divisor);

	/*
	 * Set internal loopback mode in UART
	 */
	out_8(&com_port->mcr, in_8(&com_port->mcr) | UART_MCR_LOOP);

	/* Reset FIFOs */
	out_8(&com_port->fcr, UART_FCR_RXSR | UART_FCR_TXSR);
	udelay(100);

	/* Flush RX-FIFO */
	while (NS16550_tstc(com_port))
		NS16550_getc(com_port);

	for (i = 0; i < sizeof (test_str) - 1; i++) {
		NS16550_putc(com_port, test_str[i]);
		if (NS16550_getc(com_port) != test_str[i])
			goto done;
	}
	res = 0;
done:
	if (res)
		post_log ("uart%d test failed\n", index);

	return res;
}

int uart_post_test (int flags)
{
	int i, res = 0;
	static unsigned long base[] = CONFIG_SYS_POST_UART_TABLE;

	for (i = 0; i < ARRAY_SIZE(base); i++) {
		if (test_ctlr((struct NS16550 *)base[i], i))
			res = -1;
	}
	serial_reinit_all ();

	return res;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_UART */
