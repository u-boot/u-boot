/*
 * U-boot - serial.c Blackfin Serial Driver
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * Copyright (c) 2003	Bas Vermeulen <bas@buyways.nl>,
 *			BuyWays B.V. (www.buyways.nl)
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

/* Anomaly notes:
 *  05000086 - we don't support autobaud
 *  05000099 - we only use DR bit, so losing others is not a problem
 *  05000100 - we don't use the UART_IIR register
 *  05000215 - we poll the uart (no dma/interrupts)
 *  05000225 - no workaround possible, but this shouldnt cause errors ...
 *  05000230 - we tweak the baud rate calculation slightly
 *  05000231 - we always use 1 stop bit
 *  05000309 - we always enable the uart before we modify it in anyway
 *  05000350 - we always enable the uart regardless of boot mode
 *  05000363 - we don't support break signals, so don't generate one
 */

#include <common.h>
#include <watchdog.h>
#include <asm/blackfin.h>
#include <asm/mach-common/bits/uart.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_UART_CONSOLE

#include "serial.h"

#ifdef CONFIG_DEBUG_SERIAL
static uint16_t cached_lsr[256];
static uint16_t cached_rbr[256];
static size_t cache_count;

/* The LSR is read-to-clear on some parts, so we have to make sure status
 * bits aren't inadvertently lost when doing various tests.  This also
 * works around anomaly 05000099 at the same time by keeping a cumulative
 * tally of all the status bits.
 */
static uint16_t uart_lsr_save;
static uint16_t uart_lsr_read(void)
{
	uint16_t lsr = bfin_read16(&pUART->lsr);
	uart_lsr_save |= (lsr & (OE|PE|FE|BI));
	return lsr | uart_lsr_save;
}
/* Just do the clear for everyone since it can't hurt. */
static void uart_lsr_clear(void)
{
	uart_lsr_save = 0;
	bfin_write16(&pUART->lsr, bfin_read16(&pUART->lsr) | -1);
}
#else
/* When debugging is disabled, we only care about the DR bit, so if other
 * bits get set/cleared, we don't really care since we don't read them
 * anyways (and thus anomaly 05000099 is irrelevant).
 */
static uint16_t uart_lsr_read(void)
{
	return bfin_read16(&pUART->lsr);
}
static void uart_lsr_clear(void)
{
	bfin_write16(&pUART->lsr, bfin_read16(&pUART->lsr) | -1);
}
#endif

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
	uart_lsr_clear();
	return 0;
}

void serial_putc(const char c)
{
	/* send a \r for compatibility */
	if (c == '\n')
		serial_putc('\r');

	WATCHDOG_RESET();

	/* wait for the hardware fifo to clear up */
	while (!(uart_lsr_read() & THRE))
		continue;

	/* queue the character for transmission */
	bfin_write16(&pUART->thr, c);
	SSYNC();

	WATCHDOG_RESET();
}

int serial_tstc(void)
{
	WATCHDOG_RESET();
	return (uart_lsr_read() & DR) ? 1 : 0;
}

int serial_getc(void)
{
	uint16_t uart_rbr_val;

	/* wait for data ! */
	while (!serial_tstc())
		continue;

	/* grab the new byte */
	uart_rbr_val = bfin_read16(&pUART->rbr);

#ifdef CONFIG_DEBUG_SERIAL
	/* grab & clear the LSR */
	uint16_t uart_lsr_val = uart_lsr_read();

	cached_lsr[cache_count] = uart_lsr_val;
	cached_rbr[cache_count] = uart_rbr_val;
	cache_count = (cache_count + 1) % ARRAY_SIZE(cached_lsr);

	if (uart_lsr_val & (OE|PE|FE|BI)) {
		uint16_t dll, dlh;
		printf("\n[SERIAL ERROR]\n");
		ACCESS_LATCH();
		dll = bfin_read16(&pUART->dll);
		dlh = bfin_read16(&pUART->dlh);
		ACCESS_PORT_IER();
		printf("\tDLL=0x%x DLH=0x%x\n", dll, dlh);
		do {
			--cache_count;
			printf("\t%3i: RBR=0x%02x LSR=0x%02x\n", cache_count,
				cached_rbr[cache_count], cached_lsr[cache_count]);
		} while (cache_count > 0);
		return -1;
	}
#endif
	uart_lsr_clear();

	return uart_rbr_val;
}

void serial_puts(const char *s)
{
	while (*s)
		serial_putc(*s++);
}

#endif
