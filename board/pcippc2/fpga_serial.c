/*
 * (C) Copyright 2002
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>

#include "fpga_serial.h"
#include "hardware.h"
#include "pcippc2.h"

DECLARE_GLOBAL_DATA_PTR;

  /* 8 data, 1 stop, no parity
   */
#define LCRVAL		0x03
  /* RTS/DTR
   */
#define MCRVAL		0x03
  /* Clear & enable FIFOs
   */
#define FCRVAL		0x07

static void fpga_serial_wait (void);
static void fpga_serial_print (char c);

void fpga_serial_init (int baudrate)
{
	int clock_divisor = 115200 / baudrate;

	out8 (FPGA (INT, SERIAL_CONFIG), 0x24);
	iobarrier_rw ();

	fpga_serial_wait ();

	out8 (UART (IER), 0);
	out8 (UART (LCR), LCRVAL | 0x80);
	iobarrier_rw ();
	out8 (UART (DLL), clock_divisor & 0xff);
	out8 (UART (DLM), clock_divisor >> 8);
	iobarrier_rw ();
	out8 (UART (LCR), LCRVAL);
	iobarrier_rw ();
	out8 (UART (MCR), MCRVAL);
	out8 (UART (FCR), FCRVAL);
	iobarrier_rw ();
}

void fpga_serial_putc (char c)
{
	if (c) {
		fpga_serial_print (c);
	}
}

int fpga_serial_getc (void)
{
	while ((in8 (UART (LSR)) & 0x01) == 0);

	return in8 (UART (RBR));
}

int fpga_serial_tstc (void)
{
	return (in8 (UART (LSR)) & 0x01) != 0;
}

void fpga_serial_setbrg (void)
{
	int clock_divisor = 115200 / gd->baudrate;

	fpga_serial_wait ();

	out8 (UART (LCR), LCRVAL | 0x80);
	iobarrier_rw ();
	out8 (UART (DLL), clock_divisor & 0xff);
	out8 (UART (DLM), clock_divisor >> 8);
	iobarrier_rw ();
	out8 (UART (LCR), LCRVAL);
	iobarrier_rw ();
}

static void fpga_serial_wait (void)
{
	while ((in8 (UART (LSR)) & 0x40) == 0);
}

static void fpga_serial_print (char c)
{
	if (c == '\n') {
		while ((in8 (UART (LSR)) & 0x20) == 0);

		out8 (UART (THR), '\r');
		iobarrier_rw ();
	}

	while ((in8 (UART (LSR)) & 0x20) == 0);

	out8 (UART (THR), c);
	iobarrier_rw ();

	if (c == '\n') {
		fpga_serial_wait ();
	}
}
