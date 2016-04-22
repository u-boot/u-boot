/*
 * Copyright (C) 2015, Google, Inc
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <malloc.h>
#include <sdhci.h>
#include <asm/pci.h>

int pci_mmc_init(const char *name, struct pci_device_id *mmc_supported)
{
	struct sdhci_host *mmc_host;
	u32 iobase;
	int ret;
	int i;

	for (i = 0; ; i++) {
		struct udevice *dev;

		ret = pci_find_device_id(mmc_supported, i, &dev);
		if (ret)
			return ret;
		mmc_host = malloc(sizeof(struct sdhci_host));
		if (!mmc_host)
			return -ENOMEM;

		mmc_host->name = name;
		dm_pci_read_config32(dev, PCI_BASE_ADDRESS_0, &iobase);
		mmc_host->ioaddr = (void *)iobase;
		mmc_host->quirks = 0;
		ret = add_sdhci(mmc_host, 0, 0);
		if (ret)
			return ret;
	}

	return 0;
}
