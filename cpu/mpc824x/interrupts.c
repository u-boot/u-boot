/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com
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
#include <mpc824x.h>
#include <asm/processor.h>
#include <asm/pci_io.h>
#include <commproc.h>
#include "drivers/epic.h"

/****************************************************************************/

unsigned decrementer_count;		/* count val for 1e6/HZ microseconds */

static __inline__ unsigned long get_msr (void)
{
	unsigned long msr;

	asm volatile ("mfmsr %0":"=r" (msr):);

	return msr;
}

static __inline__ void set_msr (unsigned long msr)
{
	asm volatile ("mtmsr %0"::"r" (msr));
}

static __inline__ unsigned long get_dec (void)
{
	unsigned long val;

	asm volatile ("mfdec %0":"=r" (val):);

	return val;
}


static __inline__ void set_dec (unsigned long val)
{
	asm volatile ("mtdec %0"::"r" (val));
}


void enable_interrupts (void)
{
	set_msr (get_msr () | MSR_EE);
}

/* returns flag if MSR_EE was set before */
int disable_interrupts (void)
{
	ulong msr = get_msr ();

	set_msr (msr & ~MSR_EE);
	return ((msr & MSR_EE) != 0);
}

/****************************************************************************/

int interrupt_init (void)
{
	decrementer_count = (get_bus_freq (0) / 4) / CFG_HZ;

        /*
         * It's all broken at the moment and I currently don't need
         * interrupts. If you want to fix it, have a look at the epic
         * drivers in dink32 v12. They do everthing and Motorola said
         * I could use the dink source in this project as long as
         * copyright notices remain intact.
	 */

	epicInit (EPIC_DIRECT_IRQ, 0);
	/* EPIC won't generate INT unless Current Task Pri < 15 */
	epicCurTaskPrioSet(0);

	set_dec (decrementer_count);

	set_msr (get_msr () | MSR_EE);

	return (0);
}

/****************************************************************************/

/*
 * Handle external interrupts
 */
void external_interrupt (struct pt_regs *regs)
{
	register unsigned long temp;

	pci_readl (CFG_EUMB_ADDR + EPIC_PROC_INT_ACK_REG, temp);
	sync ();					/* i'm not convinced this is needed, but dink source has it */
	temp &= 0xff;				/*get vector */

	/*TODO: handle them -... */
	epicEOI ();
}

/****************************************************************************/

/*
 * blank int handlers.
 */

void
irq_install_handler (int vec, interrupt_handler_t * handler, void *arg)
{
}

void irq_free_handler (int vec)
{

}

/*TODO: some handlers for winbond and 87308 interrupts
 and what about generic pci inteerupts?
 vga?
 */

volatile ulong timestamp = 0;

void timer_interrupt (struct pt_regs *regs)
{
	/* Restore Decrementer Count */
	set_dec (decrementer_count);

	timestamp++;

#if defined(CONFIG_WATCHDOG)
	if ((timestamp % (CFG_HZ / 2)) == 0) {
#if defined(CONFIG_OXC)
		{
			extern void oxc_wdt_reset (void);

			oxc_wdt_reset ();
		}
#endif
	}
#endif							/* CONFIG_WATCHDOG */
#if defined(CONFIG_SHOW_ACTIVITY) && defined(CONFIG_OXC)
	if ((timestamp % (CFG_HZ / 10)) == 0) {
		{
			extern void oxc_toggle_activeled (void);

			oxc_toggle_activeled ();
		}
	}
#endif
}

void reset_timer (void)
{
	timestamp = 0;
}

ulong get_timer (ulong base)
{
	return (timestamp - base);
}

void set_timer (ulong t)
{
	timestamp = t;
}
