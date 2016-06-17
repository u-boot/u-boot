/*
 * Support of SDHCI for Microchip PIC32 SoC.
 *
 * Copyright (C) 2015 Microchip Technology Inc.
 * Andrei Pistirica <andrei.pistirica@microchip.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <dm.h>
#include <common.h>
#include <sdhci.h>
#include <asm/errno.h>
#include <mach/pic32.h>

DECLARE_GLOBAL_DATA_PTR;

static int pic32_sdhci_probe(struct udevice *dev)
{
	struct sdhci_host *host = dev_get_priv(dev);
	const void *fdt = gd->fdt_blob;
	u32 f_min_max[2];
	fdt_addr_t addr;
	fdt_size_t size;
	int ret;

	addr = fdtdec_get_addr_size(fdt, dev->of_offset, "reg", &size);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	host->ioaddr	= ioremap(addr, size);
	host->name	= (char *)dev->name;
	host->quirks	= SDHCI_QUIRK_NO_HISPD_BIT | SDHCI_QUIRK_NO_CD;
	host->bus_width	= fdtdec_get_int(gd->fdt_blob, dev->of_offset,
					"bus-width", 4);

	ret = fdtdec_get_int_array(gd->fdt_blob, dev->of_offset,
				   "clock-freq-min-max", f_min_max, 2);
	if (ret) {
		printf("sdhci: clock-freq-min-max not found\n");
		return ret;
	}

	return add_sdhci(host, f_min_max[1], f_min_max[0]);
}

static const struct udevice_id pic32_sdhci_ids[] = {
	{ .compatible = "microchip,pic32mzda-sdhci" },
	{ }
};

U_BOOT_DRIVER(pic32_sdhci_drv) = {
	.name			= "pic32_sdhci",
	.id			= UCLASS_MMC,
	.of_match		= pic32_sdhci_ids,
	.probe			= pic32_sdhci_probe,
	.priv_auto_alloc_size	= sizeof(struct sdhci_host),
};
