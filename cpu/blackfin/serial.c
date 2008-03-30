/*
 * U-boot - serial.c Blackfin Serial Driver
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * Copyright (c) 2003	Bas Vermeulen <bas@buyways.nl>,
 * 			BuyWays B.V. (www.buyways.nl)
 *
 * Based heavily on:
 * blkfinserial.c: Serial driver for BlackFin DSP internal USRTs.
 * Copyright(c) 2003	Metrowerks	<mwaddel@metrowerks.com>
 * Copyright(c)	2001	Tony Z. Kou	<tonyko@arcturusnetworks.com>
 * Copyright(c)	2001-2002 Arcturus Networks Inc. <www.arcturusnetworks.com>
 *
 * Based on code from 68328 version serial driver imlpementation which was:
 * Copyright (C) 1995       David S. Miller    <davem@caip.rutgers.edu>
 * Copyright (C) 1998       Kenneth Albanowski <kjahds@kjahds.com>
 * Copyright (C) 1998, 1999 D. Jeff Dionne     <jeff@uclinux.org>
 * Copyright (C) 1999       Vladimir Gurevich  <vgurevic@cisco.com>
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <watchdog.h>
#include <asm/blackfin.h>
#include <asm/mach-common/bits/uart.h>

#if defined(UART_LSR) && (CONFIG_UART_CONSOLE != 0)
# error CONFIG_UART_CONSOLE must be 0 on parts with only one UART
#endif

#include "serial.h"

/* Symbol for our assembly to call. */
void serial_set_baud(uint32_t baud)
{
	serial_early_set_baud(baud);
}

/* Symbol for common u-boot code to call.
 * Setup the baudrate (brg: baudrate generator).
 */
void serial_setbrg(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	serial_set_baud(gd->baudrate);
}

/* Symbol for our assembly to call. */
void serial_initialize(void)
{
	serial_early_init();
}

/* Symbol for common u-boot code to call. */
int serial_init(void)
{
	serial_initialize();
	serial_setbrg();
	return 0;
}

void serial_putc(const char c)
{
	/* send a \r for compatibility */
	if (c == '\n')
		serial_putc('\r');

	WATCHDOG_RESET();

	/* wait for the hardware fifo to clear up */
	while (!(*pUART_LSR & THRE))
		continue;

	/* queue the character for transmission */
	*pUART_THR = c;
	SSYNC();

	WATCHDOG_RESET();

	/* wait for the byte to be shifted over the line */
	while (!(*pUART_LSR & TEMT))
		continue;
}

int serial_tstc(void)
{
	WATCHDOG_RESET();
	return (*pUART_LSR & DR) ? 1 : 0;
}

int serial_getc(void)
{
	uint16_t uart_lsr_val, uart_rbr_val;

	/* wait for data ! */
	while (!serial_tstc())
		continue;

	/* clear the status and grab the new byte */
	uart_lsr_val = *pUART_LSR;
	uart_rbr_val = *pUART_RBR;

	if (uart_lsr_val & (OE|PE|FE|BI)) {
		/* Some parts are read-to-clear while others are
		 * write-to-clear.  Just do the write for everyone
		 * since it cant hurt (other than code size).
		 */
		*pUART_LSR = (OE|PE|FE|BI);
		return -1;
	}

	return uart_rbr_val & 0xFF;
}

void serial_puts(const char *s)
{
	while (*s)
		serial_putc(*s++);
}
