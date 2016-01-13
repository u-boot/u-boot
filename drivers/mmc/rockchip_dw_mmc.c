/*
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dwmmc.h>
#include <errno.h>
#include <syscon.h>
#include <asm/arch/clock.h>
#include <asm/arch/periph.h>
#include <linux/err.h>

DECLARE_GLOBAL_DATA_PTR;

struct rockchip_dwmmc_priv {
	struct udevice *clk;
	struct rk3288_grf *grf;
	struct dwmci_host host;
};

static uint rockchip_dwmmc_get_mmc_clk(struct dwmci_host *host, uint freq)
{
	struct udevice *dev = host->priv;
	struct rockchip_dwmmc_priv *priv = dev_get_priv(dev);
	int ret;

	ret = clk_set_periph_rate(priv->clk, PERIPH_ID_SDMMC0 + host->dev_index,
				  freq);
	if (ret < 0) {
		debug("%s: err=%d\n", __func__, ret);
		return ret;
	}

	return freq;
}

static int rockchip_dwmmc_ofdata_to_platdata(struct udevice *dev)
{
	struct rockchip_dwmmc_priv *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;

	host->name = dev->name;
	host->ioaddr = (void *)dev_get_addr(dev);
	host->buswidth = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
					"bus-width", 4);
	host->get_mmc_clk = rockchip_dwmmc_get_mmc_clk;
	host->priv = dev;

	/* use non-removeable as sdcard and emmc as judgement */
	if (fdtdec_get_bool(gd->fdt_blob, dev->of_offset, "non-removable"))
		host->dev_index = 1;

	return 0;
}

static int rockchip_dwmmc_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct rockchip_dwmmc_priv *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;
	u32 minmax[2];
	int ret;
	int fifo_depth;

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	if (IS_ERR(priv->grf))
		return PTR_ERR(priv->grf);
	ret = uclass_get_device(UCLASS_CLK, CLK_GENERAL, &priv->clk);
	if (ret)
		return ret;

	if (fdtdec_get_int_array(gd->fdt_blob, dev->of_offset,
				 "clock-freq-min-max", minmax, 2))
		return -EINVAL;

	fifo_depth = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
				    "fifo-depth", 0);
	if (fifo_depth < 0)
		return -EINVAL;

	host->fifoth_val = MSIZE(0x2) |
		RX_WMARK(fifo_depth / 2 - 1) | TX_WMARK(fifo_depth / 2);

	if (fdtdec_get_bool(gd->fdt_blob, dev->of_offset, "fifo-mode"))
		host->fifo_mode = true;

	ret = add_dwmci(host, minmax[1], minmax[0]);
	if (ret)
		return ret;

	upriv->mmc = host->mmc;

	return 0;
}

static const struct udevice_id rockchip_dwmmc_ids[] = {
	{ .compatible = "rockchip,rk3288-dw-mshc" },
	{ }
};

U_BOOT_DRIVER(rockchip_dwmmc_drv) = {
	.name		= "rockchip_dwmmc",
	.id		= UCLASS_MMC,
	.of_match	= rockchip_dwmmc_ids,
	.ofdata_to_platdata = rockchip_dwmmc_ofdata_to_platdata,
	.probe		= rockchip_dwmmc_probe,
	.priv_auto_alloc_size = sizeof(struct rockchip_dwmmc_priv),
};
