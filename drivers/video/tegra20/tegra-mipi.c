// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 NVIDIA Corporation
 * Copyright (c) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <clk.h>
#include <misc.h>
#include <linux/delay.h>
#include <linux/iopoll.h>

#include <asm/io.h>

/* MIPI control registers 0x00 ~ 0x60 */
struct mipi_ctlr {
	uint mipi_cal_ctrl;
	uint mipi_cal_autocal_ctrl;
	uint mipi_cal_status;

	uint unused1[2];

	uint mipi_cal_config_csia;
	uint mipi_cal_config_csib;
	uint mipi_cal_config_csic;
	uint mipi_cal_config_csid;
	uint mipi_cal_config_csie;

	uint unused2[4];

	uint mipi_cal_config_dsia;
	uint mipi_cal_config_dsib;
	uint mipi_cal_config_dsic;
	uint mipi_cal_config_dsid;

	uint unused3[4];

	uint mipi_cal_bias_pad_cfg0;
	uint mipi_cal_bias_pad_cfg1;
	uint mipi_cal_bias_pad_cfg2;
};

#define MIPI_CAL_CTRL_NOISE_FILTER(x)	(((x) & 0xf) << 26)
#define MIPI_CAL_CTRL_PRESCALE(x)	(((x) & 0x3) << 24)
#define MIPI_CAL_CTRL_CLKEN_OVR		BIT(4)
#define MIPI_CAL_CTRL_START		BIT(0)

#define MIPI_CAL_STATUS_DONE		BIT(16)
#define MIPI_CAL_STATUS_ACTIVE		BIT(0)

#define MIPI_CAL_OVERIDE(x)		(((x) & 0x1) << 30)
#define MIPI_CAL_SEL(x)			(((x) & 0x1) << 21)
#define MIPI_CAL_HSPDOS(x)		(((x) & 0x1f) << 16)
#define MIPI_CAL_HSPUOS(x)		(((x) & 0x1f) << 8)
#define MIPI_CAL_TERMOS(x)		(((x) & 0x1f) << 0)

#define MIPI_CAL_BIAS_PAD_PDVCLAMP	BIT(1)
#define MIPI_CAL_BIAS_PAD_E_VCLAMP_REF	BIT(0)

#define MIPI_CAL_BIAS_PAD_DRV_DN_REF(x) (((x) & 0x7) << 16)
#define MIPI_CAL_BIAS_PAD_DRV_UP_REF(x) (((x) & 0x7) << 8)

#define MIPI_CAL_BIAS_PAD_VCLAMP(x)	(((x) & 0x7) << 16)
#define MIPI_CAL_BIAS_PAD_VAUXP(x)	(((x) & 0x7) << 4)
#define MIPI_CAL_BIAS_PAD_PDVREG	BIT(1)

struct tegra_mipi_priv {
	struct mipi_ctlr	*mipi;
	struct clk		*mipi_cal;
};

