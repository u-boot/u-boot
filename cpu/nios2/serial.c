/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * YANU Support:
 * Copyright 2010, Renato Andreola <renato.andreola@imagos.it>
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
#include <watchdog.h>
#include <asm/io.h>
#include <nios2-io.h>
#include <nios2-yanu.h>

DECLARE_GLOBAL_DATA_PTR;

/*------------------------------------------------------------------
 * JTAG acts as the serial port
 *-----------------------------------------------------------------*/
#if defined(CONFIG_CONSOLE_JTAG)

static nios_jtag_t *jtag = (nios_jtag_t *)CONFIG_SYS_NIOS_CONSOLE;

void serial_setbrg( void ){ return; }
int serial_init( void ) { return(0);}

void serial_putc (char c)
{
	unsigned val;

	while (NIOS_JTAG_WSPACE ( readl (&jtag->control)) == 0)
		WATCHDOG_RESET ();
	writel (&jtag->data, (unsigned char)c);
}

void serial_puts (const char *s)
{
	while (*s != 0)
		serial_putc (*s++);
}

int serial_tstc (void)
{
	return ( readl (&jtag->control) & NIOS_JTAG_RRDY);
}

int serial_getc (void)
{
	int c;
	unsigned val;

	while (1) {
		WATCHDOG_RESET ();
		val = readl (&jtag->data);
		if (val & NIOS_JTAG_RVALID)
			break;
	}
	c = val & 0x0ff;
	return (c);
}

#elif defined(CONFIG_CONSOLE_YANU)
/*-----------------------------------------------------------------*/
/* YANU Imagos serial port */
/*-----------------------------------------------------------------*/

static yanu_uart_t *uart = (yanu_uart_t *)CONFIG_SYS_NIOS_CONSOLE;

#if defined(CONFIG_SYS_NIOS_FIXEDBAUD)

/* Everything's already setup for fixed-baud PTF assignment*/

void serial_setbrg (void)
{
	int n, k;
	const unsigned max_uns = 0xFFFFFFFF;
	unsigned best_n, best_m, baud;

	/* compute best N and M couple */
	best_n = YANU_MAX_PRESCALER_N;
	for (n = YANU_MAX_PRESCALER_N; n >= 0; n--) {
		if ((unsigned)CONFIG_SYS_CLK_FREQ / (1 << (n + 4)) >=
		    (unsigned)CONFIG_BAUDRATE) {
			best_n = n;
			break;
		}
	}
	for (k = 0;; k++) {
		if ((unsigned)CONFIG_BAUDRATE <= (max_uns >> (15+n-k)))
			break;
	}
	best_m =
	    ((unsigned)CONFIG_BAUDRATE * (1 << (15 + n - k))) /
	    ((unsigned)CONFIG_SYS_CLK_FREQ >> k);

	baud = best_m + best_n * YANU_BAUDE;
	writel(&uart->baud, baud);

	return;
}

#else

void serial_setbrg (void)
{	
	int n, k;
	const unsigned max_uns = 0xFFFFFFFF;
	unsigned best_n, best_m, baud;

	/* compute best N and M couple */
	best_n = YANU_MAX_PRESCALER_N;
	for (n = YANU_MAX_PRESCALER_N; n >= 0; n--) {
		if ((unsigned)CONFIG_SYS_CLK_FREQ / (1 << (n + 4)) >=
		    gd->baudrate) {
			best_n = n;
			break;
		}
	}
	for (k = 0;; k++) {
		if (gd->baudrate <= (max_uns >> (15+n-k)))
			break;
	}
	best_m =
	    (gd->baudrate * (1 << (15 + n - k))) /
	    ((unsigned)CONFIG_SYS_CLK_FREQ >> k);

	baud = best_m + best_n * YANU_BAUDE;
	writel(&uart->baud, baud);

	return;
}


#endif /* CONFIG_SYS_NIOS_FIXEDBAUD */

