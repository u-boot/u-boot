// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 *
 * Based on Rockchip's sdhci.c file
 */

#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <sdhci.h>
#include <asm/cache.h>

/* 400KHz is max freq for card ID etc. Use that as min */
#define EMMC_MIN_FREQ	400000

/* Check if an operation crossed a boundary of size ADMA_BOUNDARY_ALIGN */
#define ADMA_BOUNDARY_ALGN SZ_128M
#define BOUNDARY_OK(addr, len) \
	(((addr) | (ADMA_BOUNDARY_ALGN - 1)) == (((addr) + (len) - 1) | \
	(ADMA_BOUNDARY_ALGN - 1)))

/* We split a descriptor for every crossing of the ADMA alignment boundary,
 * so we need an additional descriptor for every expected crossing.
 * As I understand it, the max expected transaction size is:
 *  CONFIG_SYS_MMC_MAX_BLK_COUNT * MMC_MAX_BLOCK_LEN
 *
 * With the way the SDHCI-ADMA driver is implemented, if ADMA_MAX_LEN was a
 * clean power of two, we'd only ever need +1 descriptor as the first
 * descriptor that got split would then bring the remaining DMA
 * destination addresses into alignment. Unfortunately, it's currently
 * hardcoded to a non-power-of-two value.
 *
 * If that ever becomes parameterized, ADMA max length can be set to
 * 0x10000, and set this to 1.
 */
#define ADMA_POTENTIAL_CROSSINGS \
	DIV_ROUND_UP((CONFIG_SYS_MMC_MAX_BLK_COUNT * MMC_MAX_BLOCK_LEN), \
		 ADMA_BOUNDARY_ALGN)
/* +1 descriptor for each crossing.
 */
#define ADMA_TABLE_EXTRA_SZ (ADMA_POTENTIAL_CROSSINGS * ADMA_DESC_LEN)

struct adi_sdhc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

void adi_dwcmshc_adma_write_desc(struct sdhci_host *host, void **desc,
				 dma_addr_t addr, int len, bool end)
{
	int tmplen, offset;

	if (likely(!len || BOUNDARY_OK(addr, len))) {
		sdhci_adma_write_desc(host, desc, addr, len, end);
		return;
	}

	offset = addr & (ADMA_BOUNDARY_ALGN - 1);
	tmplen = ADMA_BOUNDARY_ALGN - offset;
	sdhci_adma_write_desc(host, desc, addr, tmplen, false);

	addr += tmplen;
	len -= tmplen;
	sdhci_adma_write_desc(host, desc, addr, len, end);
}

struct sdhci_ops adi_dwcmshc_sdhci_ops = {
	.adma_write_desc = adi_dwcmshc_adma_write_desc,
};

static int adi_dwcmshc_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct adi_sdhc_plat *plat = dev_get_plat(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	int max_frequency, ret;
	struct clk clk;

	max_frequency = dev_read_u32_default(dev, "max-frequency", 0);
	ret = clk_get_by_index(dev, 0, &clk);

	host->quirks = 0;
	host->max_clk = max_frequency;
	/*
	 * The sdhci-driver only supports 4bit and 8bit, as sdhci_setup_cfg
	 * doesn't allow us to clear MMC_MODE_4BIT.  Consequently, we don't
	 * check for other bus-width values.
	 */
	if (host->bus_width == 8)
		host->host_caps |= MMC_MODE_8BIT;

	host->mmc = &plat->mmc;
	host->mmc->priv = host;
	host->mmc->dev = dev;
	upriv->mmc = host->mmc;

	host->ops = &adi_dwcmshc_sdhci_ops;
	host->adma_desc_table = memalign(ARCH_DMA_MINALIGN,
					 ADMA_TABLE_SZ + ADMA_TABLE_EXTRA_SZ);
	host->adma_addr = virt_to_phys(host->adma_desc_table);

	ret = sdhci_setup_cfg(&plat->cfg, host, 0, EMMC_MIN_FREQ);
	if (ret)
		return ret;

	return sdhci_probe(dev);
}

static int adi_dwcmshc_sdhci_of_to_plat(struct udevice *dev)
{
	struct sdhci_host *host = dev_get_priv(dev);

	host->name = dev->name;
	host->ioaddr = dev_read_addr_ptr(dev);
	host->bus_width = dev_read_u32_default(dev, "bus-width", 4);

	return 0;
}

static int adi_sdhci_bind(struct udevice *dev)
{
	struct adi_sdhc_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id adi_dwcmshc_sdhci_ids[] = {
	{ .compatible = "adi,dwc-sdhci" },
	{ }
};

U_BOOT_DRIVER(adi_dwcmshc_sdhci_drv) = {
	.name		= "adi_sdhci",
	.id		= UCLASS_MMC,
	.of_match	= adi_dwcmshc_sdhci_ids,
	.of_to_plat	= adi_dwcmshc_sdhci_of_to_plat,
	.ops		= &sdhci_ops,
	.bind		= adi_sdhci_bind,
	.probe		= adi_dwcmshc_sdhci_probe,
	.priv_auto	= sizeof(struct sdhci_host),
	.plat_auto	= sizeof(struct adi_sdhc_plat),
};
