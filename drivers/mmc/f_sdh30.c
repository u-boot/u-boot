// SPDX-License-Identifier: GPL-2.0+
/*
 * Socionext F_SDH30 eMMC driver
 * Copyright 2021 Linaro Ltd.
 * Copyright 2021 Socionext, Inc.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <sdhci.h>

struct f_sdh30_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

DECLARE_GLOBAL_DATA_PTR;

static int f_sdh30_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct f_sdh30_plat *plat = dev_get_plat(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	int ret;

	ret = mmc_of_parse(dev, &plat->cfg);
	if (ret)
		return ret;

	host->mmc = &plat->mmc;
	host->mmc->dev = dev;
	host->mmc->priv = host;

	ret = sdhci_setup_cfg(&plat->cfg, host, 200000000, 400000);
	if (ret)
		return ret;

	upriv->mmc = host->mmc;

	mmc_set_clock(host->mmc, host->mmc->cfg->f_min, MMC_CLK_ENABLE);

	return sdhci_probe(dev);
}

static int f_sdh30_of_to_plat(struct udevice *dev)
{
	struct sdhci_host *host = dev_get_priv(dev);

	host->name = strdup(dev->name);
	host->ioaddr = dev_read_addr_ptr(dev);
	host->bus_width = dev_read_u32_default(dev, "bus-width", 4);
	host->index = dev_read_u32_default(dev, "index", 0);

	return 0;
}

static int f_sdh30_bind(struct udevice *dev)
{
	struct f_sdh30_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id f_sdh30_mmc_ids[] = {
	{ .compatible = "fujitsu,mb86s70-sdhci-3.0" },
	{ }
};

U_BOOT_DRIVER(f_sdh30_drv) = {
	.name		= "f_sdh30_sdhci",
	.id		= UCLASS_MMC,
	.of_match	= f_sdh30_mmc_ids,
	.of_to_plat	= f_sdh30_of_to_plat,
	.ops		= &sdhci_ops,
	.bind		= f_sdh30_bind,
	.probe		= f_sdh30_sdhci_probe,
	.priv_auto	= sizeof(struct sdhci_host),
	.plat_auto	= sizeof(struct f_sdh30_plat),
};
