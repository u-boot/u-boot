/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#include <nios2.h>
#include <nios2-io.h>
#include <asm/types.h>
#include <asm/io.h>
#include <asm/ptrace.h>
#include <common.h>
#include <command.h>
#include <watchdog.h>
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#endif

#if defined(CONFIG_SYS_NIOS_TMRBASE) && !defined(CONFIG_SYS_NIOS_TMRIRQ)
#error CONFIG_SYS_NIOS_TMRIRQ not defined (see documentation)
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
	nios_timer_t *tmr =(nios_timer_t *)CONFIG_SYS_NIOS_TMRBASE;

	/* From Embedded Peripherals Handbook:
	 *
	 * "When the hardware is configured with Writeable period
	 * disabled, writing to one of the period_n registers causes
	 * the counter to reset to the fixed Timeout Period specified
	 * at system generation time."
	 *
	 * Here we force a reload to prevent early timeouts from
	 * get_timer() when the interrupt period is greater than
	 * than 1 msec.
	 *
	 * Simply write to periodl with its own value to force an
	 * internal counter reload, THEN reset the timestamp.
	 */
	writel (readl (&tmr->periodl), &tmr->periodl);
	timestamp = 0;

	/* From Embedded Peripherals Handbook:
	 *
	 * "Writing to one of the period_n registers stops the internal
	 * counter, except when the hardware is configured with Start/Stop
	 * control bits off. If Start/Stop control bits is off, writing
	 * either register does not stop the counter."
	 *
	 * In order to accomodate either configuration, the control
	 * register is re-written. If the counter is stopped, it will
	 * be restarted. If it is running, the write is essentially
	 * a nop.
	 */
	writel (NIOS_TIMER_ITO | NIOS_TIMER_CONT | NIOS_TIMER_START,
			&tmr->control);

}

ulong get_timer (ulong base)
{
	WATCHDOG_RESET ();
	return (timestamp - base);
}

/*
 * This function is derived from Blackfin code (read timebase as long long).
 * On Nios2 it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from Blackfin code.
 * On Nios2 it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	ulong tbclk;

	tbclk = CONFIG_SYS_HZ;
	return tbclk;
}

/* The board must handle this interrupt if a timer is not
 * provided.
 */
#if defined(CONFIG_SYS_NIOS_TMRBASE)
void tmr_isr (void *arg)
{
	nios_timer_t *tmr = (nios_timer_t *)arg;
	/* Interrupt is cleared by writing anything to the
	 * status register.
	 */
	writel (0, &tmr->status);
	timestamp += CONFIG_SYS_NIOS_TMRMS;
#ifdef CONFIG_STATUS_LED
	status_led_tick(timestamp);
#endif
}

static void tmr_init (void)
{
	nios_timer_t *tmr =(nios_timer_t *)CONFIG_SYS_NIOS_TMRBASE;

	writel (0, &tmr->status);
	writel (0, &tmr->control);
	writel (NIOS_TIMER_STOP, &tmr->control);

#if defined(CONFIG_SYS_NIOS_TMRCNT)
	writel (CONFIG_SYS_NIOS_TMRCNT & 0xffff, &tmr->periodl);
	writel ((CONFIG_SYS_NIOS_TMRCNT >> 16) & 0xffff, &tmr->periodh);
#endif
	writel (NIOS_TIMER_ITO | NIOS_TIMER_CONT | NIOS_TIMER_START,
			&tmr->control);
	irq_install_handler (CONFIG_SYS_NIOS_TMRIRQ, tmr_isr, (void *)tmr);
}

#endif /* CONFIG_SYS_NIOS_TMRBASE */

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

#if defined(CONFIG_SYS_NIOS_TMRBASE)
	tmr_init ();
#endif

	enable_interrupts ();
	return (0);
}


/*************************************************************************/
#if defined(CONFIG_CMD_IRQ)
int do_irqinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
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
#endif
