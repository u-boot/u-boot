/*
 * U-boot bf533_irq.h
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * This file is based on
 * linux/arch/$(ARCH)/platform/$(PLATFORM)/irq.c
 * Changed by HuTao Apr18, 2003
 *
 * Copyright was missing when I got the code so took from MIPS arch ...MaTed---
 * Copyright (C) 1994 by Waldorf GMBH, written by Ralf Baechle
 * Copyright (C) 1995, 96, 97, 98, 99, 2000, 2001 by Ralf Baechle
 *
 * Adapted for BlackFin (ADI) by Ted Ma <mated@sympatico.ca>
 * Copyright (c) 2002 Arcturus Networks Inc. (www.arcturusnetworks.com)
 * Copyright (c) 2002 Lineo, Inc. <mattw@lineo.com>
 *
 * Adapted for BlackFin BF533 by Bas Vermeulen <bas@buyways.nl>
 * Copyright (c) 2003 BuyWays B.V. (www.buyways.nl)

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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef _BF533_IRQ_H_
#define _BF533_IRQ_H_

/*
 * Interrupt source definitions
 * Event Source			Core Event Name		Number
 * 				EMU			0
 * Reset			RST			1
 * NMI				NMI			2
 * Exception			EVX			3
 * Reserved			--			4
 * Hardware Error		IVHW			5
 * Core Timer			IVTMR			6
 * PLL Wakeup Interrupt		IVG7			7
 * DMA Error (generic)		IVG7			8
 * PPI Error Interrupt		IVG7			9
 * SPORT0 Error Interrupt	IVG7			10
 * SPORT1 Error Interrupt	IVG7			11
 * SPI Error Interrupt		IVG7			12
 * UART Error Interrupt		IVG7			13
 * RTC Interrupt		IVG8			14
 * DMA0 Interrupt (PPI)		IVG8			15
 * DMA1 (SPORT0 RX)		IVG9			16
 * DMA2 (SPORT0 TX)		IVG9			17
 * DMA3 (SPORT1 RX)		IVG9			18
 * DMA4 (SPORT1 TX)		IVG9			19
 * DMA5 (PPI)			IVG10			20
 * DMA6 (UART RX)		IVG10			21
 * DMA7 (UART TX)		IVG10			22
 * Timer0			IVG11			23
 * Timer1			IVG11			24
 * Timer2			IVG11			25
 * PF Interrupt A		IVG12			26
 * PF Interrupt B		IVG12			27
 * DMA8/9 Interrupt		IVG13			28
 * DMA10/11 Interrupt		IVG13			29
 * Watchdog Timer		IVG13			30
 * Software Interrupt 1		IVG14			31
 * Software Interrupt 2		--
 * (lowest priority)		IVG15			32
 */

/* The ABSTRACT IRQ definitions */

/* The first seven of the following are fixed,
 * the rest you change if you need to
 */

#define	IRQ_EMU			0	/* Emulation */
#define	IRQ_RST			1	/* reset */
#define	IRQ_NMI			2	/* Non Maskable */
#define	IRQ_EVX			3	/* Exception */
#define	IRQ_UNUSED		4	/*  - unused interrupt */
#define	IRQ_HWERR		5	/* Hardware Error */
#define	IRQ_CORETMR		6	/* Core timer */
#define	IRQ_PLL_WAKEUP		7	/* PLL Wakeup Interrupt */
#define	IRQ_DMA_ERROR		8	/* DMA Error (general) */
#define	IRQ_PPI_ERROR		9	/* PPI Error Interrupt */
#define	IRQ_SPORT0_ERROR	10	/* SPORT0 Error Interrupt */
#define	IRQ_SPORT1_ERROR	11	/* SPORT1 Error Interrupt */
#define	IRQ_SPI_ERROR		12	/* SPI Error Interrupt */
#define	IRQ_UART_ERROR		13	/* UART Error Interrupt */
#define	IRQ_RTC			14	/* RTC Interrupt */
#define	IRQ_PPI			15	/* DMA0 Interrupt (PPI) */
#define	IRQ_SPORT0		16	/* DMA1 Interrupt (SPORT0 RX) */
#define	IRQ_SPARE1		17	/* DMA2 Interrupt (SPORT0 TX) */
#define	IRQ_SPORT1		18	/* DMA3 Interrupt (SPORT1 RX) */
#define	IRQ_SPARE2		19	/* DMA4 Interrupt (SPORT1 TX) */
#define IRQ_SPI			20	/* DMA5 Interrupt (SPI) */
#define	IRQ_UART		21	/* DMA6 Interrupt (UART RX) */
#define	IRQ_SPARE3		22	/* DMA7 Interrupt (UART TX) */
#define	IRQ_TMR0		23	/* Timer 0 */
#define	IRQ_TMR1		24	/* Timer 1 */
#define	IRQ_TMR2		25	/* Timer 2 */
#define	IRQ_PROG_INTA		26	/* Programmable Flags A (8) */
#define	IRQ_PROG_INTB		27	/* Programmable Flags B (8) */
#define	IRQ_MEM_DMA0		28	/* DMA8/9 Interrupt (Memory DMA Stream 0) */
#define	IRQ_MEM_DMA1		29	/* DMA10/11 Interrupt (Memory DMA Stream 1) */
#define	IRQ_WATCH	   	30	/* Watch Dog Timer */
#define	IRQ_SW_INT1		31	/* Software Int 1 */
#define	IRQ_SW_INT2		32	/* Software Int 2 (reserved for SYSCALL) */

#define IRQ_UART_RX_BIT		0x4000
#define IRQ_UART_TX_BIT		0x8000
#define IRQ_UART_ERROR_BIT	0x40

#define IVG7			7
#define IVG8			8
#define IVG9			9
#define IVG10			10
#define IVG11			11
#define IVG12			12
#define IVG13			13
#define IVG14			14
#define IVG15			15
#define SYS_IRQS		33

#endif
