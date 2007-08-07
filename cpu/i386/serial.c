/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
 *
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
#include <watchdog.h>
#include <asm/io.h>
#include <asm/ibmpc.h>

#ifdef CONFIG_SERIAL_SOFTWARE_FIFO
#include <malloc.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

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
	int cts;
} serial_buffer_t;

volatile serial_buffer_t buf_info;
static int serial_buffer_active=0;
#endif


static int serial_div(int baudrate)
{

	switch (baudrate) {
	case 1200:
		return 96;
	case 9600:
		return 12;
	case 19200:
		return 6;
	case 38400:
		return 3;
	case 57600:
		return 2;
	case 115200:
		return 1;
	}

	return 12;
}


/*
 * Minimal serial functions needed to use one of the SMC ports
 * as serial console interface.
 */

int serial_init(void)
{
	volatile char val;
	int bdiv = serial_div(gd->baudrate);

	outb(0x80, UART0_BASE + UART_LCR);	/* set DLAB bit */
	outb(bdiv, UART0_BASE + UART_DLL);	/* set baudrate divisor */
	outb(bdiv >> 8, UART0_BASE + UART_DLM);/* set baudrate divisor */
	outb(0x03, UART0_BASE + UART_LCR);	/* clear DLAB; set 8 bits, no parity */
	outb(0x01, UART0_BASE + UART_FCR);	/* enable FIFO */
	outb(0x0b, UART0_BASE + UART_MCR);	/* Set DTR and RTS active */
	val = inb(UART0_BASE + UART_LSR);	/* clear line status */
	val = inb(UART0_BASE + UART_RBR);	/* read receive buffer */
	outb(0x00, UART0_BASE + UART_SCR);	/* set scratchpad */
	outb(0x00, UART0_BASE + UART_IER);	/* set interrupt enable reg */

	return 0;
}


void serial_setbrg(void)
{
	unsigned short bdiv;

	bdiv = serial_div(gd->baudrate);

	outb(0x80, UART0_BASE + UART_LCR);	/* set DLAB bit */
	outb(bdiv&0xff, UART0_BASE + UART_DLL);	/* set baudrate divisor */
	outb(bdiv >> 8, UART0_BASE + UART_DLM);/* set baudrate divisor */
	outb(0x03, UART0_BASE + UART_LCR);	/* clear DLAB; set 8 bits, no parity */
}


void serial_putc(const char c)
{
	int i;

	if (c == '\n')
		serial_putc ('\r');

	/* check THRE bit, wait for transmiter available */
	for (i = 1; i < 3500; i++) {
		if ((inb (UART0_BASE + UART_LSR) & 0x20) == 0x20) {
			break;
		}
		udelay(100);
	}
	outb(c, UART0_BASE + UART_THR);	/* put character out */
}


void serial_puts(const char *s)
{
	while (*s) {
		serial_putc(*s++);
	}
}


int serial_getc(void)
{
	unsigned char status = 0;

#ifdef CONFIG_SERIAL_SOFTWARE_FIFO
	if (serial_buffer_active) {
		return serial_buffered_getc();
	}
#endif

	while (1) {
#if defined(CONFIG_HW_WATCHDOG)
		WATCHDOG_RESET();	/* Reset HW Watchdog, if needed */
#endif	/* CONFIG_HW_WATCHDOG */
		status = inb(UART0_BASE + UART_LSR);
		if ((status & asyncLSRDataReady1) != 0x0) {
			break;
		}
		if ((status & ( asyncLSRFramingError1 |
				asyncLSROverrunError1 |
				asyncLSRParityError1  |
				asyncLSRBreakInterrupt1 )) != 0) {
			outb(asyncLSRFramingError1 |
			      asyncLSROverrunError1 |
			      asyncLSRParityError1  |
			      asyncLSRBreakInterrupt1, UART0_BASE + UART_LSR);
		}
	}
	return (0x000000ff & (int) inb (UART0_BASE));
}


