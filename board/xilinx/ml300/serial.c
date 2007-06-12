/*
 *     Author: Xilinx, Inc.
 *
 *
 *     This program is free software; you can redistribute it and/or modify it
 *     under the terms of the GNU General Public License as published by the
 *     Free Software Foundation; either version 2 of the License, or (at your
 *     option) any later version.
 *
 *
 *     XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 *     COURTESY TO YOU. BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 *     ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE, APPLICATION OR STANDARD,
 *     XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION IS FREE
 *     FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE FOR OBTAINING
 *     ANY THIRD PARTY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 *     XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 *     THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY
 *     WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM
 *     CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND
 *     FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 *     Xilinx hardware products are not intended for use in life support
 *     appliances, devices, or systems. Use in such applications is
 *     expressly prohibited.
 *
 *
 *     (c) Copyright 2002-2004 Xilinx Inc.
 *     All rights reserved.
 *
 *
 *     You should have received a copy of the GNU General Public License along
 *     with this program; if not, write to the Free Software Foundation, Inc.,
 *     675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <asm/u-boot.h>
#include <asm/processor.h>
#include <common.h>
#include <command.h>
#include <config.h>

DECLARE_GLOBAL_DATA_PTR;

#define USE_CHAN1 \
	((defined XPAR_UARTNS550_0_BASEADDR) && (defined CFG_INIT_CHAN1))
#define USE_CHAN2 \
	((defined XPAR_UARTNS550_1_BASEADDR) && (defined CFG_INIT_CHAN2))

#if USE_CHAN1
#include <ns16550.h>
#endif

#if USE_CHAN1
const NS16550_t COM_PORTS[] = { (NS16550_t) (XPAR_UARTNS550_0_BASEADDR + 3)
#if USE_CHAN2
	    , (NS16550_t) (XPAR_UARTNS550_1_BASEADDR + 3)
#endif
};
#endif

int
serial_init(void)
{
#if USE_CHAN1
	int clock_divisor;

	clock_divisor = XPAR_UARTNS550_0_CLOCK_FREQ_HZ / 16 / gd->baudrate;
	(void) NS16550_init(COM_PORTS[0], clock_divisor);
#if USE_CHAN2
	clock_divisor = XPAR_UARTNS550_1_CLOCK_FREQ_HZ / 16 / gd->baudrate;
	(void) NS16550_init(COM_PORTS[1], clock_divisor);
#endif
#endif
	return 0;

}

void
serial_putc(const char c)
{
	if (c == '\n')
		NS16550_putc(COM_PORTS[CFG_DUART_CHAN], '\r');

	NS16550_putc(COM_PORTS[CFG_DUART_CHAN], c);
}

int
serial_getc(void)
{
	return NS16550_getc(COM_PORTS[CFG_DUART_CHAN]);
}

int
serial_tstc(void)
{
	return NS16550_tstc(COM_PORTS[CFG_DUART_CHAN]);
}

void
serial_setbrg(void)
{
#if USE_CHAN1
	int clock_divisor;

	clock_divisor = XPAR_UARTNS550_0_CLOCK_FREQ_HZ / 16 / gd->baudrate;
	NS16550_reinit(COM_PORTS[0], clock_divisor);
#if USE_CHAN2
	clock_divisor = XPAR_UARTNS550_1_CLOCK_FREQ_HZ / 16 / gd->baudrate;
	NS16550_reinit(COM_PORTS[1], clock_divisor);
#endif
#endif
}

void
serial_puts(const char *s)
{
	while (*s) {
		serial_putc(*s++);
	}
}

#if (CONFIG_COMMANDS & CFG_CMD_KGDB) || defined(CONFIG_CMD_KGDB)
void
kgdb_serial_init(void)
{
}

void
putDebugChar(int c)
{
	serial_putc(c);
}

void
putDebugStr(const char *str)
{
	serial_puts(str);
}

int
getDebugChar(void)
{
	return serial_getc();
}

void
kgdb_interruptible(int yes)
{
	return;
}
#endif				/* CFG_CMD_KGDB */
