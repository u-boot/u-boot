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

#include <asm/arch/clock.h>
#include <asm/io.h>

/* MIPI control registers 0x00 ~ 0x74 */
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

	uint mipi_cal_dsia_config_2;
	uint mipi_cal_dsib_config_2;
	uint mipi_cal_cilc_config_2;
	uint mipi_cal_cild_config_2;
	uint mipi_cal_csie_config_2;
};

#define MIPI_DSIA_PADS			0x60
#define MIPI_DSIB_PADS			0x180

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

#define MIPI_CAL_HSCLKPDOSDSI(x)	(((x) & 0x1f) << 8)
#define MIPI_CAL_HSCLKPUOSDSI(x)	(((x) & 0x1f) << 0)

struct tegra_mipi_priv {
	struct mipi_ctlr	*mipi;
	struct clk		*mipi_cal;
	u32 version;
};

enum {
	T114,
	T124,
};

static void tegra114_mipi_pads_cal(struct tegra_mipi_priv *priv,
				   int calibration_pads)
{
	u32 value;

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
}

static void tegra124_mipi_pads_cal(struct tegra_mipi_priv *priv,
				   int calibration_pads)
{
	u32 value;

	/* Calibrate DSI-A */
	if (calibration_pads == MIPI_DSIA_PADS) {
		printf("Calibrating DSI-A pads\n");

		value = MIPI_CAL_OVERIDE(0x0) | MIPI_CAL_SEL(0x1) |
			MIPI_CAL_HSPDOS(0x0) | MIPI_CAL_HSPUOS(0x0) |
			MIPI_CAL_TERMOS(0x0);
		writel(value, &priv->mipi->mipi_cal_config_dsia);
		writel(value, &priv->mipi->mipi_cal_config_dsib);

		value = MIPI_CAL_SEL(0x1) |
			MIPI_CAL_HSCLKPDOSDSI(0x1) |
			MIPI_CAL_HSCLKPUOSDSI(0x2);
		writel(value, &priv->mipi->mipi_cal_dsia_config_2);
		writel(value, &priv->mipi->mipi_cal_dsib_config_2);

		/* Deselect PAD C */
		value = readl(&priv->mipi->mipi_cal_cilc_config_2);
		value &= ~(MIPI_CAL_SEL(0x1));
		writel(value, &priv->mipi->mipi_cal_cilc_config_2);

		/* Deselect PAD D */
		value = readl(&priv->mipi->mipi_cal_cild_config_2);
		value &= ~(MIPI_CAL_SEL(0x1));
		writel(value, &priv->mipi->mipi_cal_cild_config_2);
	}

	/* Calibrate DSI-B */
	if (calibration_pads == MIPI_DSIB_PADS) {
		printf("Calibrating DSI-B pads\n");

		value = MIPI_CAL_OVERIDE(0x0) | MIPI_CAL_SEL(0x1) |
			MIPI_CAL_HSPDOS(0x0) | MIPI_CAL_HSPUOS(0x0) |
			MIPI_CAL_TERMOS(0x0);
		writel(value, &priv->mipi->mipi_cal_config_csic);
		writel(value, &priv->mipi->mipi_cal_config_csid);

		value = MIPI_CAL_SEL(0x1) |
			MIPI_CAL_HSCLKPDOSDSI(0x1) |
			MIPI_CAL_HSCLKPUOSDSI(0x2);
		writel(value, &priv->mipi->mipi_cal_cilc_config_2);
		writel(value, &priv->mipi->mipi_cal_cild_config_2);

		/* Deselect PAD A */
		value = readl(&priv->mipi->mipi_cal_dsia_config_2);
		value &= ~(MIPI_CAL_SEL(0x1));
		writel(value, &priv->mipi->mipi_cal_dsia_config_2);

		/* Deselect PAD B */
		value = readl(&priv->mipi->mipi_cal_dsib_config_2);
		value &= ~(MIPI_CAL_SEL(0x1));
		writel(value, &priv->mipi->mipi_cal_dsib_config_2);
	}
}

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

	switch (priv->version) {
	case T114:
		tegra114_mipi_pads_cal(priv, offset);
		break;

	case T124:
		tegra124_mipi_pads_cal(priv, offset);
		break;

	default:
		return -EINVAL;
	}

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

	reset_set_enable(priv->mipi_cal->id, 1);
	mdelay(100);
	reset_set_enable(priv->mipi_cal->id, 0);
	mdelay(1);

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

	priv->version = dev_get_driver_data(dev);

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
	{ .compatible = "nvidia,tegra114-mipi", .data = T114 },
	{ .compatible = "nvidia,tegra124-mipi", .data = T124 },
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