int serial_tstc(void)
{
	unsigned char status;

#ifdef CONFIG_SERIAL_SOFTWARE_FIFO
	if (serial_buffer_active) {
		return serial_buffered_tstc();
	}
#endif

	status = inb(UART0_BASE + UART_LSR);
	if ((status & asyncLSRDataReady1) != 0x0) {
		return (1);
	}
	if ((status & ( asyncLSRFramingError1 |
			asyncLSROverrunError1 |
			asyncLSRParityError1  |
			asyncLSRBreakInterrupt1 )) != 0) {
		outb(asyncLSRFramingError1 |
		      asyncLSROverrunError1 |
		      asyncLSRParityError1  |
		      asyncLSRBreakInterrupt1, UART0_BASE + UART_LSR);
	}
	return 0;
}


#ifdef CONFIG_SERIAL_SOFTWARE_FIFO

void serial_isr(void *arg)
{
	int space;
	int c;
	int rx_put = buf_info.rx_put;

	if (buf_info.rx_get <= rx_put) {
		space = CONFIG_SERIAL_SOFTWARE_FIFO - (rx_put - buf_info.rx_get);
	} else {
		space = buf_info.rx_get - rx_put;
	}

	while (inb(UART0_BASE + UART_LSR) & 1) {
		c = inb(UART0_BASE);
		if (space) {
			buf_info.rx_buffer[rx_put++] = c;
			space--;

			if (rx_put == buf_info.rx_get) {
				buf_info.rx_get++;
				if (rx_put == CONFIG_SERIAL_SOFTWARE_FIFO) {
					buf_info.rx_get = 0;
				}
			}

			if (rx_put == CONFIG_SERIAL_SOFTWARE_FIFO) {
				rx_put = 0;
				if (0 == buf_info.rx_get) {
					buf_info.rx_get = 1;
				}

			}

		}
		if (space < CONFIG_SERIAL_SOFTWARE_FIFO / 4) {
			/* Stop flow by setting RTS inactive */
			outb(inb(UART0_BASE + UART_MCR) & (0xFF ^ 0x02),
			      UART0_BASE + UART_MCR);
		}
	}
	buf_info.rx_put = rx_put;
}

void serial_buffered_init(void)
{
	serial_puts ("Switching to interrupt driven serial input mode.\n");
	buf_info.rx_buffer = malloc (CONFIG_SERIAL_SOFTWARE_FIFO);
	buf_info.rx_put = 0;
	buf_info.rx_get = 0;

	if (inb (UART0_BASE + UART_MSR) & 0x10) {
		serial_puts ("Check CTS signal present on serial port: OK.\n");
		buf_info.cts = 1;
	} else {
		serial_puts ("WARNING: CTS signal not present on serial port.\n");
		buf_info.cts = 0;
	}

	irq_install_handler ( VECNUM_U0 /*UART0 */ /*int vec */ ,
			      serial_isr /*interrupt_handler_t *handler */ ,
			      (void *) &buf_info /*void *arg */ );

	/* Enable "RX Data Available" Interrupt on UART */
	/* outb(inb(UART0_BASE + UART_IER) |0x01, UART0_BASE + UART_IER); */
	outb(0x01, UART0_BASE + UART_IER);

	/* Set DTR and RTS active, enable interrupts  */
	outb(inb (UART0_BASE + UART_MCR) | 0x0b, UART0_BASE + UART_MCR);

	/* Setup UART FIFO: RX trigger level: 1 byte, Enable FIFO */
	outb( /*(1 << 6) |*/  1, UART0_BASE + UART_FCR);

	serial_buffer_active = 1;
}

void serial_buffered_putc (const char c)
{
	int i;
	/* Wait for CTS */
#if defined(CONFIG_HW_WATCHDOG)
	while (!(inb (UART0_BASE + UART_MSR) & 0x10))
		WATCHDOG_RESET ();
#else
	if (buf_info.cts)  {
		for (i=0;i<1000;i++) {
			if ((inb (UART0_BASE + UART_MSR) & 0x10)) {
				break;
			}
		}
		if (i!=1000) {
			buf_info.cts = 0;
		}
	} else {
		if ((inb (UART0_BASE + UART_MSR) & 0x10)) {
			buf_info.cts = 1;
		}
	}

#endif
	serial_putc (c);
}

