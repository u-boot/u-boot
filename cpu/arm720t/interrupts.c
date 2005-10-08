/*
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <clps7111.h>
#include <asm/proc-armv/ptrace.h>
#include <asm/hardware.h>

#ifndef CONFIG_NETARM
/* we always count down the max. */
#define TIMER_LOAD_VAL 0xffff
/* macro to read the 16 bit timer */
#define READ_TIMER (IO_TC1D & 0xffff)
#else
#define IRQEN	(*(volatile unsigned int *)(NETARM_GEN_MODULE_BASE + NETARM_GEN_INTR_ENABLE))
#define TM2CTRL (*(volatile unsigned int *)(NETARM_GEN_MODULE_BASE + NETARM_GEN_TIMER2_CONTROL))
#define TM2STAT (*(volatile unsigned int *)(NETARM_GEN_MODULE_BASE + NETARM_GEN_TIMER2_STATUS))
#define TIMER_LOAD_VAL NETARM_GEN_TSTAT_CTC_MASK
#define READ_TIMER (TM2STAT & NETARM_GEN_TSTAT_CTC_MASK)
#endif

#ifdef CONFIG_S3C4510B
/* require interrupts for the S3C4510B */
# ifndef CONFIG_USE_IRQ
#  error CONFIG_USE_IRQ _must_ be defined when using CONFIG_S3C4510B
# else
static struct _irq_handler IRQ_HANDLER[N_IRQS];
# endif
#endif	/* CONFIG_S3C4510B */

#ifdef CONFIG_USE_IRQ
/* enable IRQ/FIQ interrupts */
void enable_interrupts (void)
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
int disable_interrupts (void)
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
#else /* CONFIG_USE_IRQ */
void enable_interrupts (void)
{
	return;
}
int disable_interrupts (void)
{
	return 0;
}
#endif

void bad_mode (void)
{
	panic ("Resetting CPU ...\n");
	reset_cpu (0);
}

