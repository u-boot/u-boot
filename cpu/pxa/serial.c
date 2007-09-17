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
#include <serial.h>
#include <asm/arch/pxa-regs.h>

DECLARE_GLOBAL_DATA_PTR;

#define FFUART	0
#define BTUART	1
#define STUART	2

#ifndef CONFIG_SERIAL_MULTI
#if defined (CONFIG_FFUART)
#define UART_INDEX	FFUART
#elif defined (CONFIG_BTUART)
#define UART_INDEX	BTUART
#elif defined (CONFIG_STUART)
#define UART_INDEX	STUART
#else
#error "Bad: you didn't configure serial ..."
#endif
#endif

void pxa_setbrg_dev (unsigned int uart_index)
{
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

	switch (uart_index) {
		case FFUART:
#ifdef CONFIG_CPU_MONAHANS
			CKENA |= CKENA_22_FFUART;
#else
			CKEN |= CKEN6_FFUART;
#endif /* CONFIG_CPU_MONAHANS */

			FFIER = 0;	/* Disable for now */
			FFFCR = 0;	/* No fifos enabled */

			/* set baud rate */
			FFLCR = LCR_WLS0 | LCR_WLS1 | LCR_DLAB;
			FFDLL = quot & 0xff;
			FFDLH = quot >> 8;
			FFLCR = LCR_WLS0 | LCR_WLS1;

			FFIER = IER_UUE;	/* Enable FFUART */
		break;

		case BTUART:
#ifdef CONFIG_CPU_MONAHANS
			CKENA |= CKENA_21_BTUART;
#else
			CKEN |= CKEN7_BTUART;
#endif /*  CONFIG_CPU_MONAHANS */

			BTIER = 0;
			BTFCR = 0;

			/* set baud rate */
			BTLCR = LCR_DLAB;
			BTDLL = quot & 0xff;
			BTDLH = quot >> 8;
			BTLCR = LCR_WLS0 | LCR_WLS1;

			BTIER = IER_UUE;	/* Enable BFUART */

		break;

		case STUART:
#ifdef CONFIG_CPU_MONAHANS
			CKENA |= CKENA_23_STUART;
#else
			CKEN |= CKEN5_STUART;
#endif /* CONFIG_CPU_MONAHANS */

			STIER = 0;
			STFCR = 0;

			/* set baud rate */
			STLCR = LCR_DLAB;
			STDLL = quot & 0xff;
			STDLH = quot >> 8;
			STLCR = LCR_WLS0 | LCR_WLS1;

			STIER = IER_UUE;			/* Enable STUART */
			break;

		default:
			hang();
	}
}


/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 *
 */
int pxa_init_dev (unsigned int uart_index)
{
	pxa_setbrg_dev (uart_index);

	return (0);
}


/*
 * Output a single byte to the serial port.
 */
