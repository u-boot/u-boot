/*
 * (C) Copyright 2002
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
#include <asm/arch/pxa-regs.h>

void serial_setbrg (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	unsigned int quot = 0;

	if (gd->baudrate == 1200)
		quot = 192;
	else if (gd->baudrate == 9600)
		quot = 96;
	else if (gd->baudrate == 19200)
		quot = 48;
	else if (gd->baudrate == 38400)
		quot = 24;
	else if (gd->baudrate == 57600)
		quot = 16;
	else if (gd->baudrate == 115200)
		quot = 8;
	else
		hang ();

#ifdef CONFIG_FFUART

	CKEN |= CKEN6_FFUART;

	FFIER = 0;					/* Disable for now */
	FFFCR = 0;					/* No fifos enabled */

	/* set baud rate */
	FFLCR = LCR_WLS0 | LCR_WLS1 | LCR_DLAB;
	FFDLL = quot & 0xff;
	FFDLH = quot >> 8;
	FFLCR = LCR_WLS0 | LCR_WLS1;

	FFIER = IER_UUE;			/* Enable FFUART */

#elif CONFIG_STUART
#error "Bad: not implemented yet!"
#else
#error "Bad: you didn't configured serial ..."
#endif
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
#ifdef CONFIG_FFUART
	/* wait for room in the tx FIFO on FFUART */
	while ((FFLSR & LSR_TEMT) == 0);

	FFTHR = c;
#elif CONFIG_STUART
#endif

	/* If \n, also do \r */
	if (c == '\n')
		serial_putc ('\r');
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int serial_tstc (void)
{
#ifdef CONFIG_FFUART
	return FFLSR & LSR_DR;
#elif CONFIG_STUART
#endif
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int serial_getc (void)
{
#ifdef CONFIG_FFUART
	while (!(FFLSR & LSR_DR));

	return (char) FFRBR & 0xff;
#elif CONFIG_STUART
#endif
}

void
serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}
