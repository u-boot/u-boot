/*
 * linux/arch/$(ARCH)/platform/$(PLATFORM)/irq.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
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
 * Copyright (c) 2004 LG Soft India.
 * Copyright (c) 2004 HHTech.
 *
 * Adapted for BlackFin BF561 by Bas Vermeulen <bas@buyways.nl>
 * Copyright (c) 2005 BuyWays B.V. (www.buyways.nl)
 */

#ifndef _BF561_IRQ_H_
#define _BF561_IRQ_H_

/*
 * Interrupt source definitions:
 *	Event Source		Core Event Name	    IRQ No
 *	Emulation Events		EMU		0
 *	Reset				RST		1
 *	NMI				NMI		2
 *	Exception			EVX		3
 *	Reserved			--		4
 *	Hardware Error			IVHW		5
 *	Core Timer			IVTMR		6
 *
 *	PLL Wakeup Interrupt		IVG7		7
 *	DMA1 Error (generic)		IVG7		8
 *	DMA2 Error (generic)		IVG7		9
 *	IMDMA Error (generic)		IVG7		10
 *	PPI1 Error Interrupt		IVG7		11
 *	PPI2 Error Interrupt		IVG7		12
 *	SPORT0 Error Interrupt		IVG7		13
 *	SPORT1 Error Interrupt		IVG7		14
 *	SPI Error Interrupt		IVG7		15
 *	UART Error Interrupt		IVG7		16
 *	Reserved Interrupt		IVG7		17
 *
 *	DMA1 0  Interrupt(PPI1)		IVG8		18
 *	DMA1 1  Interrupt(PPI2)		IVG8		19
 *	DMA1 2  Interrupt		IVG8		20
 *	DMA1 3  Interrupt		IVG8		21
 *	DMA1 4  Interrupt		IVG8		22
 *	DMA1 5  Interrupt		IVG8		23
 *	DMA1 6  Interrupt		IVG8		24
 *	DMA1 7  Interrupt		IVG8		25
 *	DMA1 8  Interrupt		IVG8		26
 *	DMA1 9  Interrupt		IVG8		27
 *	DMA1 10 Interrupt		IVG8		28
 *	DMA1 11 Interrupt		IVG8		29
 *
 *	DMA2 0  (SPORT0 RX)		IVG9		30
 *	DMA2 1  (SPORT0 TX)		IVG9		31
 *	DMA2 2  (SPORT1 RX)		IVG9		32
 *	DMA2 3  (SPORT2 TX)		IVG9		33
 *	DMA2 4  (SPI)			IVG9		34
 *	DMA2 5  (UART RX)		IVG9		35
 *	DMA2 6  (UART TX)		IVG9		36
 *	DMA2 7  Interrupt		IVG9		37
 *	DMA2 8  Interrupt		IVG9		38
 *	DMA2 9  Interrupt		IVG9		39
 *	DMA2 10 Interrupt		IVG9		40
 *	DMA2 11 Interrupt		IVG9		41
 *
 *	TIMER 0  Interrupt		IVG10		42
 *	TIMER 1  Interrupt		IVG10		43
 *	TIMER 2  Interrupt		IVG10		44
 *	TIMER 3  Interrupt		IVG10		45
 *	TIMER 4  Interrupt		IVG10		46
 *	TIMER 5  Interrupt		IVG10		47
 *	TIMER 6  Interrupt		IVG10		48
 *	TIMER 7  Interrupt		IVG10		49
 *	TIMER 8  Interrupt		IVG10		50
 *	TIMER 9  Interrupt		IVG10		51
 *	TIMER 10 Interrupt		IVG10		52
 *	TIMER 11 Interrupt		IVG10		53
 *
 *	Programmable Flags0 A (8)	IVG11		54
 *	Programmable Flags0 B (8)	IVG11		55
 *	Programmable Flags1 A (8)	IVG11		56
 *	Programmable Flags1 B (8)	IVG11		57
 *	Programmable Flags2 A (8)	IVG11		58
 *	Programmable Flags2 B (8)	IVG11		59
 *
 *	MDMA1 0 write/read INT		IVG8		60
 *	MDMA1 1 write/read INT		IVG8		61
 *
 *	MDMA2 0 write/read INT		IVG9		62
 *	MDMA2 1 write/read INT		IVG9		63
 *
 *	IMDMA 0 write/read INT		IVG12		64
 *	IMDMA 1 write/read INT		IVG12		65
 *
 *	Watch Dog Timer			IVG13		66
 *
 *	Reserved interrupt		IVG7		67
 *	Reserved interrupt		IVG7		68
 *	Supplemental interrupt 0	IVG7		69
 *	supplemental interrupt 1	IVG7		70
 *
 *	Software Interrupt 1		IVG14		71
 *	Software Interrupt 2		IVG15		72
 */

/*
 * The ABSTRACT IRQ definitions
 *  the first seven of the following are fixed,
 *  the rest you change if you need to.
 */
/* IVG 0-6 */
#define	IRQ_EMU			0	/* Emulation */
#define	IRQ_RST			1	/* Reset */
#define	IRQ_NMI			2	/* Non Maskable Interrupt */
#define	IRQ_EVX			3	/* Exception */
#define	IRQ_UNUSED		4	/* Reserved interrupt */
#define	IRQ_HWERR		5	/* Hardware Error */
#define	IRQ_CORETMR		6	/* Core timer */

#define	IRQ_UART_RX_BIT		0x10000000
#define	IRQ_UART_TX_BIT		0x20000000
#define	IRQ_UART_ERROR_BIT	0x200

#endif				/* _BF561_IRQ_H_ */
