/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
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


#include <nios2.h>
#include <nios2-io.h>
#include <asm/ptrace.h>
#include <common.h>
#include <command.h>
#include <watchdog.h>
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#endif

#if defined(CFG_NIOS_TMRBASE) && !defined(CFG_NIOS_TMRIRQ)
#error CFG_NIOS_TMRIRQ not defined (see documentation)
#endif

/****************************************************************************/

struct	irq_action {
	interrupt_handler_t *handler;
	void *arg;
	int count;
};

static struct irq_action vecs[32];

/*************************************************************************/
volatile ulong timestamp = 0;

void reset_timer (void)
{
	timestamp = 0;
}

ulong get_timer (ulong base)
{
	WATCHDOG_RESET ();
	return (timestamp - base);
}

void set_timer (ulong t)
{
	timestamp = t;
}


/* The board must handle this interrupt if a timer is not
 * provided.
 */
#if defined(CFG_NIOS_TMRBASE)
void tmr_isr (void *arg)
{
	nios_timer_t *tmr = (nios_timer_t *)arg;
	/* Interrupt is cleared by writing anything to the
	 * status register.
	 */
	tmr->status = 0;
	timestamp += CFG_NIOS_TMRMS;
#ifdef CONFIG_STATUS_LED
	status_led_tick(timestamp);
#endif
}

static void tmr_init (void)
{
	nios_timer_t *tmr =(nios_timer_t *)CACHE_BYPASS(CFG_NIOS_TMRBASE);

	tmr->control &= ~(NIOS_TIMER_START | NIOS_TIMER_ITO);
	tmr->control |= NIOS_TIMER_STOP;
#if defined(CFG_NIOS_TMRCNT)
	tmr->periodl = CFG_NIOS_TMRCNT & 0xffff;
	tmr->periodh = (CFG_NIOS_TMRCNT >> 16) & 0xffff;
#endif
	tmr->control |= ( NIOS_TIMER_ITO |
			  NIOS_TIMER_CONT |
			  NIOS_TIMER_START );
	irq_install_handler (CFG_NIOS_TMRIRQ, tmr_isr, (void *)tmr);
}

#endif /* CFG_NIOS_TMRBASE */

/*************************************************************************/
int disable_interrupts (void)
{
	int val = rdctl (CTL_STATUS);
	wrctl (CTL_STATUS, val & ~STATUS_IE);
	return (val & STATUS_IE);
}

void enable_interrupts( void )
{
	int val = rdctl (CTL_STATUS);
	wrctl (CTL_STATUS, val | STATUS_IE);
}

void external_interrupt (struct pt_regs *regs)
{
	unsigned irqs;
	struct irq_action *act;

	/* Evaluate only irqs that are both enabled AND pending */
	irqs = rdctl (CTL_IENABLE) & rdctl (CTL_IPENDING);
	act = vecs;

	/* Assume (as does the Nios2 HAL) that bit 0 is highest
	 * priority. NOTE: There is ALWAYS a handler assigned
	 * (the default if no other).
	 */
	while (irqs) {
		if (irqs & 1) {
			act->handler (act->arg);
			act->count++;
		}
		irqs >>=1;
		act++;
	}
}

static void def_hdlr (void *arg)
{
	unsigned irqs = rdctl (CTL_IENABLE);

	/* Disable the individual interrupt -- with gratuitous
	 * warning.
	 */
	irqs &= ~(1 << (int)arg);
	wrctl (CTL_IENABLE, irqs);
	printf ("WARNING: Disabling unhandled interrupt: %d\n",
			(int)arg);
}

/*************************************************************************/
void irq_install_handler (int irq, interrupt_handler_t *hdlr, void *arg)
{

	int flag;
	struct irq_action *act;
	unsigned ena = rdctl (CTL_IENABLE);

	if ((irq < 0) || (irq > 31))
		return;
	act = &vecs[irq];

	flag = disable_interrupts ();
	if (hdlr) {
		act->handler = hdlr;
		act->arg = arg;
		ena |= (1 << irq);		/* enable */
	} else {
		act->handler = def_hdlr;
		act->arg = (void *)irq;
		ena &= ~(1 << irq);		/* disable */
	}
	wrctl (CTL_IENABLE, ena);
	if (flag) enable_interrupts ();
}


int interrupt_init (void)
{
	int i;

	/* Assign the default handler to all */
	for (i = 0; i < 32; i++) {
		vecs[i].handler = def_hdlr;
		vecs[i].arg = (void *)i;
		vecs[i].count = 0;
	}

#if defined(CFG_NIOS_TMRBASE)
	tmr_init ();
#endif

	enable_interrupts ();
	return (0);
}


/*************************************************************************/
#if (CONFIG_COMMANDS & CFG_CMD_IRQ)
int do_irqinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	struct irq_action *act = vecs;

	printf ("\nInterrupt-Information:\n\n");
	printf ("Nr  Routine   Arg       Count\n");
	printf ("-----------------------------\n");

	for (i=0; i<32; i++) {
		if (act->handler != def_hdlr) {
			printf ("%02d  %08lx  %08lx  %d\n",
				i,
				(ulong)act->handler,
				(ulong)act->arg,
				act->count);
		}
		act++;
	}
	printf ("\n");

	return (0);
}
#endif  /* CONFIG_COMMANDS & CFG_CMD_IRQ */
