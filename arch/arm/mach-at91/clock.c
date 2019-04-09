// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Atmel Corporation
 *		      Wenyou Yang <wenyou.yang@atmel.com>
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_wdt.h>

#define EN_UPLL_TIMEOUT		500

static struct udevice *watchdog_dev __attribute__((section(".data"))) = NULL;

void at91_periph_clk_enable(int id)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

#ifdef CPU_HAS_PCR
	u32 regval;
	u32 div_value;

	if (id > AT91_PMC_PCR_PID_MASK)
		return;

	writel(id, &pmc->pcr);

	div_value = readl(&pmc->pcr) & AT91_PMC_PCR_DIV;

	regval = AT91_PMC_PCR_EN | AT91_PMC_PCR_CMD_WRITE | id | div_value;

	writel(regval, &pmc->pcr);
#else
	writel(0x01 << id, &pmc->pcer);
#endif
}

void at91_periph_clk_disable(int id)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

#ifdef CPU_HAS_PCR
	u32 regval;

	if (id > AT91_PMC_PCR_PID_MASK)
		return;

	regval = AT91_PMC_PCR_CMD_WRITE | id;

	writel(regval, &pmc->pcr);
#else
	writel(0x01 << id, &pmc->pcdr);
#endif
}

void at91_system_clk_enable(int sys_clk)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	writel(sys_clk, &pmc->scer);
}

void at91_system_clk_disable(int sys_clk)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	writel(sys_clk, &pmc->scdr);
}

int at91_upll_clk_enable(void)
{
	struct at91_pmc *pmc = (at91_pmc_t *)ATMEL_BASE_PMC;
	ulong start_time, tmp_time;

	if ((readl(&pmc->uckr) & AT91_PMC_UPLLEN) == AT91_PMC_UPLLEN)
		return 0;

	start_time = get_timer(0);
	writel(AT91_PMC_UPLLEN | AT91_PMC_BIASEN, &pmc->uckr);
	while ((readl(&pmc->sr) & AT91_PMC_LOCKU) != AT91_PMC_LOCKU) {
		tmp_time = get_timer(0);
		if ((tmp_time - start_time) > EN_UPLL_TIMEOUT) {
			printf("ERROR: failed to enable UPLL\n");
			return -1;
		}
	}

	return 0;
}

int at91_upll_clk_disable(void)
{
	struct at91_pmc *pmc = (at91_pmc_t *)ATMEL_BASE_PMC;
	ulong start_time, tmp_time;

	start_time = get_timer(0);
	writel(readl(&pmc->uckr) & ~AT91_PMC_UPLLEN, &pmc->uckr);
	while ((readl(&pmc->sr) & AT91_PMC_LOCKU) == AT91_PMC_LOCKU) {
		tmp_time = get_timer(0);
		if ((tmp_time - start_time) > EN_UPLL_TIMEOUT) {
			printf("ERROR: failed to stop UPLL\n");
			return -1;
		}
	}

	return 0;
}

void at91_usb_clk_init(u32 value)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	writel(value, &pmc->usb);
}

void at91_pllicpr_init(u32 icpr)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	writel(icpr, &pmc->pllicpr);
}

/* Called by macro WATCHDOG_RESET */
void watchdog_reset(void)
{
	static ulong next_reset;
	ulong now;

	if (!watchdog_dev)
		return;

	now = get_timer(0);

	/* Do not reset the watchdog too often */
	if (now > next_reset) {
		next_reset = now + 1000;	/* reset every 1000ms */
		wdt_reset(watchdog_dev);
	}
}

int arch_early_init_r(void)
{
	struct at91_wdt_priv *priv;

	/* Init watchdog */
	if (uclass_get_device_by_seq(UCLASS_WDT, 0, &watchdog_dev)) {
		debug("Watchdog: Not found by seq!\n");
		if (uclass_get_device(UCLASS_WDT, 0, &watchdog_dev)) {
			puts("Watchdog: Not found!\n");
			return 0;
		}
	}

	priv = dev_get_priv(watchdog_dev);
	if (!priv) {
		printf("Watchdog: priv not available!\n");
		return 0;
	}

	wdt_start(watchdog_dev, priv->timeout * 1000, 0);
	printf("Watchdog: Started\n");

	return 0;
}
