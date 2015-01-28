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

int pci_mmc_init(const char *name, struct pci_device_id *mmc_supported,
		 int num_ids)
{
	struct sdhci_host *mmc_host;
	pci_dev_t devbusfn;
	u32 iobase;
	int ret;
	int i;

	for (i = 0; i < num_ids; i++) {
		devbusfn = pci_find_devices(mmc_supported, i);
		if (devbusfn == -1)
			return -ENODEV;

		mmc_host = malloc(sizeof(struct sdhci_host));
		if (!mmc_host)
			return -ENOMEM;

		mmc_host->name = (char *)name;
		pci_read_config_dword(devbusfn, PCI_BASE_ADDRESS_0, &iobase);
		mmc_host->ioaddr = (void *)iobase;
		mmc_host->quirks = 0;
		ret = add_sdhci(mmc_host, 0, 0);
		if (ret)
			return ret;
	}

	return 0;
}
