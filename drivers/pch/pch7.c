/*
 * Copyright (C) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <pch.h>

#define BIOS_CTRL	0xd8

static int pch7_get_sbase(struct udevice *dev, ulong *sbasep)
{
	u32 rcba;

	dm_pci_read_config32(dev, PCH_RCBA, &rcba);
	/* Bits 31-14 are the base address, 13-1 are reserved, 0 is enable */
	rcba = rcba & 0xffffc000;
	*sbasep = rcba + 0x3020;

	return 0;
}

static int pch7_set_spi_protect(struct udevice *dev, bool protect)
{
	uint8_t bios_cntl;

	/* Adjust the BIOS write protect to dis/allow write commands */
	dm_pci_read_config8(dev, BIOS_CTRL, &bios_cntl);
	if (protect)
		bios_cntl &= ~BIOS_CTRL_BIOSWE;
	else
		bios_cntl |= BIOS_CTRL_BIOSWE;
	dm_pci_write_config8(dev, BIOS_CTRL, bios_cntl);

	return 0;
}

static const struct pch_ops pch7_ops = {
	.get_sbase	= pch7_get_sbase,
	.set_spi_protect = pch7_set_spi_protect,
};

static const struct udevice_id pch7_ids[] = {
	{ .compatible = "intel,pch7" },
	{ }
};

U_BOOT_DRIVER(pch7_drv) = {
	.name		= "intel-pch7",
	.id		= UCLASS_PCH,
	.of_match	= pch7_ids,
	.ops		= &pch7_ops,
};
