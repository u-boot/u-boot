/*
 * (C) Copyright 2000-2004
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

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_8xx_CPUCLK_DEFAULT) || defined(CFG_MEASURE_CPUCLK) || defined(DEBUG)

#define PITC_SHIFT 16
#define PITR_SHIFT 16
/* pitc values to time for 58/8192 seconds (about 70.8 milliseconds) */
#define SPEED_PIT_COUNTS 58
#define SPEED_PITC	 ((SPEED_PIT_COUNTS - 1) << PITC_SHIFT)
#define SPEED_PITC_INIT	 ((SPEED_PIT_COUNTS + 1) << PITC_SHIFT)

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
 *    CPU clock = count * (177 * (8192 / 58))
 *
 *		= count * 24999.7241
 *
 *    which is very close to
 *
 *		= count * 25000
 *
 * Since the count gives the CPU clock divided by 25000, we can get
 * the CPU clock rounded to the nearest 0.1 MHz by
 *
 *    CPU clock = ((count + 2) / 4) * 100000;
 *
 * The rounding is important since the measurement is sometimes going
 * to be high or low by 0.025 MHz, depending on exactly how the clocks
 * and counters interact. By rounding we get the exact answer for any
 * CPU clock that is an even multiple of 0.1 MHz.
 */

unsigned long measure_gclk(void)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile cpmtimer8xx_t *timerp = &immr->im_cpmtimer;
	ulong timer2_val;
	ulong msr_val;

#ifdef CFG_8XX_XIN
	/* dont use OSCM, only use EXTCLK/512 */
	immr->im_clkrst.car_sccr |= SCCR_RTSEL | SCCR_RTDIV;
#else
	immr->im_clkrst.car_sccr &= ~(SCCR_RTSEL | SCCR_RTDIV);
#endif

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

	timerp->cpmt_tcn2 = 0;		/* reset state		*/
	timerp->cpmt_tgcr |= TGCR_RST2;	/* enable timer 2	*/

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

	timerp->cpmt_tgcr &= ~TGCR_STP2;	/* Start Timer 2	*/
	while ((immr->im_sit.sit_piscr & PISCR_PS) == 0);
	timerp->cpmt_tgcr |= TGCR_STP2;		/* Stop  Timer 2	*/

	/* re-enable external interrupts if they were on */
	set_msr (msr_val);

	/* Disable timer and PIT
	 */
	timer2_val = timerp->cpmt_tcn2;		/* save before reset timer */

	timerp->cpmt_tgcr &= ~(TGCR_RST2 | TGCR_FRZ2 | TGCR_STP2);
	immr->im_sit.sit_piscr &= ~PISCR_PTE;

#if defined(CFG_8XX_XIN)
	/* not using OSCM, using XIN, so scale appropriately */
	return (((timer2_val + 2) / 4) * (CFG_8XX_XIN/512))/8192 * 100000L;
#else
	return ((timer2_val + 2) / 4) * 100000L;	/* convert to Hz	*/
#endif
}

#endif

#if !defined(CONFIG_8xx_CPUCLK_DEFAULT)

/*
 * get_clocks() fills in gd->cpu_clock depending on CONFIG_8xx_GCLK_FREQ
 * or (if it is not defined) measure_gclk() (which uses the ref clock)
 * from above.
 */
int get_clocks (void)
{
	uint immr = get_immr (0);	/* Return full IMMR contents */
	volatile immap_t *immap = (immap_t *) (immr & 0xFFFF0000);
	uint sccr = immap->im_clkrst.car_sccr;
	/*
	 * If for some reason measuring the gclk frequency won't
	 * work, we return the hardwired value.
	 * (For example, the cogent CMA286-60 CPU module has no
	 * separate oscillator for PITRTCLK)
	 */
#if defined(CONFIG_8xx_GCLK_FREQ)
	gd->cpu_clk = CONFIG_8xx_GCLK_FREQ;
#elif defined(CONFIG_8xx_OSCLK)
#define PLPRCR_val(a) ((pll & PLPRCR_ ## a ## _MSK) >> PLPRCR_ ## a ## _SHIFT)
	uint pll = immap->im_clkrst.car_plprcr;
	uint clk;

	if ((immr & 0x0FFF) >= MPC8xx_NEW_CLK) { /* MPC866/87x/88x series */
		clk = ((CONFIG_8xx_OSCLK / (PLPRCR_val(PDF)+1)) *
		       (PLPRCR_val(MFI) + PLPRCR_val(MFN) / (PLPRCR_val(MFD)+1))) /
			(1<<PLPRCR_val(S));
	} else {
		clk = CONFIG_8xx_OSCLK * (PLPRCR_val(MF)+1);
	}
	if (pll & PLPRCR_CSRC) {	/* Low frequency division factor is used  */
		gd->cpu_clk = clk / (2 << ((sccr >> 8) & 7));
	} else {			/* High frequency division factor is used */
		gd->cpu_clk = clk / (1 << ((sccr >> 5) & 7));
	}
#else
	gd->cpu_clk = measure_gclk();
#endif /* CONFIG_8xx_GCLK_FREQ */

	if ((sccr & SCCR_EBDF11) == 0) {
		/* No Bus Divider active */
		gd->bus_clk = gd->cpu_clk;
	} else {
		/* The MPC8xx has only one BDF: half clock speed */
		gd->bus_clk = gd->cpu_clk / 2;
	}

	return (0);
}

#else /* CONFIG_8xx_CPUCLK_DEFAULT defined, use dynamic clock setting */

static long init_pll_866 (long clk);