void pxa_putc_dev (unsigned int uart_index,const char c)
{
	switch (uart_index) {
		case FFUART:
		/* wait for room in the tx FIFO on FFUART */
			while ((FFLSR & LSR_TEMT) == 0)
				WATCHDOG_RESET ();	/* Reset HW Watchdog, if needed */
			FFTHR = c;
			break;

		case BTUART:
			while ((BTLSR & LSR_TEMT ) == 0 )
				WATCHDOG_RESET ();	/* Reset HW Watchdog, if needed */
			BTTHR = c;
			break;

		case STUART:
			while ((STLSR & LSR_TEMT ) == 0 )
				WATCHDOG_RESET ();	/* Reset HW Watchdog, if needed */
			STTHR = c;
			break;
	}

	/* If \n, also do \r */
	if (c == '\n')
		pxa_putc_dev (uart_index,'\r');
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int pxa_tstc_dev (unsigned int uart_index)
{
	switch (uart_index) {
		case FFUART:
			return FFLSR & LSR_DR;
		case BTUART:
			return BTLSR & LSR_DR;
		case STUART:
			return STLSR & LSR_DR;
	}
	return -1;
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int pxa_getc_dev (unsigned int uart_index)
{
	switch (uart_index) {
		case FFUART:
			while (!(FFLSR & LSR_DR))
			WATCHDOG_RESET ();	/* Reset HW Watchdog, if needed */
			return (char) FFRBR & 0xff;

		case BTUART:
			while (!(BTLSR & LSR_DR))
			WATCHDOG_RESET ();	/* Reset HW Watchdog, if needed */
			return (char) BTRBR & 0xff;
		case STUART:
			while (!(STLSR & LSR_DR))
			WATCHDOG_RESET ();	/* Reset HW Watchdog, if needed */
			return (char) STRBR & 0xff;
	}
	return -1;
}

void
pxa_puts_dev (unsigned int uart_index,const char *s)
{
	while (*s) {
		pxa_putc_dev (uart_index,*s++);
	}
}

#if defined (CONFIG_FFUART)
static int ffuart_init(void)
{
	return pxa_init_dev(FFUART);
}

static void ffuart_setbrg(void)
{
	return pxa_setbrg_dev(FFUART);
}

static void ffuart_putc(const char c)
{
	return pxa_putc_dev(FFUART,c);
}

static void ffuart_puts(const char *s)
{
	return pxa_puts_dev(FFUART,s);
}

static int ffuart_getc(void)
{
	return pxa_getc_dev(FFUART);
}

static int ffuart_tstc(void)
{
	return pxa_tstc_dev(FFUART);
}

struct serial_device serial_ffuart_device =
{
	"serial_ffuart",
	"PXA",
	ffuart_init,
	ffuart_setbrg,
	ffuart_getc,
	ffuart_tstc,
	ffuart_putc,
	ffuart_puts,
};
#endif

#if defined (CONFIG_BTUART)
static int btuart_init(void)
{
	return pxa_init_dev(BTUART);
}

static void btuart_setbrg(void)
{
	return pxa_setbrg_dev(BTUART);
}

static void btuart_putc(const char c)
{
	return pxa_putc_dev(BTUART,c);
}

static void btuart_puts(const char *s)
{
	return pxa_puts_dev(BTUART,s);
}

static int btuart_getc(void)
{
	return pxa_getc_dev(BTUART);
}

static int btuart_tstc(void)
{
	return pxa_tstc_dev(BTUART);
}

struct serial_device serial_btuart_device =
{
	"serial_btuart",
	"PXA",
	btuart_init,
	btuart_setbrg,
	btuart_getc,
	btuart_tstc,
	btuart_putc,
	btuart_puts,
};
#endif

#if defined (CONFIG_STUART)
static int stuart_init(void)
{
	return pxa_init_dev(STUART);
}

static void stuart_setbrg(void)
{
	return pxa_setbrg_dev(STUART);
}

static void stuart_putc(const char c)
{
	return pxa_putc_dev(STUART,c);
}

static void stuart_puts(const char *s)
{
	return pxa_puts_dev(STUART,s);
}

static int stuart_getc(void)
{
	return pxa_getc_dev(STUART);
}

static int stuart_tstc(void)
{
	return pxa_tstc_dev(STUART);
}

struct serial_device serial_stuart_device =
{
	"serial_stuart",
	"PXA",
	stuart_init,
	stuart_setbrg,
	stuart_getc,
	stuart_tstc,
	stuart_putc,
	stuart_puts,
};
#endif


#ifndef CONFIG_SERIAL_MULTI
inline int serial_init(void) {
	return (pxa_init_dev(UART_INDEX));
}
void serial_setbrg(void) {
	pxa_setbrg_dev(UART_INDEX);
}
int serial_getc(void) {
	return(pxa_getc_dev(UART_INDEX));
}
int serial_tstc(void) {
	return(pxa_tstc_dev(UART_INDEX));
}
void serial_putc(const char c) {
	pxa_putc_dev(UART_INDEX,c);
}
void serial_puts(const char *s) {
	pxa_puts_dev(UART_INDEX,s);
}
#endif	/* CONFIG_SERIAL_MULTI */
