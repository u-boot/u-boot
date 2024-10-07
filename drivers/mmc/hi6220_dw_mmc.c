// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Linaro
 * peter.griffin <peter.griffin@linaro.org>
 */

#include <clk.h>
#include <dm.h>
#include <dwmmc.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <reset.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>

DECLARE_GLOBAL_DATA_PTR;

enum hi6220_dwmmc_clk_type {
	HI6220_DWMMC_CLK_BIU,
	HI6220_DWMMC_CLK_CIU,
	HI6220_DWMMC_CLK_CNT,
};

struct hi6220_dwmmc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct hi6220_dwmmc_priv_data {
	struct dwmci_host host;
	struct clk *clks[HI6220_DWMMC_CLK_CNT];
	struct reset_ctl_bulk rsts;
};

struct hisi_mmc_data {
	unsigned int clock;
	bool use_fifo;
	u32 fifo_depth;
};

static int hi6220_dwmmc_of_to_plat(struct udevice *dev)
{
	struct hi6220_dwmmc_priv_data *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;
	int ret;

	if (CONFIG_IS_ENABLED(CLK) && CONFIG_IS_ENABLED(DM_RESET)) {
		priv->clks[HI6220_DWMMC_CLK_BIU] = devm_clk_get(dev, "biu");
		if (IS_ERR(priv->clks[HI6220_DWMMC_CLK_BIU])) {
			ret = PTR_ERR(priv->clks[HI6220_DWMMC_CLK_BIU]);
			dev_err(dev, "Failed to get BIU clock(ret = %d).\n", ret);
			return log_msg_ret("clk", ret);
		}

		priv->clks[HI6220_DWMMC_CLK_CIU] = devm_clk_get(dev, "ciu");
		if (IS_ERR(priv->clks[HI6220_DWMMC_CLK_CIU])) {
			ret = PTR_ERR(priv->clks[HI6220_DWMMC_CLK_CIU]);
			dev_err(dev, "Failed to get CIU clock(ret = %d).\n", ret);
			return log_msg_ret("clk", ret);
		}

		ret = reset_get_bulk(dev, &priv->rsts);
		if (ret) {
			dev_err(dev, "Failed to get resets(ret = %d)", ret);
			return log_msg_ret("rst", ret);
		}
	}
	host->name = dev->name;
	host->ioaddr = dev_read_addr_ptr(dev);
	host->buswidth = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					"bus-width", 4);

	/* use non-removable property for differentiating SD card and eMMC */
	if (dev_read_bool(dev, "non-removable"))
		host->dev_index = 0;
	else
		host->dev_index = 1;

	host->priv = priv;

	return 0;
}

static int hi6220_dwmmc_probe(struct udevice *dev)
{
	struct hi6220_dwmmc_plat *plat = dev_get_plat(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct hi6220_dwmmc_priv_data *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;
	struct hisi_mmc_data *mmc_data;
	int ret;

	mmc_data = (struct hisi_mmc_data *)dev_get_driver_data(dev);

	host->bus_hz = mmc_data->clock;
	if (CONFIG_IS_ENABLED(CLK) && CONFIG_IS_ENABLED(DM_RESET)) {
		ret = clk_prepare_enable(priv->clks[HI6220_DWMMC_CLK_BIU]);
		if (ret) {
			dev_err(dev, "Failed to enable biu clock(ret = %d).\n", ret);
			return log_msg_ret("clk", ret);
		}

		ret = clk_prepare_enable(priv->clks[HI6220_DWMMC_CLK_CIU]);
		if (ret) {
			dev_err(dev, "Failed to enable ciu clock(ret = %d).\n", ret);
			return log_msg_ret("clk", ret);
		}

		ret = reset_deassert_bulk(&priv->rsts);
		if (ret) {
			dev_err(dev, "Failed to deassert resets(ret = %d).\n", ret);
			return log_msg_ret("rst", ret);
		}

		host->bus_hz = clk_get_rate(priv->clks[HI6220_DWMMC_CLK_CIU]);
		if (host->bus_hz <= 0) {
			dev_err(dev, "Failed to get ciu clock rate(ret = %d).\n", ret);
			return log_msg_ret("clk", ret);
		}
	}
	dev_dbg(dev, "bus clock rate: %d.\n", host->bus_hz);

	dwmci_setup_cfg(&plat->cfg, host, host->bus_hz, 400000);
	host->mmc = &plat->mmc;

	host->fifo_mode = mmc_data->use_fifo;
	host->fifo_depth = mmc_data->fifo_depth;
	host->mmc->priv = &priv->host;
	upriv->mmc = host->mmc;
	host->mmc->dev = dev;

	return dwmci_probe(dev);
}

static int hi6220_dwmmc_bind(struct udevice *dev)
{
	struct hi6220_dwmmc_plat *plat = dev_get_plat(dev);
	int ret;

	ret = dwmci_bind(dev, &plat->mmc, &plat->cfg);
	if (ret)
		return ret;

	return 0;
}

static const struct hisi_mmc_data hi3660_mmc_data = {
	.clock = 3200000,
	.use_fifo = true,
};

static const struct hisi_mmc_data hi6220_mmc_data = {
	.clock = 50000000,
	.use_fifo = false,
};

static const struct hisi_mmc_data hi3798mv2x_mmc_data = {
	.clock = 50000000,
	.use_fifo = false,
	.fifo_depth = 256,
};

static const struct udevice_id hi6220_dwmmc_ids[] = {
	{ .compatible = "hisilicon,hi6220-dw-mshc",
	  .data = (ulong)&hi6220_mmc_data },
	{ .compatible = "hisilicon,hi3798cv200-dw-mshc",
	  .data = (ulong)&hi6220_mmc_data },
	{ .compatible = "hisilicon,hi3798mv200-dw-mshc",
	  .data = (ulong)&hi3798mv2x_mmc_data },
	{ .compatible = "hisilicon,hi3660-dw-mshc",
	  .data = (ulong)&hi3660_mmc_data },
	{ }
};

U_BOOT_DRIVER(hi6220_dwmmc_drv) = {
	.name = "hi6220_dwmmc",
	.id = UCLASS_MMC,
	.of_match = hi6220_dwmmc_ids,
	.of_to_plat = hi6220_dwmmc_of_to_plat,
	.ops = &dm_dwmci_ops,
	.bind = hi6220_dwmmc_bind,
	.probe = hi6220_dwmmc_probe,
	.priv_auto	= sizeof(struct hi6220_dwmmc_priv_data),
	.plat_auto	= sizeof(struct hi6220_dwmmc_plat),
};
