/*
 * (C) Copyright 2000
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
/*------------------------------------------------------------------------------+ */
/*
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
/*------------------------------------------------------------------------------- */

#include <common.h>
#include <commproc.h>
#include <asm/processor.h>
#include <watchdog.h>
#include "vecnum.h"

#if CONFIG_SERIAL_SOFTWARE_FIFO
#include <malloc.h>
#endif

/*****************************************************************************/
#ifdef CONFIG_IOP480

#define SPU_BASE         0x40000000

#define spu_LineStat_rc  0x00	/* Line Status Register (Read/Clear) */
#define spu_LineStat_w   0x04	/* Line Status Register (Set) */
#define spu_Handshk_rc   0x08	/* Handshake Status Register (Read/Clear) */
#define spu_Handshk_w    0x0c	/* Handshake Status Register (Set) */
#define spu_BRateDivh    0x10	/* Baud rate divisor high */
#define spu_BRateDivl    0x14	/* Baud rate divisor low */
#define spu_CtlReg       0x18	/* Control Register */
#define spu_RxCmd        0x1c	/* Rx Command Register */
#define spu_TxCmd        0x20	/* Tx Command Register */
#define spu_RxBuff       0x24	/* Rx data buffer */
#define spu_TxBuff       0x24	/* Tx data buffer */

/*-----------------------------------------------------------------------------+
  | Line Status Register.
  +-----------------------------------------------------------------------------*/
#define asyncLSRport1           0x40000000
#define asyncLSRport1set        0x40000004
#define asyncLSRDataReady             0x80
#define asyncLSRFramingError          0x40
#define asyncLSROverrunError          0x20
#define asyncLSRParityError           0x10
#define asyncLSRBreakInterrupt        0x08
#define asyncLSRTxHoldEmpty           0x04
#define asyncLSRTxShiftEmpty          0x02

/*-----------------------------------------------------------------------------+
  | Handshake Status Register.
  +-----------------------------------------------------------------------------*/
#define asyncHSRport1           0x40000008
#define asyncHSRport1set        0x4000000c
#define asyncHSRDsr                   0x80
#define asyncLSRCts                   0x40

/*-----------------------------------------------------------------------------+
  | Control Register.
  +-----------------------------------------------------------------------------*/
#define asyncCRport1            0x40000018
#define asyncCRNormal                 0x00
#define asyncCRLoopback               0x40
#define asyncCRAutoEcho               0x80
#define asyncCRDtr                    0x20
#define asyncCRRts                    0x10
#define asyncCRWordLength7            0x00
#define asyncCRWordLength8            0x08
#define asyncCRParityDisable          0x00
#define asyncCRParityEnable           0x04
#define asyncCREvenParity             0x00
#define asyncCROddParity              0x02
#define asyncCRStopBitsOne            0x00
#define asyncCRStopBitsTwo            0x01
#define asyncCRDisableDtrRts          0x00

/*-----------------------------------------------------------------------------+
  | Receiver Command Register.
  +-----------------------------------------------------------------------------*/
#define asyncRCRport1           0x4000001c
#define asyncRCRDisable               0x00
#define asyncRCREnable                0x80
#define asyncRCRIntDisable            0x00
#define asyncRCRIntEnabled            0x20
#define asyncRCRDMACh2                0x40
#define asyncRCRDMACh3                0x60
#define asyncRCRErrorInt              0x10
#define asyncRCRPauseEnable           0x08

/*-----------------------------------------------------------------------------+
  | Transmitter Command Register.
  +-----------------------------------------------------------------------------*/
#define asyncTCRport1           0x40000020
#define asyncTCRDisable               0x00
#define asyncTCREnable                0x80
#define asyncTCRIntDisable            0x00
#define asyncTCRIntEnabled            0x20
#define asyncTCRDMACh2                0x40
#define asyncTCRDMACh3                0x60
#define asyncTCRTxEmpty               0x10
#define asyncTCRErrorInt              0x08
#define asyncTCRStopPause             0x04
#define asyncTCRBreakGen              0x02

