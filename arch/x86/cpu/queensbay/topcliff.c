/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <malloc.h>
#include <pci.h>
#include <pci_ids.h>
#include <sdhci.h>

static struct pci_device_id mmc_supported[] = {
	{ PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_TCF_SDIO_0 },
	{ PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_TCF_SDIO_1 },
	{ }
};

int cpu_mmc_init(bd_t *bis)
{
	struct sdhci_host *mmc_host;
	pci_dev_t devbusfn;
	u32 iobase;
	int ret;
	int i;

	for (i = 0; i < ARRAY_SIZE(mmc_supported); i++) {
		devbusfn =  pci_find_devices(mmc_supported, i);
		if (devbusfn == -1)
			return -ENODEV;

		mmc_host = (struct sdhci_host *)malloc(sizeof(struct sdhci_host));
		if (!mmc_host)
			return -ENOMEM;

		mmc_host->name = "Topcliff SDHCI";
		pci_read_config_dword(devbusfn, PCI_BASE_ADDRESS_0, &iobase);
		mmc_host->ioaddr = (void *)iobase;
		mmc_host->quirks = 0;
		ret = add_sdhci(mmc_host, 0, 0);
		if (ret)
			return ret;
	}

	return 0;
}
