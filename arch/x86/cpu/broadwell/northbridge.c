// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2011 The Chromium Authors
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/iomap.h>
#include <asm/arch/pch.h>

static int broadwell_northbridge_early_init(struct udevice *dev)
{
	/* Move earlier? */
	dm_pci_write_config32(dev, PCIEXBAR + 4, 0);
	/* 64MiB - 0-63 buses */
	dm_pci_write_config32(dev, PCIEXBAR, MCFG_BASE_ADDRESS | 4 | 1);

	dm_pci_write_config32(dev, MCHBAR, MCH_BASE_ADDRESS | 1);
	dm_pci_write_config32(dev, DMIBAR, DMI_BASE_ADDRESS | 1);
	dm_pci_write_config32(dev, EPBAR, EP_BASE_ADDRESS | 1);
	writel(EDRAM_BASE_ADDRESS | 1, MCH_BASE_ADDRESS + EDRAMBAR);
	writel(GDXC_BASE_ADDRESS | 1, MCH_BASE_ADDRESS + GDXCBAR);

	/* Set C0000-FFFFF to access RAM on both reads and writes */
	dm_pci_write_config8(dev, PAM0, 0x30);
	dm_pci_write_config8(dev, PAM1, 0x33);
	dm_pci_write_config8(dev, PAM2, 0x33);
	dm_pci_write_config8(dev, PAM3, 0x33);
	dm_pci_write_config8(dev, PAM4, 0x33);
	dm_pci_write_config8(dev, PAM5, 0x33);
	dm_pci_write_config8(dev, PAM6, 0x33);

	/* Device enable: IGD and Mini-HD */
	dm_pci_write_config32(dev, DEVEN, DEVEN_D0EN | DEVEN_D2EN | DEVEN_D3EN);

	return 0;
}

static int broadwell_northbridge_probe(struct udevice *dev)
{
	if (!(gd->flags & GD_FLG_RELOC))
		return broadwell_northbridge_early_init(dev);

	return 0;
}

static const struct udevice_id broadwell_northbridge_ids[] = {
	{ .compatible = "intel,broadwell-northbridge" },
	{ }
};

U_BOOT_DRIVER(broadwell_northbridge_drv) = {
	.name		= "broadwell_northbridge",
	.id		= UCLASS_NORTHBRIDGE,
	.of_match	= broadwell_northbridge_ids,
	.probe		= broadwell_northbridge_probe,
};
