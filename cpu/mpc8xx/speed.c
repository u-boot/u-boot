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
#include <mpc8xx.h>
#include <asm/processor.h>

#define PITC_SHIFT 16
#define PITR_SHIFT 16
/* pitc values to time for 58/8192 seconds (about 70.8 milliseconds) */
#define SPEED_PIT_COUNTS 58
#define SPEED_PITC	 ((SPEED_PIT_COUNTS - 1) << PITC_SHIFT)
#define SPEED_PITC_INIT	 ((SPEED_PIT_COUNTS + 1) << PITC_SHIFT)

#if !defined(CONFIG_8xx_GCLK_FREQ)
/* Access functions for the Machine State Register */
static __inline__ unsigned long get_msr(void)
{
	unsigned long msr;

	asm volatile("mfmsr %0" : "=r" (msr) :);
	return msr;
}

static __inline__ void set_msr(unsigned long msr)
{
	asm volatile("mtmsr %0" : : "r" (msr));
}
#endif

/* ------------------------------------------------------------------------- */

/*
 * Measure CPU clock speed (core clock GCLK1, GCLK2),
 * also determine bus clock speed (checking bus divider factor)
 *
 * (Approx. GCLK frequency in Hz)
 *
 * Initializes timer 2 and PIT, but disables them before return.
 * [Use timer 2, because MPC823 CPUs mask 0.x do not have timers 3 and 4]
 *
 * When measuring the CPU clock against the PIT, we count cpu clocks
 * for 58/8192 seconds with a prescale divide by 177 for the cpu clock.
 * These strange values for the timing interval and prescaling are used
 * because the formula for the CPU clock is:
 *
 *   CPU clock = count * (177 * (8192 / 58))
 *
 *             = count * 24999.7241
 *
 *   which is very close to
 *
 *             = count * 25000
 *
 * Since the count gives the CPU clock divided by 25000, we can get
 * the CPU clock rounded to the nearest 0.1 MHz by
 *
 *   CPU clock = ((count + 2) / 4) * 100000;
 *
 * The rounding is important since the measurement is sometimes going
 * to be high or low by 0.025 MHz, depending on exactly how the clocks
 * and counters interact. By rounding we get the exact answer for any
 * CPU clock that is an even multiple of 0.1 MHz.
 */

int get_clocks (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	volatile immap_t *immr = (immap_t *) CFG_IMMR;
#ifndef	CONFIG_8xx_GCLK_FREQ
	volatile cpmtimer8xx_t *timerp = &immr->im_cpmtimer;
	ulong timer2_val;
	ulong msr_val;

	/* Reset + Stop Timer 2, no cascading
	 */
	timerp->cpmt_tgcr &= ~(TGCR_CAS2 | TGCR_RST2);

	/* Keep stopped, halt in debug mode
	 */
	timerp->cpmt_tgcr |= (TGCR_FRZ2 | TGCR_STP2);

	/* Timer 2 setup:
	 * Output ref. interrupt disable, int. clock
	 * Prescale by 177. Note that prescaler divides by value + 1
	 * so we must subtract 1 here.
	 */
	timerp->cpmt_tmr2 = ((177 - 1) << TMR_PS_SHIFT) | TMR_ICLK_IN_GEN;

	timerp->cpmt_tcn2 = 0;		/* reset state      */
	timerp->cpmt_tgcr |= TGCR_RST2;	/* enable timer 2   */

	/*
	 * PIT setup:
	 *
         * We want to time for SPEED_PITC_COUNTS counts (of 8192 Hz),
         * so the count value would be SPEED_PITC_COUNTS - 1.
         * But there would be an uncertainty in the start time of 1/4
         * count since when we enable the PIT the count is not
         * synchronized to the 32768 Hz oscillator. The trick here is
         * to start the count higher and wait until the PIT count
         * changes to the required value before starting timer 2.
	 *
         * One count high should be enough, but occasionally the start
         * is off by 1 or 2 counts of 32768 Hz. With the start value
         * set two counts high it seems very reliable.
         */

	immr->im_sitk.sitk_pitck = KAPWR_KEY;	/* PIT initialization */
	immr->im_sit.sit_pitc = SPEED_PITC_INIT;

	immr->im_sitk.sitk_piscrk = KAPWR_KEY;
	immr->im_sit.sit_piscr = CFG_PISCR;

	/*
	 * Start measurement - disable interrupts, just in case
	 */
	msr_val = get_msr ();
	set_msr (msr_val & ~MSR_EE);

	immr->im_sit.sit_piscr |= PISCR_PTE;

	/* spin until get exact count when we want to start */
	while (immr->im_sit.sit_pitr > SPEED_PITC);

	timerp->cpmt_tgcr &= ~TGCR_STP2;	/* Start Timer 2    */
	while ((immr->im_sit.sit_piscr & PISCR_PS) == 0);
	timerp->cpmt_tgcr |= TGCR_STP2;		/* Stop  Timer 2    */

	/* re-enable external interrupts if they were on */
	set_msr (msr_val);

	/* Disable timer and PIT
	 */
	timer2_val = timerp->cpmt_tcn2;		/* save before reset timer */

	timerp->cpmt_tgcr &= ~(TGCR_RST2 | TGCR_FRZ2 | TGCR_STP2);
	immr->im_sit.sit_piscr &= ~PISCR_PTE;

	gd->cpu_clk = ((timer2_val + 2) / 4) * 100000L;	/* convert to Hz    */

#else /* CONFIG_8xx_GCLK_FREQ */

	/*
         * If for some reason measuring the gclk frequency won't
         * work, we return the hardwired value.
         * (For example, the cogent CMA286-60 CPU module has no
         * separate oscillator for PITRTCLK)
	 */

	gd->cpu_clk = CONFIG_8xx_GCLK_FREQ;

#endif /* CONFIG_8xx_GCLK_FREQ */

	if ((immr->im_clkrst.car_sccr & SCCR_EBDF11) == 0) {
		/* No Bus Divider active */
		gd->bus_clk = gd->cpu_clk;
	} else {
		/* The MPC8xx has only one BDF: half clock speed */
		gd->bus_clk = gd->cpu_clk / 2;
	}

	return (0);
}

/* ------------------------------------------------------------------------- */
