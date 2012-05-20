/*
 * Copyright (C) 2011 by Vladimir Zapolskiy <vz@mleia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#ifndef _LPC32XX_WDT_H
#define _LPC32XX_WDT_H

#include <asm/types.h>

/* Watchdog Timer Registers */
struct wdt_regs {
	u32 isr;		/* Interrupt Status Register		*/
	u32 ctrl;		/* Control Register			*/
	u32 counter;		/* Counter Value Register		*/
	u32 mctrl;		/* Match Control Register		*/
	u32 match0;		/* Match 0 Register			*/
	u32 emr;		/* External Match Control Register	*/
	u32 pulse;		/* Reset Pulse Length Register		*/
	u32 res;		/* Reset Source Register		*/
};

/* Watchdog Timer Control Register bits */
#define WDTIM_CTRL_PAUSE_EN		(1 << 2)
#define WDTIM_CTRL_RESET_COUNT		(1 << 1)
#define WDTIM_CTRL_COUNT_ENAB		(1 << 0)

/* Watchdog Timer Match Control Register bits */
#define WDTIM_MCTRL_RESFRC2		(1 << 6)
#define WDTIM_MCTRL_RESFRC1		(1 << 5)
#define WDTIM_MCTRL_M_RES2		(1 << 4)
#define WDTIM_MCTRL_M_RES1		(1 << 3)
#define WDTIM_MCTRL_STOP_COUNT0		(1 << 2)
#define WDTIM_MCTRL_RESET_COUNT0	(1 << 1)
#define WDTIM_MCTRL_MR0_INT		(1 << 0)

#endif /* _LPC32XX_WDT_H */
