/*
 * U-boot - serial.c Serial driver for BF533
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * This file is based on
 * bf533_serial.c: Serial driver for BlackFin BF533 DSP internal UART.
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
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/segment.h>
#include <asm/bitops.h>
#include <asm/delay.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include "bf533_serial.h"

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
	DECLARE_GLOBAL_DATA_PTR;

	calc_baud();

	for (i = 0; i < sizeof(baud_table) / sizeof(int); i++) {
		if (gd->baudrate == baud_table[i])
			break;
	}

	/* Enable UART */
	*pUART_GCTL |= UART_GCTL_UCEN;
	sync();

	/* Set DLAB in LCR to Access DLL and DLH */
	ACCESS_LATCH;
	sync();

	*pUART_DLL = hw_baud_table[i].dl_low;
	sync();
	*pUART_DLH = hw_baud_table[i].dl_high;
	sync();

	/* Clear DLAB in LCR to Access THR RBR IER */
	ACCESS_PORT_IER;
	sync();

	/* Enable  ERBFI and ELSI interrupts
	 * to poll SIC_ISR register*/
	*pUART_IER = UART_IER_ELSI | UART_IER_ERBFI | UART_IER_ETBEI;
	sync();

	/* Set LCR to Word Lengh 8-bit word select */
	*pUART_LCR = UART_LCR_WLS8;
	sync();

	return;
}

int serial_init(void)
{
	serial_setbrg();
	return (0);
}

void serial_putc(const char c)
{
	if ((*pUART_LSR) & UART_LSR_TEMT) {
		if (c == '\n')
			serial_putc('\r');

		local_put_char(c);
	}

	while (!((*pUART_LSR) & UART_LSR_TEMT))
		SYNC_ALL;

	return;
}

int serial_tstc(void)
{
	if (*pUART_LSR & UART_LSR_DR)
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
	while (!((isr_val =
		  *(volatile unsigned long *)SIC_ISR) & IRQ_UART_RX_BIT)) ;
	asm("csync;");

	uart_lsr_val = *pUART_LSR;	/* Clear status bit */
	uart_rbr_val = *pUART_RBR;	/* getc() */

	if (isr_val & IRQ_UART_ERROR_BIT) {
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

	save_and_cli(flags);

	/* Poll for TX Interruput */
	while (!((isr_val = *pSIC_ISR) & IRQ_UART_TX_BIT)) ;
	asm("csync;");

	*pUART_THR = ch;	/* putc() */

	if (isr_val & IRQ_UART_ERROR_BIT) {
		printf("?");
	}

	restore_flags(flags);

	return;
}