/* This function sets up PLL (init_pll_866() is called) and
 * fills gd->cpu_clk and gd->bus_clk according to the environment
 * variable 'cpuclk' or to CONFIG_8xx_CPUCLK_DEFAULT (if 'cpuclk'
 * contains invalid value).
 * This functions requires an MPC866 or newer series CPU.
 */
int get_clocks_866 (void)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	char		  tmp[64];
	long		  cpuclk = 0;
	long		  sccr_reg;

	if (getenv_r ("cpuclk", tmp, sizeof (tmp)) > 0)
		cpuclk = simple_strtoul (tmp, NULL, 10) * 1000000;

	if ((CFG_8xx_CPUCLK_MIN > cpuclk) || (CFG_8xx_CPUCLK_MAX < cpuclk))
		cpuclk = CONFIG_8xx_CPUCLK_DEFAULT;

	gd->cpu_clk = init_pll_866 (cpuclk);
#if defined(CFG_MEASURE_CPUCLK)
	gd->cpu_clk = measure_gclk ();
#endif

	/* if cpu clock <= 66 MHz then set bus division factor to 1,
	 * otherwise set it to 2
	 */
	sccr_reg = immr->im_clkrst.car_sccr;
	sccr_reg &= ~SCCR_EBDF11;

	if (gd->cpu_clk <= 66000000) {
		sccr_reg |= SCCR_EBDF00;	/* bus division factor = 1 */
		gd->bus_clk = gd->cpu_clk;
	} else {
		sccr_reg |= SCCR_EBDF01;	/* bus division factor = 2 */
		gd->bus_clk = gd->cpu_clk / 2;
	}
	immr->im_clkrst.car_sccr = sccr_reg;

	return (0);
}

/* Adjust sdram refresh rate to actual CPU clock.
 */
int sdram_adjust_866 (void)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	long		  mamr;

	mamr = immr->im_memctl.memc_mamr;
	mamr &= ~MAMR_PTA_MSK;
	mamr |= ((gd->cpu_clk / CFG_PTA_PER_CLK) << MAMR_PTA_SHIFT);
	immr->im_memctl.memc_mamr = mamr;

	return (0);
}

/* Configure PLL for MPC866/859/885 CPU series
 * PLL multiplication factor is set to the value nearest to the desired clk,
 * assuming a oscclk of 10 MHz.
 */
static long init_pll_866 (long clk)
{
	extern void plprcr_write_866 (long);

	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	long		  n, plprcr;
	char		  mfi, mfn, mfd, s, pdf;
	long		  step_mfi, step_mfn;

	if (clk < 20000000) {
		clk *= 2;
		pdf = 1;
	} else {
		pdf = 0;
	}

	if (clk < 40000000) {
		s = 2;
		step_mfi = CONFIG_8xx_OSCLK / 4;
		mfd = 7;
		step_mfn = CONFIG_8xx_OSCLK / 30;
	} else if (clk < 80000000) {
		s = 1;
		step_mfi = CONFIG_8xx_OSCLK / 2;
		mfd = 14;
		step_mfn = CONFIG_8xx_OSCLK / 30;
	} else {
		s = 0;
		step_mfi = CONFIG_8xx_OSCLK;
		mfd = 29;
		step_mfn = CONFIG_8xx_OSCLK / 30;
	}

	/* Calculate integer part of multiplication factor
	 */
	n = clk / step_mfi;
	mfi = (char)n;

	/* Calculate numerator of fractional part of multiplication factor
	 */
	n = clk - (n * step_mfi);
	mfn = (char)(n / step_mfn);

	/* Calculate effective clk
	 */
	n = ((mfi * step_mfi) + (mfn * step_mfn)) / (pdf + 1);

	immr->im_clkrstk.cark_plprcrk = KAPWR_KEY;

	plprcr = (immr->im_clkrst.car_plprcr & ~(PLPRCR_MFN_MSK
			| PLPRCR_MFD_MSK | PLPRCR_S_MSK
			| PLPRCR_MFI_MSK | PLPRCR_DBRMO
			| PLPRCR_PDF_MSK))
			| (mfn << PLPRCR_MFN_SHIFT)
			| (mfd << PLPRCR_MFD_SHIFT)
			| (s << PLPRCR_S_SHIFT)
			| (mfi << PLPRCR_MFI_SHIFT)
			| (pdf << PLPRCR_PDF_SHIFT);

	if( (mfn > 0) && ((mfd / mfn) > 10) )
		plprcr |= PLPRCR_DBRMO;

	plprcr_write_866 (plprcr);		/* set value using SIU4/9 workaround */
	immr->im_clkrstk.cark_plprcrk = 0x00000000;

	return (n);
}

#endif /* CONFIG_8xx_CPUCLK_DEFAULT */

#if defined(CONFIG_TQM8xxL) && !defined(CONFIG_TQM866M) \
    && !defined(CONFIG_TQM885D)
/*
 * Adjust sdram refresh rate to actual CPU clock
 * and set timebase source according to actual CPU clock
 */
int adjust_sdram_tbs_8xx (void)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	long		  mamr;
	long              sccr;

	mamr = immr->im_memctl.memc_mamr;
	mamr &= ~MAMR_PTA_MSK;
	mamr |= ((gd->cpu_clk / CFG_PTA_PER_CLK) << MAMR_PTA_SHIFT);
	immr->im_memctl.memc_mamr = mamr;

	if (gd->cpu_clk < 67000000) {
		sccr = immr->im_clkrst.car_sccr;
		sccr |= SCCR_TBS;
		immr->im_clkrst.car_sccr = sccr;
	}

	return (0);
}
#endif /* CONFIG_TQM8xxL/M, !TQM866M, !TQM885D */

/* ------------------------------------------------------------------------- */
