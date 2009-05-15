/*
 * (C) Copyright 2002
 * Lineo, Inc. <www.lineo.com>
 * Bernhard Kuhn <bkuhn@lineo.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
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
#include <asm/arch/hardware.h>

void board_reset(void) __attribute__((__weak__));

/*
 * Reset the cpu by setting up the watchdog timer and let him time out
 * or toggle a GPIO pin on the AT91RM9200DK board
 */
void reset_cpu (ulong ignored)
{

#if defined(CONFIG_AT91RM9200_USART)
	/*shutdown the console to avoid strange chars during reset */
	serial_exit();
#endif

	if (board_reset)
		board_reset();

	/* this is the way Linux does it */

	/* FIXME:
	 * These defines should be moved into
	 * include/asm-arm/arch-at91rm9200/AT91RM9200.h
	 * as soon as the whitespace fix gets applied.
	 */
	#define AT91C_ST_RSTEN (0x1 << 16)
	#define AT91C_ST_EXTEN (0x1 << 17)
	#define AT91C_ST_WDRST (0x1 <<  0)
	#define ST_WDMR *((unsigned long *)0xfffffd08)	/* watchdog mode register */
	#define ST_CR *((unsigned long *)0xfffffd00)	/* system clock control register */

	ST_WDMR = AT91C_ST_RSTEN | AT91C_ST_EXTEN | 1 ;
	ST_CR = AT91C_ST_WDRST;

	while (1);
	/* Never reached */
}
