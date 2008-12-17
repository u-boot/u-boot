/*
 * (C) Copyright 2000-2002	Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * (C) Copyright 2003		Martin Winistoerfer, martinwinistoerfer@gmx.ch.
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
 * Foundation,
 */

/*
 * File:		interrupt.c
 *
 * Discription:		Contains interrupt routines needed by U-Boot
 *
 */

#include <common.h>
#include <command.h>
#include <mpc5xx.h>
#include <asm/processor.h>

#if defined(CONFIG_PATI)
/* PATI uses IRQs for PCI doorbell */
#undef NR_IRQS
#define NR_IRQS 16
#endif

struct interrupt_action {
	interrupt_handler_t *handler;
	void *arg;
	int count;
};

static struct interrupt_action irq_vecs[NR_IRQS];

/*
 * Initialise interrupts
 */

int interrupt_init_cpu (ulong *decrementer_count)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	int vec;

	/* Decrementer used here for status led */
	*decrementer_count = get_tbclk () / CONFIG_SYS_HZ;

	/* Disable all interrupts */
	immr->im_siu_conf.sc_simask = 0;
	for (vec=0; vec<NR_IRQS; vec++) {
		irq_vecs[vec].handler = NULL;
		irq_vecs[vec].arg = NULL;
		irq_vecs[vec].count = 0;
	}

	return (0);
}

/*
 * Handle external interrupts
 */
void external_interrupt (struct pt_regs *regs)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	int irq;
	ulong simask, newmask;
	ulong vec, v_bit;

	/*
	 * read the SIVEC register and shift the bits down
	 * to get the irq number
	 */
	vec = immr->im_siu_conf.sc_sivec;
	irq = vec >> 26;
	v_bit = 0x80000000UL >> irq;

	/*
	 * Read Interrupt Mask Register and Mask Interrupts
	 */
	simask = immr->im_siu_conf.sc_simask;
	newmask = simask & (~(0xFFFF0000 >> irq));
	immr->im_siu_conf.sc_simask = newmask;

	if (!(irq & 0x1)) {		/* External Interrupt ?     */
		ulong siel;

		/*
		 * Read Interrupt Edge/Level Register
		 */
		siel = immr->im_siu_conf.sc_siel;

		if (siel & v_bit) {	/* edge triggered interrupt ?   */
			/*
			 * Rewrite SIPEND Register to clear interrupt
			 */
			immr->im_siu_conf.sc_sipend = v_bit;
		}
	}

	if (irq_vecs[irq].handler != NULL) {
		irq_vecs[irq].handler (irq_vecs[irq].arg);
	} else {
		printf ("\nBogus External Interrupt IRQ %d Vector %ld\n",
				irq, vec);
		/* turn off the bogus interrupt to avoid it from now */
		simask &= ~v_bit;
	}
	/*
	 * Re-Enable old Interrupt Mask
	 */
	immr->im_siu_conf.sc_simask = simask;
}

/*
 * Install and free an interrupt handler
 */
void irq_install_handler (int vec, interrupt_handler_t * handler,
						  void *arg)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	/* SIU interrupt */
	if (irq_vecs[vec].handler != NULL) {
		printf ("SIU interrupt %d 0x%x\n",
			vec,
			(uint) handler);
	}
	irq_vecs[vec].handler = handler;
	irq_vecs[vec].arg = arg;
	immr->im_siu_conf.sc_simask |= 1 << (31 - vec);
#if 0
	printf ("Install SIU interrupt for vector %d ==> %p\n",
		vec, handler);
#endif
}

void irq_free_handler (int vec)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	/* SIU interrupt */
#if 0
	printf ("Free CPM interrupt for vector %d\n",
		vec);
#endif
	immr->im_siu_conf.sc_simask &= ~(1 << (31 - vec));
	irq_vecs[vec].handler = NULL;
	irq_vecs[vec].arg = NULL;
}

/*
 *  Timer interrupt - gets called when  bit 0 of DEC changes from
 *  0. Decrementer is enabled with bit TBE in TBSCR.
 */
void timer_interrupt_cpu (struct pt_regs *regs)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;

#if 0
	printf ("*** Timer Interrupt *** ");
#endif
	/* Reset Timer Status Bit and Timers Interrupt Status */
	immr->im_clkrstk.cark_plprcrk = KAPWR_KEY;
	__asm__ ("nop");
	immr->im_clkrst.car_plprcr |= PLPRCR_TEXPS | PLPRCR_TMIST;

	return;
}

#if defined(CONFIG_CMD_IRQ)
/*******************************************************************************
 *
 * irqinfo - print information about IRQs
 *
 */
int do_irqinfo(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int vec;

	printf ("\nInterrupt-Information:\n");
	printf ("Nr  Routine   Arg       Count\n");

	for (vec=0; vec<NR_IRQS; vec++) {
		if (irq_vecs[vec].handler != NULL) {
			printf ("%02d  %08lx  %08lx  %d\n",
				vec,
				(ulong)irq_vecs[vec].handler,
				(ulong)irq_vecs[vec].arg,
				irq_vecs[vec].count);
		}
	}
	return 0;
}


#endif
