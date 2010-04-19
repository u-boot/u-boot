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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_st.h>

void board_reset(void) __attribute__((__weak__));

void reset_cpu(ulong ignored)
{
	at91_st_t *st = (at91_st_t *) AT91_ST_BASE;
#if defined(CONFIG_AT91RM9200_USART)
	/*shutdown the console to avoid strange chars during reset */
	serial_exit();
#endif

	if (board_reset)
		board_reset();

	/* Reset the cpu by setting up the watchdog timer */
	writel(AT91_ST_WDMR_RSTEN | AT91_ST_WDMR_EXTEN | AT91_ST_WDMR_WDV(2),
		&st->wdmr);
	writel(AT91_ST_CR_WDRST, &st->cr);
	/* and let it timeout */
	while (1)
		;
	/* Never reached */
}
