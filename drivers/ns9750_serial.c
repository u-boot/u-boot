/***********************************************************************
 *
 * Copyright (C) 2004 by FS Forth-Systeme GmbH.
 * All rights reserved.
 *
 * $Id: ns9750_serial.c,v 1.1 2004/02/16 10:37:20 mpietrek Exp $
 * @Author: Markus Pietrek
 * @Descr: Serial driver for the NS9750. Only one UART is supported yet.
 * @References: [1] NS9750 Hardware Reference/December 2003
 * @TODO: Implement Character GAP Timer when chip is fixed for PLL bypass
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 ***********************************************************************/

#include <common.h>

#ifdef CFG_NS9750_UART

#include "ns9750_bbus.h"	/* for GPIOs */
#include "ns9750_ser.h"		/* for serial configuration */

DECLARE_GLOBAL_DATA_PTR;

#define CONSOLE CONFIG_CONS_INDEX

static unsigned int calcBitrateRegister( void );
static unsigned int calcRxCharGapRegister( void );

static char cCharsAvailable; /* Numbers of chars in unCharCache */
static unsigned int unCharCache; /* unCharCache is only valid if
				  * cCharsAvailable > 0 */

/***********************************************************************
 * @Function: serial_init
 * @Return: 0
 * @Descr: configures GPIOs and UART. Requires BBUS Master Reset turned off
 ***********************************************************************/

int serial_init( void )
{
	unsigned int aunGPIOTxD[] = { 0, 8, 40, 44 };
	unsigned int aunGPIORxD[] = { 1, 9, 41, 45 };

	cCharsAvailable = 0;

	/* configure TxD and RxD pins for their special function */
	set_gpio_cfg_reg_val( aunGPIOTxD[ CONSOLE ],
			      NS9750_GPIO_CFG_FUNC_0 | NS9750_GPIO_CFG_OUTPUT );
	set_gpio_cfg_reg_val( aunGPIORxD[ CONSOLE ],
			      NS9750_GPIO_CFG_FUNC_0 | NS9750_GPIO_CFG_INPUT );

	/* configure serial engine */
	*get_ser_reg_addr_channel( NS9750_SER_CTRL_A, CONSOLE ) =
		NS9750_SER_CTRL_A_CE |
		NS9750_SER_CTRL_A_STOP |
		NS9750_SER_CTRL_A_WLS_8;

	serial_setbrg();

	*get_ser_reg_addr_channel( NS9750_SER_CTRL_B, CONSOLE ) =
		NS9750_SER_CTRL_B_RCGT;

	return 0;
}

/***********************************************************************
 * @Function: serial_putc
 * @Return: n/a
 * @Descr: writes one character to the FIFO. Blocks until FIFO is not full
 ***********************************************************************/

void serial_putc( const char c )
{
	if (c == '\n')
		serial_putc( '\r' );

	while (!(*get_ser_reg_addr_channel( NS9750_SER_STAT_A, CONSOLE) &
		 NS9750_SER_STAT_A_TRDY ) ) {
		/* do nothing, wait for characters in FIFO sent */
	}

	*(volatile char*) get_ser_reg_addr_channel( NS9750_SER_FIFO,
						    CONSOLE) = c;
}

/***********************************************************************
 * @Function: serial_puts
 * @Return: n/a
 * @Descr: writes non-zero string to the FIFO.
 ***********************************************************************/

void serial_puts( const char *s )
{
	while (*s) {
		serial_putc( *s++ );
	}
}

/***********************************************************************
 * @Function: serial_getc
 * @Return: the character read
 * @Descr: performs only 8bit accesses to the FIFO. No error handling
 ***********************************************************************/

int serial_getc( void )
{
	int i;

	while (!serial_tstc() ) {
		/* do nothing, wait for incoming characters */
	}

	/*  at least one character in unCharCache */
	i = (int) (unCharCache & 0xff);

	unCharCache >>= 8;
	cCharsAvailable--;

	return i;
}

/***********************************************************************
 * @Function: serial_tstc
 * @Return: 0 if no input available, otherwise != 0
 * @Descr: checks for incoming FIFO not empty. Stores the incoming chars in
 *	   unCharCache and the numbers of characters in cCharsAvailable
 ***********************************************************************/

int serial_tstc( void )
{
	unsigned int unRegCache;

	if ( cCharsAvailable )
		return 1;

	unRegCache = *get_ser_reg_addr_channel( NS9750_SER_STAT_A,CONSOLE );
	if( unRegCache & NS9750_SER_STAT_A_RBC ) {
		*get_ser_reg_addr_channel( NS9750_SER_STAT_A, CONSOLE ) =
			NS9750_SER_STAT_A_RBC;
		unRegCache = *get_ser_reg_addr_channel( NS9750_SER_STAT_A,
							CONSOLE );
	}

	if ( unRegCache & NS9750_SER_STAT_A_RRDY ) {
		cCharsAvailable = (unRegCache & NS9750_SER_STAT_A_RXFDB_MA)>>20;
		if ( !cCharsAvailable )
			cCharsAvailable = 4;

		unCharCache = *get_ser_reg_addr_channel( NS9750_SER_FIFO,
							 CONSOLE );
		return 1;
	}

	/* no chars available */
	return 0;
}

void serial_setbrg( void )
{
	*get_ser_reg_addr_channel( NS9750_SER_BITRATE, CONSOLE ) =
		calcBitrateRegister();
	*get_ser_reg_addr_channel( NS9750_SER_RX_CHAR_TIMER, CONSOLE ) =
		calcRxCharGapRegister();
}

/***********************************************************************
 * @Function: calcBitrateRegister
 * @Return: value for the serial bitrate register
 * @Descr: register value depends on clock frequency and baudrate
 ***********************************************************************/

static unsigned int calcBitrateRegister( void )
{
	return ( NS9750_SER_BITRATE_EBIT |
		 NS9750_SER_BITRATE_CLKMUX_BCLK |
		 NS9750_SER_BITRATE_TMODE |
		 NS9750_SER_BITRATE_TCDR_16 |
		 NS9750_SER_BITRATE_RCDR_16 |
		 ( ( ( ( CONFIG_SYS_CLK_FREQ / 8 ) / /* BBUS clock,[1] Fig. 38 */
		       ( gd->baudrate * 16 ) ) - 1 ) &
		   NS9750_SER_BITRATE_N_MA ) );
}

/***********************************************************************
 * @Function: calcRxCharGapRegister
 * @Return: value for the character gap timer register
 * @Descr: register value depends on clock frequency and baudrate. Currently 0
 *	   is used as there is a bug with the gap timer in PLL bypass mode.
 ***********************************************************************/

static unsigned int calcRxCharGapRegister( void )
{
	return NS9750_SER_RX_CHAR_TIMER_TRUN;
}

#endif /* CFG_NS9750_UART */
