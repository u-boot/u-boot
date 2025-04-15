// SPDX-License-Identifier: GPL-2.0
/*
 * Clock drivers for Qualcomm ipq9574
 *
 * (C) Copyright 2025 Linaro Ltd.
 */

#include <linux/types.h>
#include <clk-uclass.h>
#include <dm.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/bug.h>
#include <linux/bitops.h>
#include <dt-bindings/clock/qcom,ipq9574-gcc.h>
#include <dt-bindings/reset/qcom,ipq9574-gcc.h>

#include "clock-qcom.h"

#define GCC_BLSP1_AHB_CBCR			0x1004
#define GCC_BLSP1_UART3_APPS_CMD_RCGR		0x402C
#define GCC_BLSP1_UART3_APPS_CBCR		0x4054

#define GCC_SDCC1_APPS_CBCR			0x3302C
#define GCC_SDCC1_AHB_CBCR			0x33034
#define GCC_SDCC1_APPS_CMD_RCGR			0x33004
#define GCC_SDCC1_ICE_CORE_CBCR			0x33030

static ulong ipq9574_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case GCC_BLSP1_UART3_APPS_CLK:
		clk_rcg_set_rate_mnd(priv->base, GCC_BLSP1_UART3_APPS_CMD_RCGR,
				     0, 144, 15625, CFG_CLK_SRC_GPLL0, 16);
		return rate;
	case GCC_SDCC1_APPS_CLK:
		clk_rcg_set_rate_mnd(priv->base, GCC_SDCC1_APPS_CMD_RCGR,
				     11, 0, 0, CFG_CLK_SRC_GPLL2, 16);
		return rate;
	default:
		return -EINVAL;
	}
}

static const struct gate_clk ipq9574_clks[] = {
	GATE_CLK(GCC_BLSP1_UART3_APPS_CLK,	 0x4054, 0x00000001),
	GATE_CLK(GCC_BLSP1_AHB_CLK,		 0x1004, 0x00000001),
	GATE_CLK(GCC_SDCC1_AHB_CLK,		0x33034, 0x00000001),
	GATE_CLK(GCC_SDCC1_APPS_CLK,		0x3302C, 0x00000001),
	GATE_CLK(GCC_SDCC1_ICE_CORE_CLK,	0x33030, 0x00000001),
};

static int ipq9574_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	debug("%s: clk %s\n", __func__, ipq9574_clks[clk->id].name);

	if (!ipq9574_clks[clk->id].reg)
		return -EINVAL;

	qcom_gate_clk_en(priv, clk->id);

	return 0;
}

static const struct qcom_reset_map ipq9574_gcc_resets[] = {
	[GCC_SDCC_BCR] = { 0x33000 },
};

static struct msm_clk_data ipq9574_gcc_data = {
	.resets = ipq9574_gcc_resets,
	.num_resets = ARRAY_SIZE(ipq9574_gcc_resets),
	.enable = ipq9574_enable,
	.set_rate = ipq9574_set_rate,
};

static const struct udevice_id gcc_ipq9574_of_match[] = {
	{
		.compatible = "qcom,ipq9574-gcc",
		.data = (ulong)&ipq9574_gcc_data,
	},
	{ }
};

U_BOOT_DRIVER(gcc_ipq9574) = {
	.name		= "gcc_ipq9574",
	.id		= UCLASS_NOP,
	.of_match	= gcc_ipq9574_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
