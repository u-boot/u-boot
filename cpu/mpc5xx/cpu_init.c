/*
 * (C) Copyright 2003  Martin Winistoerfer, martinwinistoerfer@gmx.ch.
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
 * File:		cpu_init.c
 *
 * Discription:		Contains initialisation functions to setup
 *    			the cpu properly
 *
 */

#include <common.h>
#include <mpc5xx.h>
#include <watchdog.h>

/*
 * Setup essential cpu registers to run
 */
void cpu_init_f (volatile immap_t * immr)
{
	volatile memctl5xx_t *memctl = &immr->im_memctl;
	ulong reg;

	/* SYPCR - contains watchdog control. This will enable watchdog */
	/* if CONFIG_WATCHDOG is set */
	immr->im_siu_conf.sc_sypcr = CFG_SYPCR;

#if defined(CONFIG_WATCHDOG)
	reset_5xx_watchdog (immr);
#endif

	/* SIUMCR - contains debug pin configuration */
	immr->im_siu_conf.sc_siumcr |= CFG_SIUMCR;

	/* Initialize timebase. Unlock TBSCRK */
	immr->im_sitk.sitk_tbscrk = KAPWR_KEY;
	immr->im_sit.sit_tbscr = CFG_TBSCR;

	/* Full IMB bus speed */
	immr->im_uimb.uimb_umcr = CFG_UMCR;

	/* Time base and decrementer will be enables (TBE) */
	/* in init_timebase() in time.c called from board_init_f(). */

	/* Initialize the PIT. Unlock PISCRK */
	immr->im_sitk.sitk_piscrk = KAPWR_KEY;
	immr->im_sit.sit_piscr = CFG_PISCR;

#if !defined(CONFIG_PATI)
	/* PATI sest PLL in start.S */
	/* PLL (CPU clock) settings */
	immr->im_clkrstk.cark_plprcrk = KAPWR_KEY;

	/* If CFG_PLPRCR (set in the various *_config.h files) tries to
	 * set the MF field, then just copy CFG_PLPRCR over car_plprcr,
	 * otherwise OR in CFG_PLPRCR so we do not change the currentMF
	 * field value.
	 */
#if ((CFG_PLPRCR & PLPRCR_MF_MSK) != 0)
	reg = CFG_PLPRCR;			/* reset control bits   */
#else
	reg = immr->im_clkrst.car_plprcr;
	reg &= PLPRCR_MF_MSK;			/* isolate MF field */
	reg |= CFG_PLPRCR;			/* reset control bits   */
#endif
	immr->im_clkrst.car_plprcr = reg;

#endif /* !defined(CONFIG_PATI) */

	/* System integration timers. CFG_MASK has EBDF configuration */
	immr->im_clkrstk.cark_sccrk = KAPWR_KEY;
	reg = immr->im_clkrst.car_sccr;
	reg &= SCCR_MASK;
	reg |= CFG_SCCR;
	immr->im_clkrst.car_sccr = reg;

	/* Memory Controller */
	memctl->memc_br0 = CFG_BR0_PRELIM;
	memctl->memc_or0 = CFG_OR0_PRELIM;

#if (defined(CFG_OR1_PRELIM) && defined(CFG_BR1_PRELIM))
	memctl->memc_or1 = CFG_OR1_PRELIM;
	memctl->memc_br1 = CFG_BR1_PRELIM;
#endif

#if defined(CFG_OR2_PRELIM) && defined(CFG_BR2_PRELIM)
	memctl->memc_or2 = CFG_OR2_PRELIM;
	memctl->memc_br2 = CFG_BR2_PRELIM;
#endif

#if defined(CFG_OR3_PRELIM) && defined(CFG_BR3_PRELIM)
	memctl->memc_or3 = CFG_OR3_PRELIM;
	memctl->memc_br3 = CFG_BR3_PRELIM;
#endif

}

/*
 * Initialize higher level parts of cpu
 */
int cpu_init_r (void)
{
  	/* Nothing to do at the moment */
	return (0);
}
