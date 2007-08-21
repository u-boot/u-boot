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
	volatile u8 *cpld = (volatile u8 *)(CFG_CS2_BASE + 3);
	volatile u8 *fpga = (volatile u8 *)(CFG_CS3_BASE + 14);
	int pllmult_nopci[] = { 20, 10, 24, 18, 12, 6, 16, 8 };
	int pllmult_pci[] = { 12, 6, 16, 8 };
	int vco, bPci, temp, fbtemp, pcrvalue;
	int *pPllmult = NULL;
	u16 fbpll_mask;
	u8 cpldmode;

	/* To determine PCI is present or not */
	if (((ccm->ccr & CCM_CCR_360_FBCONFIG_MASK) == 0x00e0) ||
	    ((ccm->ccr & CCM_CCR_360_FBCONFIG_MASK) == 0x0060)) {
		pPllmult = &pllmult_pci[0];
		fbpll_mask = 3;
		bPci = 1;
	} else {
		pPllmult = &pllmult_nopci[0];
		fbpll_mask = 7;
#ifdef CONFIG_PCI
		gd->pci_clk = 0;
#endif
		bPci = 0;
	}

#ifdef CONFIG_M54455EVB
	/* Temporary place here, belongs in board/freescale/... */
	/* Temporary read from CCR- fixed fb issue, must be the same clock
	   as pci or input clock, causing cpld/fpga read inconsistancy */
	fbtemp = pPllmult[ccm->ccr & fbpll_mask];

	/* Break down into small pieces, code still in flex bus */
	pcrvalue = pll->pcr & 0xFFFFF0FF;
	temp = fbtemp - 1;
	pcrvalue |= PLL_PCR_OUTDIV3(temp);

	pll->pcr = pcrvalue;

	cpldmode = *cpld & 0x03;
	if (cpldmode == 0) {
		/* RCON mode */
		vco = pPllmult[ccm->rcon & fbpll_mask] * CFG_INPUT_CLKSRC;

		if ((vco < CLOCK_PLL_FVCO_MIN) || (vco > CLOCK_PLL_FVCO_MAX)) {
			/* invaild range, re-set in PCR */
			int temp = ((pll->pcr & PLL_PCR_OUTDIV2_MASK) >> 4) + 1;
			int i, j, bus;

			j = (pll->pcr & 0xFF000000) >> 24;
			for (i = j; i < 0xFF; i++) {
				vco = i * CFG_INPUT_CLKSRC;
				if (vco >= CLOCK_PLL_FVCO_MIN) {
					bus = vco / temp;
					if (bus <= CLOCK_PLL_FSYS_MIN - MHZ)
						continue;
					else
						break;
				}
			}
			pcrvalue = pll->pcr & 0x00FF00FF;
			fbtemp = ((i - 1) << 8) | ((i - 1) << 12);
			pcrvalue |= ((i << 24) | fbtemp);

			pll->pcr = pcrvalue;
		}
		gd->vco_clk = vco;	/* Vco clock */
	} else if (cpldmode == 2) {
		/* Normal mode */
		vco = pPllmult[ccm->ccr & fbpll_mask] * CFG_INPUT_CLKSRC;
		gd->vco_clk = vco;	/* Vco clock */
	} else if (cpldmode == 3) {
		/* serial mode */
	}
#endif				/* CONFIG_M54455EVB */

	if ((ccm->ccr & CCM_MISCCR_LIMP) == CCM_MISCCR_LIMP) {
		/* Limp mode */
	} else {
		gd->inp_clk = CFG_INPUT_CLKSRC;	/* Input clock */

		temp = (pll->pcr & PLL_PCR_OUTDIV1_MASK) + 1;
		gd->cpu_clk = vco / temp;	/* cpu clock */

		temp = ((pll->pcr & PLL_PCR_OUTDIV2_MASK) >> 4) + 1;
		gd->bus_clk = vco / temp;	/* bus clock */

		temp = ((pll->pcr & PLL_PCR_OUTDIV3_MASK) >> 8) + 1;
		gd->flb_clk = vco / temp;	/* FlexBus clock */

#ifdef CONFIG_PCI
		if (bPci) {
			temp = ((pll->pcr & PLL_PCR_OUTDIV4_MASK) >> 12) + 1;
			gd->pci_clk = vco / temp;	/* PCI clock */
		}
#endif
	}

	return (0);
}
