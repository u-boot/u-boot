/*
 *
 * Copyright (C) 2004-2007, 2012 Freescale Semiconductor, Inc.
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
#include <asm/io.h>

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
	ccm_t *ccm = (ccm_t *)MMAP_CCM;
	int i, j;

	/* Check bounds of divider */
	if (lpdiv < CLOCK_LPD_MIN)
		lpdiv = CLOCK_LPD_MIN;
	if (lpdiv > CLOCK_LPD_MAX)
		lpdiv = CLOCK_LPD_MAX;

	/* Round divider down to nearest power of two */
	for (i = 0, j = lpdiv; j != 1; j >>= 1, i++) ;

	/* Apply the divider to the system clock */
	clrsetbits_be16(&ccm->cdr, 0x0f00, CCM_CDR_LPDIV(i));

	/* Enable Limp Mode */
	setbits_be16(&ccm->misccr, CCM_MISCCR_LIMP);
}

/*
 * brief   Exit Limp mode
 * warning The PLL should be set and locked prior to exiting Limp mode
 */
void clock_exit_limp(void)
{
	ccm_t *ccm = (ccm_t *)MMAP_CCM;
	pll_t *pll = (pll_t *)MMAP_PLL;

	/* Exit Limp mode */
	clrbits_be16(&ccm->misccr, CCM_MISCCR_LIMP);

	/* Wait for the PLL to lock */
	while (!(in_be32(&pll->psr) & PLL_PSR_LOCK))
		;
}

/*
 * get_clocks() fills in gd->cpu_clock and gd->bus_clk
 */
int get_clocks(void)
{

	ccm_t *ccm = (ccm_t *)MMAP_CCM;
	pll_t *pll = (pll_t *)MMAP_PLL;
	int pllmult_nopci[] = { 20, 10, 24, 18, 12, 6, 16, 8 };
	int pllmult_pci[] = { 12, 6, 16, 8 };
	int vco = 0, bPci, temp, fbtemp, pcrvalue;
	int *pPllmult = NULL;
	u16 fbpll_mask;

#ifdef CONFIG_M54455EVB
	u8 *cpld = (u8 *)(CONFIG_SYS_CS2_BASE + 3);
#endif
	u8 bootmode;

	/* To determine PCI is present or not */
	if (((in_be16(&ccm->ccr) & CCM_CCR_360_FBCONFIG_MASK) == 0x00e0) ||
	    ((in_be16(&ccm->ccr) & CCM_CCR_360_FBCONFIG_MASK) == 0x0060)) {
		pPllmult = &pllmult_pci[0];
		fbpll_mask = 3;		/* 11b */
		bPci = 1;
	} else {
		pPllmult = &pllmult_nopci[0];
		fbpll_mask = 7;		/* 111b */
#ifdef CONFIG_PCI
		gd->pci_clk = 0;
#endif
		bPci = 0;
	}

#ifdef CONFIG_M54455EVB
	bootmode = (in_8(cpld) & 0x03);

	if (bootmode != 3) {
		/* Temporary read from CCR- fixed fb issue, must be the same clock
		   as pci or input clock, causing cpld/fpga read inconsistancy */
		fbtemp = pPllmult[ccm->ccr & fbpll_mask];

		/* Break down into small pieces, code still in flex bus */
		pcrvalue = in_be32(&pll->pcr) & 0xFFFFF0FF;
		temp = fbtemp - 1;
		pcrvalue |= PLL_PCR_OUTDIV3(temp);

		out_be32(&pll->pcr, pcrvalue);
	}
#endif
#ifdef CONFIG_M54451EVB
	/* No external logic to read the bootmode, hard coded from built */
#ifdef CONFIG_CF_SBF
	bootmode = 3;
#else
	bootmode = 2;

	/* default value is 16 mul, set to 20 mul */
	pcrvalue = (in_be32(&pll->pcr) & 0x00FFFFFF) | 0x14000000;
	out_be32(&pll->pcr, pcrvalue);
	while ((in_be32(&pll->psr) & PLL_PSR_LOCK) != PLL_PSR_LOCK)
		;
#endif
#endif

	if (bootmode == 0) {
		/* RCON mode */
		vco = pPllmult[ccm->rcon & fbpll_mask] * CONFIG_SYS_INPUT_CLKSRC;

		if ((vco < CLOCK_PLL_FVCO_MIN) || (vco > CLOCK_PLL_FVCO_MAX)) {
			/* invaild range, re-set in PCR */
			int temp = ((in_be32(&pll->pcr) & PLL_PCR_OUTDIV2_MASK) >> 4) + 1;
			int i, j, bus;

			j = (in_be32(&pll->pcr) & 0xFF000000) >> 24;
			for (i = j; i < 0xFF; i++) {
				vco = i * CONFIG_SYS_INPUT_CLKSRC;
				if (vco >= CLOCK_PLL_FVCO_MIN) {
					bus = vco / temp;
					if (bus <= CLOCK_PLL_FSYS_MIN - MHZ)
						continue;
					else
						break;
				}
			}
			pcrvalue = in_be32(&pll->pcr) & 0x00FF00FF;
			fbtemp = ((i - 1) << 8) | ((i - 1) << 12);
			pcrvalue |= ((i << 24) | fbtemp);

			out_be32(&pll->pcr, pcrvalue);
		}
		gd->vco_clk = vco;	/* Vco clock */
	} else if (bootmode == 2) {
		/* Normal mode */
		vco =  ((in_be32(&pll->pcr) & 0xFF000000) >> 24) * CONFIG_SYS_INPUT_CLKSRC;
		if ((vco < CLOCK_PLL_FVCO_MIN) || (vco > CLOCK_PLL_FVCO_MAX)) {
			/* Default value */
			pcrvalue = (in_be32(&pll->pcr) & 0x00FFFFFF);
			pcrvalue |= pPllmult[in_be16(&ccm->ccr) & fbpll_mask] << 24;
			out_be32(&pll->pcr, pcrvalue);
			vco = ((in_be32(&pll->pcr) & 0xFF000000) >> 24) * CONFIG_SYS_INPUT_CLKSRC;
		}
		gd->vco_clk = vco;	/* Vco clock */
	} else if (bootmode == 3) {
		/* serial mode */
		vco =  ((in_be32(&pll->pcr) & 0xFF000000) >> 24) * CONFIG_SYS_INPUT_CLKSRC;
		gd->vco_clk = vco;	/* Vco clock */
	}

	if ((in_be16(&ccm->ccr) & CCM_MISCCR_LIMP) == CCM_MISCCR_LIMP) {
		/* Limp mode */
	} else {
		gd->inp_clk = CONFIG_SYS_INPUT_CLKSRC;	/* Input clock */

		temp = (in_be32(&pll->pcr) & PLL_PCR_OUTDIV1_MASK) + 1;
		gd->cpu_clk = vco / temp;	/* cpu clock */

		temp = ((in_be32(&pll->pcr) & PLL_PCR_OUTDIV2_MASK) >> 4) + 1;
		gd->bus_clk = vco / temp;	/* bus clock */

		temp = ((in_be32(&pll->pcr) & PLL_PCR_OUTDIV3_MASK) >> 8) + 1;
		gd->flb_clk = vco / temp;	/* FlexBus clock */

#ifdef CONFIG_PCI
		if (bPci) {
			temp = ((in_be32(&pll->pcr) & PLL_PCR_OUTDIV4_MASK) >> 12) + 1;
			gd->pci_clk = vco / temp;	/* PCI clock */
		}
#endif
	}

#ifdef CONFIG_FSL_I2C
	gd->i2c1_clk = gd->bus_clk;
#endif

	return (0);
}
