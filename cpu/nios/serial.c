/*
 * (C) Copyright 2003, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
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
#include <nios-io.h>


static nios_uart_t *uart = (nios_uart_t *)CFG_NIOS_CONSOLE;

#if defined(CFG_NIOS_FIXEDBAUD)

/* Everything's already setup for fixed-baud PTF
 * assignment
 */
void serial_setbrg( void ){ return; }
int serial_init( void ) { return(0);}

#else

void serial_setbrg( void )
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned div;

	div = (CONFIG_SYS_CLK_FREQ/gd->baudrate)-1;
	uart->divisor = div;
	return;
}

int serial_init( void )
{
	serial_setbrg();
	return(0);
}

#endif /* CFG_NIOS_FIXEDBAUD */


void serial_putc( char c )
{
	if (c == '\n')
		serial_putc('\r');
	while( (uart->status & NIOS_UART_TRDY) == 0 )
		WATCHDOG_RESET ();
	uart->txdata = (unsigned char)c;
}

void serial_puts( const char *s )
{
	while( *s != 0 ) {
		serial_putc( *s++ );
	}
}

int serial_tstc( void )
{
	return( uart->status & NIOS_UART_RRDY);
}

int serial_getc( void )
{
	while( serial_tstc() == 0 )
		WATCHDOG_RESET ();
	return( uart->rxdata & 0x00ff );
}
