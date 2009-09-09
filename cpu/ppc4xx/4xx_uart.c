/*
 * (C) Copyright 2000-2006
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

/*
 * This source code is dual-licensed.  You may use it under the terms of the
 * GNU General Public License version 2, or under the license below.
 *
 * This source code has been made available to you by IBM on an AS-IS
 * basis.  Anyone receiving this source is licensed under IBM
 * copyrights to use it in any way he or she deems fit, including
 * copying it, modifying it, compiling it, and redistributing it either
 * with or without modifications.  No license under IBM patents or
 * patent applications is to be implied by the copyright license.
 *
 * Any user of this software should understand that IBM cannot provide
 * technical support for this software and will not be responsible for
 * any consequences resulting from the use of this software.
 *
 * Any person who transfers this source code or any derivative work
 * must include the IBM copyright notice, this paragraph, and the
 * preceding two paragraphs in the transferred software.
 *
 * COPYRIGHT   I B M   CORPORATION 1995
 * LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
 */

#include <common.h>
#include <commproc.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <watchdog.h>
#include <ppc4xx.h>

#ifdef CONFIG_SERIAL_MULTI
#include <serial.h>
#endif

#ifdef CONFIG_SERIAL_SOFTWARE_FIFO
#include <malloc.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_405GP) || defined(CONFIG_405CR) || \
    defined(CONFIG_405EP) || defined(CONFIG_405EZ) || \
    defined(CONFIG_405EX) || defined(CONFIG_440)

#if defined(CONFIG_440)
#if defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define UART0_BASE	(CONFIG_SYS_PERIPHERAL_BASE + 0x00000300)
#define UART1_BASE	(CONFIG_SYS_PERIPHERAL_BASE + 0x00000400)
#else
#define UART0_BASE	(CONFIG_SYS_PERIPHERAL_BASE + 0x00000200)
#define UART1_BASE	(CONFIG_SYS_PERIPHERAL_BASE + 0x00000300)
#endif

#if defined(CONFIG_440SP) || defined(CONFIG_440SPE)
#define UART2_BASE	(CONFIG_SYS_PERIPHERAL_BASE + 0x00000600)
#endif

#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define UART2_BASE	(CONFIG_SYS_PERIPHERAL_BASE + 0x00000500)
#define UART3_BASE	(CONFIG_SYS_PERIPHERAL_BASE + 0x00000600)
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
    defined(CONFIG_440SP) || defined(CONFIG_440SPE) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define UART2_SDR	SDR0_UART2
#endif
#if defined(CONFIG_440EP) || defined(CONFIG_440EPX) || \
    defined(CONFIG_440GR) || defined(CONFIG_440GRX) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
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
#else /* CONFIG_405GP || CONFIG_405CR */
#define UART0_BASE      0xef600300
#define UART1_BASE      0xef600400
#define CR0_MASK        0x00001fff
#define CR0_EXTCLK_ENA  0x000000c0
#define CR0_UDIV_POS    1
#define UDIV_MAX        32
#endif

/* using serial port 0 or 1 as U-Boot console ? */
#if defined(CONFIG_UART1_CONSOLE)
#define ACTING_UART0_BASE	UART1_BASE
#define ACTING_UART1_BASE	UART0_BASE
#else
#define ACTING_UART0_BASE	UART0_BASE
#define ACTING_UART1_BASE	UART1_BASE
#endif

#if defined(CONFIG_405EP) && defined(CONFIG_SYS_EXT_SERIAL_CLOCK)
#error "External serial clock not supported on AMCC PPC405EP!"
#endif

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

/*-----------------------------------------------------------------------------+
  | Line Status Register.
  +-----------------------------------------------------------------------------*/
#define asyncLSRDataReady1            0x01
#define asyncLSROverrunError1         0x02
#define asyncLSRParityError1          0x04
#define asyncLSRFramingError1         0x08
#define asyncLSRBreakInterrupt1       0x10
#define asyncLSRTxHoldEmpty1          0x20
#define asyncLSRTxShiftEmpty1         0x40
#define asyncLSRRxFifoError1          0x80

