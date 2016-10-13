/*
 * (C) Copyright 2013 Faraday Technology
 * Kuo-Jung Su <dantesu@faraday-tech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <sdhci.h>

#ifndef CONFIG_FTSDC021_CLOCK
#define CONFIG_FTSDC021_CLOCK   clk_get_rate("MMC")
#endif

int ftsdc021_sdhci_init(u32 regbase)
{
	struct sdhci_host *host = NULL;
	uint32_t freq = CONFIG_FTSDC021_CLOCK;

	host = calloc(1, sizeof(struct sdhci_host));
	if (!host) {
		puts("sdh_host malloc fail!\n");
		return -ENOMEM;
	}

	host->name = "FTSDC021";
	host->ioaddr = (void __iomem *)regbase;
	host->quirks = 0;
	add_sdhci(host, freq, 0);

	return 0;
}