void show_regs (struct pt_regs *regs)
{
	unsigned long flags;
	const char *processor_modes[] =
		{ "USER_26", "FIQ_26", "IRQ_26", "SVC_26", "UK4_26", "UK5_26",
"UK6_26", "UK7_26",
		"UK8_26", "UK9_26", "UK10_26", "UK11_26", "UK12_26", "UK13_26",
				"UK14_26", "UK15_26",
		"USER_32", "FIQ_32", "IRQ_32", "SVC_32", "UK4_32", "UK5_32",
				"UK6_32", "ABT_32",
		"UK8_32", "UK9_32", "UK10_32", "UND_32", "UK12_32", "UK13_32",
				"UK14_32", "SYS_32"
	};

	flags = condition_codes (regs);

	printf ("pc : [<%08lx>]	   lr : [<%08lx>]\n"
			"sp : %08lx  ip : %08lx	 fp : %08lx\n",
			instruction_pointer (regs),
			regs->ARM_lr, regs->ARM_sp, regs->ARM_ip, regs->ARM_fp);
	printf ("r10: %08lx  r9 : %08lx	 r8 : %08lx\n",
			regs->ARM_r10, regs->ARM_r9, regs->ARM_r8);
	printf ("r7 : %08lx  r6 : %08lx	 r5 : %08lx  r4 : %08lx\n",
			regs->ARM_r7, regs->ARM_r6, regs->ARM_r5, regs->ARM_r4);
	printf ("r3 : %08lx  r2 : %08lx	 r1 : %08lx  r0 : %08lx\n",
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
	bad_mode ();
}

void do_irq (struct pt_regs *pt_regs)
{
#if defined(CONFIG_IMPA7) || defined(CONFIG_EP7312) || defined(CONFIG_NETARM) || defined(CONFIG_ARMADILLO)
	printf ("interrupt request\n");
	show_regs (pt_regs);
	bad_mode ();
#elif defined(CONFIG_S3C4510B)
	unsigned int pending;

	while ( (pending = GET_REG( REG_INTOFFSET)) != 0x54) {  /* sentinal value for no pending interrutps */
		IRQ_HANDLER[pending>>2].m_func( IRQ_HANDLER[pending>>2].m_data);

		/* clear pending interrupt */
		PUT_REG( REG_INTPEND, (1<<(pending>>2)));
	}
#elif defined(CONFIG_INTEGRATOR) && defined(CONFIG_ARCH_INTEGRATOR)
	/* No do_irq() for IntegratorAP/CM720T as yet */
#else
#error do_irq() not defined for this CPU type
#endif
}


#ifdef CONFIG_S3C4510B
static void default_isr( void *data) {
	printf ("default_isr():  called for IRQ %d\n", (int)data);
}

static void timer_isr( void *data) {
	unsigned int *pTime = (unsigned int *)data;

	(*pTime)++;
	if ( !(*pTime % (CFG_HZ/4))) {
		/* toggle LED 0 */
		PUT_REG( REG_IOPDATA, GET_REG(REG_IOPDATA) ^ 0x1);
	}

}
#endif

#if defined(CONFIG_INTEGRATOR) && defined(CONFIG_ARCH_INTEGRATOR)
	/* Use IntegratorAP routines in board/integratorap.c */
#else

static ulong timestamp;
static ulong lastdec;

int interrupt_init (void)
{

#if defined(CONFIG_NETARM)
	/* disable all interrupts */
	IRQEN = 0;

	/* operate timer 2 in non-prescale mode */
	TM2CTRL = ( NETARM_GEN_TIMER_SET_HZ(CFG_HZ) |
		    NETARM_GEN_TCTL_ENABLE |
		    NETARM_GEN_TCTL_INIT_COUNT(TIMER_LOAD_VAL));

	/* set timer 2 counter */
	lastdec = TIMER_LOAD_VAL;
#elif defined(CONFIG_IMPA7) || defined(CONFIG_EP7312) || defined(CONFIG_ARMADILLO)
	/* disable all interrupts */
	IO_INTMR1 = 0;

	/* operate timer 1 in prescale mode */
	IO_SYSCON1 |= SYSCON1_TC1M;

	/* select 2kHz clock source for timer 1 */
	IO_SYSCON1 &= ~SYSCON1_TC1S;

	/* set timer 1 counter */
	lastdec = IO_TC1D = TIMER_LOAD_VAL;
#elif defined(CONFIG_S3C4510B)
	int i;

	/* install default interrupt handlers */
	for ( i = 0; i < N_IRQS; i++) {
		IRQ_HANDLER[i].m_data = (void *)i;
		IRQ_HANDLER[i].m_func = default_isr;
	}

	/* configure interrupts for IRQ mode */
	PUT_REG( REG_INTMODE, 0x0);
	/* clear any pending interrupts */
	PUT_REG( REG_INTPEND, 0x1FFFFF);

	lastdec = 0;

	/* install interrupt handler for timer */
	IRQ_HANDLER[INT_TIMER0].m_data = (void *)&timestamp;
	IRQ_HANDLER[INT_TIMER0].m_func = timer_isr;

	/* configure free running timer 0 */
	PUT_REG( REG_TMOD, 0x0);
	/* Stop timer 0 */
	CLR_REG( REG_TMOD, TM0_RUN);

	/* Configure for interval mode */
	CLR_REG( REG_TMOD, TM1_TOGGLE);

	/*
	 * Load Timer data register with count down value.
	 * count_down_val = CFG_SYS_CLK_FREQ/CFG_HZ
	 */
	PUT_REG( REG_TDATA0, (CFG_SYS_CLK_FREQ / CFG_HZ));

	/*
	 * Enable global interrupt
	 * Enable timer0 interrupt
	 */
	CLR_REG( REG_INTMASK, ((1<<INT_GLOBAL) | (1<<INT_TIMER0)));

	/* Start timer */
	SET_REG( REG_TMOD, TM0_RUN);

#else
#error No interrupt_init() defined for this CPU type
#endif
	timestamp = 0;

	return (0);
}

#endif /* ! IntegratorAP */

/*
 * timer without interrupts
 */


#if defined(CONFIG_IMPA7) || defined(CONFIG_EP7312) || defined(CONFIG_NETARM) || defined(CONFIG_ARMADILLO)

void reset_timer (void)
{
	reset_timer_masked ();
}

ulong get_timer (ulong base)
{
	return get_timer_masked () - base;
}

void set_timer (ulong t)
{
	timestamp = t;
}

void udelay (unsigned long usec)
{
	ulong tmo;

	tmo = usec / 1000;
	tmo *= CFG_HZ;
	tmo /= 1000;

	tmo += get_timer (0);

	while (get_timer_masked () < tmo)
		/*NOP*/;
}

void reset_timer_masked (void)
{
	/* reset time */
	lastdec = READ_TIMER;
	timestamp = 0;
}

ulong get_timer_masked (void)
{
	ulong now = READ_TIMER;

	if (lastdec >= now) {
		/* normal mode */
		timestamp += lastdec - now;
	} else {
		/* we have an overflow ... */
		timestamp += lastdec + TIMER_LOAD_VAL - now;
	}
	lastdec = now;

	return timestamp;
}

void udelay_masked (unsigned long usec)
{
	ulong tmo;
	ulong endtime;
	signed long diff;

	if (usec >= 1000) {
		tmo = usec / 1000;
		tmo *= CFG_HZ;
		tmo /= 1000;
	} else {
		tmo = usec * CFG_HZ;
		tmo /= (1000*1000);
	}

	endtime = get_timer_masked () + tmo;

	do {
		ulong now = get_timer_masked ();
		diff = endtime - now;
	} while (diff >= 0);
}

#elif defined(CONFIG_S3C4510B)

ulong get_timer (ulong base)
{
	return timestamp - base;
}

void udelay (unsigned long usec)
{
	u32 ticks;

	ticks = (usec * CFG_HZ) / 1000000;

	ticks += get_timer (0);

	while (get_timer (0) < ticks)
		/*NOP*/;

}

#elif defined(CONFIG_INTEGRATOR) && defined(CONFIG_ARCH_INTEGRATOR)
	/* No timer routines for IntegratorAP/CM720T as yet */
#else
#error Timer routines not defined for this CPU type
#endif