static int tegra_mipi_calibrate(struct udevice *dev, int offset, const void *buf,
				int size)
{
	struct tegra_mipi_priv *priv = dev_get_priv(dev);
	u32 value;

	value = MIPI_CAL_BIAS_PAD_DRV_DN_REF(0x2) |
		MIPI_CAL_BIAS_PAD_DRV_UP_REF(0x0);
	writel(value, &priv->mipi->mipi_cal_bias_pad_cfg1);

	value = readl(&priv->mipi->mipi_cal_bias_pad_cfg2);
	value &= ~MIPI_CAL_BIAS_PAD_VCLAMP(0x7);
	value &= ~MIPI_CAL_BIAS_PAD_VAUXP(0x7);
	writel(value, &priv->mipi->mipi_cal_bias_pad_cfg2);

	value = MIPI_CAL_OVERIDE(0x0) | MIPI_CAL_SEL(0x1) |
		MIPI_CAL_HSPDOS(0x0) | MIPI_CAL_HSPUOS(0x4) |
		MIPI_CAL_TERMOS(0x5);
	writel(value, &priv->mipi->mipi_cal_config_dsia);
	writel(value, &priv->mipi->mipi_cal_config_dsib);

	/* Deselect PAD C */
	value = readl(&priv->mipi->mipi_cal_config_dsic);
	value &= ~(MIPI_CAL_SEL(0x1));
	writel(value, &priv->mipi->mipi_cal_config_dsic);

	/* Deselect PAD D */
	value = readl(&priv->mipi->mipi_cal_config_dsid);
	value &= ~(MIPI_CAL_SEL(0x1));
	writel(value, &priv->mipi->mipi_cal_config_dsid);

	value = readl(&priv->mipi->mipi_cal_ctrl);
	value &= ~MIPI_CAL_CTRL_NOISE_FILTER(0xf);
	value &= ~MIPI_CAL_CTRL_PRESCALE(0x3);
	value |= MIPI_CAL_CTRL_NOISE_FILTER(0xa) |
		 MIPI_CAL_CTRL_PRESCALE(0x2) |
		 MIPI_CAL_CTRL_CLKEN_OVR;
	writel(value, &priv->mipi->mipi_cal_ctrl);

	/* clear any pending status bits */
	value = readl(&priv->mipi->mipi_cal_status);
	writel(value, &priv->mipi->mipi_cal_status);

	value = readl(&priv->mipi->mipi_cal_ctrl);
	value |= MIPI_CAL_CTRL_START;
	writel(value, &priv->mipi->mipi_cal_ctrl);

	/*
	 * Wait for min 72uS to let calibration logic finish calibration
	 * sequence codes before waiting for pads idle state to apply the
	 * results.
	 */
	udelay(80);

	return readl_poll_sleep_timeout(&priv->mipi->mipi_cal_status, value,
					!(value & MIPI_CAL_STATUS_ACTIVE) &&
					(value & MIPI_CAL_STATUS_DONE), 100,
					250000);
}

static int tegra_mipi_enable(struct udevice *dev, bool val)
{
	struct tegra_mipi_priv *priv = dev_get_priv(dev);
	u32 value;

	clk_enable(priv->mipi_cal);

	value = readl(&priv->mipi->mipi_cal_bias_pad_cfg0);
	value &= ~MIPI_CAL_BIAS_PAD_PDVCLAMP;
	value |= MIPI_CAL_BIAS_PAD_E_VCLAMP_REF;
	writel(value, &priv->mipi->mipi_cal_bias_pad_cfg0);

	value = readl(&priv->mipi->mipi_cal_bias_pad_cfg2);
	value &= ~MIPI_CAL_BIAS_PAD_PDVREG;
	writel(value, &priv->mipi->mipi_cal_bias_pad_cfg2);

	return 0;
}

static const struct misc_ops tegra_mipi_ops = {
	.write = tegra_mipi_calibrate,
	.set_enabled = tegra_mipi_enable,
};

static int tegra_mipi_probe(struct udevice *dev)
{
	struct tegra_mipi_priv *priv = dev_get_priv(dev);

	priv->mipi = (struct mipi_ctlr *)dev_read_addr_ptr(dev);
	if (!priv->mipi) {
		log_debug("%s: no MIPI controller address\n", __func__);
		return -EINVAL;
	}

	priv->mipi_cal = devm_clk_get(dev, NULL);
	if (IS_ERR(priv->mipi_cal)) {
		log_debug("%s: Could not get MIPI clock: %ld\n",
			  __func__, PTR_ERR(priv->mipi_cal));
		return PTR_ERR(priv->mipi_cal);
	}

	return 0;
}

static const struct udevice_id tegra_mipi_ids[] = {
	{ .compatible = "nvidia,tegra114-mipi" },
	{ }
};

U_BOOT_DRIVER(tegra_mipi) = {
	.name           = "tegra_mipi",
	.id             = UCLASS_MISC,
	.ops		= &tegra_mipi_ops,
	.of_match       = tegra_mipi_ids,
	.probe          = tegra_mipi_probe,
	.priv_auto	= sizeof(struct tegra_mipi_priv),
};
