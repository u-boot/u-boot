/*
 * (C) Copyright 2000-2002	Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * (C) Copyright 2003		Martin Winistoerfer, martinwinistoerfer@gmx.ch.
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
 * Foundation, 
 */

/*
 * File:		interrupt.c
 * 
 * Discription:		Contains interrupt routines needed by U-Boot
 *
 */

#include <common.h>
#include <watchdog.h>
#include <mpc5xx.h>
#include <asm/processor.h>

/************************************************************************/

unsigned decrementer_count;	/* count value for 1e6/HZ microseconds	*/

/************************************************************************/

struct interrupt_action {
	interrupt_handler_t *handler;
	void *arg;
};

static struct interrupt_action irq_vecs[NR_IRQS];

/*
 * Local function prototypes 
 */
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

/*
 * Enable interrupts 
 */ 
void enable_interrupts (void)
{
	set_msr (get_msr () | MSR_EE);
}

/* 
 * Returns flag if MSR_EE was set before 
 */
int disable_interrupts (void)
{
	ulong msr = get_msr ();

	set_msr (msr & ~MSR_EE);
	return ((msr & MSR_EE) != 0);
}

/*
 * Initialise interrupts 
 */

int interrupt_init (void)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;

	/* Decrementer used here for status led */
	decrementer_count = get_tbclk () / CFG_HZ;

	/* Disable all interrupts */
	immr->im_siu_conf.sc_simask = 0;

	set_dec (decrementer_count);

	set_msr (get_msr () | MSR_EE);
	return (0);
}

/*
 * Handle external interrupts
 */
void external_interrupt (struct pt_regs *regs)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	int irq;
	ulong simask, newmask;
	ulong vec, v_bit;

	/*
	 * read the SIVEC register and shift the bits down
	 * to get the irq number
	 */
	vec = immr->im_siu_conf.sc_sivec;
	irq = vec >> 26;
	v_bit = 0x80000000UL >> irq;

	/*
	 * Read Interrupt Mask Register and Mask Interrupts
	 */
	simask = immr->im_siu_conf.sc_simask;
	newmask = simask & (~(0xFFFF0000 >> irq));
	immr->im_siu_conf.sc_simask = newmask;

	if (!(irq & 0x1)) {		/* External Interrupt ?     */
		ulong siel;

		/*
		 * Read Interrupt Edge/Level Register
		 */
		siel = immr->im_siu_conf.sc_siel;

		if (siel & v_bit) {	/* edge triggered interrupt ?   */
			/*
			 * Rewrite SIPEND Register to clear interrupt
			 */
			immr->im_siu_conf.sc_sipend = v_bit;
		}
	}

	if (irq_vecs[irq].handler != NULL) {
		irq_vecs[irq].handler (irq_vecs[irq].arg);
	} else {
		printf ("\nBogus External Interrupt IRQ %d Vector %ld\n",
				irq, vec);
		/* turn off the bogus interrupt to avoid it from now */
		simask &= ~v_bit;
	}
	/*
	 * Re-Enable old Interrupt Mask
	 */
	immr->im_siu_conf.sc_simask = simask;
}

/*
 * Install and free an interrupt handler
 */
void irq_install_handler (int vec, interrupt_handler_t * handler,
						  void *arg)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	/* SIU interrupt */
	if (irq_vecs[vec].handler != NULL) {
		printf ("SIU interrupt %d 0x%x\n",
			vec,
			(uint) handler);
	}
	irq_vecs[vec].handler = handler;
	irq_vecs[vec].arg = arg;
	immr->im_siu_conf.sc_simask |= 1 << (31 - vec);
#if 0
	printf ("Install SIU interrupt for vector %d ==> %p\n",
		vec, handler);
#endif
}

void irq_free_handler (int vec)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	/* SIU interrupt */
#if 0
	printf ("Free CPM interrupt for vector %d\n",
		vec);
#endif
	immr->im_siu_conf.sc_simask &= ~(1 << (31 - vec));
	irq_vecs[vec].handler = NULL;
	irq_vecs[vec].arg = NULL;
}

volatile ulong timestamp = 0;

/*
 *  Timer interrupt - gets called when  bit 0 of DEC changes from 
 *  0. Decrementer is enabled with bit TBE in TBSCR.
 */
void timer_interrupt (struct pt_regs *regs)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;

#ifdef CONFIG_STATUS_LED
	extern void status_led_tick (ulong);
#endif
#if 0
	printf ("*** Timer Interrupt *** ");
#endif
	/* Reset Timer Status Bit and Timers Interrupt Status */
	immr->im_clkrstk.cark_plprcrk = KAPWR_KEY;
	__asm__ ("nop");
	immr->im_clkrst.car_plprcr |= PLPRCR_TEXPS | PLPRCR_TMIST;
	
	/* Restore Decrementer Count */
	set_dec (decrementer_count);

	timestamp++;

#ifdef CONFIG_STATUS_LED
	status_led_tick (timestamp);
#endif /* CONFIG_STATUS_LED */

#if defined(CONFIG_WATCHDOG)
	/*
	 * The shortest watchdog period of all boards
	 * is approx. 1 sec, thus re-trigger watchdog at least
	 * every 500 ms = CFG_HZ / 2
	 */
	if ((timestamp % (CFG_HZ / 2)) == 0) {
		reset_5xx_watchdog (immr);
	}
#endif /* CONFIG_WATCHDOG */
}

/*
 * Reset timer 
 */
void reset_timer (void)
{
	timestamp = 0;
}

/*
 * Get Timer
 */
ulong get_timer (ulong base)
{
	return (timestamp - base);
}

/*
 * Set timer
 */
void set_timer (ulong t)
{
	timestamp = t;
}
