/*
 * U-boot - serial.c Serial driver for BF537
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * This file is based on
 * bf537_serial.c: Serial driver for BlackFin BF537 internal UART.
 * Copyright (c) 2003	Bas Vermeulen <bas@buyways.nl>,
 * 			BuyWays B.V. (www.buyways.nl)
 *
 * Based heavily on blkfinserial.c
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <asm/system.h>
#include <asm/bitops.h>
#include <asm/delay.h>
#include <asm/io.h>
#include "serial.h"
#include <asm/mach-common/bits/uart.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long pll_div_fact;

void calc_baud(void)
{
	unsigned char i;
	int temp;
	u_long sclk = get_sclk();

	for (i = 0; i < sizeof(baud_table) / sizeof(int); i++) {
		temp = sclk / (baud_table[i] * 8);
		if ((temp & 0x1) == 1) {
			temp++;
		}
		temp = temp / 2;
		hw_baud_table[i].dl_high = (temp >> 8) & 0xFF;
		hw_baud_table[i].dl_low = (temp) & 0xFF;
	}
}

void serial_setbrg(void)
{
	int i;

	calc_baud();

	for (i = 0; i < sizeof(baud_table) / sizeof(int); i++) {
		if (gd->baudrate == baud_table[i])
			break;
	}

	/* Enable UART */
	*pUART0_GCTL |= UCEN;
	SSYNC();

	/* Set DLAB in LCR to Access DLL and DLH */
	ACCESS_LATCH;
	SSYNC();

	*pUART0_DLL = hw_baud_table[i].dl_low;
	SSYNC();
	*pUART0_DLH = hw_baud_table[i].dl_high;
	SSYNC();

	/* Clear DLAB in LCR to Access THR RBR IER */
	ACCESS_PORT_IER;
	SSYNC();

	/* Enable  ERBFI and ELSI interrupts
	 * to poll SIC_ISR register*/
	*pUART0_IER = ELSI | ERBFI | ETBEI;
	SSYNC();

	/* Set LCR to Word Lengh 8-bit word select */
	*pUART0_LCR = WLS_8;
	SSYNC();

	return;
}

int serial_init(void)
{
	serial_setbrg();
	return (0);
}

void serial_putc(const char c)
{
	if ((*pUART0_LSR) & TEMT) {
		if (c == '\n')
			serial_putc('\r');

		local_put_char(c);
	}

	while (!((*pUART0_LSR) & TEMT))
		SYNC_ALL;

	return;
}

int serial_tstc(void)
{
	if (*pUART0_LSR & DR)
		return 1;
	else
		return 0;
}

int serial_getc(void)
{
	unsigned short uart_lsr_val, uart_rbr_val;
	unsigned long isr_val;
	int ret;

	/* Poll for RX Interrupt */
	while (!serial_tstc())
		continue;
	asm("csync;");

	uart_lsr_val = *pUART0_LSR;	/* Clear status bit */
	uart_rbr_val = *pUART0_RBR;	/* getc() */

	if (uart_lsr_val & (OE|PE|FE|BI)) {
		ret = -1;
	} else {
		ret = uart_rbr_val & 0xff;
	}

	return ret;
}

void serial_puts(const char *s)
{
	while (*s) {
		serial_putc(*s++);
	}
}

static void local_put_char(char ch)
{
	int flags = 0;
	unsigned long isr_val;

	/* Poll for TX Interruput */
	while (!(*pUART0_LSR & THRE))
		continue;
	asm("csync;");

	*pUART0_THR = ch;	/* putc() */

	return;
}