/*-----------------------------------------------------------------------------+
  | Miscellanies defines.
  +-----------------------------------------------------------------------------*/
#define asyncTxBufferport1      0x40000024
#define asyncRxBufferport1      0x40000024
#define asyncDLABLsbport1       0x40000014
#define asyncDLABMsbport1       0x40000010
#define asyncXOFFchar                 0x13
#define asyncXONchar                  0x11


/*
 * Minimal serial functions needed to use one of the SMC ports
 * as serial console interface.
 */

int serial_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	volatile char val;
	unsigned short br_reg;

	br_reg = ((((CONFIG_CPUCLOCK * 1000000) / 16) / gd->baudrate) - 1);

	/*
	 * Init onboard UART
	 */
	out8 (SPU_BASE + spu_LineStat_rc, 0x78); /* Clear all bits in Line Status Reg */
	out8 (SPU_BASE + spu_BRateDivl, (br_reg & 0x00ff)); /* Set baud rate divisor... */
	out8 (SPU_BASE + spu_BRateDivh, ((br_reg & 0xff00) >> 8)); /* ... */
	out8 (SPU_BASE + spu_CtlReg, 0x08);	/* Set 8 bits, no parity and 1 stop bit */
	out8 (SPU_BASE + spu_RxCmd, 0xb0);	/* Enable Rx */
	out8 (SPU_BASE + spu_TxCmd, 0x9c);	/* Enable Tx */
	out8 (SPU_BASE + spu_Handshk_rc, 0xff);	/* Clear Handshake */
	val = in8 (SPU_BASE + spu_RxBuff);	/* Dummy read, to clear receiver */

	return (0);
}


void serial_setbrg (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	unsigned short br_reg;

	br_reg = ((((CONFIG_CPUCLOCK * 1000000) / 16) / gd->baudrate) - 1);

	out8 (SPU_BASE + spu_BRateDivl, (br_reg & 0x00ff)); /* Set baud rate divisor... */
	out8 (SPU_BASE + spu_BRateDivh, ((br_reg & 0xff00) >> 8)); /* ... */
}


void serial_putc (const char c)
{
	if (c == '\n')
		serial_putc ('\r');

	/* load status from handshake register */
	if (in8 (SPU_BASE + spu_Handshk_rc) != 00)
		out8 (SPU_BASE + spu_Handshk_rc, 0xff);	/* Clear Handshake */

	out8 (SPU_BASE + spu_TxBuff, c);	/* Put char */

	while ((in8 (SPU_BASE + spu_LineStat_rc) & 04) != 04) {
		if (in8 (SPU_BASE + spu_Handshk_rc) != 00)
			out8 (SPU_BASE + spu_Handshk_rc, 0xff);	/* Clear Handshake */
	}
}


void serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}


int serial_getc ()
{
	unsigned char status = 0;

	while (1) {
		status = in8 (asyncLSRport1);
		if ((status & asyncLSRDataReady) != 0x0) {
			break;
		}
		if ((status & ( asyncLSRFramingError |
				asyncLSROverrunError |
				asyncLSRParityError  |
				asyncLSRBreakInterrupt )) != 0) {
			(void) out8 (asyncLSRport1,
				     asyncLSRFramingError |
				     asyncLSROverrunError |
				     asyncLSRParityError  |
				     asyncLSRBreakInterrupt );
		}
	}
	return (0x000000ff & (int) in8 (asyncRxBufferport1));
}


int serial_tstc ()
{
	unsigned char status;

	status = in8 (asyncLSRport1);
	if ((status & asyncLSRDataReady) != 0x0) {
		return (1);
	}
	if ((status & ( asyncLSRFramingError |
			asyncLSROverrunError |
			asyncLSRParityError  |
			asyncLSRBreakInterrupt )) != 0) {
		(void) out8 (asyncLSRport1,
			     asyncLSRFramingError |
			     asyncLSROverrunError |
			     asyncLSRParityError  |
			     asyncLSRBreakInterrupt);
	}
	return 0;
}

