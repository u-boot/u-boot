/*
 * ColdFire Internal Memory Map and Defines
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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

#ifndef __IMMAP_H
#define __IMMAP_H
    
#ifdef CONFIG_M5329
#include <asm/immap_5329.h>
#include <asm/m5329.h>

#define CFG_FEC0_IOBASE		(MMAP_FEC)
#define CFG_UART_BASE		(MMAP_UART0 + (CFG_UART_PORT * 0x4000))
#define CFG_MCFRTC_BASE		(MMAP_RTC)

/* Timer */
#ifdef CONFIG_MCFTMR
#define CFG_UDELAY_BASE		(MMAP_DTMR0)
#define CFG_TMR_BASE		(MMAP_DTMR1)
#define CFG_TMRINTR_NO		(INT0_HI_DTMR1)
#define CFG_TMRINTR_MASK	(INTC_IPRH_INT33)
#define CFG_TMRINTR_PRI		(6)
#define CFG_TIMER_PRESCALER	(((CFG_CLK / 1000000) - 1) << 8)
#endif

#ifdef CONFIG_MCFPIT
#define CFG_UDELAY_BASE		(MMAP_PIT0)
#define CFG_PIT_BASE		(MMAP_PIT1)
#define CFG_PIT_PRESCALE	(6)
#endif

#define CFG_INTR_BASE		(MMAP_INTC0)
#define CFG_NUM_IRQS		(128)
#endif				/* CONFIG_M5329 */
    
#endif				/* __IMMAP_H */
