/*
 * (C) Copyright 2013 Inc.
 *
 * Xilinx Zynq SD Host Controller Interface
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <sdhci.h>
#include <asm/arch/sys_proto.h>

int zynq_sdhci_init(u32 regbase)
{
	struct sdhci_host *host = NULL;

	host = (struct sdhci_host *)malloc(sizeof(struct sdhci_host));
	if (!host) {
		printf("zynq_sdhci_init: sdhci_host malloc fail\n");
		return 1;
	}

	host->name = "zynq_sdhci";
	host->ioaddr = (void *)regbase;
	host->quirks = SDHCI_QUIRK_NO_CD | SDHCI_QUIRK_WAIT_SEND_CMD;
	host->version = sdhci_readw(host, SDHCI_HOST_VERSION);

	host->host_caps = MMC_MODE_HC;

	add_sdhci(host, 52000000, 52000000 >> 9);
	return 0;
}
