/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dwmmc.h>
#include <malloc.h>
#include <netdev.h>
#include <phy.h>
#include "axs10x.h"

DECLARE_GLOBAL_DATA_PTR;

int board_mmc_init(bd_t *bis)
{
	struct dwmci_host *host = NULL;

	host = malloc(sizeof(struct dwmci_host));
	if (!host) {
		printf("dwmci_host malloc fail!\n");
		return 1;
	}

	memset(host, 0, sizeof(struct dwmci_host));
	host->name = "Synopsys Mobile storage";
	host->ioaddr = (void *)ARC_DWMMC_BASE;
	host->buswidth = 4;
	host->dev_index = 0;
	host->bus_hz = 50000000;

	add_dwmci(host, host->bus_hz, 400000);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	if (designware_initialize(ARC_DWGMAC_BASE,
				  PHY_INTERFACE_MODE_RGMII) >= 0)
		return 1;

	return 0;
}


#define AXS_MB_CREG	0xE0011000

int board_early_init_f(void)
{
	if (readl((void __iomem *)AXS_MB_CREG + 0x234) & (1 << 28))
		gd->board_type = AXS_MB_V3;
	else
		gd->board_type = AXS_MB_V2;

	return 0;
}
