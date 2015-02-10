/*
 * Copyright (C) 2013 Atmel Corporation
 *		      Bo Shen <voice.shen@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pit.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/at91_wdt.h>
#include <asm/arch/clk.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

static void switch_to_main_crystal_osc(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	u32 tmp;

	tmp = readl(&pmc->mor);
	tmp &= ~AT91_PMC_MOR_OSCOUNT(0xff);
	tmp &= ~AT91_PMC_MOR_KEY(0xff);
	tmp |= AT91_PMC_MOR_MOSCEN;
	tmp |= AT91_PMC_MOR_OSCOUNT(8);
	tmp |= AT91_PMC_MOR_KEY(0x37);
	writel(tmp, &pmc->mor);
	while (!(readl(&pmc->sr) & AT91_PMC_IXR_MOSCS))
		;

	tmp = readl(&pmc->mor);
	tmp &= ~AT91_PMC_MOR_OSCBYPASS;
	tmp &= ~AT91_PMC_MOR_KEY(0xff);
	tmp |= AT91_PMC_MOR_KEY(0x37);
	writel(tmp, &pmc->mor);

	tmp = readl(&pmc->mor);
	tmp |= AT91_PMC_MOR_MOSCSEL;
	tmp &= ~AT91_PMC_MOR_KEY(0xff);
	tmp |= AT91_PMC_MOR_KEY(0x37);
	writel(tmp, &pmc->mor);

	while (!(readl(&pmc->sr) & AT91_PMC_IXR_MOSCSELS))
		;

	/* Wait until MAINRDY field is set to make sure main clock is stable */
	while (!(readl(&pmc->mcfr) & AT91_PMC_MAINRDY))
		;

#ifndef CONFIG_SAMA5D4
	tmp = readl(&pmc->mor);
	tmp &= ~AT91_PMC_MOR_MOSCRCEN;
	tmp &= ~AT91_PMC_MOR_KEY(0xff);
	tmp |= AT91_PMC_MOR_KEY(0x37);
	writel(tmp, &pmc->mor);
#endif
}

__weak void matrix_init(void)
{
	/* This only be used for sama5d4 soc now */
}

__weak void redirect_int_from_saic_to_aic(void)
{
	/* This only be used for sama5d4 soc now */
}

void s_init(void)
{
	switch_to_main_crystal_osc();

	/* disable watchdog */
	at91_disable_wdt();

	/* PMC configuration */
	at91_pmc_init();

	at91_clock_init(CONFIG_SYS_AT91_MAIN_CLOCK);

	matrix_init();

	redirect_int_from_saic_to_aic();

	timer_init();

	board_early_init_f();

	preloader_console_init();

	mem_init();
}
