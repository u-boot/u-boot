/*
 * (C) Copyright 2008 Michal Simek <monstr@monstr.eu>
 * Clean driver and add xilinx constant from header file
 *
 * (C) Copyright 2004 Atmark Techno, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <asm/io.h>

#define RX_FIFO_OFFSET		0 /* receive FIFO, read only */
#define TX_FIFO_OFFSET		4 /* transmit FIFO, write only */
#define STATUS_REG_OFFSET	8 /* status register, read only */

#define SR_TX_FIFO_FULL		0x08 /* transmit FIFO full */
#define SR_RX_FIFO_VALID_DATA	0x01 /* data in receive FIFO */
#define SR_RX_FIFO_FULL		0x02 /* receive FIFO full */

#define UARTLITE_STATUS		(CONFIG_SERIAL_BASE + STATUS_REG_OFFSET)
#define UARTLITE_TX_FIFO	(CONFIG_SERIAL_BASE + TX_FIFO_OFFSET)
#define UARTLITE_RX_FIFO	(CONFIG_SERIAL_BASE + RX_FIFO_OFFSET)

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
	if (c == '\n')
		serial_putc('\r');
	while (in_be32((u32 *) UARTLITE_STATUS) & SR_TX_FIFO_FULL);
	out_be32((u32 *) UARTLITE_TX_FIFO, (unsigned char) (c & 0xff));
}

void serial_puts(const char * s)
{
	while (*s) {
		serial_putc(*s++);
	}
}

int serial_getc(void)
{
	while (!(in_be32((u32 *) UARTLITE_STATUS) & SR_RX_FIFO_VALID_DATA));
	return in_be32((u32 *) UARTLITE_RX_FIFO) & 0xff;
}

int serial_tstc(void)
{
	return (in_be32((u32 *) UARTLITE_STATUS) & SR_RX_FIFO_VALID_DATA);
}
