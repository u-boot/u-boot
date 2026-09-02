// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek display pulse-width-modulation controller driver.
 *
 * Copyright (c) 2015 MediaTek Inc.
 * Author: YH Huang <yh.huang@mediatek.com>
 *
 * Copyright (C) 2026 BayLibre, SAS
 */

#include <clk.h>
#include <dm.h>
#include <pwm.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/time.h>

#define DISP_PWM_EN		0x00

#define PWM_CLKDIV_MASK		GENMASK(25, 16)

#define PWM_PERIOD_BIT_WIDTH	12
#define PWM_PERIOD_MASK		GENMASK(PWM_PERIOD_BIT_WIDTH - 1, 0)

#define PWM_HIGH_WIDTH_MASK	GENMASK(28, 16)

struct mtk_disp_pwm_data {
	u32 enable_mask;
	u32 con0;
	u32 con0_sel;
	u32 con1;
	bool has_commit;
	u32 commit;
	u32 commit_mask;
	u32 bls_debug;
	u32 bls_debug_mask;
};

struct mtk_disp_pwm_priv {
	void __iomem *base;
	struct clk clk_main;
	struct clk clk_mm;
	const struct mtk_disp_pwm_data *data;
};

static int mtk_disp_pwm_set_config(struct udevice *dev, uint channel,
				   uint period_ns, uint duty_ns)
{
	struct mtk_disp_pwm_priv *priv = dev_get_priv(dev);
	const struct mtk_disp_pwm_data *data = priv->data;
	u64 div, clk_div, period, high_width;
	ulong rate;

	if (channel)
		return -EINVAL;

	if (duty_ns > period_ns)
		return -EINVAL;

	rate = clk_get_rate(&priv->clk_main);
	if (IS_ERR_VALUE(rate) || !rate)
		return -EINVAL;

	/*
	 * period_ns = 10^9 * (clk_div + 1) * (period + 1) / rate
	 * duty_ns = 10^9 * (clk_div + 1) * high_width / rate
	 */
	clk_div = (((u64)period_ns * rate) / NSEC_PER_SEC) >> PWM_PERIOD_BIT_WIDTH;
	if (clk_div > FIELD_MAX(PWM_CLKDIV_MASK))
		return -EINVAL;

	div = NSEC_PER_SEC * (clk_div + 1);
	period = ((u64)period_ns * rate) / div;
	if (period > 0)
		period--;

	high_width = ((u64)duty_ns * rate) / div;

	clrsetbits_le32(priv->base + data->con0, PWM_CLKDIV_MASK,
			FIELD_PREP(PWM_CLKDIV_MASK, clk_div));
	clrsetbits_le32(priv->base + data->con1,
			PWM_PERIOD_MASK | PWM_HIGH_WIDTH_MASK,
			FIELD_PREP(PWM_PERIOD_MASK, period) |
			FIELD_PREP(PWM_HIGH_WIDTH_MASK, high_width));

	if (data->has_commit) {
		setbits_le32(priv->base + data->commit, data->commit_mask);
		clrbits_le32(priv->base + data->commit, data->commit_mask);
	}

	return 0;
}

static int mtk_disp_pwm_set_enable(struct udevice *dev, uint channel, bool enable)
{
	struct mtk_disp_pwm_priv *priv = dev_get_priv(dev);

	if (channel)
		return -EINVAL;

	clrsetbits_le32(priv->base + DISP_PWM_EN, priv->data->enable_mask,
			field_prep(priv->data->enable_mask, enable));

	return 0;
}

static int mtk_disp_pwm_probe(struct udevice *dev)
{
	struct mtk_disp_pwm_priv *priv = dev_get_priv(dev);
	const struct mtk_disp_pwm_data *data;
	int ret;

	data = (const void *)dev_get_driver_data(dev);
	priv->data = data;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base) {
		dev_err(dev, "failed to get base address\n");
		return -EINVAL;
	}

	ret = clk_get_by_name(dev, "main", &priv->clk_main);
	if (ret) {
		dev_err(dev, "failed to get main clock: %d\n", ret);
		return ret;
	}

	ret = clk_get_by_name(dev, "mm", &priv->clk_mm);
	if (ret) {
		dev_err(dev, "failed to get mm clock: %d\n", ret);
		return ret;
	}

	ret = clk_enable(&priv->clk_main);
	if (ret) {
		dev_err(dev, "failed to enable main clock: %d\n", ret);
		return ret;
	}

	ret = clk_enable(&priv->clk_mm);
	if (ret) {
		dev_err(dev, "failed to enable mm clock: %d\n", ret);
		clk_disable(&priv->clk_main);
		return ret;
	}

	if (data->bls_debug && !data->has_commit) {
		/* Disable double buffer and select manual mode */
		setbits_le32(priv->base + data->bls_debug, data->bls_debug_mask);
		setbits_le32(priv->base + data->con0, data->con0_sel);
	}

	return 0;
}

static const struct pwm_ops mtk_disp_pwm_ops = {
	.set_config = mtk_disp_pwm_set_config,
	.set_enable = mtk_disp_pwm_set_enable,
};

static const struct mtk_disp_pwm_data mt8183_pwm_data = {
	.enable_mask = BIT(0),
	.con0 = 0x18,
	.con0_sel = 0x0,
	.con1 = 0x1c,
	.has_commit = false,
	.bls_debug = 0x80,
	.bls_debug_mask = GENMASK(1, 0),
};

static const struct udevice_id mtk_disp_pwm_ids[] = {
	{ .compatible = "mediatek,mt8183-disp-pwm", .data = (ulong)&mt8183_pwm_data },
	{ }
};

U_BOOT_DRIVER(mtk_disp_pwm) = {
	.name		= "mtk_disp_pwm",
	.id		= UCLASS_PWM,
	.of_match	= mtk_disp_pwm_ids,
	.ops		= &mtk_disp_pwm_ops,
	.probe		= mtk_disp_pwm_probe,
	.priv_auto	= sizeof(struct mtk_disp_pwm_priv),
};
