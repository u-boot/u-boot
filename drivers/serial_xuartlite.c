/*
 * (C) Copyright 2004 Atmark Techno, Inc.
 *
 * Yasushi SHOJI <yashi@atmark-techno.com>
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

#ifdef	XILINX_UARTLITE

#include <asm/serial_xuartlite.h>

/* FIXME: we should convert these to in32 and out32 */
#define IO_WORD(offset)	     (*(volatile unsigned long *)(offset))
#define IO_SERIAL(offset)    IO_WORD(CONFIG_SERIAL_BASE + (offset))

#define IO_SERIAL_RX_FIFO   IO_SERIAL(XUL_RX_FIFO_OFFSET)
#define IO_SERIAL_TX_FIFO   IO_SERIAL(XUL_TX_FIFO_OFFSET)
#define IO_SERIAL_STATUS    IO_SERIAL(XUL_STATUS_REG_OFFSET)
#define IO_SERIAL_CONTROL   IO_SERIAL(XUL_CONTROL_REG_OFFSET)

int serial_init(void)
{
	/* FIXME: Nothing for now. We should initialize fifo, etc */
	return 0;
}

void serial_setbrg(void)
{
	/* FIXME: what's this for? */
}

void serial_putc(const char c)
{
	if (c == '\n') serial_putc('\r');
	while (IO_SERIAL_STATUS & XUL_SR_TX_FIFO_FULL);
	IO_SERIAL_TX_FIFO = (unsigned char) (c & 0xff);
}

void serial_puts(const char * s)
{
	while (*s) {
		serial_putc(*s++);
	}
}

int serial_getc(void)
{
	while (!(IO_SERIAL_STATUS & XUL_SR_RX_FIFO_VALID_DATA));
	return IO_SERIAL_RX_FIFO & 0xff;
}

int serial_tstc(void)
{
	return (IO_SERIAL_STATUS & XUL_SR_RX_FIFO_VALID_DATA);
}

#endif	/* CONFIG_MICROBLZE */