#endif	/* CONFIG_IOP480 */


/*****************************************************************************/
#if defined(CONFIG_405GP) || defined(CONFIG_405CR) || defined(CONFIG_440)

#if defined(CONFIG_440)
#define UART0_BASE  CFG_PERIPHERAL_BASE + 0x00000200
#define UART1_BASE  CFG_PERIPHERAL_BASE + 0x00000300
#define CR0_MASK        0x3fff0000
#define CR0_EXTCLK_ENA  0x00600000
#define CR0_UDIV_POS    16
#else
#define UART_BASE_PTR   0xF800FFFC;	/* pointer to uart base */
#define UART0_BASE      0xef600300
#define UART1_BASE      0xef600400
#define CR0_MASK        0x00001fff
#define CR0_EXTCLK_ENA  0x00000c00
#define CR0_UDIV_POS    1
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
/*#define asyncLSRport1           UART0_BASE+0x05 */
#define asyncLSRDataReady1            0x01
#define asyncLSROverrunError1         0x02
#define asyncLSRParityError1          0x04
#define asyncLSRFramingError1         0x08
#define asyncLSRBreakInterrupt1       0x10
#define asyncLSRTxHoldEmpty1          0x20
#define asyncLSRTxShiftEmpty1         0x40
#define asyncLSRRxFifoError1          0x80

/*-----------------------------------------------------------------------------+
  | Miscellanies defines.
  +-----------------------------------------------------------------------------*/
/*#define asyncTxBufferport1      UART0_BASE+0x00 */
/*#define asyncRxBufferport1      UART0_BASE+0x00 */


#if CONFIG_SERIAL_SOFTWARE_FIFO
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


#if defined(CONFIG_440) && !defined(CFG_EXT_SERIAL_CLOCK)
static void serial_divs (int baudrate, unsigned long *pudiv,
			 unsigned short *pbdiv )
{
	sys_info_t	sysinfo;
	unsigned long div;		/* total divisor udiv * bdiv */
	unsigned long umin;		/* minimum udiv	*/
	unsigned short diff;    /* smallest diff */
	unsigned long udiv;     /* best udiv */

	unsigned short idiff;   /* current diff */
	unsigned short ibdiv;   /* current bdiv */
	unsigned long i;
	unsigned long est;      /* current estimate */

	get_sys_info( &sysinfo );

	udiv = 32;     /* Assume lowest possible serial clk */
	div = sysinfo.freqPLB/(16*baudrate); /* total divisor */
	umin = sysinfo.pllOpbDiv<<1; /* 2 x OPB divisor */
	diff = 32;      /* highest possible */

	/* i is the test udiv value -- start with the largest
	 * possible (32) to minimize serial clock and constrain
	 * search to umin.
	 */
	for( i = 32; i > umin; i-- ){
		ibdiv = div/i;
		est = i * ibdiv;
		idiff = (est > div) ? (est-div) : (div-est);
		if( idiff == 0 ){
			udiv = i;
			break;      /* can't do better */
		}
		else if( idiff < diff ){
			udiv = i;       /* best so far */
			diff = idiff;   /* update lowest diff*/
		}
	}

	*pudiv = udiv;
	*pbdiv = div/udiv;

}
#endif /* defined(CONFIG_440) && !defined(CFG_EXT_SERIAL_CLK */


/*
 * Minimal serial functions needed to use one of the SMC ports
 * as serial console interface.
 */

#if defined(CONFIG_440)
int serial_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	unsigned long reg;
	unsigned long udiv;
	unsigned short bdiv;
	volatile char val;
#ifdef CFG_EXT_SERIAL_CLOCK
	unsigned long tmp;
