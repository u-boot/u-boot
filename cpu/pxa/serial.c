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
#include <watchdog.h>
#include <asm/arch/pxa-regs.h>

void serial_setbrg (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	unsigned int quot = 0;

	if (gd->baudrate == 1200)
		quot = 768;
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

#elif defined(CONFIG_BTUART)
	CKEN |= CKEN7_BTUART;

	BTIER = 0;
	BTFCR = 0;

	/* set baud rate */
	BTLCR = LCR_DLAB;
	BTDLL = quot & 0xff;
	BTDLH = quot >> 8;
	BTLCR = LCR_WLS0 | LCR_WLS1;

	BTIER = IER_UUE;			/* Enable BFUART */

#elif defined(CONFIG_STUART)
	CKEN |= CKEN5_STUART;

	STIER = 0;
	STFCR = 0;

	/* set baud rate */
	STLCR = LCR_DLAB;
	STDLL = quot & 0xff;
	STDLH = quot >> 8;
	STLCR = LCR_WLS0 | LCR_WLS1;

	STIER = IER_UUE;			/* Enable STUART */

#else
#error "Bad: you didn't configure serial ..."
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
	while ((FFLSR & LSR_TEMT) == 0)
		WATCHDOG_RESET ();	/* Reset HW Watchdog, if needed */
	FFTHR = c;
#elif defined(CONFIG_BTUART)
	while ((BTLSR & LSR_TEMT ) == 0 )
		WATCHDOG_RESET ();	/* Reset HW Watchdog, if needed */
	BTTHR = c;
#elif defined(CONFIG_STUART)
	while ((STLSR & LSR_TEMT ) == 0 )
		WATCHDOG_RESET ();	/* Reset HW Watchdog, if needed */
	STTHR = c;
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
#elif defined(CONFIG_BTUART)
	return BTLSR & LSR_DR;
#elif defined(CONFIG_STUART)
	return STLSR & LSR_DR;
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
	while (!(FFLSR & LSR_DR))
		WATCHDOG_RESET ();	/* Reset HW Watchdog, if needed */
	return (char) FFRBR & 0xff;
#elif defined(CONFIG_BTUART)
	while (!(BTLSR & LSR_DR))
		WATCHDOG_RESET ();	/* Reset HW Watchdog, if needed */
	return (char) BTRBR & 0xff;
#elif defined(CONFIG_STUART)
	while (!(STLSR & LSR_DR))
		WATCHDOG_RESET ();	/* Reset HW Watchdog, if needed */
	return (char) STRBR & 0xff;
#endif
}

void
serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}
