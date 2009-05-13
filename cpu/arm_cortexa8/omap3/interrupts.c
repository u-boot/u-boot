/*
 * (C) Copyright 2008
 * Texas Instruments
 *
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Moahmmed Khasim <khasim@ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
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
#include <asm/io.h>
#include <asm/proc-armv/ptrace.h>

#ifdef CONFIG_USE_IRQ
/* enable IRQ interrupts */
void enable_interrupts(void)
{
	unsigned long temp;
	__asm__ __volatile__("mrs %0, cpsr\n"
			     "bic %0, %0, #0x80\n" "msr cpsr_c, %0":"=r"(temp)
			     ::"memory");
}

/*
 * disable IRQ/FIQ interrupts
 * returns true if interrupts had been enabled before we disabled them
 */
int disable_interrupts(void)
{
	unsigned long old, temp;
	__asm__ __volatile__("mrs %0, cpsr\n"
			     "orr %1, %0, #0xc0\n"
			     "msr cpsr_c, %1":"=r"(old), "=r"(temp)
			     ::"memory");
	return (old & 0x80) == 0;
}
#else
void enable_interrupts(void)
{
	return;
}
int disable_interrupts(void)
{
	return 0;
}
#endif

void bad_mode(void)
{
	panic("Resetting CPU ...\n");
	reset_cpu(0);
}

void show_regs(struct pt_regs *regs)
{
	unsigned long flags;
	const char *processor_modes[] = {
		"USER_26", "FIQ_26", "IRQ_26", "SVC_26",
		"UK4_26", "UK5_26", "UK6_26", "UK7_26",
		"UK8_26", "UK9_26", "UK10_26", "UK11_26",
		"UK12_26", "UK13_26", "UK14_26", "UK15_26",
		"USER_32", "FIQ_32", "IRQ_32", "SVC_32",
		"UK4_32", "UK5_32", "UK6_32", "ABT_32",
		"UK8_32", "UK9_32", "UK10_32", "UND_32",
		"UK12_32", "UK13_32", "UK14_32", "SYS_32",
	};

	flags = condition_codes(regs);

	printf("pc : [<%08lx>]    lr : [<%08lx>]\n"
		"sp : %08lx  ip : %08lx  fp : %08lx\n",
		instruction_pointer(regs),
		regs->ARM_lr, regs->ARM_sp, regs->ARM_ip, regs->ARM_fp);
	printf("r10: %08lx  r9 : %08lx  r8 : %08lx\n",
		regs->ARM_r10, regs->ARM_r9, regs->ARM_r8);
	printf("r7 : %08lx  r6 : %08lx  r5 : %08lx  r4 : %08lx\n",
		regs->ARM_r7, regs->ARM_r6, regs->ARM_r5, regs->ARM_r4);
	printf("r3 : %08lx  r2 : %08lx  r1 : %08lx  r0 : %08lx\n",
		regs->ARM_r3, regs->ARM_r2, regs->ARM_r1, regs->ARM_r0);
	printf("Flags: %c%c%c%c",
		flags & CC_N_BIT ? 'N' : 'n',
		flags & CC_Z_BIT ? 'Z' : 'z',
		flags & CC_C_BIT ? 'C' : 'c', flags & CC_V_BIT ? 'V' : 'v');
	printf("  IRQs %s  FIQs %s  Mode %s%s\n",
		interrupts_enabled(regs) ? "on" : "off",
		fast_interrupts_enabled(regs) ? "on" : "off",
		processor_modes[processor_mode(regs)],
		thumb_mode(regs) ? " (T)" : "");
}

void do_undefined_instruction(struct pt_regs *pt_regs)
{
	printf("undefined instruction\n");
	show_regs(pt_regs);
	bad_mode();
}

void do_software_interrupt(struct pt_regs *pt_regs)
{
	printf("software interrupt\n");
	show_regs(pt_regs);
	bad_mode();
}

void do_prefetch_abort(struct pt_regs *pt_regs)
{
	printf("prefetch abort\n");
	show_regs(pt_regs);
	bad_mode();
}

void do_data_abort(struct pt_regs *pt_regs)
{
	printf("data abort\n");
	show_regs(pt_regs);
	bad_mode();
}

void do_not_used(struct pt_regs *pt_regs)
{
	printf("not used\n");
	show_regs(pt_regs);
	bad_mode();
}

void do_fiq(struct pt_regs *pt_regs)
{
	printf("fast interrupt request\n");
	show_regs(pt_regs);
	bad_mode();
}

void do_irq(struct pt_regs *pt_regs)
{
	printf("interrupt request\n");
	show_regs(pt_regs);
	bad_mode();
}


static ulong timestamp;
static ulong lastinc;
static gptimer_t *timer_base = (gptimer_t *)CONFIG_SYS_TIMERBASE;

/*
 * Nothing really to do with interrupts, just starts up a counter.
 * We run the counter with 13MHz, divided by 8, resulting in timer
 * frequency of 1.625MHz. With 32bit counter register, counter
 * overflows in ~44min
 */

/* 13MHz / 8 = 1.625MHz */
#define TIMER_CLOCK	(V_SCLK / (2 << CONFIG_SYS_PTV))
#define TIMER_LOAD_VAL	0xffffffff

int interrupt_init(void)
{
	/* start the counter ticking up, reload value on overflow */
	writel(TIMER_LOAD_VAL, &timer_base->tldr);
	/* enable timer */
	writel((CONFIG_SYS_PTV << 2) | TCLR_PRE | TCLR_AR | TCLR_ST,
		&timer_base->tclr);

	reset_timer_masked();	/* init the timestamp and lastinc value */

	return 0;
}

/*
 * timer without interrupts
 */
void reset_timer(void)
{
	reset_timer_masked();
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

void set_timer(ulong t)
{
	timestamp = t;
}

/* delay x useconds */
void udelay(unsigned long usec)
{
	long tmo = usec * (TIMER_CLOCK / 1000) / 1000;
	unsigned long now, last = readl(&timer_base->tcrr);

	while (tmo > 0) {
		now = readl(&timer_base->tcrr);
		if (last > now) /* count up timer overflow */
			tmo -= TIMER_LOAD_VAL - last + now;
		else
			tmo -= now - last;
		last = now;
	}
}

void reset_timer_masked(void)
{
	/* reset time, capture current incrementer value time */
	lastinc = readl(&timer_base->tcrr) / (TIMER_CLOCK / CONFIG_SYS_HZ);
	timestamp = 0;		/* start "advancing" time stamp from 0 */
}

ulong get_timer_masked(void)
{
	/* current tick value */
	ulong now = readl(&timer_base->tcrr) / (TIMER_CLOCK / CONFIG_SYS_HZ);

	if (now >= lastinc)	/* normal mode (non roll) */
		/* move stamp fordward with absoulte diff ticks */
		timestamp += (now - lastinc);
	else	/* we have rollover of incrementer */
		timestamp += ((TIMER_LOAD_VAL / (TIMER_CLOCK / CONFIG_SYS_HZ))
				- lastinc) + now;
	lastinc = now;
	return timestamp;
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