#endif

	reg = mfdcr(cntrl0) & ~CR0_MASK;
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

	reg |= (udiv - 1) << CR0_UDIV_POS;	/* set the UART divisor */
	mtdcr (cntrl0, reg);

	out8 (UART0_BASE + UART_LCR, 0x80);	/* set DLAB bit */
	out8 (UART0_BASE + UART_DLL, bdiv);	/* set baudrate divisor */
	out8 (UART0_BASE + UART_DLM, bdiv >> 8);/* set baudrate divisor */
	out8 (UART0_BASE + UART_LCR, 0x03);	/* clear DLAB; set 8 bits, no parity */
	out8 (UART0_BASE + UART_FCR, 0x00);	/* disable FIFO */
	out8 (UART0_BASE + UART_MCR, 0x00);	/* no modem control DTR RTS */
	val = in8 (UART0_BASE + UART_LSR);	/* clear line status */
	val = in8 (UART0_BASE + UART_RBR);	/* read receive buffer */
	out8 (UART0_BASE + UART_SCR, 0x00);	/* set scratchpad */
	out8 (UART0_BASE + UART_IER, 0x00);	/* set interrupt enable reg */

	return (0);
}

#else /* !defined(CONFIG_440) */

int serial_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	unsigned long reg;
	unsigned long tmp;
	unsigned long clk;
	unsigned long udiv;
	unsigned short bdiv;
	volatile char val;

	reg = mfdcr(cntrl0) & ~CR0_MASK;
#ifdef CFG_EXT_SERIAL_CLOCK
	clk = CFG_EXT_SERIAL_CLOCK;
	udiv = 1;
	reg |= CR0_EXTCLK_ENA;
#else
	clk = gd->cpu_clk;
#ifdef CFG_405_UART_ERRATA_59
	udiv = 31;			/* Errata 59: stuck at 31 */
#else
	tmp = CFG_BASE_BAUD * 16;
	udiv = (clk + tmp / 2) / tmp;
#endif
#endif

	reg |= (udiv - 1) << CR0_UDIV_POS;	/* set the UART divisor */
	mtdcr (cntrl0, reg);

	tmp = gd->baudrate * udiv * 16;
	bdiv = (clk + tmp / 2) / tmp;

	out8 (UART0_BASE + UART_LCR, 0x80);	/* set DLAB bit */
	out8 (UART0_BASE + UART_DLL, bdiv);	/* set baudrate divisor */
	out8 (UART0_BASE + UART_DLM, bdiv >> 8);/* set baudrate divisor */
	out8 (UART0_BASE + UART_LCR, 0x03);	/* clear DLAB; set 8 bits, no parity */
	out8 (UART0_BASE + UART_FCR, 0x00);	/* disable FIFO */
	out8 (UART0_BASE + UART_MCR, 0x00);	/* no modem control DTR RTS */
	val = in8 (UART0_BASE + UART_LSR);	/* clear line status */
	val = in8 (UART0_BASE + UART_RBR);	/* read receive buffer */
	out8 (UART0_BASE + UART_SCR, 0x00);	/* set scratchpad */
	out8 (UART0_BASE + UART_IER, 0x00);	/* set interrupt enable reg */

	return (0);
}

#endif /* if defined(CONFIG_440) */

void serial_setbrg (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	unsigned long tmp;
	unsigned long clk;
	unsigned long udiv;
	unsigned short bdiv;

#ifdef CFG_EXT_SERIAL_CLOCK
	clk = CFG_EXT_SERIAL_CLOCK;
#else
	clk = gd->cpu_clk;
#endif
	udiv = ((mfdcr (cntrl0) & 0x3e) >> 1) + 1;
	tmp = gd->baudrate * udiv * 16;
	bdiv = (clk + tmp / 2) / tmp;

	out8 (UART0_BASE + UART_LCR, 0x80);	/* set DLAB bit */
	out8 (UART0_BASE + UART_DLL, bdiv);	/* set baudrate divisor */
	out8 (UART0_BASE + UART_DLM, bdiv >> 8);/* set baudrate divisor */
	out8 (UART0_BASE + UART_LCR, 0x03);	/* clear DLAB; set 8 bits, no parity */
}


