/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mmc.h>
#include <pci_ids.h>

static struct pci_device_id mmc_supported[] = {
	{ PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_TCF_SDIO_0 },
	{ PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_TCF_SDIO_1 },
};

int cpu_mmc_init(bd_t *bis)
{
	return pci_mmc_init("Topcliff SDHCI", mmc_supported,
			    ARRAY_SIZE(mmc_supported));
}