int serial_init (void)
{
	unsigned action,control;

	/* status register cleanup */
	action =  YANU_ACTION_RRRDY     |
		YANU_ACTION_RTRDY       |
		YANU_ACTION_ROE         |
		YANU_ACTION_RBRK        |
		YANU_ACTION_RFE         |
		YANU_ACTION_RPE         |
	    YANU_ACTION_RFE | YANU_ACTION_RFIFO_CLEAR | YANU_ACTION_TFIFO_CLEAR;

	writel(&uart->action, action);
	
	/*  control register cleanup */
	/* no interrupts enabled */
	/* one stop bit */
	/* hardware flow control disabled */
	/* 8 bits */
	control = (0x7 << YANU_CONTROL_BITS_POS);
	/* enven parity just to be clean */
	control |= YANU_CONTROL_PAREVEN;
	/* we set threshold for fifo */
	control |= YANU_CONTROL_RDYDLY * YANU_RXFIFO_DLY;
	control |= YANU_CONTROL_TXTHR *  YANU_TXFIFO_THR;

	writel(&uart->control, control);

	/* to set baud rate */
	serial_setbrg();

	return (0);
}


/*-----------------------------------------------------------------------
 * YANU CONSOLE
 *---------------------------------------------------------------------*/
void serial_putc (char c)
{
	int tx_chars;
	unsigned status;

	if (c == '\n')
		serial_putc ('\r');
	
	while (1) {
		status = readl(&uart->status);
		tx_chars = (status>>YANU_TFIFO_CHARS_POS)
			& ((1<<YANU_TFIFO_CHARS_N)-1);
		if (tx_chars < YANU_TXFIFO_SIZE-1)
			break;
		WATCHDOG_RESET ();
	}

	writel(&uart->data, (unsigned char)c);
}

void serial_puts (const char *s)
{
	while (*s != 0) {
		serial_putc (*s++);
	}
}


int serial_tstc(void)
{
	unsigned status ;

	status = readl(&uart->status);
	return (((status >> YANU_RFIFO_CHARS_POS) &
		 ((1 << YANU_RFIFO_CHARS_N) - 1)) > 0);
}	

int serial_getc (void)
{
	while (serial_tstc() == 0)
		WATCHDOG_RESET ();
	
	/* first we pull the char */
	writel(&uart->action, YANU_ACTION_RFIFO_PULL);

	return(readl(&uart->data) & YANU_DATA_CHAR_MASK);
}

#else /*CONFIG_CONSOLE_YANU*/

/*------------------------------------------------------------------
 * UART the serial port
 *-----------------------------------------------------------------*/

static nios_uart_t *uart = (nios_uart_t *) CONFIG_SYS_NIOS_CONSOLE;

#if defined(CONFIG_SYS_NIOS_FIXEDBAUD)

/* Everything's already setup for fixed-baud PTF
 * assignment
 */
void serial_setbrg (void){ return; }
int serial_init (void) { return (0);}

#else

void serial_setbrg (void)
{
	unsigned div;

	div = (CONFIG_SYS_CLK_FREQ/gd->baudrate)-1;
	writel (&uart->divisor,div);
	return;
}

int serial_init (void)
{
	serial_setbrg ();
	return (0);
}

#endif /* CONFIG_SYS_NIOS_FIXEDBAUD */


/*-----------------------------------------------------------------------
 * UART CONSOLE
 *---------------------------------------------------------------------*/
void serial_putc (char c)
{
	if (c == '\n')
		serial_putc ('\r');
	while ((readl (&uart->status) & NIOS_UART_TRDY) == 0)
		WATCHDOG_RESET ();
	writel (&uart->txdata,(unsigned char)c);
}

void serial_puts (const char *s)
{
	while (*s != 0) {
		serial_putc (*s++);
	}
}

int serial_tstc (void)
{
	return (readl (&uart->status) & NIOS_UART_RRDY);
}

int serial_getc (void)
{
	while (serial_tstc () == 0)
		WATCHDOG_RESET ();
	return (readl (&uart->rxdata) & 0x00ff );
}

#endif /* CONFIG_JTAG_CONSOLE */