#ifdef CONFIG_SERIAL_SOFTWARE_FIFO
/*-----------------------------------------------------------------------------+
  | Fifo
  +-----------------------------------------------------------------------------*/
typedef struct {
	char *rx_buffer;
	ulong rx_put;
	ulong rx_get;
} serial_buffer_t;

volatile static serial_buffer_t buf_info;
#endif

static void serial_init_common(u32 base, u32 udiv, u16 bdiv)
{
	PPC4xx_SYS_INFO sys_info;
	u8 val;

	get_sys_info(&sys_info);

	/* Correct UART frequency in bd-info struct now that
	 * the UART divisor is available
	 */
#ifdef CONFIG_SYS_EXT_SERIAL_CLOCK
	gd->uart_clk = CONFIG_SYS_EXT_SERIAL_CLOCK;
#else
	gd->uart_clk = sys_info.freqUART / udiv;
#endif

	out_8((u8 *)base + UART_LCR, 0x80);	/* set DLAB bit */
	out_8((u8 *)base + UART_DLL, bdiv);	/* set baudrate divisor */
	out_8((u8 *)base + UART_DLM, bdiv >> 8); /* set baudrate divisor */
	out_8((u8 *)base + UART_LCR, 0x03);	/* clear DLAB; set 8 bits, no parity */
	out_8((u8 *)base + UART_FCR, 0x00);	/* disable FIFO */
	out_8((u8 *)base + UART_MCR, 0x00);	/* no modem control DTR RTS */
	val = in_8((u8 *)base + UART_LSR);	/* clear line status */
	val = in_8((u8 *)base + UART_RBR);	/* read receive buffer */
	out_8((u8 *)base + UART_SCR, 0x00);	/* set scratchpad */
	out_8((u8 *)base + UART_IER, 0x00);	/* set interrupt enable reg */
}

#if (defined(CONFIG_440) || defined(CONFIG_405EX)) &&	\
    !defined(CONFIG_SYS_EXT_SERIAL_CLOCK)
static void serial_divs (int baudrate, unsigned long *pudiv,
			 unsigned short *pbdiv)
{
	sys_info_t sysinfo;
	unsigned long div;		/* total divisor udiv * bdiv */
	unsigned long umin;		/* minimum udiv	*/
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
			break;      /* can't do better */
		} else if (idiff < diff) {
			udiv = i;       /* best so far */
			diff = idiff;   /* update lowest diff*/
		}
	}

	*pudiv = udiv;
	*pbdiv = div / udiv;
}

#elif defined(CONFIG_405EZ)

static void serial_divs (int baudrate, unsigned long *pudiv,
			 unsigned short *pbdiv)
{
	sys_info_t sysinfo;
	unsigned long div;		/* total divisor udiv * bdiv */
	unsigned long umin;		/* minimum udiv	*/
	unsigned short diff;		/* smallest diff */
	unsigned long udiv;		/* best udiv */
	unsigned short idiff;		/* current diff */
	unsigned short ibdiv;		/* current bdiv */
	unsigned long i;
	unsigned long est;		/* current estimate */
	unsigned long plloutb;
	unsigned long cpr_pllc;
	u32 reg;

	/* check the pll feedback source */
	mfcpr(CPR0_PLLC, cpr_pllc);

	get_sys_info(&sysinfo);

	plloutb = ((CONFIG_SYS_CLK_FREQ * ((cpr_pllc & PLLC_SRC_MASK) ?
					   sysinfo.pllFwdDivB : sysinfo.pllFwdDiv) *
		    sysinfo.pllFbkDiv) / sysinfo.pllFwdDivB);
	udiv = 256;			/* Assume lowest possible serial clk */
	div = plloutb / (16 * baudrate); /* total divisor */
	umin = (plloutb / get_OPB_freq()) << 1;	/* 2 x OPB divisor */
	diff = 256;			/* highest possible */

	/* i is the test udiv value -- start with the largest
	 * possible (256) to minimize serial clock and constrain
	 * search to umin.
	 */
	for (i = 256; i > umin; i--) {
		ibdiv = div / i;
		est = i * ibdiv;
		idiff = (est > div) ? (est-div) : (div-est);
		if (idiff == 0) {
			udiv = i;
			break;      /* can't do better */
		} else if (idiff < diff) {
			udiv = i;       /* best so far */
			diff = idiff;   /* update lowest diff*/
		}
	}

	*pudiv = udiv;
	mfcpr(CPC0_PERD0, reg);
	reg &= ~0x0000ffff;
	reg |= ((udiv - 0) << 8) | (udiv - 0);
	mtcpr(CPC0_PERD0, reg);
	*pbdiv = div / udiv;
}
#endif /* defined(CONFIG_440) && !defined(CONFIG_SYS_EXT_SERIAL_CLK) */

