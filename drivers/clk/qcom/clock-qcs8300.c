// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2024-2025, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 */

#include <linux/types.h>
#include <clk-uclass.h>
#include <dm.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/bug.h>
#include <linux/bitops.h>
#include <dt-bindings/clock/qcom,qcs8300-gcc.h>
#include "clock-qcom.h"

#define USB30_PRIM_MOCK_UTMI_CLK_CMD_RCGR 0xf038
#define USB30_PRIM_MASTER_CLK_CMD_RCGR 0xf020

static ulong qcs8300_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	if (clk->id < priv->data->num_clks)
		debug("%s: %s, requested rate=%ld\n",
		      __func__, priv->data->clks[clk->id].name, rate);

	switch (clk->id) {
	case GCC_USB30_PRIM_MOCK_UTMI_CLK:
		WARN(rate != 19200000, "Unexpected rate for USB30_PRIM_MOCK_UTMI_CLK: %lu\n", rate);
		clk_rcg_set_rate(priv->base, USB30_PRIM_MOCK_UTMI_CLK_CMD_RCGR, 0, CFG_CLK_SRC_CXO);
		return rate;
	case GCC_USB30_PRIM_MASTER_CLK:
		WARN(rate != 200000000, "Unexpected rate for USB30_PRIM_MASTER_CLK: %lu\n", rate);
		clk_rcg_set_rate_mnd(priv->base, USB30_PRIM_MASTER_CLK_CMD_RCGR,
				     1, 0, 0, CFG_CLK_SRC_GPLL0_ODD, 8);
		clk_rcg_set_rate(priv->base, 0xf064, 0, 0);
		return rate;
	default:
		return 0;
	}
}

static const struct gate_clk qcs8300_clks[] = {
	GATE_CLK(GCC_CFG_NOC_USB3_PRIM_AXI_CLK, 0x1b088, BIT(0)),
	GATE_CLK(GCC_USB30_PRIM_MASTER_CLK, 0x1b018, BIT(0)),
	GATE_CLK(GCC_AGGRE_USB3_PRIM_AXI_CLK, 0x1b084, BIT(0)),
	GATE_CLK(GCC_USB30_PRIM_SLEEP_CLK, 0x1b020, BIT(0)),
	GATE_CLK(GCC_USB30_PRIM_MOCK_UTMI_CLK, 0x1b024, BIT(0)),
	GATE_CLK(GCC_USB3_PRIM_PHY_AUX_CLK, 0x1b05c, BIT(0)),
	GATE_CLK(GCC_USB3_PRIM_PHY_COM_AUX_CLK, 0x1b060, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_AHB_CLK, 0x83020, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_AXI_CLK, 0x83018, BIT(0)),
	GATE_CLK(GCC_AGGRE_UFS_PHY_AXI_CLK, 0x830d4, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_UNIPRO_CORE_CLK, 0x83064, BIT(0)),
};

static int qcs8300_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	if (priv->data->num_clks < clk->id) {
		debug("%s: unknown clk id %lu\n", __func__, clk->id);
		return 0;
	}

	debug("%s: clk %ld: %s\n", __func__, clk->id, qcs8300_clks[clk->id].name);

	switch (clk->id) {
	case GCC_AGGRE_USB3_PRIM_AXI_CLK:
		qcom_gate_clk_en(priv, GCC_USB30_PRIM_MASTER_CLK);
		fallthrough;
	case GCC_USB30_PRIM_MASTER_CLK:
		qcom_gate_clk_en(priv, GCC_USB3_PRIM_PHY_AUX_CLK);
		qcom_gate_clk_en(priv, GCC_USB3_PRIM_PHY_COM_AUX_CLK);
		break;
	}

	qcom_gate_clk_en(priv, clk->id);

	return 0;
}

static const struct qcom_reset_map qcs8300_gcc_resets[] = {
	[GCC_EMAC0_BCR] = { 0xb6000 },
	[GCC_PCIE_0_BCR] = { 0xa9000 },
	[GCC_PCIE_0_LINK_DOWN_BCR] = { 0xbf000 },
	[GCC_PCIE_0_NOCSR_COM_PHY_BCR] = { 0xbf008 },
	[GCC_PCIE_0_PHY_BCR] = { 0xa9144 },
	[GCC_PCIE_0_PHY_NOCSR_COM_PHY_BCR] = { 0xbf00c },
	[GCC_PCIE_1_BCR] = { 0x77000 },
	[GCC_PCIE_1_LINK_DOWN_BCR] = { 0xae084 },
	[GCC_PCIE_1_NOCSR_COM_PHY_BCR] = { 0xae090 },
	[GCC_PCIE_1_PHY_BCR] = { 0xae08c },
	[GCC_PCIE_1_PHY_NOCSR_COM_PHY_BCR] = { 0xae094 },
	[GCC_SDCC1_BCR] = { 0x20000 },
	[GCC_UFS_PHY_BCR] = { 0x83000 },
	[GCC_USB20_PRIM_BCR] = { 0x1c000 },
	[GCC_USB2_PHY_PRIM_BCR] = { 0x5c01c },
	[GCC_USB2_PHY_SEC_BCR] = { 0x5c020 },
	[GCC_USB30_PRIM_BCR] = { 0x1b000 },
	[GCC_USB3_DP_PHY_PRIM_BCR] = { 0x5c008 },
	[GCC_USB3_PHY_PRIM_BCR] = { 0x5c000 },
	[GCC_USB3_PHY_TERT_BCR] = { 0x5c024 },
	[GCC_USB3_UNIPHY_MP0_BCR] = { 0x5c00c },
	[GCC_USB3_UNIPHY_MP1_BCR] = { 0x5c010 },
	[GCC_USB3PHY_PHY_PRIM_BCR] = { 0x5c004 },
	[GCC_USB3UNIPHY_PHY_MP0_BCR] = { 0x5c014 },
	[GCC_USB3UNIPHY_PHY_MP1_BCR] = { 0x5c018 },
	[GCC_USB_PHY_CFG_AHB2PHY_BCR] = { 0x76000 },
	[GCC_VIDEO_BCR] = { 0x34000 },
};

static const struct qcom_power_map qcs8300_gdscs[] = {
	[GCC_UFS_PHY_GDSC] = { 0x83004 },
	[GCC_USB30_PRIM_GDSC] = { 0x1B004 },
};

static struct msm_clk_data qcs8300_gcc_data = {
	.resets = qcs8300_gcc_resets,
	.num_resets = ARRAY_SIZE(qcs8300_gcc_resets),
	.clks = qcs8300_clks,
	.num_clks = ARRAY_SIZE(qcs8300_clks),

	.power_domains = qcs8300_gdscs,
	.num_power_domains = ARRAY_SIZE(qcs8300_gdscs),

	.enable = qcs8300_enable,
	.set_rate = qcs8300_set_rate,
};

static const struct udevice_id gcc_qcs8300_of_match[] = {
	{
		.compatible = "qcom,qcs8300-gcc",
		.data = (ulong)&qcs8300_gcc_data,
	},
	{ }
};

U_BOOT_DRIVER(gcc_qcs8300) = {
	.name		= "gcc_qcs8300",
	.id		= UCLASS_NOP,
	.of_match	= gcc_qcs8300_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
