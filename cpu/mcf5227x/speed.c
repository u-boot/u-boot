/*
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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
#include <asm/processor.h>

#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Low Power Divider specifications
 */
#define CLOCK_LPD_MIN		(1 << 0)	/* Divider (decoded) */
#define CLOCK_LPD_MAX		(1 << 15)	/* Divider (decoded) */

#define CLOCK_PLL_FVCO_MAX	540000000
#define CLOCK_PLL_FVCO_MIN	300000000

#define CLOCK_PLL_FSYS_MAX	266666666
#define CLOCK_PLL_FSYS_MIN	100000000
#define MHZ			1000000

void clock_enter_limp(int lpdiv)
{
	volatile ccm_t *ccm = (volatile ccm_t *)MMAP_CCM;
	int i, j;

	/* Check bounds of divider */
	if (lpdiv < CLOCK_LPD_MIN)
		lpdiv = CLOCK_LPD_MIN;
	if (lpdiv > CLOCK_LPD_MAX)
		lpdiv = CLOCK_LPD_MAX;

	/* Round divider down to nearest power of two */
	for (i = 0, j = lpdiv; j != 1; j >>= 1, i++) ;

	/* Apply the divider to the system clock */
	ccm->cdr = (ccm->cdr & 0xF0FF) | CCM_CDR_LPDIV(i);

	/* Enable Limp Mode */
	ccm->misccr |= CCM_MISCCR_LIMP;
}

/*
 * brief   Exit Limp mode
 * warning The PLL should be set and locked prior to exiting Limp mode
 */
void clock_exit_limp(void)
{
	volatile ccm_t *ccm = (volatile ccm_t *)MMAP_CCM;
	volatile pll_t *pll = (volatile pll_t *)MMAP_PLL;

	/* Exit Limp mode */
	ccm->misccr &= ~CCM_MISCCR_LIMP;

	/* Wait for the PLL to lock */
	while (!(pll->psr & PLL_PSR_LOCK)) ;
}

/*
 * get_clocks() fills in gd->cpu_clock and gd->bus_clk
 */
int get_clocks(void)
{

	volatile ccm_t *ccm = (volatile ccm_t *)MMAP_CCM;
	volatile pll_t *pll = (volatile pll_t *)MMAP_PLL;
	int vco, temp, pcrvalue, pfdr;
	u8 bootmode;

	bootmode = (ccm->ccr & 0x000C) >> 2;

	pcrvalue = pll->pcr & 0xFF0F0FFF;
	pfdr = pcrvalue >> 24;

	if (pfdr != 0x1E) {
		/* serial mode */
	} else {
		/* Normal Mode */
		vco = pfdr * CFG_INPUT_CLKSRC;
		gd->vco_clk = vco;
	}

	if ((ccm->ccr & CCM_MISCCR_LIMP) == CCM_MISCCR_LIMP) {
		/* Limp mode */
	} else {
		gd->inp_clk = CFG_INPUT_CLKSRC;	/* Input clock */

		temp = (pll->pcr & PLL_PCR_OUTDIV1_MASK) + 1;
		gd->cpu_clk = vco / temp;	/* cpu clock */

		temp = ((pll->pcr & PLL_PCR_OUTDIV2_MASK) >> 4) + 1;
		gd->flb_clk = vco / temp;	/* flexbus clock */
		gd->bus_clk = gd->flb_clk;
	}

	return (0);
}
