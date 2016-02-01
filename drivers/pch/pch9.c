/*
 * Copyright (C) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <pch.h>

#define SBASE_ADDR	0x54

static int pch9_get_spi_base(struct udevice *dev, ulong *sbasep)
{
	uint32_t sbase_addr;

	dm_pci_read_config32(dev, SBASE_ADDR, &sbase_addr);
	*sbasep = sbase_addr & 0xfffffe00;

	return 0;
}

static const struct pch_ops pch9_ops = {
	.get_spi_base	= pch9_get_spi_base,
};

static const struct udevice_id pch9_ids[] = {
	{ .compatible = "intel,pch9" },
	{ }
};

U_BOOT_DRIVER(pch9_drv) = {
	.name		= "intel-pch9",
	.id		= UCLASS_PCH,
	.of_match	= pch9_ids,
	.ops		= &pch9_ops,
};
