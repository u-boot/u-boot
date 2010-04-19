/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * Hacked for MPC8260 by Murray.Jensen@cmst.csiro.au, 22-Oct-00
 */

#include <common.h>
#include <command.h>
#include <mpc8260.h>
#include <mpc8260_irq.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

/****************************************************************************/

struct irq_action {
	interrupt_handler_t *handler;
	void *arg;
	ulong count;
};

static struct irq_action irq_handlers[NR_IRQS];

static ulong ppc_cached_irq_mask[NR_MASK_WORDS];

/****************************************************************************/
/* this section was ripped out of arch/ppc/kernel/ppc8260_pic.c in the	    */
/* Linux/PPC 2.4.x source. There was no copyright notice in that file.	    */

/* The 8260 internal interrupt controller.  It is usually
 * the only interrupt controller.
 * There are two 32-bit registers (high/low) for up to 64
 * possible interrupts.
 *
 * Now, the fun starts.....Interrupt Numbers DO NOT MAP
 * in a simple arithmetic fashion to mask or pending registers.
 * That is, interrupt 4 does not map to bit position 4.
 * We create two tables, indexed by vector number, to indicate
 * which register to use and which bit in the register to use.
 */
static u_char irq_to_siureg[] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

static u_char irq_to_siubit[] = {
	31, 16, 17, 18, 19, 20, 21, 22,
	23, 24, 25, 26, 27, 28, 29, 30,
	29, 30, 16, 17, 18, 19, 20, 21,
	22, 23, 24, 25, 26, 27, 28, 31,
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 10, 11, 12, 13, 14, 15,
	15, 14, 13, 12, 11, 10, 9, 8,
	7, 6, 5, 4, 3, 2, 1, 0
};

static void m8260_mask_irq (unsigned int irq_nr)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	int bit, word;
	volatile uint *simr;

	bit = irq_to_siubit[irq_nr];
	word = irq_to_siureg[irq_nr];

	simr = &(immr->im_intctl.ic_simrh);
	ppc_cached_irq_mask[word] &= ~(1 << (31 - bit));
	simr[word] = ppc_cached_irq_mask[word];
}

static void m8260_unmask_irq (unsigned int irq_nr)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	int bit, word;
	volatile uint *simr;

	bit = irq_to_siubit[irq_nr];
	word = irq_to_siureg[irq_nr];

	simr = &(immr->im_intctl.ic_simrh);
	ppc_cached_irq_mask[word] |= (1 << (31 - bit));
	simr[word] = ppc_cached_irq_mask[word];
}

static void m8260_mask_and_ack (unsigned int irq_nr)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	int bit, word;
	volatile uint *simr, *sipnr;

	bit = irq_to_siubit[irq_nr];
	word = irq_to_siureg[irq_nr];

	simr = &(immr->im_intctl.ic_simrh);
	sipnr = &(immr->im_intctl.ic_sipnrh);
	ppc_cached_irq_mask[word] &= ~(1 << (31 - bit));
	simr[word] = ppc_cached_irq_mask[word];
	sipnr[word] = 1 << (31 - bit);
}

static int m8260_get_irq (struct pt_regs *regs)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	int irq;
	unsigned long bits;

	/* For MPC8260, read the SIVEC register and shift the bits down
	 * to get the irq number.         */
	bits = immr->im_intctl.ic_sivec;
	irq = bits >> 26;
	return irq;
}

/* end of code ripped out of arch/ppc/kernel/ppc8260_pic.c		    */
/****************************************************************************/

int interrupt_init_cpu (unsigned *decrementer_count)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;

	*decrementer_count = (gd->bus_clk / 4) / CONFIG_SYS_HZ;

	/* Initialize the default interrupt mapping priorities */
	immr->im_intctl.ic_sicr = 0;
	immr->im_intctl.ic_siprr = 0x05309770;
	immr->im_intctl.ic_scprrh = 0x05309770;
	immr->im_intctl.ic_scprrl = 0x05309770;

	/* disable all interrupts and clear all pending bits */
	immr->im_intctl.ic_simrh = ppc_cached_irq_mask[0] = 0;
	immr->im_intctl.ic_simrl = ppc_cached_irq_mask[1] = 0;
	immr->im_intctl.ic_sipnrh = 0xffffffff;
	immr->im_intctl.ic_sipnrl = 0xffffffff;

#ifdef CONFIG_HYMOD
	/*
	 * ensure all external interrupt sources default to trigger on
	 * high-to-low transition (i.e. edge triggered active low)
	 */
	immr->im_intctl.ic_siexr = -1;
#endif

	return (0);
}

/****************************************************************************/

/*
 * Handle external interrupts
 */
void external_interrupt (struct pt_regs *regs)
{
	int irq, unmask = 1;

	irq = m8260_get_irq (regs);

	m8260_mask_and_ack (irq);

	enable_interrupts ();

	if (irq_handlers[irq].handler != NULL)
		(*irq_handlers[irq].handler) (irq_handlers[irq].arg);
	else {
		printf ("\nBogus External Interrupt IRQ %d\n", irq);
		/*
		 * turn off the bogus interrupt, otherwise it
		 * might repeat forever
		 */
		unmask = 0;
	}

	if (unmask)
		m8260_unmask_irq (irq);
}

/****************************************************************************/

/*
 * Install and free an interrupt handler.
 */

void
irq_install_handler (int irq, interrupt_handler_t * handler, void *arg)
{
	if (irq < 0 || irq >= NR_IRQS) {
		printf ("irq_install_handler: bad irq number %d\n", irq);
		return;
	}

	if (irq_handlers[irq].handler != NULL)
		printf ("irq_install_handler: 0x%08lx replacing 0x%08lx\n",
				(ulong) handler, (ulong) irq_handlers[irq].handler);

	irq_handlers[irq].handler = handler;
	irq_handlers[irq].arg = arg;

	m8260_unmask_irq (irq);
}

void irq_free_handler (int irq)
{
	if (irq < 0 || irq >= NR_IRQS) {
		printf ("irq_free_handler: bad irq number %d\n", irq);
		return;
	}

	m8260_mask_irq (irq);

	irq_handlers[irq].handler = NULL;
	irq_handlers[irq].arg = NULL;
}

/****************************************************************************/

void timer_interrupt_cpu (struct pt_regs *regs)
{
	/* nothing to do here */
	return;
}

/****************************************************************************/

#if defined(CONFIG_CMD_IRQ)

/* ripped this out of ppc4xx/interrupts.c */

/*******************************************************************************
*
* irqinfo - print information about PCI devices
*
*/
void
do_irqinfo (cmd_tbl_t * cmdtp, bd_t * bd, int flag, int argc, char *argv[])
{
	int irq, re_enable;

	re_enable = disable_interrupts ();

	puts ("\nInterrupt-Information:\n"
		"Nr  Routine   Arg       Count\n");

	for (irq = 0; irq < 32; irq++)
		if (irq_handlers[irq].handler != NULL)
			printf ("%02d  %08lx  %08lx  %ld\n", irq,
					(ulong) irq_handlers[irq].handler,
					(ulong) irq_handlers[irq].arg,
					irq_handlers[irq].count);

	if (re_enable)
		enable_interrupts ();
}

#endif
