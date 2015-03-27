/*
 * (C) Copyright 2014 DENX Software Engineering
 *     Heiko Schocher <hs@denx.de>
 *
 * Based on:
 * Copyright (C) 2013 Atmel Corporation
 *		      Bo Shen <voice.shen@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91sam9_matrix.h>
#include <asm/arch/at91_pit.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/at91_wdt.h>
#include <asm/arch/clk.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

static void enable_ext_reset(void)
{
	struct at91_rstc *rstc = (struct at91_rstc *)ATMEL_BASE_RSTC;

	writel(AT91_RSTC_KEY | AT91_RSTC_MR_URSTEN, &rstc->mr);
}

void lowlevel_clock_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	if (!(readl(&pmc->sr) & AT91_PMC_MOSCS)) {
		/* Enable Main Oscillator */
		writel(AT91_PMC_MOSCS | (0x40 << 8), &pmc->mor);

		/* Wait until Main Oscillator is stable */
		while (!(readl(&pmc->sr) & AT91_PMC_MOSCS))
			;
	}

	/* After stabilization, switch to Main Oscillator */
	if ((readl(&pmc->mckr) & AT91_PMC_CSS) == AT91_PMC_CSS_SLOW) {
		unsigned long tmp;

		tmp = readl(&pmc->mckr);
		tmp &= ~AT91_PMC_CSS;
		tmp |= AT91_PMC_CSS_MAIN;
		writel(tmp, &pmc->mckr);
		while (!(readl(&pmc->sr) & AT91_PMC_MCKRDY))
			;

		tmp &= ~AT91_PMC_PRES;
		tmp |= AT91_PMC_PRES_1;
		writel(tmp, &pmc->mckr);
		while (!(readl(&pmc->sr) & AT91_PMC_MCKRDY))
			;
	}

	return;
}

void __weak matrix_init(void)
{
}

void __weak at91_spl_board_init(void)
{
}

void __weak spl_board_init(void)
{
}

void board_init_f(ulong dummy)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	lowlevel_clock_init();
	at91_disable_wdt();

	/*
	 * At this stage the main oscillator is supposed to be enabled
	 * PCK = MCK = MOSC
	 */
	writel(0x00, &pmc->pllicpr);

	/* Configure PLLA = MOSC * (PLL_MULA + 1) / PLL_DIVA */
	at91_plla_init(CONFIG_SYS_AT91_PLLA);

	/* PCK = PLLA = 2 * MCK */
	at91_mck_init(CONFIG_SYS_MCKR);

	/* Switch MCK on PLLA output */
	at91_mck_init(CONFIG_SYS_MCKR_CSS);

#if defined(CONFIG_SYS_AT91_PLLB)
	/* Configure PLLB */
	at91_pllb_init(CONFIG_SYS_AT91_PLLB);
#endif

	/* Enable External Reset */
	enable_ext_reset();

	/* Initialize matrix */
	matrix_init();

	gd->arch.mck_rate_hz = CONFIG_SYS_MASTER_CLOCK;
	/*
	 * init timer long enough for using in spl.
	 */
	timer_init();

	/* enable clocks for all PIOs */
#if defined(CONFIG_AT91SAM9X5) || defined(CONFIG_AT91SAM9N12)
	at91_periph_clk_enable(ATMEL_ID_PIOAB);
	at91_periph_clk_enable(ATMEL_ID_PIOCD);
#else
	at91_periph_clk_enable(ATMEL_ID_PIOA);
	at91_periph_clk_enable(ATMEL_ID_PIOB);
	at91_periph_clk_enable(ATMEL_ID_PIOC);
#endif
	/* init console */
	at91_seriald_hw_init();
	preloader_console_init();

	mem_init();

	at91_spl_board_init();
}
