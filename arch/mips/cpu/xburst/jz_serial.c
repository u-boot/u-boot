/*
 * Jz4740 UART support
 * Copyright (c) 2011
 * Qi Hardware, Xiangfu Liu <xiangfu@sharism.cc>
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

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/jz4740.h>

/*
 * serial_init - initialize a channel
 *
 * This routine initializes the number of data bits, parity
 * and set the selected baud rate. Interrupts are disabled.
 * Set the modem control signals if the option is selected.
 *
 * RETURNS: N/A
 */
struct jz4740_uart *uart = (struct jz4740_uart *)CONFIG_SYS_UART_BASE;

int serial_init(void)
{
	/* Disable port interrupts while changing hardware */
	writeb(0, &uart->dlhr_ier);

	/* Disable UART unit function */
	writeb(~UART_FCR_UUE, &uart->iir_fcr);

	/* Set both receiver and transmitter in UART mode (not SIR) */
	writeb(~(SIRCR_RSIRE | SIRCR_TSIRE), &uart->isr);

	/*
	 * Set databits, stopbits and parity.
	 * (8-bit data, 1 stopbit, no parity)
	 */
	writeb(UART_LCR_WLEN_8 | UART_LCR_STOP_1, &uart->lcr);

	/* Set baud rate */
	serial_setbrg();

	/* Enable UART unit, enable and clear FIFO */
	writeb(UART_FCR_UUE | UART_FCR_FE | UART_FCR_TFLS | UART_FCR_RFLS,
	       &uart->iir_fcr);

	return 0;
}

void serial_setbrg(void)
{
	u32 baud_div, tmp;

	baud_div = CONFIG_SYS_EXTAL / 16 / CONFIG_BAUDRATE;

	tmp = readb(&uart->lcr);
	tmp |= UART_LCR_DLAB;
	writeb(tmp, &uart->lcr);

	writeb((baud_div >> 8) & 0xff, &uart->dlhr_ier);
	writeb(baud_div & 0xff, &uart->rbr_thr_dllr);

	tmp &= ~UART_LCR_DLAB;
	writeb(tmp, &uart->lcr);
}

int serial_tstc(void)
{
	if (readb(&uart->lsr) & UART_LSR_DR)
		return 1;

	return 0;
}

void serial_putc(const char c)
{
	if (c == '\n')
		serial_putc('\r');

	/* Wait for fifo to shift out some bytes */
	while (!((readb(&uart->lsr) & (UART_LSR_TDRQ | UART_LSR_TEMT)) == 0x60))
		;

	writeb((u8)c, &uart->rbr_thr_dllr);
}

int serial_getc(void)
{
	while (!serial_tstc())
		;

	return readb(&uart->rbr_thr_dllr);
}

void serial_puts(const char *s)
{
	while (*s)
		serial_putc(*s++);
}