/*
 * Minimal serial functions needed to use one of the SMC ports
 * as serial console interface.
 */

#if defined(CONFIG_440)
int serial_init_dev(unsigned long base)
{
	unsigned long reg;
	unsigned long udiv;
	unsigned short bdiv;
#ifdef CONFIG_SYS_EXT_SERIAL_CLOCK
	unsigned long tmp;
#endif

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

	serial_init_common(base, udiv, bdiv);

	return (0);
}

#else /* !defined(CONFIG_440) */

int serial_init_dev (unsigned long base)
{
	unsigned long reg;
	unsigned long tmp;
	unsigned long clk;
	unsigned long udiv;
	unsigned short bdiv;

#ifdef CONFIG_405EX
	clk = tmp = 0;
	mfsdr(UART0_SDR, reg);
	reg &= ~CR0_MASK;
#ifdef CONFIG_SYS_EXT_SERIAL_CLOCK
	reg |= CR0_EXTCLK_ENA;
	udiv = 1;
	tmp  = gd->baudrate * 16;
	bdiv = (CONFIG_SYS_EXT_SERIAL_CLOCK + tmp / 2) / tmp;
#else
	serial_divs(gd->baudrate, &udiv, &bdiv);
#endif
	reg |= (udiv - UDIV_SUBTRACT) << CR0_UDIV_POS;  /* set the UART divisor */

	/*
	 * Configure input clock to baudrate generator for all
	 * available serial ports here
	 */
	mtsdr(UART0_SDR, reg);

#if defined(UART1_SDR)
	mtsdr(UART1_SDR, reg);
#endif

#elif defined(CONFIG_405EZ)
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
#endif /* CONFIG_405EX */

	serial_init_common(base, udiv, bdiv);

	return (0);
}

#endif /* if defined(CONFIG_440) */

void serial_setbrg_dev(unsigned long base)
{
	serial_init_dev(base);
}

void serial_putc_dev(unsigned long base, const char c)
{
	int i;

	if (c == '\n')
		serial_putc_dev(base, '\r');

	/* check THRE bit, wait for transmiter available */
	for (i = 1; i < 3500; i++) {
		if ((in_8((u8 *)base + UART_LSR) & 0x20) == 0x20)
			break;
		udelay (100);
	}

	out_8((u8 *)base + UART_THR, c);	/* put character out */
}

void serial_puts_dev (unsigned long base, const char *s)
{
	while (*s)
		serial_putc_dev (base, *s++);
}

int serial_getc_dev (unsigned long base)
{
	unsigned char status = 0;

	while (1) {
#if defined(CONFIG_HW_WATCHDOG)
		WATCHDOG_RESET ();	/* Reset HW Watchdog, if needed */
#endif	/* CONFIG_HW_WATCHDOG */

		status = in_8((u8 *)base + UART_LSR);
		if ((status & asyncLSRDataReady1) != 0x0)
			break;

		if ((status & ( asyncLSRFramingError1 |
				asyncLSROverrunError1 |
				asyncLSRParityError1  |
				asyncLSRBreakInterrupt1 )) != 0) {
			out_8((u8 *)base + UART_LSR,
			      asyncLSRFramingError1 |
			      asyncLSROverrunError1 |
			      asyncLSRParityError1  |
			      asyncLSRBreakInterrupt1);
		}
	}

	return (0x000000ff & (int) in_8((u8 *)base));
}

