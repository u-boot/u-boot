/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <command.h>

#include <asm/mcfuart.h>

#ifdef CONFIG_M5272
#include <asm/m5272.h>
#endif

#ifdef CONFIG_M5282
#include <asm/m5282.h>
#endif

#ifdef CONFIG_M5249
#include <asm/m5249.h>
#endif

#ifdef CONFIG_M5249
#define DoubleClock(a) ((double)(CFG_CLK/2) / 32.0 / (double)(a))
#else
#define DoubleClock(a) ((double)(CFG_CLK) / 32.0 / (double)(a))
#endif

void rs_serial_setbaudrate(int port,int baudrate)
{
#if defined(CONFIG_M5272) || defined(CONFIG_M5249)
	volatile unsigned char	*uartp;
	double clock, fraction;

	if (port == 0)
	  uartp = (volatile unsigned char *) (CFG_MBAR + MCFUART_BASE1);
	else
	  uartp = (volatile unsigned char *) (CFG_MBAR + MCFUART_BASE2);

	clock = DoubleClock(baudrate);      /* Set baud above */

	fraction = ((clock - (int)clock) * 16.0) + 0.5;

	uartp[MCFUART_UBG1] = (((int)clock >> 8) & 0xff);  /* set msb baud */
	uartp[MCFUART_UBG2] = ((int)clock & 0xff);  /* set lsb baud */
	uartp[MCFUART_UFPD] = ((int)fraction & 0xf);  /* set baud fraction adjust */
#endif
};

void rs_serial_init(int port,int baudrate)
{
	volatile unsigned char	*uartp;

	/*
	 *	Reset UART, get it into known state...
	 */
	if (port == 0)
		uartp = (volatile unsigned char *) (CFG_MBAR + MCFUART_BASE1);
	else
		uartp = (volatile unsigned char *) (CFG_MBAR + MCFUART_BASE2);

	uartp[MCFUART_UCR] = MCFUART_UCR_CMDRESETRX;  /* reset RX */
	uartp[MCFUART_UCR] = MCFUART_UCR_CMDRESETTX;  /* reset TX */
	uartp[MCFUART_UCR] = MCFUART_UCR_CMDRESETMRPTR;  /* reset MR pointer */
	uartp[MCFUART_UCR] = MCFUART_UCR_CMDRESETERR;  /* reset Error pointer */

	/*
	 * Set port for CONSOLE_BAUD_RATE, 8 data bits, 1 stop bit, no parity.
	 */
	uartp[MCFUART_UMR] = MCFUART_MR1_PARITYNONE | MCFUART_MR1_CS8;
	uartp[MCFUART_UMR] = MCFUART_MR2_STOP1;

	rs_serial_setbaudrate(port,baudrate);

	uartp[MCFUART_UCSR] = MCFUART_UCSR_RXCLKTIMER | MCFUART_UCSR_TXCLKTIMER;
	uartp[MCFUART_UCR] = MCFUART_UCR_RXENABLE | MCFUART_UCR_TXENABLE;

	return;
}

/****************************************************************************/
/*
 *	Output a single character, using UART polled mode.
 *	This is used for console output.
 */

void rs_put_char(char ch)
{
	volatile unsigned char	*uartp;
	int			i;

	uartp = (volatile unsigned char *) (CFG_MBAR + MCFUART_BASE1);

	for (i = 0; (i < 0x10000); i++) {
		if (uartp[MCFUART_USR] & MCFUART_USR_TXREADY)
			break;
	}
	uartp[MCFUART_UTB] = ch;
	return;
}

int rs_is_char(void)
{
	volatile unsigned char	*uartp;

	uartp = (volatile unsigned char *) (CFG_MBAR + MCFUART_BASE1);
	return((uartp[MCFUART_USR] & MCFUART_USR_RXREADY) ? 1 : 0);
}

int rs_get_char(void)
{
	volatile unsigned char	*uartp;

	uartp = (volatile unsigned char *) (CFG_MBAR + MCFUART_BASE1);
	return(uartp[MCFUART_URB]);
}

void serial_setbrg(void) {
	DECLARE_GLOBAL_DATA_PTR;
	rs_serial_setbaudrate(0,gd->bd->bi_baudrate);
}

int serial_init(void) {
	DECLARE_GLOBAL_DATA_PTR;
	rs_serial_init(0,gd->baudrate);
	return 0;
}


void serial_putc(const char c) {
	if (c == '\n')
		serial_putc ('\r');
	rs_put_char(c);
}

void serial_puts (const char *s) {
	while (*s) {
		serial_putc(*s++);
	}
}

int serial_getc(void) {
	while(!rs_is_char());
	return rs_get_char();
}

int serial_tstc() {
	return rs_is_char();
}
