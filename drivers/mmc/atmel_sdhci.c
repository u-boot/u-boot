/*
 * Copyright (C) 2015 Atmel Corporation
 *		      Wenyou.Yang <wenyou.yang@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <sdhci.h>
#include <asm/arch/clk.h>

#define ATMEL_SDHC_MIN_FREQ	400000

#ifndef CONFIG_DM_MMC
int atmel_sdhci_init(void *regbase, u32 id)
{
	struct sdhci_host *host;
	u32 max_clk, min_clk = ATMEL_SDHC_MIN_FREQ;

	host = (struct sdhci_host *)calloc(1, sizeof(struct sdhci_host));
	if (!host) {
		printf("%s: sdhci_host calloc failed\n", __func__);
		return -ENOMEM;
	}

	host->name = "atmel_sdhci";
	host->ioaddr = regbase;
	host->quirks = 0;
	host->version = sdhci_readw(host, SDHCI_HOST_VERSION);
	max_clk = at91_get_periph_generated_clk(id);
	if (!max_clk) {
		printf("%s: Failed to get the proper clock\n", __func__);
		free(host);
		return -ENODEV;
	}

	add_sdhci(host, max_clk, min_clk);

	return 0;
}

#else

DECLARE_GLOBAL_DATA_PTR;

struct atmel_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

static int atmel_sdhci_get_clk(struct udevice *dev, int index, struct clk *clk)
{
	struct udevice *dev_clk;
	int periph, ret;

	ret = clk_get_by_index(dev, index, clk);
	if (ret)
		return ret;

	periph = fdtdec_get_uint(gd->fdt_blob, clk->dev->of_offset, "reg", -1);
	if (periph < 0)
		return -EINVAL;

	dev_clk = dev_get_parent(clk->dev);
	ret = clk_request(dev_clk, clk);
	if (ret)
		return ret;

	clk->id = periph;

	return 0;
}

static int atmel_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct atmel_sdhci_plat *plat = dev_get_platdata(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	u32 max_clk;
	u32 caps, caps_1;
	u32 clk_base, clk_mul;
	ulong gck_rate;
	struct clk clk;
	int ret;

	ret = atmel_sdhci_get_clk(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	host->name = dev->name;
	host->ioaddr = (void *)dev_get_addr(dev);

	host->quirks = 0;
	host->bus_width	= fdtdec_get_int(gd->fdt_blob, dev->of_offset,
					 "bus-width", 4);

	caps = sdhci_readl(host, SDHCI_CAPABILITIES);
	clk_base = (caps & SDHCI_CLOCK_V3_BASE_MASK) >> SDHCI_CLOCK_BASE_SHIFT;
	caps_1 = sdhci_readl(host, SDHCI_CAPABILITIES_1);
	clk_mul = (caps_1 & SDHCI_CLOCK_MUL_MASK) >> SDHCI_CLOCK_MUL_SHIFT;
	gck_rate = clk_base * 1000000 * (clk_mul + 1);

	ret = atmel_sdhci_get_clk(dev, 1, &clk);
	if (ret)
		return ret;

	ret = clk_set_rate(&clk, gck_rate);
	if (ret)
		return ret;

	max_clk = clk_get_rate(&clk);
	if (!max_clk)
		return -EINVAL;

	ret = sdhci_setup_cfg(&plat->cfg, host, max_clk, ATMEL_SDHC_MIN_FREQ);
	if (ret)
		return ret;

	host->mmc = &plat->mmc;
	host->mmc->dev = dev;
	host->mmc->priv = host;
	upriv->mmc = host->mmc;

	clk_free(&clk);

	return sdhci_probe(dev);
}

static int atmel_sdhci_bind(struct udevice *dev)
{
	struct atmel_sdhci_plat *plat = dev_get_platdata(dev);
	int ret;

	ret = sdhci_bind(dev, &plat->mmc, &plat->cfg);
	if (ret)
		return ret;

	return 0;
}

static const struct udevice_id atmel_sdhci_ids[] = {
	{ .compatible = "atmel,sama5d2-sdhci" },
	{ }
};

U_BOOT_DRIVER(atmel_sdhci_drv) = {
	.name		= "atmel_sdhci",
	.id		= UCLASS_MMC,
	.of_match	= atmel_sdhci_ids,
	.ops		= &sdhci_ops,
	.bind		= atmel_sdhci_bind,
	.probe		= atmel_sdhci_probe,
	.priv_auto_alloc_size = sizeof(struct sdhci_host),
	.platdata_auto_alloc_size = sizeof(struct atmel_sdhci_plat),
};
#endif
