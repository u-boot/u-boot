// SPDX-License-Identifier: GPL-2.0
/*
 * Clock drivers for Qualcomm ipq5424
 *
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <clk-uclass.h>
#include <dm.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/bug.h>
#include <linux/bitops.h>
#include <dt-bindings/clock/qcom,ipq5424-gcc.h>
#include <dt-bindings/reset/qcom,ipq5424-gcc.h>

#include "clock-qcom.h"

#define GCC_IM_SLEEP_CBCR	0x1834020u

static ulong ipq5424_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case GCC_QUPV3_UART1_CLK:
		clk_rcg_set_rate_mnd(priv->base, priv->data->clks[clk->id].reg,
				     0, 144, 15625, CFG_CLK_SRC_GPLL0, 16);
		return rate;
	case GCC_SDCC1_APPS_CLK:
		clk_rcg_set_rate_mnd(priv->base, priv->data->clks[clk->id].reg,
				     5, 0, 0, CFG_CLK_SRC_GPLL2_MAIN, 16);
		return rate;
	}
	return 0;
}

static const struct gate_clk ipq5424_clks[] = {
	GATE_CLK(GCC_QUPV3_UART1_CLK, 0x302c, BIT(0)),
	GATE_CLK(GCC_SDCC1_AHB_CLK, 0x3303c, BIT(0)),
	GATE_CLK(GCC_SDCC1_APPS_CLK, 0x33004, BIT(1)),
	GATE_CLK(GCC_IM_SLEEP_CLK, 0x34020, BIT(0)),
};

static int ipq5424_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	if (clk->id >= ARRAY_SIZE(ipq5424_clks) || !ipq5424_clks[clk->id].reg)
		return -EINVAL;

	qcom_gate_clk_en(priv, clk->id);

	return 0;
}

static const struct qcom_reset_map ipq5424_gcc_resets[] = {
	[GCC_SDCC_BCR] = { 0x33000 },
};

static struct msm_clk_data ipq5424_gcc_data = {
	.resets = ipq5424_gcc_resets,
	.num_resets = ARRAY_SIZE(ipq5424_gcc_resets),
	.clks = ipq5424_clks,
	.num_clks = ARRAY_SIZE(ipq5424_clks),

	.enable = ipq5424_enable,
	.set_rate = ipq5424_set_rate,
};

static const struct udevice_id gcc_ipq5424_of_match[] = {
	{
		.compatible = "qcom,ipq5424-gcc",
		.data = (ulong)&ipq5424_gcc_data,
	},
	{ }
};

static int ipq5424_clk_probe(struct udevice *dev)
{
	/* Enable the sleep clock needed for the MMC block reset */
	writel(BIT(0), GCC_IM_SLEEP_CBCR);

	return 0;
}

U_BOOT_DRIVER(gcc_ipq5424) = {
	.name		= "gcc_ipq5424",
	.id		= UCLASS_NOP,
	.of_match	= gcc_ipq5424_of_match,
	.probe		= ipq5424_clk_probe,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
