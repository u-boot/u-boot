/*
 * (C) Copyright 2013 Inc.
 *
 * Xilinx Zynq SD Host Controller Interface
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <malloc.h>
#include <sdhci.h>
#include <asm/arch/sys_proto.h>

int zynq_sdhci_init(phys_addr_t regbase)
{
	struct sdhci_host *host = NULL;

	host = (struct sdhci_host *)malloc(sizeof(struct sdhci_host));
	if (!host) {
		printf("zynq_sdhci_init: sdhci_host malloc fail\n");
		return 1;
	}

	host->name = "zynq_sdhci";
	host->ioaddr = (void *)regbase;
	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD |
		       SDHCI_QUIRK_BROKEN_R1B;
	host->version = sdhci_readw(host, SDHCI_HOST_VERSION);

	add_sdhci(host, CONFIG_ZYNQ_SDHCI_MAX_FREQ, 0);
	return 0;
}
