/*
 * U-boot bf561_serial.h
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
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

#ifndef _BF561_SERIAL_H_
#define _BF561_SERIAL_H_

#define BYTE_REF(addr)		(*((volatile char*)addr))
#define HALFWORD_REF(addr)	(*((volatile short*)addr))
#define WORD_REF(addr)		(*((volatile long*)addr))

#define UART_THR_LO		HALFWORD_REF(UART_THR)
#define UART_RBR_LO		HALFWORD_REF(UART_RBR)
#define UART_DLL_LO		HALFWORD_REF(UART_DLL)
#define UART_IER_LO		HALFWORD_REF(UART_IER)
#define UART_IER_ERBFI		0x01
#define UART_IER_ETBEI		0x02
#define UART_IER_ELSI		0x04
#define UART_IER_EDDSI		0x08

#define UART_DLH_LO		HALFWORD_REF(UART_DLH)
#define UART_IIR_LO		HALFWORD_REF(UART_IIR)
#define UART_IIR_NOINT		0x01
#define UART_IIR_STATUS		0x06
#define UART_IIR_LSR		0x06
#define UART_IIR_RBR		0x04
#define UART_IIR_THR		0x02
#define UART_IIR_MSR		0x00

#define UART_LCR_LO		HALFWORD_REF(UART_LCR)
#define UART_LCR_WLS5		0
#define UART_LCR_WLS6		0x01
#define UART_LCR_WLS7		0x02
#define UART_LCR_WLS8		0x03
#define UART_LCR_STB		0x04
#define UART_LCR_PEN		0x08
#define UART_LCR_EPS		0x10
#define UART_LCR_SP		0x20
#define UART_LCR_SB		0x40
#define UART_LCR_DLAB		0x80

#define UART_MCR_LO		HALFWORD_REF(UART_MCR)

#define UART_LSR_LO		HALFWORD_REF(UART_LSR)
#define UART_LSR_DR		0x01
#define UART_LSR_OE		0x02
#define UART_LSR_PE		0x04
#define UART_LSR_FE		0x08
#define UART_LSR_BI		0x10
#define UART_LSR_THRE		0x20
#define UART_LSR_TEMT		0x40

#define UART_MSR_LO		HALFWORD_REF(UART_MSR)
#define UART_SCR_LO		HALFWORD_REF(UART_SCR)
#define UART_GCTL_LO		HALFWORD_REF(UART_GCTL)
#define UART_GCTL_UCEN		0x01

#endif
