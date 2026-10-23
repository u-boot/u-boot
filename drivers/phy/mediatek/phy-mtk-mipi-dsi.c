// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2015 MediaTek Inc.
 * Copyright (c) 2026 BayLibre, SAS
 */

#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <generic-phy.h>
#include <linux/kernel.h>
#include <phy-mipi-dphy.h>

#include "phy-mtk-mipi-dsi.h"

#define MIPITX_DRIVE_DEFAULT_UA		4600
#define MIPITX_DRIVE_MIN_UA		3000
#define MIPITX_DRIVE_MAX_UA		6000

static int mtk_mipi_tx_power_on(struct phy *phy)
{
	struct mtk_mipi_tx *mipi_tx = dev_get_priv(phy->dev);
	int ret;

	if (!mipi_tx->data_rate) {
		dev_err(phy->dev, "No data rate set, configure the PHY first\n");
		return -EINVAL;
	}

	ret = clk_enable(&mipi_tx->ref_clk);
	if (ret)
		return ret;

	/* Power up core and enable PLL */
	ret = mipi_tx->driver_data->pll_enable(mipi_tx);
	if (ret) {
		clk_disable(&mipi_tx->ref_clk);
		return ret;
	}

	/* Enable DSI Lane LDO outputs, disable pad tie low */
	mipi_tx->driver_data->mipi_tx_enable_signal(mipi_tx);

	return 0;
}

static int mtk_mipi_tx_power_off(struct phy *phy)
{
	struct mtk_mipi_tx *mipi_tx = dev_get_priv(phy->dev);

	/* Enable pad tie low, disable DSI Lane LDO outputs */
	mipi_tx->driver_data->mipi_tx_disable_signal(mipi_tx);

	/* Disable PLL and power down core */
	mipi_tx->driver_data->pll_disable(mipi_tx);

	clk_disable(&mipi_tx->ref_clk);

	return 0;
}

/*
 * Equivalent of the kernel PLL clock's determine_rate and set_rate ops: clamp
 * the requested data rate to what the PLL can lock to and latch it, the
 * hardware is programmed when the PHY is powered on.
 */
static int mtk_mipi_tx_configure(struct phy *phy, void *params)
{
	struct mtk_mipi_tx *mipi_tx = dev_get_priv(phy->dev);
	struct phy_configure_opts_mipi_dphy *opts = params;

	if (!opts)
		return -EINVAL;

	mipi_tx->data_rate =
		mipi_tx->driver_data->pll_determine_rate(mipi_tx,
							 opts->hs_clk_rate);

	return 0;
}

static int mtk_mipi_tx_probe(struct udevice *dev)
{
	struct mtk_mipi_tx *mipi_tx = dev_get_priv(dev);
	int ret;

	mipi_tx->dev = dev;
	mipi_tx->driver_data =
		(const struct mtk_mipitx_data *)dev_get_driver_data(dev);
	if (!mipi_tx->driver_data)
		return -ENODEV;

	mipi_tx->regs = dev_remap_addr(dev);
	if (!mipi_tx->regs)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &mipi_tx->ref_clk);
	if (ret) {
		dev_err(dev, "Failed to get reference clock: %d\n", ret);
		return ret;
	}

	mipi_tx->mipitx_drive = dev_read_u32_default(dev, "drive-strength-microamp",
						     MIPITX_DRIVE_DEFAULT_UA);
	if (mipi_tx->mipitx_drive > MIPITX_DRIVE_MAX_UA ||
	    mipi_tx->mipitx_drive < MIPITX_DRIVE_MIN_UA) {
		dev_warn(dev, "drive-strength-microamp is invalid %u, not in %u ~ %u\n",
			 mipi_tx->mipitx_drive, MIPITX_DRIVE_MIN_UA,
			 MIPITX_DRIVE_MAX_UA);
		mipi_tx->mipitx_drive = clamp_val(mipi_tx->mipitx_drive,
						  MIPITX_DRIVE_MIN_UA,
						  MIPITX_DRIVE_MAX_UA);
	}

	/*
	 * The kernel reads the per-lane impedance trim codes from the
	 * "calibration-data" nvmem cell, falling back to a mid-scale default
	 * when unavailable. This driver does not support this yet, so
	 * rt_code[] is left zeroed and the SoC layer applies the same default.
	 */

	return 0;
}

static const struct phy_ops mtk_mipi_tx_phy_ops = {
	.power_on	= mtk_mipi_tx_power_on,
	.power_off	= mtk_mipi_tx_power_off,
	.configure	= mtk_mipi_tx_configure,
};

static const struct udevice_id mtk_mipi_tx_ids[] = {
	{ .compatible = "mediatek,mt8183-mipi-tx",
	  .data = (ulong)&mt8183_mipitx_data,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mtk_mipi_tx) = {
	.name		= "mtk-mipi-tx",
	.id		= UCLASS_PHY,
	.of_match	= mtk_mipi_tx_ids,
	.ops		= &mtk_mipi_tx_phy_ops,
	.probe		= mtk_mipi_tx_probe,
	.priv_auto	= sizeof(struct mtk_mipi_tx),
};
