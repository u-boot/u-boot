/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <watchdog.h>
#include <mpc8xx.h>
#include <mpc8xx_irq.h>
#include <asm/processor.h>
#include <commproc.h>

/************************************************************************/

unsigned decrementer_count;	/* count value for 1e6/HZ microseconds	*/

/************************************************************************/

/*
 * CPM interrupt vector functions.
 */
struct interrupt_action {
	interrupt_handler_t *handler;
	void *arg;
};

static struct interrupt_action cpm_vecs[CPMVEC_NR];
static struct interrupt_action irq_vecs[NR_IRQS];

static void cpm_interrupt_init (void);
static void cpm_interrupt (void *regs);

/************************************************************************/

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

/************************************************************************/

int interrupt_init (void)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;

	decrementer_count = get_tbclk () / CFG_HZ;

	/* disable all interrupts */
	immr->im_siu_conf.sc_simask = 0;

	/* Configure CPM interrupts */
	cpm_interrupt_init ();

	set_dec (decrementer_count);

	set_msr (get_msr () | MSR_EE);

	return (0);
}

/************************************************************************/

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

/************************************************************************/

/*
 * CPM interrupt handler
 */
static void cpm_interrupt (void *regs)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	uint vec;

	/*
	 * Get the vector by setting the ACK bit
	 * and then reading the register.
	 */
	immr->im_cpic.cpic_civr = 1;
	vec = immr->im_cpic.cpic_civr;
	vec >>= 11;

	if (cpm_vecs[vec].handler != NULL) {
		(*cpm_vecs[vec].handler) (cpm_vecs[vec].arg);
	} else {
		immr->im_cpic.cpic_cimr &= ~(1 << vec);
		printf ("Masking bogus CPM interrupt vector 0x%x\n", vec);
	}
	/*
	 * After servicing the interrupt,
	 * we have to remove the status indicator.
	 */
	immr->im_cpic.cpic_cisr |= (1 << vec);
}

/*
 * The CPM can generate the error interrupt when there is a race
 * condition between generating and masking interrupts. All we have
 * to do is ACK it and return. This is a no-op function so we don't
 * need any special tests in the interrupt handler.
 */
static void cpm_error_interrupt (void *dummy)
{
}

/************************************************************************/
/*
 * Install and free an interrupt handler
 */
void irq_install_handler (int vec, interrupt_handler_t * handler,
						  void *arg)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;

	if ((vec & CPMVEC_OFFSET) != 0) {
		/* CPM interrupt */
		vec &= 0xffff;
		if (cpm_vecs[vec].handler != NULL) {
			printf ("CPM interrupt 0x%x replacing 0x%x\n",
				(uint) handler,
				(uint) cpm_vecs[vec].handler);
		}
		cpm_vecs[vec].handler = handler;
		cpm_vecs[vec].arg = arg;
		immr->im_cpic.cpic_cimr |= (1 << vec);
#if 0
		printf ("Install CPM interrupt for vector %d ==> %p\n",
			vec, handler);
#endif
	} else {
		/* SIU interrupt */
		if (irq_vecs[vec].handler != NULL) {
			printf ("SIU interrupt %d 0x%x replacing 0x%x\n",
				vec,
				(uint) handler,
				(uint) cpm_vecs[vec].handler);
		}
		irq_vecs[vec].handler = handler;
		irq_vecs[vec].arg = arg;
		immr->im_siu_conf.sc_simask |= 1 << (31 - vec);
#if 0
		printf ("Install SIU interrupt for vector %d ==> %p\n",
			vec, handler);
#endif
	}
}

void irq_free_handler (int vec)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;

	if ((vec & CPMVEC_OFFSET) != 0) {
		/* CPM interrupt */
		vec &= 0xffff;
#if 0
		printf ("Free CPM interrupt for vector %d ==> %p\n",
			vec, cpm_vecs[vec].handler);
#endif
		immr->im_cpic.cpic_cimr &= ~(1 << vec);
		cpm_vecs[vec].handler = NULL;
		cpm_vecs[vec].arg = NULL;
	} else {
		/* SIU interrupt */
#if 0
		printf ("Free CPM interrupt for vector %d ==> %p\n",
			vec, cpm_vecs[vec].handler);
#endif
		immr->im_siu_conf.sc_simask &= ~(1 << (31 - vec));
		irq_vecs[vec].handler = NULL;
		irq_vecs[vec].arg = NULL;
	}
}

/************************************************************************/

static void cpm_interrupt_init (void)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;

	/*
	 * Initialize the CPM interrupt controller.
	 */

	immr->im_cpic.cpic_cicr =
		(CICR_SCD_SCC4 |
		 CICR_SCC_SCC3 |
		 CICR_SCB_SCC2 |
		 CICR_SCA_SCC1) | ((CPM_INTERRUPT / 2) << 13) | CICR_HP_MASK;

	immr->im_cpic.cpic_cimr = 0;

	/*
	 * Install the error handler.
	 */
	irq_install_handler (CPMVEC_ERROR, cpm_error_interrupt, NULL);

	immr->im_cpic.cpic_cicr |= CICR_IEN;

	/*
	 * Install the cpm interrupt handler
	 */
	irq_install_handler (CPM_INTERRUPT, cpm_interrupt, NULL);
}

/************************************************************************/

volatile ulong timestamp = 0;

/*
 * timer_interrupt - gets called when the decrementer overflows,
 * with interrupts disabled.
 * Trivial implementation - no need to be really accurate.
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
	/* Reset Timer Expired and Timers Interrupt Status */
	immr->im_clkrstk.cark_plprcrk = KAPWR_KEY;
	__asm__ ("nop");
	immr->im_clkrst.car_plprcr |= PLPRCR_TEXPS | PLPRCR_TMIST;
	/* Restore Decrementer Count */
	set_dec (decrementer_count);

	timestamp++;

#ifdef CONFIG_STATUS_LED
	status_led_tick (timestamp);
#endif /* CONFIG_STATUS_LED */

#if defined(CONFIG_WATCHDOG) || defined(CFG_CMA_LCD_HEARTBEAT)

	/*
	 * The shortest watchdog period of all boards (except LWMON)
	 * is approx. 1 sec, thus re-trigger watchdog at least
	 * every 500 ms = CFG_HZ / 2
	 */
#ifndef CONFIG_LWMON
	if ((timestamp % (CFG_HZ / 2)) == 0) {
#else
	if ((timestamp % (CFG_HZ / 20)) == 0) {
#endif

#if defined(CFG_CMA_LCD_HEARTBEAT)
		extern void lcd_heartbeat (void);

		lcd_heartbeat ();
#endif /* CFG_CMA_LCD_HEARTBEAT */

#if defined(CONFIG_WATCHDOG)
		reset_8xx_watchdog (immr);
#endif /* CONFIG_WATCHDOG */

	}
#endif /* CONFIG_WATCHDOG || CFG_CMA_LCD_HEARTBEAT */
}

/************************************************************************/

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

/************************************************************************/
