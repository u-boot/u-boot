/*
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
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

/*
 * interrupts.c - just enough support for the decrementer/timer
 */

#include <common.h>
#include <mpc8xx.h>
#include <mpc8xx_irq.h>
#include <asm/processor.h>
#include <commproc.h>
#include <command.h>

/****************************************************************************/

unsigned decrementer_count;	     /* count value for 1e6/HZ microseconds */

/****************************************************************************/

static __inline__ unsigned long
get_msr(void)
{
	unsigned long msr;

	asm volatile("mfmsr %0" : "=r" (msr) :);
	return msr;
}

static __inline__ void
set_msr(unsigned long msr)
{
	asm volatile("mtmsr %0" : : "r" (msr));
}

static __inline__ unsigned long
get_dec(void)
{
	unsigned long val;

	asm volatile("mfdec %0" : "=r" (val) :);
	return val;
}


static __inline__ void
set_dec(unsigned long val)
{
	asm volatile("mtdec %0" : : "r" (val));
}


void
enable_interrupts(void)
{
	set_msr (get_msr() | MSR_EE);
}

/* returns flag if MSR_EE was set before */
int
disable_interrupts(void)
{
	ulong msr = get_msr();
	set_msr (msr & ~MSR_EE);
	return ((msr & MSR_EE) != 0);
}

/****************************************************************************/

int interrupt_init(void)
{
#if defined(DEBUG) && !defined(CONFIG_AMIGAONEG3SE)
	printf("interrupt_init: GT main cause reg: %08x:%08x\n",
	       GTREGREAD(LOW_INTERRUPT_CAUSE_REGISTER),
	       GTREGREAD(HIGH_INTERRUPT_CAUSE_REGISTER));
	printf("interrupt_init: ethernet cause regs: %08x %08x %08x\n",
	       GTREGREAD(ETHERNET0_INTERRUPT_CAUSE_REGISTER),
	       GTREGREAD(ETHERNET1_INTERRUPT_CAUSE_REGISTER),
	       GTREGREAD(ETHERNET2_INTERRUPT_CAUSE_REGISTER));
	printf("interrupt_init: ethernet mask regs:  %08x %08x %08x\n",
	       GTREGREAD(ETHERNET0_INTERRUPT_MASK_REGISTER),
	       GTREGREAD(ETHERNET1_INTERRUPT_MASK_REGISTER),
	       GTREGREAD(ETHERNET2_INTERRUPT_MASK_REGISTER));
	puts("interrupt_init: setting decrementer_count\n");
#endif
	decrementer_count = get_tbclk() / CFG_HZ;

#ifdef DEBUG
	puts("interrupt_init: setting actual decremter\n");
#endif
	set_dec (get_tbclk() / CFG_HZ);

#ifdef DEBUG
	printf("interrupt_init: enabling interrupts (msr = %08lx)\n",
		get_msr());
#endif
	set_msr (get_msr() | MSR_EE);

#ifdef DEBUG
	printf("interrupt_init: done. (msr = %08lx)\n", get_msr());
#endif
	return (0);
}

/****************************************************************************/

/*
 * Handle external interrupts
 */
void
external_interrupt(struct pt_regs *regs)
{
	puts("external_interrupt (oops!)\n");
}

volatile ulong timestamp = 0;

/*
 * timer_interrupt - gets called when the decrementer overflows,
 * with interrupts disabled.
 * Trivial implementation - no need to be really accurate.
 */
void
timer_interrupt(struct pt_regs *regs)
{
	set_dec(decrementer_count);
	timestamp++;

#if defined(CONFIG_WATCHDOG)
	if ((timestamp % (CFG_HZ / 2)) == 0) {
#if defined(CONFIG_PCIPPC2)
		extern void pcippc2_wdt_reset (void);

		pcippc2_wdt_reset();
#endif
	}
#endif /* CONFIG_WATCHDOG */
}

/****************************************************************************/

void
reset_timer(void)
{
	timestamp = 0;
}

ulong
get_timer(ulong base)
{
	return (timestamp - base);
}

void
set_timer(ulong t)
{
	timestamp = t;
}

/****************************************************************************/

/*
 * Install and free a interrupt handler.
 */

void
irq_install_handler(int vec, interrupt_handler_t *handler, void *arg)
{

}

void
irq_free_handler(int vec)
{

}

/****************************************************************************/

void
do_irqinfo(cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	puts("IRQ related functions are unimplemented currently.\n");
}
