/*
 * (C) Copyright 2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Author: Igor Lisitsin <igor@emcraft.com>
 *
 * Copyright 2010, Stefan Roese, DENX Software Engineering, sr@denx.de
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
#include <ppc4xx.h>
#include <ns16550.h>
#include <asm/io.h>

/*
 * UART test
 *
 * The controllers are configured to loopback mode and several
 * characters are transmitted.
 */

#include <post.h>

#if CONFIG_POST & CONFIG_SYS_POST_UART

/*
 * This table defines the UART's that should be tested and can
 * be overridden in the board config file
 */
#ifndef CONFIG_SYS_POST_UART_TABLE
#define CONFIG_SYS_POST_UART_TABLE	{UART0_BASE, UART1_BASE, UART2_BASE, UART3_BASE}
#endif

#include <asm/processor.h>
#include <serial.h>

#if defined(CONFIG_440)
#if defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define UART0_BASE  CONFIG_SYS_PERIPHERAL_BASE + 0x00000300
#define UART1_BASE  CONFIG_SYS_PERIPHERAL_BASE + 0x00000400
#define UART2_BASE  CONFIG_SYS_PERIPHERAL_BASE + 0x00000500
#define UART3_BASE  CONFIG_SYS_PERIPHERAL_BASE + 0x00000600
#else
#define UART0_BASE  CONFIG_SYS_PERIPHERAL_BASE + 0x00000200
#define UART1_BASE  CONFIG_SYS_PERIPHERAL_BASE + 0x00000300
#endif

#if defined(CONFIG_440SP) || defined(CONFIG_440SPE)
#define UART2_BASE  CONFIG_SYS_PERIPHERAL_BASE + 0x00000600
#endif

#if defined(CONFIG_440GP)
#define CR0_MASK        0x3fff0000
#define CR0_EXTCLK_ENA  0x00600000
#define CR0_UDIV_POS    16
#define UDIV_SUBTRACT	1
#define UART0_SDR	CPC0_CR0
#define MFREG(a, d)	d = mfdcr(a)
#define MTREG(a, d)	mtdcr(a, d)
#else /* #if defined(CONFIG_440GP) */
/* all other 440 PPC's access clock divider via sdr register */
#define CR0_MASK        0xdfffffff
#define CR0_EXTCLK_ENA  0x00800000
#define CR0_UDIV_POS    0
#define UDIV_SUBTRACT	0
#define UART0_SDR	SDR0_UART0
#define UART1_SDR	SDR0_UART1
#if defined(CONFIG_440EP) || defined(CONFIG_440EPX) || \
    defined(CONFIG_440GR) || defined(CONFIG_440GRX) || \
    defined(CONFIG_440SP) || defined(CONFIG_440SPE)
#define UART2_SDR	SDR0_UART2
#endif
#if defined(CONFIG_440EP) || defined(CONFIG_440EPX) || \
    defined(CONFIG_440GR) || defined(CONFIG_440GRX)
#define UART3_SDR	SDR0_UART3
#endif
#define MFREG(a, d)	mfsdr(a, d)
#define MTREG(a, d)	mtsdr(a, d)
#endif /* #if defined(CONFIG_440GP) */
#elif defined(CONFIG_405EP) || defined(CONFIG_405EZ)
#define UART0_BASE      0xef600300
#define UART1_BASE      0xef600400
#define UCR0_MASK       0x0000007f
#define UCR1_MASK       0x00007f00
#define UCR0_UDIV_POS   0
#define UCR1_UDIV_POS   8
#define UDIV_MAX        127
#elif defined(CONFIG_405EX)
#define UART0_BASE	0xef600200
#define UART1_BASE	0xef600300
#define CR0_MASK	0x000000ff
#define CR0_EXTCLK_ENA	0x00800000
#define CR0_UDIV_POS	0
#define UDIV_SUBTRACT	0
#define UART0_SDR	SDR0_UART0
#define UART1_SDR	SDR0_UART1
#define MFREG(a, d)	mfsdr(a, d)
#define MTREG(a, d)	mtsdr(a, d)
#else /* CONFIG_405GP || CONFIG_405CR */
#define UART0_BASE      0xef600300
#define UART1_BASE      0xef600400
#define CR0_MASK        0x00001fff
#define CR0_EXTCLK_ENA  0x000000c0
#define CR0_UDIV_POS    1
#define UDIV_MAX        32
#endif