int serial_tstc_dev (unsigned long base)
{
	unsigned char status;

	status = in_8((u8 *)base + UART_LSR);
	if ((status & asyncLSRDataReady1) != 0x0)
		return (1);

	if ((status & ( asyncLSRFramingError1 |
			asyncLSROverrunError1 |
			asyncLSRParityError1  |
			asyncLSRBreakInterrupt1 )) != 0) {
		out_8((u8 *)base + UART_LSR,
		      asyncLSRFramingError1 |
		      asyncLSROverrunError1 |
		      asyncLSRParityError1  |
		      asyncLSRBreakInterrupt1);
	}

	return 0;
}

#ifdef CONFIG_SERIAL_SOFTWARE_FIFO

void serial_isr (void *arg)
{
	int space;
	int c;
	const int rx_get = buf_info.rx_get;
	int rx_put = buf_info.rx_put;

	if (rx_get <= rx_put)
		space = CONFIG_SERIAL_SOFTWARE_FIFO - (rx_put - rx_get);
	else
		space = rx_get - rx_put;

	while (serial_tstc_dev (ACTING_UART0_BASE)) {
		c = serial_getc_dev (ACTING_UART0_BASE);
		if (space) {
			buf_info.rx_buffer[rx_put++] = c;
			space--;
		}
		if (rx_put == CONFIG_SERIAL_SOFTWARE_FIFO)
			rx_put = 0;
		if (space < CONFIG_SERIAL_SOFTWARE_FIFO / 4) {
			/* Stop flow by setting RTS inactive */
			out_8((u8 *)ACTING_UART0_BASE + UART_MCR,
			      in_8((u8 *)ACTING_UART0_BASE + UART_MCR) &
			      (0xFF ^ 0x02));
		}
	}
	buf_info.rx_put = rx_put;
}

void serial_buffered_init (void)
{
	serial_puts ("Switching to interrupt driven serial input mode.\n");
	buf_info.rx_buffer = malloc (CONFIG_SERIAL_SOFTWARE_FIFO);
	buf_info.rx_put = 0;
	buf_info.rx_get = 0;

	if (in_8((u8 *)ACTING_UART0_BASE + UART_MSR) & 0x10)
		serial_puts ("Check CTS signal present on serial port: OK.\n");
	else
		serial_puts ("WARNING: CTS signal not present on serial port.\n");

	irq_install_handler ( VECNUM_U0 /*UART0 */ /*int vec */ ,
			      serial_isr /*interrupt_handler_t *handler */ ,
			      (void *) &buf_info /*void *arg */ );

	/* Enable "RX Data Available" Interrupt on UART */
	out_8(ACTING_UART0_BASE + UART_IER, 0x01);
	/* Set DTR active */
	out_8(ACTING_UART0_BASE + UART_MCR,
	      in_8((u8 *)ACTING_UART0_BASE + UART_MCR) | 0x01);
	/* Start flow by setting RTS active */
	out_8(ACTING_UART0_BASE + UART_MCR,
	      in_8((u8 *)ACTING_UART0_BASE + UART_MCR) | 0x02);
	/* Setup UART FIFO: RX trigger level: 4 byte, Enable FIFO */
	out_8(ACTING_UART0_BASE + UART_FCR, (1 << 6) | 1);
}

void serial_buffered_putc (const char c)
{
	/* Wait for CTS */
#if defined(CONFIG_HW_WATCHDOG)
	while (!(in_8((u8 *)ACTING_UART0_BASE + UART_MSR) & 0x10))
		WATCHDOG_RESET ();
#else
	while (!(in_8((u8 *)ACTING_UART0_BASE + UART_MSR) & 0x10));
#endif
	serial_putc (c);
}

