/*
 *
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2008 Freescale Semiconductor, Inc.
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

/* PLL min/max specifications */
#define MAX_FVCO	500000	/* KHz */
#define MAX_FSYS	80000	/* KHz */
#define MIN_FSYS	58333	/* KHz */

#ifdef CONFIG_MCF5301x
#define FREF		20000	/* KHz */
#define MAX_MFD		63	/* Multiplier */
#define MIN_MFD		0	/* Multiplier */
#define USBDIV		8

/* Low Power Divider specifications */
#define MIN_LPD		(0)	/* Divider (not encoded) */
#define MAX_LPD		(15)	/* Divider (not encoded) */
#define DEFAULT_LPD	(0)	/* Divider (not encoded) */
#endif

#ifdef CONFIG_MCF532x
#define FREF		16000	/* KHz */
#define MAX_MFD		135	/* Multiplier */
#define MIN_MFD		88	/* Multiplier */

/* Low Power Divider specifications */
#define MIN_LPD		(1 << 0)	/* Divider (not encoded) */
#define MAX_LPD		(1 << 15)	/* Divider (not encoded) */
#define DEFAULT_LPD	(1 << 1)	/* Divider (not encoded) */
#endif

#define BUSDIV		6	/* Divider */

/* Get the value of the current system clock */
int get_sys_clock(void)
{
	volatile ccm_t *ccm = (volatile ccm_t *)(MMAP_CCM);
	volatile pll_t *pll = (volatile pll_t *)(MMAP_PLL);
	int divider;

	/* Test to see if device is in LIMP mode */
	if (ccm->misccr & CCM_MISCCR_LIMP) {
		divider = ccm->cdr & CCM_CDR_LPDIV(0xF);
#ifdef CONFIG_MCF5301x
		return (FREF / (3 * (1 << divider)));
#endif
#ifdef CONFIG_MCF532x
		return (FREF / (2 << divider));
#endif
	} else {
#ifdef CONFIG_MCF5301x
		u32 pfdr = (pll->pcr & 0x3F) + 1;
		u32 refdiv = (1 << ((pll->pcr & PLL_PCR_REFDIV(7)) >> 8));
		u32 busdiv = ((pll->pdr & 0x00F0) >> 4) + 1;

		return (((FREF * pfdr) / refdiv) / busdiv);
#endif
#ifdef CONFIG_MCF532x
		return ((FREF * pll->pfdr) / (BUSDIV * 4));
#endif
	}
}

/*
 * Initialize the Low Power Divider circuit
 *
 * Parameters:
 *  div     Desired system frequency divider
 *
 * Return Value:
 *  The resulting output system frequency
 */
int clock_limp(int div)
{
	volatile ccm_t *ccm = (volatile ccm_t *)(MMAP_CCM);
	u32 temp;

	/* Check bounds of divider */
	if (div < MIN_LPD)
		div = MIN_LPD;
	if (div > MAX_LPD)
		div = MAX_LPD;

	/* Save of the current value of the SSIDIV so we don't overwrite the value */
	temp = (ccm->cdr & CCM_CDR_SSIDIV(0xFF));

	/* Apply the divider to the system clock */
	ccm->cdr = (CCM_CDR_LPDIV(div) | CCM_CDR_SSIDIV(temp));

	ccm->misccr |= CCM_MISCCR_LIMP;

	return (FREF / (3 * (1 << div)));
}

/* Exit low power LIMP mode */
int clock_exit_limp(void)
{
	volatile ccm_t *ccm = (volatile ccm_t *)(MMAP_CCM);
	int fout;

	/* Exit LIMP mode */
	ccm->misccr &= (~CCM_MISCCR_LIMP);

	/* Wait for PLL to lock */
	while (!(ccm->misccr & CCM_MISCCR_PLL_LOCK)) ;

	fout = get_sys_clock();

	return fout;
}

/* Initialize the PLL
 *
 * Parameters:
 *  fref    PLL reference clock frequency in KHz
 *  fsys    Desired PLL output frequency in KHz
 *  flags   Operating parameters
 *
 * Return Value:
 *  The resulting output system frequency
 */