DECLARE_GLOBAL_DATA_PTR;

static void uart_post_init_common(struct NS16550 *com_port, unsigned short bdiv)
{
	volatile char val;

	out_8(&com_port->lcr, 0x80);	/* set DLAB bit */
	out_8(&com_port->dll, bdiv);	/* set baudrate divisor */
	out_8(&com_port->dlm, bdiv >> 8); /* set baudrate divisor */
	out_8(&com_port->lcr, 0x03);	/* clear DLAB; set 8 bits, no parity */
	out_8(&com_port->fcr, 0x00);	/* disable FIFO */
	out_8(&com_port->mcr, 0x10);	/* enable loopback mode */
	val = in_8(&com_port->lsr);	/* clear line status */
	val = in_8(&com_port->rbr);	/* read receive buffer */
	out_8(&com_port->scr, 0x00);	/* set scratchpad */
	out_8(&com_port->ier, 0x00);	/* set interrupt enable reg */
}

#if defined(CONFIG_440) || defined(CONFIG_405EX)
#if !defined(CONFIG_SYS_EXT_SERIAL_CLOCK)
static void serial_divs (int baudrate, unsigned long *pudiv,
			 unsigned short *pbdiv)
{
	sys_info_t sysinfo;
	unsigned long div;		/* total divisor udiv * bdiv */
	unsigned long umin;		/* minimum udiv */
	unsigned short diff;		/* smallest diff */
	unsigned long udiv;		/* best udiv */
	unsigned short idiff;		/* current diff */
	unsigned short ibdiv;		/* current bdiv */
	unsigned long i;
	unsigned long est;		/* current estimate */

	get_sys_info(&sysinfo);

	udiv = 32;			/* Assume lowest possible serial clk */
	div = sysinfo.freqPLB / (16 * baudrate); /* total divisor */
	umin = sysinfo.pllOpbDiv << 1;	/* 2 x OPB divisor */
	diff = 32;			/* highest possible */

	/* i is the test udiv value -- start with the largest
	 * possible (32) to minimize serial clock and constrain
	 * search to umin.
	 */
	for (i = 32; i > umin; i--) {
		ibdiv = div / i;
		est = i * ibdiv;
		idiff = (est > div) ? (est-div) : (div-est);
		if (idiff == 0) {
			udiv = i;
			break;	/* can't do better */
		} else if (idiff < diff) {
			udiv = i;	/* best so far */
			diff = idiff;	/* update lowest diff*/
		}
	}

	*pudiv = udiv;
	*pbdiv = div / udiv;
}
#endif

static int uart_post_init (struct NS16550 *com_port)
{
	unsigned long reg = 0;
	unsigned long udiv;
	unsigned short bdiv;
#ifdef CONFIG_SYS_EXT_SERIAL_CLOCK
	unsigned long tmp;
#endif
	int i;

	for (i = 0; i < 3500; i++) {
		if (in_8(&com_port->lsr) & UART_LSR_THRE)
			break;
		udelay (100);
	}
	MFREG(UART0_SDR, reg);
	reg &= ~CR0_MASK;

#ifdef CONFIG_SYS_EXT_SERIAL_CLOCK
	reg |= CR0_EXTCLK_ENA;
	udiv = 1;
	tmp  = gd->baudrate * 16;
	bdiv = (CONFIG_SYS_EXT_SERIAL_CLOCK + tmp / 2) / tmp;
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

	uart_post_init_common(com_port, bdiv);

	return 0;
}

