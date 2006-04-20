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

#endif /* defined(CONFIG_IMPA7) || defined(CONFIG_EP7312) */
