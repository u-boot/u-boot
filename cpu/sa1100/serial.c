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
#include <SA-1100.h>

void serial_setbrg (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	unsigned int reg = 0;

	if (gd->baudrate == 1200)
		reg = 191;
	else if (gd->baudrate == 9600)
		reg = 23;
	else if (gd->baudrate == 19200)
		reg = 11;
	else if (gd->baudrate == 38400)
		reg = 5;
	else if (gd->baudrate == 57600)
		reg = 3;
	else if (gd->baudrate == 115200)
		reg = 1;
	else
		hang ();

#ifdef CONFIG_SERIAL1
	/* SA1110 uart function */
	Ser1SDCR0 |= SDCR0_SUS;

	/* Wait until port is ready ... */
	while(Ser1UTSR1 & UTSR1_TBY) {}

	/* init serial serial 1 */
	Ser1UTCR3 = 0x00;
	Ser1UTSR0 = 0xff;
	Ser1UTCR0 = ( UTCR0_1StpBit | UTCR0_8BitData );
	Ser1UTCR1 = 0;
	Ser1UTCR2 = (u32)reg;
	Ser1UTCR3 = ( UTCR3_RXE | UTCR3_TXE );
#elif defined(CONFIG_SERIAL3)
	/* Wait until port is ready ... */
	while (Ser3UTSR1 & UTSR1_TBY) {
	}

	/* init serial serial 3 */
	Ser3UTCR3 = 0x00;
	Ser3UTSR0 = 0xff;
	Ser3UTCR0 = (UTCR0_1StpBit | UTCR0_8BitData);
	Ser3UTCR1 = 0;
	Ser3UTCR2 = (u32) reg;
	Ser3UTCR3 = (UTCR3_RXE | UTCR3_TXE);
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
#ifdef CONFIG_SERIAL1
	/* wait for room in the tx FIFO on SERIAL1 */
	while ((Ser1UTSR0 & UTSR0_TFS) == 0);

	Ser1UTDR = c;
#elif defined(CONFIG_SERIAL3)
	/* wait for room in the tx FIFO on SERIAL3 */
	while ((Ser3UTSR0 & UTSR0_TFS) == 0);

	Ser3UTDR = c;
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
#ifdef CONFIG_SERIAL1
	return Ser1UTSR1 & UTSR1_RNE;
#elif defined(CONFIG_SERIAL3)
	return Ser3UTSR1 & UTSR1_RNE;
#endif
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int serial_getc (void)
{
#ifdef CONFIG_SERIAL1
	while (!(Ser1UTSR1 & UTSR1_RNE));

	return (char) Ser1UTDR & 0xff;
#elif defined(CONFIG_SERIAL3)
	while (!(Ser3UTSR1 & UTSR1_RNE));

	return (char) Ser3UTDR & 0xff;
#endif
}

void
serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}
