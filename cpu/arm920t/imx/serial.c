/*
 * (c) 2004 Sascha Hauer <sascha@saschahauer.de>
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
#if defined (CONFIG_IMX)

#include <asm/arch/imx-regs.h>

#ifndef CONFIG_IMX_SERIAL_NONE

#if defined CONFIG_IMX_SERIAL1
#define UART_BASE IMX_UART1_BASE
#elif defined CONFIG_IMX_SERIAL2
#define UART_BASE IMX_UART2_BASE
#else
#error "define CONFIG_IMX_SERIAL1, CONFIG_IMX_SERIAL2 or CONFIG_IMX_SERIAL_NONE"
#endif

struct imx_serial {
	volatile uint32_t urxd[16];
	volatile uint32_t utxd[16];
	volatile uint32_t ucr1;
	volatile uint32_t ucr2;
	volatile uint32_t ucr3;
	volatile uint32_t ucr4;
	volatile uint32_t ufcr;
	volatile uint32_t usr1;
	volatile uint32_t usr2;
	volatile uint32_t uesc;
	volatile uint32_t utim;
	volatile uint32_t ubir;
	volatile uint32_t ubmr;
	volatile uint32_t ubrc;
	volatile uint32_t bipr[4];
	volatile uint32_t bmpr[4];
	volatile uint32_t uts;
};

void serial_setbrg (void)
{
	serial_init();
}

extern void imx_gpio_mode(int gpio_mode);

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 *
 */
int serial_init (void)
{
	volatile struct imx_serial* base = (struct imx_serial *)UART_BASE;
#ifdef CONFIG_IMX_SERIAL1
	imx_gpio_mode(PC11_PF_UART1_TXD);
	imx_gpio_mode(PC12_PF_UART1_RXD);
#else
	imx_gpio_mode(PB30_PF_UART2_TXD);
	imx_gpio_mode(PB31_PF_UART2_RXD);
#endif

	/* Disable UART */
	base->ucr1 &= ~UCR1_UARTEN;

	/* Set to default POR state */

	base->ucr1 = 0x00000004;
	base->ucr2 = 0x00000000;
	base->ucr3 = 0x00000000;
	base->ucr4 = 0x00008040;
	base->uesc = 0x0000002B;
	base->utim = 0x00000000;
	base->ubir = 0x00000000;
	base->ubmr = 0x00000000;
	base->uts  = 0x00000000;
	/* Set clocks */
	base->ucr4 |= UCR4_REF16;

	/* Configure FIFOs */
	base->ufcr = 0xa81;

	/* Set the numerator value minus one of the BRM ratio */
	base->ubir = (CONFIG_BAUDRATE / 100) - 1;

	/* Set the denominator value minus one of the BRM ratio	*/
	base->ubmr = 10000 - 1;

	/* Set to 8N1 */
	base->ucr2 &= ~UCR2_PREN;
	base->ucr2 |= UCR2_WS;
	base->ucr2 &= ~UCR2_STPB;

	/* Ignore RTS */
	base->ucr2 |= UCR2_IRTS;

	/* Enable UART */
	base->ucr1 |= UCR1_UARTEN | UCR1_UARTCLKEN;

	/* Enable FIFOs */
	base->ucr2 |= UCR2_SRST | UCR2_RXEN | UCR2_TXEN;

	/* Clear status flags */
	base->usr2 |= USR2_ADET  |
	          USR2_DTRF  |
	          USR2_IDLE  |
	          USR2_IRINT |
	          USR2_WAKE  |
	          USR2_RTSF  |
	          USR2_BRCD  |
	          USR2_ORE   |
	          USR2_RDR;

	/* Clear status flags */
	base->usr1 |= USR1_PARITYERR |
	          USR1_RTSD      |
	          USR1_ESCF      |
	          USR1_FRAMERR   |
	          USR1_AIRINT    |
	          USR1_AWAKE;
	return (0);
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is successful, the character read is
 * written into its argument c.
 */
int serial_getc (void)
{
	volatile struct imx_serial* base = (struct imx_serial *)UART_BASE;
	unsigned char ch;

	while(base->uts & UTS_RXEMPTY);

	ch = (char)base->urxd[0];

	return ch;
}

#ifdef CONFIG_HWFLOW
static int hwflow = 0; /* turned off by default */
int hwflow_onoff(int on)
{
}
#endif

/*
 * Output a single byte to the serial port.
 */
void serial_putc (const char c)
{
	volatile struct imx_serial* base = (struct imx_serial *)UART_BASE;

	/* Wait for Tx FIFO not full */
	while (base->uts & UTS_TXFULL);

	base->utxd[0] = c;

	/* If \n, also do \r */
	if (c == '\n')
		serial_putc ('\r');
}

/*
 * Test whether a character is in the RX buffer
 */
int serial_tstc (void)
{
	volatile struct imx_serial* base = (struct imx_serial *)UART_BASE;

	/* If receive fifo is empty, return false */
	if (base->uts & UTS_RXEMPTY)
		return 0;
	return 1;
}

void
serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}
#endif /* CONFIG_IMX_SERIAL_NONE */
#endif /* defined CONFIG_IMX */
