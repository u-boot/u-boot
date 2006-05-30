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

/* enable IRQ/FIQ interrupts */
void enable_interrupts(void)
{
	unsigned long temp;
	__asm__ __volatile__("mrs %0, cpsr\n"
			     "bic %0, %0, #0x80\n"
			     "msr cpsr_c, %0"
			     : "=r" (temp)
			     :
			     : "memory");
}

/*
 * disable IRQ/FIQ interrupts
 * returns true if interrupts had been enabled before we disabled them
 */
int disable_interrupts(void)
{
	unsigned long old,temp;
	__asm__ __volatile__("mrs %0, cpsr\n"
			     "orr %1, %0, #0x80\n"
			     "msr cpsr_c, %1"
			     : "=r" (old), "=r" (temp)
			     :
			     : "memory");
	return (old & 0x80) == 0;
}

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

#else /* #ifdef CONFIG_USE_IRQ */
void enable_interrupts (void)
{
	return;
}
int disable_interrupts (void)
{
	return 0;
}
#endif /* #ifdef CONFIG_USE_IRQ */

void bad_mode (void)
{
	panic ("Resetting CPU ...\n");
	reset_cpu (0);
}

void show_regs (struct pt_regs *regs)
{
	unsigned long flags;
	const char *processor_modes[] = {
	"USER_26",	"FIQ_26",	"IRQ_26",	"SVC_26",
	"UK4_26",	"UK5_26",	"UK6_26",	"UK7_26",
	"UK8_26",	"UK9_26",	"UK10_26",	"UK11_26",
	"UK12_26",	"UK13_26",	"UK14_26",	"UK15_26",
	"USER_32",	"FIQ_32",	"IRQ_32",	"SVC_32",
	"UK4_32",	"UK5_32",	"UK6_32",	"ABT_32",
	"UK8_32",	"UK9_32",	"UK10_32",	"UND_32",
	"UK12_32",	"UK13_32",	"UK14_32",	"SYS_32"
	};

	flags = condition_codes (regs);

	printf ("pc : [<%08lx>]    lr : [<%08lx>]\n"
		"sp : %08lx  ip : %08lx  fp : %08lx\n",
		instruction_pointer (regs),
		regs->ARM_lr, regs->ARM_sp, regs->ARM_ip, regs->ARM_fp);
	printf ("r10: %08lx  r9 : %08lx  r8 : %08lx\n",
		regs->ARM_r10, regs->ARM_r9, regs->ARM_r8);
	printf ("r7 : %08lx  r6 : %08lx  r5 : %08lx  r4 : %08lx\n",
		regs->ARM_r7, regs->ARM_r6, regs->ARM_r5, regs->ARM_r4);
	printf ("r3 : %08lx  r2 : %08lx  r1 : %08lx  r0 : %08lx\n",
		regs->ARM_r3, regs->ARM_r2, regs->ARM_r1, regs->ARM_r0);
	printf ("Flags: %c%c%c%c",
		flags & CC_N_BIT ? 'N' : 'n',
		flags & CC_Z_BIT ? 'Z' : 'z',
		flags & CC_C_BIT ? 'C' : 'c', flags & CC_V_BIT ? 'V' : 'v');
	printf ("  IRQs %s  FIQs %s  Mode %s%s\n",
		interrupts_enabled (regs) ? "on" : "off",
		fast_interrupts_enabled (regs) ? "on" : "off",
		processor_modes[processor_mode (regs)],
		thumb_mode (regs) ? " (T)" : "");
}

void do_undefined_instruction (struct pt_regs *pt_regs)
{
	printf ("undefined instruction\n");
	show_regs (pt_regs);
	bad_mode ();
}

void do_software_interrupt (struct pt_regs *pt_regs)
{
	printf ("software interrupt\n");
	show_regs (pt_regs);
	bad_mode ();
}

void do_prefetch_abort (struct pt_regs *pt_regs)
{
	printf ("prefetch abort\n");
	show_regs (pt_regs);
	bad_mode ();
}

void do_data_abort (struct pt_regs *pt_regs)
{
	printf ("data abort\n");
	show_regs (pt_regs);
	bad_mode ();
}

void do_not_used (struct pt_regs *pt_regs)
{
	printf ("not used\n");
	show_regs (pt_regs);
	bad_mode ();
}

void do_fiq (struct pt_regs *pt_regs)
{
	printf ("fast interrupt request\n");
	show_regs (pt_regs);
	printf("IRQ=%08lx FIQ=%08lx\n", *IXP425_ICIH, *IXP425_ICFH);
}

void do_irq (struct pt_regs *pt_regs)
{
#ifdef CONFIG_USE_IRQ
	int irq = next_irq();

	IRQ_HANDLER[irq].m_func(IRQ_HANDLER[irq].m_data);
#else
	printf ("interrupt request\n");
	show_regs (pt_regs);
	bad_mode ();
#endif
}

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
