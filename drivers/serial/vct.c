/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
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

#ifdef CONFIG_VCT_PLATINUMAVC
#define UART_1_BASE		0xBDC30000
#else
#define UART_1_BASE		0xBF89C000
#endif

#define	UART_RBR_OFF		0x00	/* receiver buffer reg		*/
#define	UART_THR_OFF		0x00	/* transmit holding  reg	*/
#define	UART_DLL_OFF		0x00	/* divisor latch low reg	*/
#define	UART_IER_OFF		0x04	/* interrupt enable reg		*/
#define	UART_DLH_OFF		0x04	/* receiver buffer reg		*/
#define	UART_FCR_OFF		0x08	/* fifo control register	*/
#define	UART_LCR_OFF		0x0c	/* line control register	*/
#define	UART_MCR_OFF		0x10	/* modem control register	*/
#define	UART_LSR_OFF		0x14	/* line status register		*/
#define	UART_MSR_OFF		0x18	/* modem status register	*/
#define	UART_SCR_OFF		0x1c	/* scratch pad register		*/

#define UART_RCV_DATA_RDY	0x01	/* Data Received		*/
#define UART_XMT_HOLD_EMPTY	0x20
#define UART_TRANSMIT_EMPTY	0x40

/* 7 bit on line control reg. enalbing rw to dll and dlh */
#define UART_LCR_DLAB		0x0080

#define UART___9600_BDR		0x84
#define UART__19200_BDR		0x42
#define UART_115200_BDR		0x08

#define UART_DIS_ALL_INTER	0x00	/* disable all interrupts	*/

#define UART_5DATA_BITS		0x0000	/*   5 [bits]  1.5 bits   2	*/
#define UART_6DATA_BITS		0x0001	/*   6 [bits]  1   bits   2	*/
#define UART_7DATA_BITS		0x0002	/*   7 [bits]  1   bits   2	*/
#define UART_8DATA_BITS		0x0003	/*   8 [bits]  1   bits   2	*/

static void vct_uart_set_baud_rate(u32 address, u32 dh, u32 dl)
{
	u32 val = __raw_readl(UART_1_BASE + UART_LCR_OFF);

	/* set 7 bit on 1 */
	val |= UART_LCR_DLAB;
	__raw_writel(val, UART_1_BASE + UART_LCR_OFF);

	__raw_writel(dl, UART_1_BASE + UART_DLL_OFF);
	__raw_writel(dh, UART_1_BASE + UART_DLH_OFF);

	/* set 7 bit on 0 */
	val &= ~UART_LCR_DLAB;
	__raw_writel(val, UART_1_BASE + UART_LCR_OFF);

	return;
}

int serial_init(void)
{
	__raw_writel(UART_DIS_ALL_INTER, UART_1_BASE + UART_IER_OFF);
	vct_uart_set_baud_rate(UART_1_BASE, 0, UART_115200_BDR);
	__raw_writel(UART_8DATA_BITS, UART_1_BASE + UART_LCR_OFF);

	return 0;
}

void serial_setbrg(void)
{
	/*
	 * Baudrate change not supported currently, fixed to 115200 baud
	 */
}

void serial_putc(const char c)
{
	if (c == '\n')
		serial_putc('\r');

	while (!(UART_XMT_HOLD_EMPTY & __raw_readl(UART_1_BASE + UART_LSR_OFF)))
		;

	__raw_writel(c, UART_1_BASE + UART_THR_OFF);
}

void serial_puts(const char *s)
{
	while (*s)
		serial_putc(*s++);
}

int serial_getc(void)
{
	while (!(UART_RCV_DATA_RDY & __raw_readl(UART_1_BASE + UART_LSR_OFF)))
		;

	return __raw_readl(UART_1_BASE + UART_RBR_OFF) & 0xff;
}

int serial_tstc(void)
{
	if (!(UART_RCV_DATA_RDY & __raw_readl(UART_1_BASE + UART_LSR_OFF)))
		return 0;

	return 1;
}
