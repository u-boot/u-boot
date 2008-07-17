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
#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <asm/processor.h>
#include <asm/interrupt.h>
#include <ppc4xx.h>
#include <ppc_asm.tmpl>
#include <commproc.h>
#include <asm/io.h>
#include <asm/xilinx_irq.h>

DECLARE_GLOBAL_DATA_PTR;

void pic_enable(void)
{
	debug("Xilinx PIC at 0x%8x\n", intc);

	/*
	 * Disable all external interrupts until they are
	 * explicitly requested.
	 */
	out_be32((u32 *) IER, 0);

	/* Acknowledge any pending interrupts just in case. */
	out_be32((u32 *) IAR, 0xffffffff);

	/* Turn on the Master Enable. */
	out_be32((u32 *) MER, 0x3UL);

	return;
}

int xilinx_pic_irq_get(void)
{
	u32 irq;
	irq = in_be32((u32 *) IVR);

	/* If no interrupt is pending then all bits of the IVR are set to 1. As
	 * the IVR is as many bits wide as numbers of inputs are available.
	 * Therefore, if all bits of the IVR are set to one, its content will
	 * be bigger than XPAR_INTC_MAX_NUM_INTR_INPUTS.
	 */
	if (irq >= XPAR_INTC_MAX_NUM_INTR_INPUTS)
		irq = -1;	/* report no pending interrupt. */

	debug("get_irq: %d\n", irq);
	return (irq);
}

void pic_irq_enable(unsigned int irq)
{
	u32 mask = IRQ_MASK(irq);
	debug("enable: %d\n", irq);
	out_be32((u32 *) SIE, mask);
}

void pic_irq_disable(unsigned int irq)
{
	u32 mask = IRQ_MASK(irq);
	debug("disable: %d\n", irq);
	out_be32((u32 *) CIE, mask);
}

void pic_irq_ack(unsigned int irq)
{
	u32 mask = IRQ_MASK(irq);
	debug("ack: %d\n", irq);
	out_be32((u32 *) IAR, mask);
}

void external_interrupt(struct pt_regs *regs)
{
	int irq;

	irq = xilinx_pic_irq_get();
	if (irq < 0)
		return;

	interrupt_run_handler(irq);

	return;
}
