/*
 * (C) Copyright 2002-2004
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
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

#if defined(CONFIG_IMPA7) || defined(CONFIG_EP7312) || defined(CONFIG_ARMADILLO)

#include <clps7111.h>

DECLARE_GLOBAL_DATA_PTR;

void serial_setbrg (void)
{
	unsigned int reg = 0;

	switch (gd->baudrate) {
	case   1200:	reg = 191;	break;
	case   9600:	reg =  23;	break;
	case  19200:	reg =  11;	break;
	case  38400:	reg =   5;	break;
	case  57600:	reg =   3;	break;
	case 115200:	reg =   1;	break;
	default:	hang ();	break;
	}

	/* init serial serial 1,2 */
	IO_SYSCON1 = SYSCON1_UART1EN;
	IO_SYSCON2 = SYSCON2_UART2EN;

	reg |= UBRLCR_WRDLEN8;

	IO_UBRLCR1 = reg;
	IO_UBRLCR2 = reg;
}


/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 *
 */
int serial_init (void)
{
	serial_setbrg ();

	return (0);
}


/*
 * Output a single byte to the serial port.
 */
void serial_putc (const char c)
{
	int tmo;

	/* If \n, also do \r */
	if (c == '\n')
		serial_putc ('\r');

	tmo = get_timer (0) + 1 * CFG_HZ;
	while (IO_SYSFLG1 & SYSFLG1_UTXFF)
		if (get_timer (0) > tmo)
			break;

	IO_UARTDR1 = c;
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int serial_tstc (void)
{
	return !(IO_SYSFLG1 & SYSFLG1_URXFE);
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int serial_getc (void)
{
	while (IO_SYSFLG1 & SYSFLG1_URXFE);

	return IO_UARTDR1 & 0xff;
}

void
serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}

#elif defined(CONFIG_LPC2292)

#include <asm/arch/hardware.h>

void serial_setbrg (void)
{
	unsigned short divisor = 0;

	switch (gd->baudrate) {
	case   1200:	divisor = 3072;	break;
	case   9600:	divisor =  384;	break;
	case  19200:	divisor =  192;	break;
	case  38400:	divisor =   96;	break;
	case  57600:	divisor =   64;	break;
	case 115200:	divisor =   32;	break;
	default:	hang ();	break;
	}

	/* init serial UART0 */
	PUT8(U0LCR, 0);
	PUT8(U0IER, 0);
	PUT8(U0LCR, 0x80);	/* DLAB=1 */
	PUT8(U0DLL, (unsigned char)(divisor & 0x00FF));
	PUT8(U0DLM, (unsigned char)(divisor >> 8));
	PUT8(U0LCR, 0x03);	/* 8N1, DLAB=0  */
	PUT8(U0FCR, 1);		/* Enable RX and TX FIFOs */
}

int serial_init (void)
{
	unsigned long pinsel0;

	serial_setbrg ();

	pinsel0 = GET32(PINSEL0);
	pinsel0 &= ~(0x00000003);
	pinsel0 |= 5;
	PUT32(PINSEL0, pinsel0);

	return (0);
}

void serial_putc (const char c)
{
	if (c == '\n')
	{
		while((GET8(U0LSR) & (1<<5)) == 0); /* Wait for empty U0THR */
		PUT8(U0THR, '\r');
	}

	while((GET8(U0LSR) & (1<<5)) == 0); /* Wait for empty U0THR */
	PUT8(U0THR, c);
}

int serial_getc (void)
{
	while((GET8(U0LSR) & 1) == 0);
	return GET8(U0RBR);
}

void
serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}

/* Test if there is a byte to read */
int serial_tstc (void)
{
	return (GET8(U0LSR) & 1);
}

#endif