void serial_putc (const char c)
{
	int i;

	if (c == '\n')
		serial_putc ('\r');

	/* check THRE bit, wait for transmiter available */
	for (i = 1; i < 3500; i++) {
		if ((in8 (UART0_BASE + UART_LSR) & 0x20) == 0x20)
			break;
		udelay (100);
	}
	out8 (UART0_BASE + UART_THR, c);	/* put character out */
}


void serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}


int serial_getc ()
{
	unsigned char status = 0;

	while (1) {
#if defined(CONFIG_HW_WATCHDOG)
		WATCHDOG_RESET ();	/* Reset HW Watchdog, if needed */
#endif	/* CONFIG_HW_WATCHDOG */
		status = in8 (UART0_BASE + UART_LSR);
		if ((status & asyncLSRDataReady1) != 0x0) {
			break;
		}
		if ((status & ( asyncLSRFramingError1 |
				asyncLSROverrunError1 |
				asyncLSRParityError1  |
				asyncLSRBreakInterrupt1 )) != 0) {
			out8 (UART0_BASE + UART_LSR,
			      asyncLSRFramingError1 |
			      asyncLSROverrunError1 |
			      asyncLSRParityError1  |
			      asyncLSRBreakInterrupt1);
		}
	}
	return (0x000000ff & (int) in8 (UART0_BASE));
}


int serial_tstc ()
{
	unsigned char status;

	status = in8 (UART0_BASE + UART_LSR);
	if ((status & asyncLSRDataReady1) != 0x0) {
		return (1);
	}
	if ((status & ( asyncLSRFramingError1 |
			asyncLSROverrunError1 |
			asyncLSRParityError1  |
			asyncLSRBreakInterrupt1 )) != 0) {
		out8 (UART0_BASE + UART_LSR,
		      asyncLSRFramingError1 |
		      asyncLSROverrunError1 |
		      asyncLSRParityError1  |
		      asyncLSRBreakInterrupt1);
	}
	return 0;
}


#if CONFIG_SERIAL_SOFTWARE_FIFO

