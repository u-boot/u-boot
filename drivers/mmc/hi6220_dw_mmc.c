// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Linaro
 * peter.griffin <peter.griffin@linaro.org>
 */

#include <common.h>
#include <dwmmc.h>
#include <malloc.h>
#include <linux/errno.h>

#define	DWMMC_MAX_CH_NUM		4

#define	DWMMC_MAX_FREQ			50000000
#define	DWMMC_MIN_FREQ			400000

/* Source clock is configured to 100MHz by ATF bl1*/
#define MMC0_DEFAULT_FREQ		100000000

static int hi6220_dwmci_core_init(struct dwmci_host *host, int index)
{
	host->name = "Hisilicon DWMMC";

	host->dev_index = index;

	/* Add the mmc channel to be registered with mmc core */
	if (add_dwmci(host, DWMMC_MAX_FREQ, DWMMC_MIN_FREQ)) {
		printf("DWMMC%d registration failed\n", index);
		return -1;
	}
	return 0;
}

/*
 * This function adds the mmc channel to be registered with mmc core.
 * index -	mmc channel number.
 * regbase -	register base address of mmc channel specified in 'index'.
 * bus_width -	operating bus width of mmc channel specified in 'index'.
 */
int hi6220_dwmci_add_port(int index, u32 regbase, int bus_width)
{
	struct dwmci_host *host = NULL;

	host = calloc(1, sizeof(struct dwmci_host));
	if (!host) {
		pr_err("dwmci_host calloc failed!\n");
		return -ENOMEM;
	}

	host->ioaddr = (void *)(ulong)regbase;
	host->buswidth = bus_width;
	host->bus_hz = MMC0_DEFAULT_FREQ;

	return hi6220_dwmci_core_init(host, index);
}
