/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/ixp425.h>

#ifdef CONFIG_USE_IRQ
#include <asm/proc-armv/ptrace.h>

/*
 * When interrupts are enabled, use timer 2 for time/delay generation...
 */

#define FREQ		66666666
#define CLOCK_TICK_RATE	(((FREQ / CFG_HZ & ~IXP425_OST_RELOAD_MASK) + 1) * CFG_HZ)
#define LATCH		((CLOCK_TICK_RATE + CFG_HZ/2) / CFG_HZ)	/* For divider */

struct _irq_handler {
	void                *m_data;
	void (*m_func)( void *data);
};

static struct _irq_handler IRQ_HANDLER[N_IRQS];

static volatile ulong timestamp;

static void default_isr(void *data)
{
	printf("default_isr():  called for IRQ %d, Interrupt Status=%x PR=%x\n",
	       (int)data, *IXP425_ICIP, *IXP425_ICIH);
}

static int next_irq(void)
{
	return (((*IXP425_ICIH & 0x000000fc) >> 2) - 1);
}

static void timer_isr(void *data)
{
	unsigned int *pTime = (unsigned int *)data;

	(*pTime)++;

	/*
	 * Reset IRQ source
	 */
	*IXP425_OSST = IXP425_OSST_TIMER_2_PEND;
}

ulong get_timer (ulong base)
{
	return timestamp - base;
}

void reset_timer (void)
{
	timestamp = 0;
}

#endif /* #ifdef CONFIG_USE_IRQ */

#ifdef CONFIG_USE_IRQ
void do_irq (struct pt_regs *pt_regs)
{
	int irq = next_irq();

	IRQ_HANDLER[irq].m_func(IRQ_HANDLER[irq].m_data);
}
#endif

int interrupt_init (void)
{
#ifdef CONFIG_USE_IRQ
	int i;

	/* install default interrupt handlers */
	for (i = 0; i < N_IRQS; i++) {
		IRQ_HANDLER[i].m_data = (void *)i;
		IRQ_HANDLER[i].m_func = default_isr;
	}

	/* install interrupt handler for timer */
	IRQ_HANDLER[IXP425_TIMER_2_IRQ].m_data = (void *)&timestamp;
	IRQ_HANDLER[IXP425_TIMER_2_IRQ].m_func = timer_isr;

	/* setup the Timer counter value */
	*IXP425_OSRT2 = (LATCH & ~IXP425_OST_RELOAD_MASK) | IXP425_OST_ENABLE;

	/* configure interrupts for IRQ mode */
	*IXP425_ICLR = 0x00000000;

	/* enable timer irq */
	*IXP425_ICMR = (1 << IXP425_TIMER_2_IRQ);
#endif

	return (0);
}
