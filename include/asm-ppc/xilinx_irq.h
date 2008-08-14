/*
 * (C) Copyright 2008
 * Ricado Ribalda-Universidad Autonoma de Madrid-ricardo.ribalda@uam.es
 * This work has been supported by: QTechnology  http://qtec.com/
 * Based on interrupts.c Wolfgang Denk-DENX Software Engineering-wd@denx.de
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
