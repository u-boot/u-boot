/*
 * (C) Copyright 2007
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

/*
 * UART test
 *
 * The controllers are configured to loopback mode and several
 * characters are transmitted.
 */

#ifdef CONFIG_POST

#include <post.h>

#if CONFIG_POST & CFG_POST_UART

#include <asm/processor.h>
#include <serial.h>

#define UART0_BASE  CFG_PERIPHERAL_BASE + 0x00000300
#define UART1_BASE  CFG_PERIPHERAL_BASE + 0x00000400
#define UART2_BASE  CFG_PERIPHERAL_BASE + 0x00000500
#define UART3_BASE  CFG_PERIPHERAL_BASE + 0x00000600

#define CR0_MASK        0xdfffffff
#define CR0_EXTCLK_ENA  0x00800000
#define CR0_UDIV_POS    0
#define UDIV_SUBTRACT	0
#define UART0_SDR	sdr_uart0
#define UART1_SDR	sdr_uart1
#define UART2_SDR	sdr_uart2
#define UART3_SDR	sdr_uart3
#define MFREG(a, d)	mfsdr(a, d)
#define MTREG(a, d)	mtsdr(a, d)

#define UART_RBR    0x00
#define UART_THR    0x00
#define UART_IER    0x01
#define UART_IIR    0x02
#define UART_FCR    0x02
#define UART_LCR    0x03
#define UART_MCR    0x04
#define UART_LSR    0x05
#define UART_MSR    0x06
#define UART_SCR    0x07
#define UART_DLL    0x00
#define UART_DLM    0x01

/*
  Line Status Register.
*/
#define asyncLSRDataReady1            0x01
#define asyncLSROverrunError1         0x02
#define asyncLSRParityError1          0x04
#define asyncLSRFramingError1         0x08
#define asyncLSRBreakInterrupt1       0x10
#define asyncLSRTxHoldEmpty1          0x20
#define asyncLSRTxShiftEmpty1         0x40
#define asyncLSRRxFifoError1          0x80

DECLARE_GLOBAL_DATA_PTR;

static int uart_post_init (unsigned long dev_base)
{
	unsigned long reg;
	unsigned long udiv;
	unsigned short bdiv;
	volatile char val;
#ifdef CFG_EXT_SERIAL_CLOCK
	unsigned long tmp;
#endif
	int i;

	for (i = 0; i < 3500; i++) {
		if (in8 (dev_base + UART_LSR) & asyncLSRTxHoldEmpty1)
			break;
		udelay (100);
	}
	MFREG(UART0_SDR, reg);
	reg &= ~CR0_MASK;

#ifdef CFG_EXT_SERIAL_CLOCK
	reg |= CR0_EXTCLK_ENA;
	udiv = 1;
	tmp  = gd->baudrate * 16;
	bdiv = (CFG_EXT_SERIAL_CLOCK + tmp / 2) / tmp;
#else
	/* For 440, the cpu clock is on divider chain A, UART on divider
	 * chain B ... so cpu clock is irrelevant. Get the "optimized"
	 * values that are subject to the 1/2 opb clock constraint
	 */
	serial_divs (gd->baudrate, &udiv, &bdiv);
#endif

	reg |= (udiv - UDIV_SUBTRACT) << CR0_UDIV_POS;	/* set the UART divisor */

	/*
	 * Configure input clock to baudrate generator for all
	 * available serial ports here
	 */
	MTREG(UART0_SDR, reg);
#if defined(UART1_SDR)
	MTREG(UART1_SDR, reg);
#endif
#if defined(UART2_SDR)
	MTREG(UART2_SDR, reg);
#endif
#if defined(UART3_SDR)
	MTREG(UART3_SDR, reg);
#endif

	out8(dev_base + UART_LCR, 0x80);	/* set DLAB bit */
	out8(dev_base + UART_DLL, bdiv);	/* set baudrate divisor */
	out8(dev_base + UART_DLM, bdiv >> 8);	/* set baudrate divisor */
	out8(dev_base + UART_LCR, 0x03);	/* clear DLAB; set 8 bits, no parity */
	out8(dev_base + UART_FCR, 0x00);	/* disable FIFO */
	out8(dev_base + UART_MCR, 0x10);	/* enable loopback mode */
	val = in8(dev_base + UART_LSR);		/* clear line status */
	val = in8(dev_base + UART_RBR);		/* read receive buffer */
	out8(dev_base + UART_SCR, 0x00);	/* set scratchpad */
	out8(dev_base + UART_IER, 0x00);	/* set interrupt enable reg */

	return 0;
}

static void uart_post_putc (unsigned long dev_base, char c)
{
	int i;

	out8 (dev_base + UART_THR, c);	/* put character out */

	/* Wait for transfer completion */
	for (i = 0; i < 3500; i++) {
		if (in8 (dev_base + UART_LSR) & asyncLSRTxHoldEmpty1)
			break;
		udelay (100);
	}
}

static int uart_post_getc (unsigned long dev_base)
{
	int i;

	/* Wait for character available */
	for (i = 0; i < 3500; i++) {
		if (in8 (dev_base + UART_LSR) & asyncLSRDataReady1)
			break;
		udelay (100);
	}
	return 0xff & in8 (dev_base + UART_RBR);
}

static int test_ctlr (unsigned long dev_base, int index)
{
	int res = -1;
	char test_str[] = "*** UART Test String ***\r\n";
	int i;

	uart_post_init (dev_base);

	for (i = 0; i < sizeof (test_str) - 1; i++) {
		uart_post_putc (dev_base, test_str[i]);
		if (uart_post_getc (dev_base) != test_str[i])
			goto done;
	}
	res = 0;
done:
	if (res)
		post_log ("uart%d test failed\n", index);

	return res;
}

int uart_post_test (int flags)
{
	int i, res = 0;
	static unsigned long base[] = {
		UART0_BASE, UART1_BASE, UART2_BASE, UART3_BASE
	};

	for (i = 0; i < sizeof (base) / sizeof (base[0]); i++) {
		if (test_ctlr (base[i], i))
			res = -1;
	}
	serial_reinit_all ();

	return res;
}

#endif /* CONFIG_POST & CFG_POST_UART */

#endif /* CONFIG_POST */