void serial_buffered_puts (const char *s)
{
	serial_puts (s);
}

int serial_buffered_getc (void)
{
	int space;
	int c;
	int rx_get = buf_info.rx_get;
	int rx_put;

#if defined(CONFIG_HW_WATCHDOG)
	while (rx_get == buf_info.rx_put)
		WATCHDOG_RESET ();
#else
	while (rx_get == buf_info.rx_put);
#endif
	c = buf_info.rx_buffer[rx_get++];
	if (rx_get == CONFIG_SERIAL_SOFTWARE_FIFO)
		rx_get = 0;
	buf_info.rx_get = rx_get;

	rx_put = buf_info.rx_put;
	if (rx_get <= rx_put)
		space = CONFIG_SERIAL_SOFTWARE_FIFO - (rx_put - rx_get);
	else
		space = rx_get - rx_put;

	if (space > CONFIG_SERIAL_SOFTWARE_FIFO / 2) {
		/* Start flow by setting RTS active */
		out_8(ACTING_UART0_BASE + UART_MCR,
		      in_8((u8 *)ACTING_UART0_BASE + UART_MCR) | 0x02);
	}

	return c;
}

int serial_buffered_tstc (void)
{
	return (buf_info.rx_get != buf_info.rx_put) ? 1 : 0;
}

#endif	/* CONFIG_SERIAL_SOFTWARE_FIFO */

#if defined(CONFIG_CMD_KGDB)
/*
  AS HARNOIS : according to CONFIG_KGDB_SER_INDEX kgdb uses serial port
  number 0 or number 1
  - if CONFIG_KGDB_SER_INDEX = 1 => serial port number 0 :
  configuration has been already done
  - if CONFIG_KGDB_SER_INDEX = 2 => serial port number 1 :
  configure port 1 for serial I/O with rate = CONFIG_KGDB_BAUDRATE
*/
#if (CONFIG_KGDB_SER_INDEX & 2)
void kgdb_serial_init (void)
{
	u8 val;
	u16 br_reg;

	get_clocks ();
	br_reg = (((((gd->cpu_clk / 16) / 18) * 10) / CONFIG_KGDB_BAUDRATE) +
		  5) / 10;
	/*
	 * Init onboard 16550 UART
	 */
	out_8((u8 *)ACTING_UART1_BASE + UART_LCR, 0x80);	/* set DLAB bit */
	out_8((u8 *)ACTING_UART1_BASE + UART_DLL, (br_reg & 0x00ff)); /* set divisor for 9600 baud */
	out_8((u8 *)ACTING_UART1_BASE + UART_DLM, ((br_reg & 0xff00) >> 8)); /* set divisor for 9600 baud */
	out_8((u8 *)ACTING_UART1_BASE + UART_LCR, 0x03);	/* line control 8 bits no parity */
	out_8((u8 *)ACTING_UART1_BASE + UART_FCR, 0x00);	/* disable FIFO */
	out_8((u8 *)ACTING_UART1_BASE + UART_MCR, 0x00);	/* no modem control DTR RTS */
	val = in_8((u8 *)ACTING_UART1_BASE + UART_LSR);		/* clear line status */
	val = in_8((u8 *)ACTING_UART1_BASE + UART_RBR);		/* read receive buffer */
	out_8((u8 *)ACTING_UART1_BASE + UART_SCR, 0x00);	/* set scratchpad */
	out_8((u8 *)ACTING_UART1_BASE + UART_IER, 0x00);	/* set interrupt enable reg */
}

void putDebugChar (const char c)
{
	if (c == '\n')
		serial_putc ('\r');

	out_8((u8 *)ACTING_UART1_BASE + UART_THR, c);	/* put character out */

	/* check THRE bit, wait for transfer done */
	while ((in_8((u8 *)ACTING_UART1_BASE + UART_LSR) & 0x20) != 0x20);
}

