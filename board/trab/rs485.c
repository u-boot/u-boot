/*
 * (C) Copyright 2003
 * Martin Krause, TQ-Systems GmbH, <martin.krause@tqs.de>
 *
 * Based on cpu/arm920t/serial.c, by Gary Jennejohn
 * (C) Copyright 2002 Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
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
#include <s3c2400.h>
#include "rs485.h"

static void rs485_setbrg (void);
static void rs485_cfgio (void);
static void set_rs485re(unsigned char rs485re_state);
static void set_rs485de(unsigned char rs485de_state);
static void rs485_setbrg (void);
#ifdef	NOT_USED
static void trab_rs485_disable_tx(void);
static void trab_rs485_disable_rx(void);
#endif

#define UART_NR	S3C24X0_UART1

/* CPLD-Register for controlling TRAB hardware functions */
#define CPLD_RS485_RE           ((volatile unsigned long *)0x04028000)

static void rs485_setbrg (void)
{
	struct s3c24x0_uart * const uart = s3c24x0_get_base_uart(UART_NR);
	int i;
	unsigned int reg = 0;

	/* value is calculated so : (int)(PCLK/16./baudrate) -1 */
	/* reg = (33000000 / (16 * gd->baudrate)) - 1; */
	reg = (33000000 / (16 * 38400)) - 1;

	/* FIFO enable, Tx/Rx FIFO clear */
	uart->UFCON = 0x07;
	uart->UMCON = 0x0;
	/* Normal,No parity,1 stop,8 bit */
	uart->ULCON = 0x3;
	/*
	 * tx=level,rx=edge,disable timeout int.,enable rx error int.,
	 * normal,interrupt or polling
	 */
	uart->UCON = 0x245;
	uart->UBRDIV = reg;

	for (i = 0; i < 100; i++);
}

static void rs485_cfgio (void)
{
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();

	gpio->PFCON &= ~(0x3 << 2);
	gpio->PFCON |=  (0x2 << 2); /* configure GPF1 as RXD1 */

	gpio->PFCON &= ~(0x3 << 6);
	gpio->PFCON |=  (0x2 << 6); /* configure GPF3 as TXD1 */

	gpio->PFUP |= (1 << 1); /* disable pullup on GPF1 */
	gpio->PFUP |= (1 << 3); /* disable pullup on GPF3 */

	gpio->PACON &= ~(1 << 11); /* set GPA11 (RS485_DE) to output */
}

/*
 * Initialise the rs485 port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 *
 */
int rs485_init (void)
{
	rs485_cfgio ();
	rs485_setbrg ();

	return (0);
}

/*
 * Read a single byte from the rs485 port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int rs485_getc (void)
{
	struct s3c24x0_uart * const uart = s3c24x0_get_base_uart(UART_NR);

	/* wait for character to arrive */
	while (!(uart->UTRSTAT & 0x1));

	return uart->URXH & 0xff;
}

/*
 * Output a single byte to the rs485 port.
 */
void rs485_putc (const char c)
{
	struct s3c24x0_uart * const uart = s3c24x0_get_base_uart(UART_NR);

	/* wait for room in the tx FIFO */
	while (!(uart->UTRSTAT & 0x2));

	uart->UTXH = c;

	/* If \n, also do \r */
	if (c == '\n')
		rs485_putc ('\r');
}

/*
 * Test whether a character is in the RX buffer
 */
int rs485_tstc (void)
{
	struct s3c24x0_uart * const uart = s3c24x0_get_base_uart(UART_NR);

	return uart->UTRSTAT & 0x1;
}

void rs485_puts (const char *s)
{
	while (*s) {
		rs485_putc (*s++);
	}
}


/*
 * State table:
 *	RE	DE	Result
 *	1	1	XMIT
 *	0	0	RCV
 *	1	0	Shutdown
 */

/* function that controls the receiver enable for the rs485 */
/* rs485re_state reflects the level (0/1) of the RE pin */

static void set_rs485re(unsigned char rs485re_state)
{
	if(rs485re_state)
		*CPLD_RS485_RE = 0x010000;
	else
		*CPLD_RS485_RE = 0x0;
}

/* function that controls the sender enable for the rs485 */
/* rs485de_state reflects the level (0/1) of the DE pin */

static void set_rs485de(unsigned char rs485de_state)
{
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();

	/* This is on PORT A bit 11 */
	if(rs485de_state)
		gpio->PADAT |= (1 << 11);
	else
		gpio->PADAT &= ~(1 << 11);
}


void trab_rs485_enable_tx(void)
{
	set_rs485de(1);
	set_rs485re(1);
}

void trab_rs485_enable_rx(void)
{
	set_rs485re(0);
	set_rs485de(0);
}

#ifdef	NOT_USED
static void trab_rs485_disable_tx(void)
{
	set_rs485de(0);
}

static void trab_rs485_disable_rx(void)
{
	set_rs485re(1);
}
#endif
