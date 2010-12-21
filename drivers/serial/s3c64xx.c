/*
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <common.h>

#include <asm/arch/s3c6400.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SERIAL1
#define UART_NR	S3C64XX_UART0

#elif defined(CONFIG_SERIAL2)
#define UART_NR	S3C64XX_UART1

#elif defined(CONFIG_SERIAL3)
#define UART_NR	S3C64XX_UART2

#else
#error "Bad: you didn't configure serial ..."
#endif

#define barrier() asm volatile("" ::: "memory")

/*
 * The coefficient, used to calculate the baudrate on S3C6400 UARTs is
 * calculated as
 * C = UBRDIV * 16 + number_of_set_bits_in_UDIVSLOT
 * however, section 31.6.11 of the datasheet doesn't recomment using 1 for 1,
 * 3 for 2, ... (2^n - 1) for n, instead, they suggest using these constants:
 */
static const int udivslot[] = {
	0,
	0x0080,
	0x0808,
	0x0888,
	0x2222,
	0x4924,
	0x4a52,
	0x54aa,
	0x5555,
	0xd555,
	0xd5d5,
	0xddd5,
	0xdddd,
	0xdfdd,
	0xdfdf,
	0xffdf,
};

void serial_setbrg(void)
{
	s3c64xx_uart *const uart = s3c64xx_get_base_uart(UART_NR);
	u32 pclk = get_PCLK();
	u32 baudrate = gd->baudrate;
	int i;

	i = (pclk / baudrate) % 16;

	uart->UBRDIV = pclk / baudrate / 16 - 1;
	uart->UDIVSLOT = udivslot[i];

	for (i = 0; i < 100; i++)
		barrier();
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 */
int serial_init(void)
{
	s3c64xx_uart *const uart = s3c64xx_get_base_uart(UART_NR);

	/* reset and enable FIFOs, set triggers to the maximum */
	uart->UFCON = 0xff;
	uart->UMCON = 0;
	/* 8N1 */
	uart->ULCON = 3;
	/* No interrupts, no DMA, pure polling */
	uart->UCON = 5;

	serial_setbrg();

	return 0;
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int serial_getc(void)
{
	s3c64xx_uart *const uart = s3c64xx_get_base_uart(UART_NR);

	/* wait for character to arrive */
	while (!(uart->UTRSTAT & 0x1));

	return uart->URXH & 0xff;
}

#ifdef CONFIG_MODEM_SUPPORT
static int be_quiet;
void disable_putc(void)
{
	be_quiet = 1;
}

void enable_putc(void)
{
	be_quiet = 0;
}
#endif


/*
 * Output a single byte to the serial port.
 */
void serial_putc(const char c)
{
	s3c64xx_uart *const uart = s3c64xx_get_base_uart(UART_NR);

#ifdef CONFIG_MODEM_SUPPORT
	if (be_quiet)
		return;
#endif

	/* wait for room in the tx FIFO */
	while (!(uart->UTRSTAT & 0x2));

	uart->UTXH = c;

	/* If \n, also do \r */
	if (c == '\n')
		serial_putc('\r');
}

/*
 * Test whether a character is in the RX buffer
 */
int serial_tstc(void)
{
	s3c64xx_uart *const uart = s3c64xx_get_base_uart(UART_NR);

	return uart->UTRSTAT & 0x1;
}

void serial_puts(const char *s)
{
	while (*s)
		serial_putc(*s++);
}
