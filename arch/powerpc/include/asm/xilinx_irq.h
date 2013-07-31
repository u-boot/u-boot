/*
 * (C) Copyright 2008
 * Ricado Ribalda-Universidad Autonoma de Madrid-ricardo.ribalda@uam.es
 * This work has been supported by: QTechnology  http://qtec.com/
 * Based on interrupts.c Wolfgang Denk-DENX Software Engineering-wd@denx.de
 * SPDX-License-Identifier:	GPL-2.0+
*/
#ifndef XILINX_IRQ_H
#define XILINX_IRQ_H

#define intc	XPAR_INTC_0_BASEADDR
#define ISR	(intc + (0 * 4))	/* Interrupt Status Register */
#define IPR	(intc + (1 * 4))	/* Interrupt Pending Register */
#define IER	(intc + (2 * 4))	/* Interrupt Enable Register */
#define IAR	(intc + (3 * 4))	/* Interrupt Acknowledge Register */
#define SIE	(intc + (4 * 4))	/* Set Interrupt Enable bits */
#define CIE	(intc + (5 * 4))	/* Clear Interrupt Enable bits */
#define IVR	(intc + (6 * 4))	/* Interrupt Vector Register */
#define MER	(intc + (7 * 4))	/* Master Enable Register */

#define IRQ_MASK(irq)	(1 << (irq & 0x1f))

#define IRQ_MAX		XPAR_INTC_MAX_NUM_INTR_INPUTS

#endif
