// SPDX-License-Identifier: GPL-2.0-only
/*
 * RPMh clock driver for Qualcomm SoCs
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <dt-bindings/clock/qcom,rpmh.h>

/* On-board TCXO */
#define TCXO_RATE	38400000

/* bi_tcxo_div2 divided after RPMh output */
#define TCXO_DIV2_RATE	(TCXO_RATE / 2)

static ulong qcom_rpmh_clk_set_rate(struct clk *clk, ulong rate)
{
	return (clk->rate = rate);
}

static ulong qcom_rpmh_clk_get_rate(struct clk *clk)
{
	ulong cxo_rate = dev_get_driver_data(clk->dev);

	switch (clk->id) {
	case RPMH_CXO_CLK:
		return cxo_rate;
	default:
		return clk->rate;
	}
}

static int qcom_rpmh_clk_nop(struct clk *clk)
{
	return 0;
}

static struct clk_ops qcom_rpmh_clk_ops = {
	.set_rate = qcom_rpmh_clk_set_rate,
	.get_rate = qcom_rpmh_clk_get_rate,
	.enable = qcom_rpmh_clk_nop,
	.disable = qcom_rpmh_clk_nop,
};

static const struct udevice_id qcom_rpmh_clk_ids[] = {
	{ .compatible = "qcom,sa8775p-rpmh-clk", .data = TCXO_DIV2_RATE },
	{ .compatible = "qcom,sm8550-rpmh-clk", .data = TCXO_DIV2_RATE },
	{ .compatible = "qcom,sm8650-rpmh-clk", .data = TCXO_DIV2_RATE },
	{ }
};

U_BOOT_DRIVER(qcom_rpmh_clk) = {
	.name		= "qcom_rpmh_clk",
	.id		= UCLASS_CLK,
	.of_match	= qcom_rpmh_clk_ids,
	.ops		= &qcom_rpmh_clk_ops,
	.flags		= DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