void serial_buffered_puts(const char *s)
{
	serial_puts (s);
}

int serial_buffered_getc(void)
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
	if (rx_get == CONFIG_SERIAL_SOFTWARE_FIFO) {
		rx_get = 0;
	}
	buf_info.rx_get = rx_get;

	rx_put = buf_info.rx_put;
	if (rx_get <= rx_put) {
		space = CONFIG_SERIAL_SOFTWARE_FIFO - (rx_put - rx_get);
	} else {
		space = rx_get - rx_put;
	}
	if (space > CONFIG_SERIAL_SOFTWARE_FIFO / 2) {
		/* Start flow by setting RTS active */
		outb(inb (UART0_BASE + UART_MCR) | 0x02, UART0_BASE + UART_MCR);
	}

	return c;
}

int serial_buffered_tstc(void)
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
void kgdb_serial_init(void)
{
	volatile char val;
	bdiv = serial_div (CONFIG_KGDB_BAUDRATE);

	/*
	 * Init onboard 16550 UART
	 */
	outb(0x80, UART1_BASE + UART_LCR);	/* set DLAB bit */
	outb(bdiv & 0xff), UART1_BASE + UART_DLL);	/* set divisor for 9600 baud */
	outb(bdiv >> 8), UART1_BASE + UART_DLM);	/* set divisor for 9600 baud */
	outb(0x03, UART1_BASE + UART_LCR);	/* line control 8 bits no parity */
	outb(0x00, UART1_BASE + UART_FCR);	/* disable FIFO */
	outb(0x00, UART1_BASE + UART_MCR);	/* no modem control DTR RTS */
	val = inb(UART1_BASE + UART_LSR);	/* clear line status */
	val = inb(UART1_BASE + UART_RBR);	/* read receive buffer */
	outb(0x00, UART1_BASE + UART_SCR);	/* set scratchpad */
	outb(0x00, UART1_BASE + UART_IER);	/* set interrupt enable reg */
}


void putDebugChar(const char c)
{
	if (c == '\n')
		serial_putc ('\r');

	outb(c, UART1_BASE + UART_THR);	/* put character out */

	/* check THRE bit, wait for transfer done */
	while ((inb(UART1_BASE + UART_LSR) & 0x20) != 0x20);
}


void putDebugStr(const char *s)
{
	while (*s) {
		serial_putc(*s++);
	}
}


int getDebugChar(void)
{
	unsigned char status = 0;

	while (1) {
		status = inb(UART1_BASE + UART_LSR);
		if ((status & asyncLSRDataReady1) != 0x0) {
			break;
		}
		if ((status & ( asyncLSRFramingError1 |
				asyncLSROverrunError1 |
				asyncLSRParityError1  |
				asyncLSRBreakInterrupt1 )) != 0) {
			outb(asyncLSRFramingError1 |
			     asyncLSROverrunError1 |
			     asyncLSRParityError1  |
			     asyncLSRBreakInterrupt1, UART1_BASE + UART_LSR);
		}
	}
	return (0x000000ff & (int) inb(UART1_BASE));
}


void kgdb_interruptible(int yes)
{
	return;
}

#else	/* ! (CONFIG_KGDB_SER_INDEX & 2) */

void kgdb_serial_init(void)
{
	serial_printf ("[on serial] ");
}

void putDebugChar(int c)
{
	serial_putc (c);
}

void putDebugStr(const char *str)
{
	serial_puts (str);
}

int getDebugChar(void)
{
	return serial_getc ();
}

void kgdb_interruptible(int yes)
{
	return;
}
#endif	/* (CONFIG_KGDB_SER_INDEX & 2) */
#endif