#else /* CONFIG_440 */

static int uart_post_init (struct NS16550 *com_port)
{
	unsigned long reg;
	unsigned long tmp;
	unsigned long clk;
	unsigned long udiv;
	unsigned short bdiv;
	int i;

	for (i = 0; i < 3500; i++) {
		if (in_8(&com_port->lsr) & UART_LSR_THRE)
			break;
		udelay (100);
	}

#if defined(CONFIG_405EZ)
	serial_divs(gd->baudrate, &udiv, &bdiv);
	clk = tmp = reg = 0;
#else
#ifdef CONFIG_405EP
	reg = mfdcr(CPC0_UCR) & ~(UCR0_MASK | UCR1_MASK);
	clk = gd->cpu_clk;
	tmp = CONFIG_SYS_BASE_BAUD * 16;
	udiv = (clk + tmp / 2) / tmp;
	if (udiv > UDIV_MAX)                    /* max. n bits for udiv */
		udiv = UDIV_MAX;
	reg |= (udiv) << UCR0_UDIV_POS;	        /* set the UART divisor */
	reg |= (udiv) << UCR1_UDIV_POS;	        /* set the UART divisor */
	mtdcr (CPC0_UCR, reg);
#else /* CONFIG_405EP */
	reg = mfdcr(CPC0_CR0) & ~CR0_MASK;
#ifdef CONFIG_SYS_EXT_SERIAL_CLOCK
	clk = CONFIG_SYS_EXT_SERIAL_CLOCK;
	udiv = 1;
	reg |= CR0_EXTCLK_ENA;
#else
	clk = gd->cpu_clk;
#ifdef CONFIG_SYS_405_UART_ERRATA_59
	udiv = 31;			/* Errata 59: stuck at 31 */
#else
	tmp = CONFIG_SYS_BASE_BAUD * 16;
	udiv = (clk + tmp / 2) / tmp;
	if (udiv > UDIV_MAX)                    /* max. n bits for udiv */
		udiv = UDIV_MAX;
#endif
#endif
	reg |= (udiv - 1) << CR0_UDIV_POS;	/* set the UART divisor */
	mtdcr (CPC0_CR0, reg);
#endif /* CONFIG_405EP */
	tmp = gd->baudrate * udiv * 16;
	bdiv = (clk + tmp / 2) / tmp;
#endif /* CONFIG_405EZ */

	uart_post_init_common(com_port, bdiv);

	return 0;
}
#endif /* CONFIG_440 */

static void uart_post_putc (struct NS16550 *com_port, char c)
{
	int i;

	out_8(&com_port->thr, c);	/* put character out */

	/* Wait for transfer completion */
	for (i = 0; i < 3500; i++) {
		if (in_8(&com_port->lsr) & UART_LSR_THRE)
			break;
		udelay (100);
	}
}

static int uart_post_getc (struct NS16550 *com_port)
{
	int i;

	/* Wait for character available */
	for (i = 0; i < 3500; i++) {
		if (in_8(&com_port->lsr) & UART_LSR_DR)
			break;
		udelay (100);
	}

	return 0xff & in_8(&com_port->rbr);
}

static int test_ctlr (struct NS16550 *com_port, int index)
{
	int res = -1;
	char test_str[] = "*** UART Test String ***\r\n";
	int i;

	uart_post_init (com_port);

	for (i = 0; i < sizeof (test_str) - 1; i++) {
		uart_post_putc (com_port, test_str[i]);
		if (uart_post_getc (com_port) != test_str[i])
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
	static unsigned long base[] = CONFIG_SYS_POST_UART_TABLE;

	for (i = 0; i < ARRAY_SIZE(base); i++) {
		if (test_ctlr((struct NS16550 *)base[i], i))
			res = -1;
	}
	serial_reinit_all ();

	return res;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_UART */
