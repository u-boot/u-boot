/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2003, Psyent Corporation <www.psyent.com>
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


#include <nios.h>
#include <nios-io.h>
#include <asm/ptrace.h>
#include <common.h>
#include <command.h>
#include <watchdog.h>
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#endif

/****************************************************************************/

struct	irq_action {
	interrupt_handler_t *handler;
	void *arg;
	int count;
};

static struct irq_action irq_vecs[64];

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
void timer_interrupt (struct pt_regs *regs)
{
	/* Interrupt is cleared by writing anything to the
	 * status register.
	 */
	nios_timer_t *tmr = (nios_timer_t *)CFG_NIOS_TMRBASE;
	tmr->status = 0;
	timestamp += CFG_NIOS_TMRMS;
#ifdef CONFIG_STATUS_LED
	status_led_tick(timestamp);
#endif
}
#endif

/*************************************************************************/
int disable_interrupts (void)
{
	int val = 0;

	/* Writing anything to CLR_IE disables interrupts */
	val = rdctl (CTL_STATUS);
	wrctl (CTL_CLR_IE, 0);
	return (val & STATUS_IE);
}

void enable_interrupts( void )
{
	/* Writing anything SET_IE enables interrupts */
	wrctl (CTL_SET_IE, 0);
}

void external_interrupt (struct pt_regs *regs)
{
	unsigned vec;

	vec = (regs->status & STATUS_IPRI) >> 9;	/* ipri */

	irq_vecs[vec].count++;
	if (irq_vecs[vec].handler != NULL) {
		(*irq_vecs[vec].handler)(irq_vecs[vec].arg);
	} else {
		/* A sad side-effect of masking a bogus interrupt is
		 * that lower priority interrupts will also be disabled.
		 * This is probably not what we want ... so hang insted.
		 */
		printf ("Unhandled interrupt: 0x%x\n", vec);
		disable_interrupts ();
		hang ();
	}
}

/*************************************************************************/
int interrupt_init (void)
{
	int vec;

#if defined(CFG_NIOS_TMRBASE)
	nios_timer_t *tmr = (nios_timer_t *)CFG_NIOS_TMRBASE;

	tmr->control &= ~NIOS_TIMER_ITO;
	tmr->control |= NIOS_TIMER_STOP;
#if defined(CFG_NIOS_TMRCNT)
	tmr->periodl = CFG_NIOS_TMRCNT & 0xffff;
	tmr->periodh = (CFG_NIOS_TMRCNT >> 16) & 0xffff;
#endif
#endif

	for (vec=0; vec<64; vec++ ) {
		irq_vecs[vec].handler = NULL;
		irq_vecs[vec].arg = NULL;
		irq_vecs[vec].count = 0;
	}

	/* Need timus interruptus -- start the lopri timer */
#if defined(CFG_NIOS_TMRBASE)
	tmr->control |= ( NIOS_TIMER_ITO |
			  NIOS_TIMER_CONT |
			  NIOS_TIMER_START );
	ipri (CFG_NIOS_TMRIRQ + 1);
#endif
	enable_interrupts ();
	return (0);
}

void irq_install_handler (int vec, interrupt_handler_t *handler, void *arg)
{
	struct irq_action *irqa = irq_vecs;
	int   i = vec;
	int flag;

	if (irqa[i].handler != NULL) {
		printf ("Interrupt vector %d: handler 0x%x "
			"replacing 0x%x\n",
			vec, (uint)handler, (uint)irqa[i].handler);
	}

	flag = disable_interrupts ();
	irqa[i].handler = handler;
	irqa[i].arg = arg;
	if (flag )
		enable_interrupts ();
}

/*************************************************************************/
#if (CONFIG_COMMANDS & CFG_CMD_IRQ)
int do_irqinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int vec;

	printf ("\nInterrupt-Information:\n");
	printf ("Nr  Routine   Arg       Count\n");

	for (vec=0; vec<64; vec++) {
		if (irq_vecs[vec].handler != NULL) {
			printf ("%02d  %08lx  %08lx  %d\n",
				vec,
				(ulong)irq_vecs[vec].handler<<1,
				(ulong)irq_vecs[vec].arg,
				irq_vecs[vec].count);
		}
	}

	return (0);
}
#endif  /* CONFIG_COMMANDS & CFG_CMD_IRQ */
