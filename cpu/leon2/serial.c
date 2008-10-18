/* GRLIB APBUART Serial controller driver
 *
 * (C) Copyright 2008
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
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
 *
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/leon.h>

DECLARE_GLOBAL_DATA_PTR;

/* Force cache miss each time a serial controller reg is read */
#define CACHE_BYPASS 1

#ifdef CACHE_BYPASS
#define READ_BYTE(var)  SPARC_NOCACHE_READ_BYTE((unsigned int)&(var))
#define READ_HWORD(var) SPARC_NOCACHE_READ_HWORD((unsigned int)&(var))
#define READ_WORD(var)  SPARC_NOCACHE_READ((unsigned int)&(var))
#define READ_DWORD(var) SPARC_NOCACHE_READ_DWORD((unsigned int)&(var))
#endif

int serial_init(void)
{
	LEON2_regs *leon2 = (LEON2_regs *) LEON2_PREGS;
	LEON2_Uart_regs *regs;
	unsigned int tmp;

	/* Init LEON2 UART
	 *
	 * Set scaler / baud rate
	 *
	 * Receiver & transmitter enable
	 */
#if LEON2_CONSOLE_SELECT == LEON_CONSOLE_UART1
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_1;
#else
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_2;
#endif

	regs->UART_Scaler = CONFIG_SYS_LEON2_UART1_SCALER;

	/* Let bit 11 be unchanged (debug bit for GRMON) */
	tmp = READ_WORD(regs->UART_Control);

	regs->UART_Control = ((tmp & LEON2_UART_CTRL_DBG) |
			      (LEON2_UART1_LOOPBACK_ENABLE << 7) |
			      (LEON2_UART1_FLOWCTRL_ENABLE << 6) |
			      (LEON2_UART1_PARITY_ENABLE << 5) |
			      (LEON2_UART1_ODDPAR_ENABLE << 4) |
			      LEON2_UART_CTRL_RE | LEON2_UART_CTRL_TE);

	return 0;
}

void serial_putc(const char c)
{
	if (c == '\n')
		serial_putc_raw('\r');

	serial_putc_raw(c);
}

void serial_putc_raw(const char c)
{
	LEON2_regs *leon2 = (LEON2_regs *) LEON2_PREGS;
	LEON2_Uart_regs *regs;

#if LEON2_CONSOLE_SELECT == LEON_CONSOLE_UART1
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_1;
#else
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_2;
#endif

	/* Wait for last character to go. */
	while (!(READ_WORD(regs->UART_Status) & LEON2_UART_STAT_THE)) ;

	/* Send data */
	regs->UART_Channel = c;

#ifdef LEON_DEBUG
	/* Wait for data to be sent */
	while (!(READ_WORD(regs->UART_Status) & LEON2_UART_STAT_TSE)) ;
#endif
}

void serial_puts(const char *s)
{
	while (*s) {
		serial_putc(*s++);
	}
}

int serial_getc(void)
{
	LEON2_regs *leon2 = (LEON2_regs *) LEON2_PREGS;
	LEON2_Uart_regs *regs;

#if LEON2_CONSOLE_SELECT == LEON_CONSOLE_UART1
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_1;
#else
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_2;
#endif

	/* Wait for a character to arrive. */
	while (!(READ_WORD(regs->UART_Status) & LEON2_UART_STAT_DR)) ;

	/* read data */
	return READ_WORD(regs->UART_Channel);
}

int serial_tstc(void)
{
	LEON2_regs *leon2 = (LEON2_regs *) LEON2_PREGS;
	LEON2_Uart_regs *regs;

#if LEON2_CONSOLE_SELECT == LEON_CONSOLE_UART1
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_1;
#else
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_2;
#endif

	return (READ_WORD(regs->UART_Status) & LEON2_UART_STAT_DR);
}

/* set baud rate for uart */
void serial_setbrg(void)
{
	/* update baud rate settings, read it from gd->baudrate */
	unsigned int scaler;
	LEON2_regs *leon2 = (LEON2_regs *) LEON2_PREGS;
	LEON2_Uart_regs *regs;

#if LEON2_CONSOLE_SELECT == LEON_CONSOLE_UART1
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_1;
#else
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_2;
#endif

	if (gd->baudrate > 0) {
		scaler =
		    (((CONFIG_SYS_CLK_FREQ * 10) / (gd->baudrate * 8)) -
		     5) / 10;
		regs->UART_Scaler = scaler;
	}
}
