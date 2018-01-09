/*
 * Andestech ATFSDC010 SD/MMC driver
 *
 * (C) Copyright 2017
 * Rick Chen, NDS32 Software Engineering, rick@andestech.com

 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dt-structs.h>
#include <errno.h>
#include <mapmem.h>
#include <mmc.h>
#include <pwrseq.h>
#include <syscon.h>
#include <linux/err.h>
#include <faraday/ftsdc010.h>
#include "ftsdc010_mci.h"

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_IS_ENABLED(OF_PLATDATA)
struct nds_mmc {
	fdt32_t		bus_width;
	bool		cap_mmc_highspeed;
	bool		cap_sd_highspeed;
	fdt32_t		clock_freq_min_max[2];
	struct phandle_2_cell	clocks[4];
	fdt32_t		fifo_depth;
	fdt32_t		reg[2];
};
#endif

struct nds_mmc_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct nds_mmc dtplat;
#endif
	struct mmc_config cfg;
	struct mmc mmc;
};

struct ftsdc_priv {
	struct clk clk;
	struct ftsdc010_chip chip;
	int fifo_depth;
	bool fifo_mode;
	u32 minmax[2];
};

static int nds32_mmc_ofdata_to_platdata(struct udevice *dev)
{
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	struct ftsdc_priv *priv = dev_get_priv(dev);
	struct ftsdc010_chip *chip = &priv->chip;
	chip->name = dev->name;
	chip->ioaddr = (void *)devfdt_get_addr(dev);
	chip->buswidth = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					"bus-width", 4);
	chip->priv = dev;
	priv->fifo_depth = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
				    "fifo-depth", 0);
	priv->fifo_mode = fdtdec_get_bool(gd->fdt_blob, dev_of_offset(dev),
					  "fifo-mode");
	if (fdtdec_get_int_array(gd->fdt_blob, dev_of_offset(dev),
			 "clock-freq-min-max", priv->minmax, 2)) {
		int val = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
				  "max-frequency", -EINVAL);
		if (val < 0)
			return val;

		priv->minmax[0] = 400000;  /* 400 kHz */
		priv->minmax[1] = val;
	} else {
		debug("%s: 'clock-freq-min-max' property was deprecated.\n",
		__func__);
	}
#endif
	chip->sclk = priv->minmax[1];
	chip->regs = chip->ioaddr;
	return 0;
}

static int nds32_mmc_probe(struct udevice *dev)
{
	struct nds_mmc_plat *plat = dev_get_platdata(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct ftsdc_priv *priv = dev_get_priv(dev);
	struct ftsdc010_chip *chip = &priv->chip;
	struct udevice *pwr_dev __maybe_unused;
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	int ret;
	struct nds_mmc *dtplat = &plat->dtplat;
	chip->name = dev->name;
	chip->ioaddr = map_sysmem(dtplat->reg[0], dtplat->reg[1]);
	chip->buswidth = dtplat->bus_width;
	chip->priv = dev;
	chip->dev_index = 1;
	memcpy(priv->minmax, dtplat->clock_freq_min_max, sizeof(priv->minmax));
	ret = clk_get_by_index_platdata(dev, 0, dtplat->clocks, &priv->clk);
	if (ret < 0)
		return ret;
#endif
	ftsdc_setup_cfg(&plat->cfg, dev->name, chip->buswidth, chip->caps,
			priv->minmax[1] , priv->minmax[0]);
	chip->mmc = &plat->mmc;
	chip->mmc->priv = &priv->chip;
	chip->mmc->dev = dev;
	upriv->mmc = chip->mmc;
	return ftsdc010_probe(dev);
}

static int nds32_mmc_bind(struct udevice *dev)
{
	struct nds_mmc_plat *plat = dev_get_platdata(dev);
	return ftsdc010_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id nds32_mmc_ids[] = {
	{ .compatible = "andestech,atsdc010" },
	{ }
};

U_BOOT_DRIVER(nds32_mmc_drv) = {
	.name		= "nds32_mmc",
	.id		= UCLASS_MMC,
	.of_match	= nds32_mmc_ids,
	.ofdata_to_platdata = nds32_mmc_ofdata_to_platdata,
	.ops		= &dm_ftsdc010_ops,
	.bind		= nds32_mmc_bind,
	.probe		= nds32_mmc_probe,
	.priv_auto_alloc_size = sizeof(struct ftsdc_priv),
	.platdata_auto_alloc_size = sizeof(struct nds_mmc_plat),
};