int clock_pll(int fsys, int flags)
{
#ifdef CONFIG_MCF532x
	volatile u32 *sdram_workaround = (volatile u32 *)(MMAP_SDRAM + 0x80);
#endif
	volatile sdram_t *sdram = (volatile sdram_t *)(MMAP_SDRAM);
	volatile pll_t *pll = (volatile pll_t *)(MMAP_PLL);
	int fref, temp, fout, mfd;
	u32 i;

	fref = FREF;

	if (fsys == 0) {
		/* Return current PLL output */
#ifdef CONFIG_MCF5301x
		u32 busdiv = ((pll->pdr >> 4) & 0x0F) + 1;
		mfd = (pll->pcr & 0x3F) + 1;

		return (fref * mfd) / busdiv;
#endif
#ifdef CONFIG_MCF532x
		mfd = pll->pfdr;

		return (fref * mfd / (BUSDIV * 4));
#endif
	}

	/* Check bounds of requested system clock */
	if (fsys > MAX_FSYS)
		fsys = MAX_FSYS;

	if (fsys < MIN_FSYS)
		fsys = MIN_FSYS;

	/*
	 * Multiplying by 100 when calculating the temp value,
	 * and then dividing by 100 to calculate the mfd allows
	 * for exact values without needing to include floating
	 * point libraries.
	 */
	temp = (100 * fsys) / fref;
#ifdef CONFIG_MCF5301x
	mfd = (BUSDIV * temp) / 100;

	/* Determine the output frequency for selected values */
	fout = ((fref * mfd) / BUSDIV);
#endif
#ifdef CONFIG_MCF532x
	mfd = (4 * BUSDIV * temp) / 100;

	/* Determine the output frequency for selected values */
	fout = ((fref * mfd) / (BUSDIV * 4));
#endif

/* must not tamper with SDRAMC if running from SDRAM */
#if !defined(CONFIG_MONITOR_IS_IN_RAM)
	/*
	 * Check to see if the SDRAM has already been initialized.
	 * If it has then the SDRAM needs to be put into self refresh
	 * mode before reprogramming the PLL.
	 */
	if (sdram->ctrl & SDRAMC_SDCR_REF)
		sdram->ctrl &= ~SDRAMC_SDCR_CKE;

	/*
	 * Initialize the PLL to generate the new system clock frequency.
	 * The device must be put into LIMP mode to reprogram the PLL.
	 */

	/* Enter LIMP mode */
	clock_limp(DEFAULT_LPD);

#ifdef CONFIG_MCF5301x
	pll->pdr =
	    PLL_PDR_OUTDIV1((BUSDIV / 3) - 1)	|
	    PLL_PDR_OUTDIV2(BUSDIV - 1)	|
	    PLL_PDR_OUTDIV3((BUSDIV / 2) - 1)	|
	    PLL_PDR_OUTDIV4(USBDIV - 1);

	pll->pcr &= PLL_PCR_FBDIV_UNMASK;
	pll->pcr |= PLL_PCR_FBDIV(mfd - 1);
#endif
#ifdef CONFIG_MCF532x
	/* Reprogram PLL for desired fsys */
	pll->podr = (PLL_PODR_CPUDIV(BUSDIV / 3) | PLL_PODR_BUSDIV(BUSDIV));

	pll->pfdr = mfd;
#endif

	/* Exit LIMP mode */
	clock_exit_limp();

	/* Return the SDRAM to normal operation if it is in use. */
	if (sdram->ctrl & SDRAMC_SDCR_REF)
		sdram->ctrl |= SDRAMC_SDCR_CKE;

#ifdef CONFIG_MCF532x
	/*
	 * software workaround for SDRAM opeartion after exiting LIMP
	 * mode errata
	 */
	*sdram_workaround = CONFIG_SYS_SDRAM_BASE;
#endif

	/* wait for DQS logic to relock */
	for (i = 0; i < 0x200; i++) ;
#endif /* !defined(CONFIG_MONITOR_IS_IN_RAM) */

	return fout;
}

/* get_clocks() fills in gd->cpu_clock and gd->bus_clk */
int get_clocks(void)
{
	gd->bus_clk = clock_pll(CONFIG_SYS_CLK / 1000, 0) * 1000;
	gd->cpu_clk = (gd->bus_clk * 3);

#ifdef CONFIG_FSL_I2C
	gd->i2c1_clk = gd->bus_clk;
#endif

	return (0);
}