void serial_isr (void *arg)
{
	int space;
	int c;
	const int rx_get = buf_info.rx_get;
	int rx_put = buf_info.rx_put;

	if (rx_get <= rx_put) {
		space = CONFIG_SERIAL_SOFTWARE_FIFO - (rx_put - rx_get);
	} else {
		space = rx_get - rx_put;
	}
	while (serial_tstc ()) {
		c = serial_getc ();
		if (space) {
			buf_info.rx_buffer[rx_put++] = c;
			space--;
		}
		if (rx_put == CONFIG_SERIAL_SOFTWARE_FIFO)
			rx_put = 0;
		if (space < CONFIG_SERIAL_SOFTWARE_FIFO / 4) {
			/* Stop flow by setting RTS inactive */
			out8 (UART0_BASE + UART_MCR,
			      in8 (UART0_BASE + UART_MCR) & (0xFF ^ 0x02));
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

	if (in8 (UART0_BASE + UART_MSR) & 0x10) {
		serial_puts ("Check CTS signal present on serial port: OK.\n");
	} else {
		serial_puts ("WARNING: CTS signal not present on serial port.\n");
	}

	irq_install_handler ( VECNUM_U0 /*UART0 */ /*int vec */ ,
			      serial_isr /*interrupt_handler_t *handler */ ,
			      (void *) &buf_info /*void *arg */ );

	/* Enable "RX Data Available" Interrupt on UART */
	/* out8(UART0_BASE + UART_IER, in8(UART0_BASE + UART_IER) |0x01); */
	out8 (UART0_BASE + UART_IER, 0x01);
	/* Set DTR active */
	out8 (UART0_BASE + UART_MCR, in8 (UART0_BASE + UART_MCR) | 0x01);
	/* Start flow by setting RTS active */
	out8 (UART0_BASE + UART_MCR, in8 (UART0_BASE + UART_MCR) | 0x02);
	/* Setup UART FIFO: RX trigger level: 4 byte, Enable FIFO */
	out8 (UART0_BASE + UART_FCR, (1 << 6) | 1);
}

void serial_buffered_putc (const char c)
{
	/* Wait for CTS */
#if defined(CONFIG_HW_WATCHDOG)
	while (!(in8 (UART0_BASE + UART_MSR) & 0x10))
		WATCHDOG_RESET ();
#else
	while (!(in8 (UART0_BASE + UART_MSR) & 0x10));
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
	if (rx_get <= rx_put) {
		space = CONFIG_SERIAL_SOFTWARE_FIFO - (rx_put - rx_get);
	} else {
		space = rx_get - rx_put;
	}
	if (space > CONFIG_SERIAL_SOFTWARE_FIFO / 2) {
		/* Start flow by setting RTS active */
		out8 (UART0_BASE + UART_MCR, in8 (UART0_BASE + UART_MCR) | 0x02);
	}

	return c;
}

int serial_buffered_tstc (void)
{
	return (buf_info.rx_get != buf_info.rx_put) ? 1 : 0;
}

#endif	/* CONFIG_SERIAL_SOFTWARE_FIFO */


#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
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
	DECLARE_GLOBAL_DATA_PTR;

	volatile char val;
	unsigned short br_reg;

	get_clocks ();
	br_reg = (((((gd->cpu_clk / 16) / 18) * 10) / CONFIG_KGDB_BAUDRATE) +
		  5) / 10;
	/*
	 * Init onboard 16550 UART
	 */
	out8 (UART1_BASE + UART_LCR, 0x80);	/* set DLAB bit */
	out8 (UART1_BASE + UART_DLL, (br_reg & 0x00ff));	/* set divisor for 9600 baud */
	out8 (UART1_BASE + UART_DLM, ((br_reg & 0xff00) >> 8));	/* set divisor for 9600 baud */
	out8 (UART1_BASE + UART_LCR, 0x03);	/* line control 8 bits no parity */
	out8 (UART1_BASE + UART_FCR, 0x00);	/* disable FIFO */
	out8 (UART1_BASE + UART_MCR, 0x00);	/* no modem control DTR RTS */
	val = in8 (UART1_BASE + UART_LSR);	/* clear line status */
	val = in8 (UART1_BASE + UART_RBR);	/* read receive buffer */
	out8 (UART1_BASE + UART_SCR, 0x00);	/* set scratchpad */
	out8 (UART1_BASE + UART_IER, 0x00);	/* set interrupt enable reg */
}


void putDebugChar (const char c)
{
	if (c == '\n')
		serial_putc ('\r');

	out8 (UART1_BASE + UART_THR, c);	/* put character out */

	/* check THRE bit, wait for transfer done */
	while ((in8 (UART1_BASE + UART_LSR) & 0x20) != 0x20);
}


void putDebugStr (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}


int getDebugChar (void)
{
	unsigned char status = 0;

	while (1) {
		status = in8 (UART1_BASE + UART_LSR);
		if ((status & asyncLSRDataReady1) != 0x0) {
			break;
		}
		if ((status & ( asyncLSRFramingError1 |
				asyncLSROverrunError1 |
				asyncLSRParityError1  |
				asyncLSRBreakInterrupt1 )) != 0) {
			out8 (UART1_BASE + UART_LSR,
			      asyncLSRFramingError1 |
			      asyncLSROverrunError1 |
			      asyncLSRParityError1  |
			      asyncLSRBreakInterrupt1);
		}
	}
	return (0x000000ff & (int) in8 (UART1_BASE));
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
#endif	/* CFG_CMD_KGDB */

#endif	/* CONFIG_405GP || CONFIG_405CR */
