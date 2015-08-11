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

	add_sdhci(host, 52000000, 52000000 >> 9);
	return 0;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
int zynq_sdhci_of_init(const void *blob)
{
	int offset = 0;
	u32 ret = 0;
	phys_addr_t reg;

	debug("ZYNQ SDHCI: Initialization\n");

	do {
		offset = fdt_node_offset_by_compatible(blob, offset,
					"arasan,sdhci-8.9a");
		if (offset != -1) {
			reg = fdtdec_get_addr(blob, offset, "reg");
			if (reg != FDT_ADDR_T_NONE) {
				ret |= zynq_sdhci_init(reg);
			} else {
				debug("ZYNQ SDHCI: Can't get base address\n");
				return -1;
			}
		}
	} while (offset != -1);

	return ret;
}
#endif