void putDebugStr (const char *s)
{
	while (*s)
		serial_putc (*s++);
}

int getDebugChar (void)
{
	unsigned char status = 0;

	while (1) {
		status = in_8((u8 *)ACTING_UART1_BASE + UART_LSR);
		if ((status & asyncLSRDataReady1) != 0x0)
			break;

		if ((status & (asyncLSRFramingError1 |
			       asyncLSROverrunError1 |
			       asyncLSRParityError1  |
			       asyncLSRBreakInterrupt1 )) != 0) {
			out_8((u8 *)ACTING_UART1_BASE + UART_LSR,
			      asyncLSRFramingError1 |
			      asyncLSROverrunError1 |
			      asyncLSRParityError1  |
			      asyncLSRBreakInterrupt1);
		}
	}

	return (0x000000ff & (int) in_8((u8 *)ACTING_UART1_BASE));
}

void kgdb_interruptible (int yes)
{
	return;
}

#else	/* ! (CONFIG_KGDB_SER_INDEX & 2) */

void kgdb_serial_init (void)
{
	serial_printf ("[on serial] ");
}

void putDebugChar (int c)
{
	serial_putc (c);
}

void putDebugStr (const char *str)
{
	serial_puts (str);
}

int getDebugChar (void)
{
	return serial_getc ();
}

void kgdb_interruptible (int yes)
{
	return;
}
#endif	/* (CONFIG_KGDB_SER_INDEX & 2) */
#endif


#if defined(CONFIG_SERIAL_MULTI)
int serial0_init(void)
{
	return (serial_init_dev(UART0_BASE));
}

int serial1_init(void)
{
	return (serial_init_dev(UART1_BASE));
}

void serial0_setbrg (void)
{
	serial_setbrg_dev(UART0_BASE);
}

void serial1_setbrg (void)
{
	serial_setbrg_dev(UART1_BASE);
}

void serial0_putc(const char c)
{
	serial_putc_dev(UART0_BASE,c);
}

void serial1_putc(const char c)
{
	serial_putc_dev(UART1_BASE, c);
}

void serial0_puts(const char *s)
{
	serial_puts_dev(UART0_BASE, s);
}

void serial1_puts(const char *s)
{
	serial_puts_dev(UART1_BASE, s);
}

int serial0_getc(void)
{
	return(serial_getc_dev(UART0_BASE));
}

int serial1_getc(void)
{
	return(serial_getc_dev(UART1_BASE));
}

int serial0_tstc(void)
{
	return (serial_tstc_dev(UART0_BASE));
}

int serial1_tstc(void)
{
	return (serial_tstc_dev(UART1_BASE));
}

struct serial_device serial0_device =
{
	"serial0",
	"UART0",
	serial0_init,
	serial0_setbrg,
	serial0_getc,
	serial0_tstc,
	serial0_putc,
	serial0_puts,
};

struct serial_device serial1_device =
{
	"serial1",
	"UART1",
	serial1_init,
	serial1_setbrg,
	serial1_getc,
	serial1_tstc,
	serial1_putc,
	serial1_puts,
};
#else
/*
 * Wrapper functions
 */
int serial_init(void)
{
	return serial_init_dev(ACTING_UART0_BASE);
}

void serial_setbrg(void)
{
	serial_setbrg_dev(ACTING_UART0_BASE);
}

void serial_putc(const char c)
{
	serial_putc_dev(ACTING_UART0_BASE, c);
}

void serial_puts(const char *s)
{
	serial_puts_dev(ACTING_UART0_BASE, s);
}

int serial_getc(void)
{
	return serial_getc_dev(ACTING_UART0_BASE);
}

int serial_tstc(void)
{
	return serial_tstc_dev(ACTING_UART0_BASE);
}
#endif /* CONFIG_SERIAL_MULTI */

#endif	/* CONFIG_405GP || CONFIG_405CR */
