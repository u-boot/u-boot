/*
 * Copyright (C) 2013 Atmel Corporation
 *		      Bo Shen <voice.shen@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_wdt.h>
#include <asm/arch/clk.h>
#include <spl.h>

static void at91_disable_wdt(void)
{
	struct at91_wdt *wdt = (struct at91_wdt *)ATMEL_BASE_WDT;

	writel(AT91_WDT_MR_WDDIS, &wdt->mr);
}

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

	tmp = readl(&pmc->mor);
	tmp &= ~AT91_PMC_MOR_MOSCRCEN;
	tmp &= ~AT91_PMC_MOR_KEY(0xff);
	tmp |= AT91_PMC_MOR_KEY(0x37);
	writel(tmp, &pmc->mor);
}

void at91_plla_init(u32 pllar)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	writel(pllar, &pmc->pllar);
	while (!(readl(&pmc->sr) & (AT91_PMC_LOCKA | AT91_PMC_MCKRDY)))
		;
}

void at91_mck_init(u32 mckr)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	u32 tmp;

	tmp = readl(&pmc->mckr);
	tmp &= ~(AT91_PMC_MCKR_PRES_MASK |
		 AT91_PMC_MCKR_MDIV_MASK |
		 AT91_PMC_MCKR_PLLADIV_2);
	tmp |= mckr & (AT91_PMC_MCKR_PRES_MASK |
		       AT91_PMC_MCKR_MDIV_MASK |
		       AT91_PMC_MCKR_PLLADIV_2);
	writel(tmp, &pmc->mckr);

	while (!(readl(&pmc->sr) & AT91_PMC_MCKRDY))
		;
}


u32 spl_boot_device(void)
{
#ifdef CONFIG_SYS_USE_MMC
	return BOOT_DEVICE_MMC1;
#elif CONFIG_SYS_USE_NANDFLASH
	return BOOT_DEVICE_NAND;
#elif CONFIG_SYS_USE_SERIALFLASH
	return BOOT_DEVICE_SPI;
#endif
	return BOOT_DEVICE_NONE;
}

u32 spl_boot_mode(void)
{
	switch (spl_boot_device()) {
#ifdef CONFIG_SYS_USE_MMC
	case BOOT_DEVICE_MMC1:
		return MMCSD_MODE_FAT;
		break;
#endif
	case BOOT_DEVICE_NONE:
	default:
		hang();
	}
}

void s_init(void)
{
	switch_to_main_crystal_osc();

	/* disable watchdog */
	at91_disable_wdt();

	/* PMC configuration */
	at91_pmc_init();

	at91_clock_init(CONFIG_SYS_AT91_MAIN_CLOCK);

	timer_init();

	board_early_init_f();

	preloader_console_init();

	mem_init();
}
