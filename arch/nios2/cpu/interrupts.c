/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/nios2.h>
#include <asm/types.h>
#include <asm/io.h>
#include <asm/ptrace.h>
#include <common.h>
#include <command.h>
#include <watchdog.h>
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#endif

struct nios_timer {
	u32	status;		/* Timer status reg */
	u32	control;	/* Timer control reg */
	u32	periodl;	/* Timeout period low */
	u32	periodh;	/* Timeout period high */
	u32	snapl;		/* Snapshot low */
	u32	snaph;		/* Snapshot high */
};

/* status register */
#define NIOS_TIMER_TO		(1 << 0)	/* Timeout */
#define NIOS_TIMER_RUN		(1 << 1)	/* Timer running */

/* control register */
#define NIOS_TIMER_ITO		(1 << 0)	/* Timeout int ena */
#define NIOS_TIMER_CONT		(1 << 1)	/* Continuous mode */
#define NIOS_TIMER_START	(1 << 2)	/* Start timer */
#define NIOS_TIMER_STOP		(1 << 3)	/* Stop timer */

#if defined(CONFIG_SYS_TIMER_BASE) && !defined(CONFIG_SYS_TIMER_IRQ)
#error CONFIG_SYS_TIMER_IRQ not defined (see documentation)
#endif

/****************************************************************************/

struct	irq_action {
	interrupt_handler_t *handler;
	void *arg;
	int count;
};

static struct irq_action vecs[32];

/*************************************************************************/
static volatile ulong timestamp;

/*
 * The board must handle this interrupt if a timer is not
 * provided.
 */
void tmr_isr (void *arg)
{
	struct nios_timer *tmr = (struct nios_timer *)arg;
	/* Interrupt is cleared by writing anything to the
	 * status register.
	 */
	writel (0, &tmr->status);
	timestamp += CONFIG_SYS_NIOS_TMRMS;
#ifdef CONFIG_STATUS_LED
	status_led_tick(timestamp);
#endif
}

unsigned long notrace timer_read_counter(void)
{
	struct nios_timer *tmr = (struct nios_timer *)CONFIG_SYS_TIMER_BASE;
	u32 val;

	/* Trigger update */
	writel(0x0, &tmr->snapl);

	/* Read timer value */
	val = readl(&tmr->snapl) & 0xffff;
	val |= (readl(&tmr->snaph) & 0xffff) << 16;

	return ~val;
}

int timer_init(void)
{
	struct nios_timer *tmr = (struct nios_timer *)CONFIG_SYS_TIMER_BASE;

	writel (0, &tmr->status);
	writel (0, &tmr->control);
	writel (NIOS_TIMER_STOP, &tmr->control);

	writel (0xffff, &tmr->periodl);
	writel (0xffff, &tmr->periodh);

	writel (NIOS_TIMER_CONT | NIOS_TIMER_START, &tmr->control);
	/* FIXME */
	irq_install_handler(CONFIG_SYS_TIMER_IRQ, tmr_isr, (void *)tmr);

	return 0;
}

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
