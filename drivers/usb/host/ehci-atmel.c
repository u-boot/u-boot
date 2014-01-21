/*
 * (C) Copyright 2012
 * Atmel Semiconductor <www.atmel.com>
 * Written-by: Bo Shen <voice.shen@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <watchdog.h>
#include <usb.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/clk.h>

#include "ehci.h"

/* Enable UTMI PLL time out 500us
 * 10 times as datasheet specified
 */
#define EN_UPLL_TIMEOUT	500UL

int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	at91_pmc_t *pmc = (at91_pmc_t *)ATMEL_BASE_PMC;
	ulong start_time, tmp_time;

	start_time = get_timer(0);
	/* Enable UTMI PLL */
	writel(AT91_PMC_UPLLEN | AT91_PMC_BIASEN, &pmc->uckr);
	while ((readl(&pmc->sr) & AT91_PMC_LOCKU) != AT91_PMC_LOCKU) {
		WATCHDOG_RESET();
		tmp_time = get_timer(0);
		if ((tmp_time - start_time) > EN_UPLL_TIMEOUT) {
			printf("ERROR: failed to enable UPLL\n");
			return -1;
		}
	}

	/* Enable USB Host clock */
	writel(1 << ATMEL_ID_UHPHS, &pmc->pcer);

	*hccr = (struct ehci_hccr *)ATMEL_BASE_EHCI;
	*hcor = (struct ehci_hcor *)((uint32_t)*hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	return 0;
}

int ehci_hcd_stop(int index)
{
	at91_pmc_t *pmc = (at91_pmc_t *)ATMEL_BASE_PMC;
	ulong start_time, tmp_time;

	/* Disable USB Host Clock */
	writel(1 << ATMEL_ID_UHPHS, &pmc->pcdr);

	start_time = get_timer(0);
	/* Disable UTMI PLL */
	writel(readl(&pmc->uckr) & ~AT91_PMC_UPLLEN, &pmc->uckr);
	while ((readl(&pmc->sr) & AT91_PMC_LOCKU) == AT91_PMC_LOCKU) {
		WATCHDOG_RESET();
		tmp_time = get_timer(0);
		if ((tmp_time - start_time) > EN_UPLL_TIMEOUT) {
			printf("ERROR: failed to stop UPLL\n");
			return -1;
		}
	}

	return 0;
}
