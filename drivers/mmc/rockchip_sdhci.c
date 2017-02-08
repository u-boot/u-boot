/*
 * (C) Copyright 2016 Fuzhou Rockchip Electronics Co., Ltd
 *
 * Rockchip SD Host Controller Interface
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <malloc.h>
#include <sdhci.h>
#include <clk.h>

DECLARE_GLOBAL_DATA_PTR;
/* 400KHz is max freq for card ID etc. Use that as min */
#define EMMC_MIN_FREQ	400000

struct rockchip_sdhc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct rockchip_sdhc {
	struct sdhci_host host;
	void *base;
};

static int arasan_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct rockchip_sdhc_plat *plat = dev_get_platdata(dev);
	struct rockchip_sdhc *prv = dev_get_priv(dev);
	struct sdhci_host *host = &prv->host;
	int max_frequency, ret;
	struct clk clk;


	max_frequency = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
			"max-frequency", 0);
	ret = clk_get_by_index(dev, 0, &clk);
	if (!ret) {
		ret = clk_set_rate(&clk, max_frequency);
		if (IS_ERR_VALUE(ret))
			printf("%s clk set rate fail!\n", __func__);
	} else {
		printf("%s fail to get clk\n", __func__);
	}

	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD;
	host->max_clk = max_frequency;

	ret = sdhci_setup_cfg(&plat->cfg, host, 0, EMMC_MIN_FREQ);

	host->mmc = &plat->mmc;
	if (ret)
		return ret;
	host->mmc->priv = &prv->host;
	host->mmc->dev = dev;
	upriv->mmc = host->mmc;

	return sdhci_probe(dev);
}

static int arasan_sdhci_ofdata_to_platdata(struct udevice *dev)
{
	struct sdhci_host *host = dev_get_priv(dev);

	host->name = dev->name;
	host->ioaddr = dev_get_addr_ptr(dev);

	return 0;
}

static int rockchip_sdhci_bind(struct udevice *dev)
{
	struct rockchip_sdhc_plat *plat = dev_get_platdata(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id arasan_sdhci_ids[] = {
	{ .compatible = "arasan,sdhci-5.1" },
	{ }
};

U_BOOT_DRIVER(arasan_sdhci_drv) = {
	.name		= "arasan_sdhci",
	.id		= UCLASS_MMC,
	.of_match	= arasan_sdhci_ids,
	.ofdata_to_platdata = arasan_sdhci_ofdata_to_platdata,
	.ops		= &sdhci_ops,
	.bind		= rockchip_sdhci_bind,
	.probe		= arasan_sdhci_probe,
	.priv_auto_alloc_size = sizeof(struct rockchip_sdhc),
	.platdata_auto_alloc_size = sizeof(struct rockchip_sdhc_plat),
};
