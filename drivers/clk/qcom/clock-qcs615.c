// SPDX-License-Identifier: GPL-2.0
/*
 * Clock drivers for Qualcomm qcs615
 *
 * (C) Copyright 2024 Linaro Ltd.
 */

#include <linux/types.h>
#include <clk-uclass.h>
#include <dm.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/bug.h>
#include <linux/bitops.h>
#include <dt-bindings/clock/qcom,qcs615-gcc.h>
#include "clock-qcom.h"

#define USB30_PRIM_MOCK_UTMI_CLK_CMD_RCGR	0xf034
#define USB30_PRIM_MASTER_CLK_CMD_RCGR		0xf01c
#define USB3_PRIM_PHY_AUX_CMD_RCGR		0xf060

#define GCC_QUPV3_WRAP0_S0_CLK_ENA_BIT BIT(10)
#define GCC_QUPV3_WRAP0_S1_CLK_ENA_BIT BIT(11)
#define GCC_QUPV3_WRAP0_S2_CLK_ENA_BIT BIT(12)
#define GCC_QUPV3_WRAP0_S3_CLK_ENA_BIT BIT(13)
#define GCC_QUPV3_WRAP0_S4_CLK_ENA_BIT BIT(14)
#define GCC_QUPV3_WRAP0_S5_CLK_ENA_BIT BIT(15)

#define GCC_QUPV3_WRAP1_S0_CLK_ENA_BIT BIT(22)
#define GCC_QUPV3_WRAP1_S1_CLK_ENA_BIT BIT(23)
#define GCC_QUPV3_WRAP1_S2_CLK_ENA_BIT BIT(24)
#define GCC_QUPV3_WRAP1_S3_CLK_ENA_BIT BIT(25)
#define GCC_QUPV3_WRAP1_S4_CLK_ENA_BIT BIT(26)
#define GCC_QUPV3_WRAP1_S5_CLK_ENA_BIT BIT(27)

static ulong qcs615_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	if (clk->id < priv->data->num_clks)
		debug("%s: %s, requested rate=%ld\n", __func__,
		      priv->data->clks[clk->id].name, rate);

	switch (clk->id) {
	case GCC_USB30_PRIM_MOCK_UTMI_CLK:
		WARN(rate != 19200000, "Unexpected rate for USB30_PRIM_MOCK_UTMI_CLK: %lu\n", rate);
		clk_rcg_set_rate(priv->base, USB30_PRIM_MOCK_UTMI_CLK_CMD_RCGR, 0, CFG_CLK_SRC_CXO);
		return rate;
	case GCC_USB30_PRIM_MASTER_CLK:
		WARN(rate != 200000000, "Unexpected rate for USB30_PRIM_MASTER_CLK: %lu\n", rate);
		clk_rcg_set_rate_mnd(priv->base, USB30_PRIM_MASTER_CLK_CMD_RCGR,
				     5, 0, 0, CFG_CLK_SRC_GPLL0, 8);
		clk_rcg_set_rate(priv->base, USB3_PRIM_PHY_AUX_CMD_RCGR, 0, 0);
		return rate;
	default:
		return 0;
	}
}

static const struct gate_clk qcs615_clks[] = {
	GATE_CLK(GCC_CFG_NOC_USB3_PRIM_AXI_CLK, 0xf078, BIT(0)),
	GATE_CLK(GCC_USB30_PRIM_MASTER_CLK, 0xf010, BIT(0)),
	GATE_CLK(GCC_AGGRE_USB3_PRIM_AXI_CLK, 0xf07c, BIT(0)),
	GATE_CLK(GCC_USB30_PRIM_SLEEP_CLK, 0xf014, BIT(0)),
	GATE_CLK(GCC_USB30_PRIM_MOCK_UTMI_CLK, 0xf018, BIT(0)),
	GATE_CLK(GCC_USB3_PRIM_PHY_AUX_CLK, 0xf050, BIT(0)),
	GATE_CLK(GCC_USB3_PRIM_PHY_COM_AUX_CLK, 0xf054, BIT(0)),
	GATE_CLK(GCC_USB3_PRIM_PHY_PIPE_CLK, 0xf058, BIT(0)),
	GATE_CLK(GCC_QUPV3_WRAP0_S0_CLK, 0x5200c, GCC_QUPV3_WRAP0_S0_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP0_S1_CLK, 0x5200c, GCC_QUPV3_WRAP0_S1_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP0_S2_CLK, 0x5200c, GCC_QUPV3_WRAP0_S2_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP0_S3_CLK, 0x5200c, GCC_QUPV3_WRAP0_S3_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP0_S4_CLK, 0x5200c, GCC_QUPV3_WRAP0_S4_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP0_S5_CLK, 0x5200c, GCC_QUPV3_WRAP0_S5_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP1_S0_CLK, 0x5200c, GCC_QUPV3_WRAP1_S0_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP1_S1_CLK, 0x5200c, GCC_QUPV3_WRAP1_S1_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP1_S2_CLK, 0x5200c, GCC_QUPV3_WRAP1_S2_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP1_S3_CLK, 0x5200c, GCC_QUPV3_WRAP1_S3_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP1_S4_CLK, 0x5200c, GCC_QUPV3_WRAP1_S4_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP1_S5_CLK, 0x5200c, GCC_QUPV3_WRAP1_S5_CLK_ENA_BIT),
	GATE_CLK(GCC_DISP_HF_AXI_CLK, 0xb038, BIT(0)),
	GATE_CLK(GCC_DISP_AHB_CLK, 0xb032, BIT(0))
};

static int qcs615_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	if (priv->data->num_clks < clk->id) {
		debug("%s: unknown clk id %lu\n", __func__, clk->id);
		return 0;
	}

	debug("%s: clk %ld: %s\n", __func__, clk->id, qcs615_clks[clk->id].name);

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

static const struct qcom_reset_map qcs615_gcc_resets[] = {
	[GCC_EMAC_BCR] = { 0x6000 },
	[GCC_QUSB2PHY_PRIM_BCR] = { 0xd000 },
	[GCC_QUSB2PHY_SEC_BCR] = { 0xd004 },
	[GCC_USB30_PRIM_BCR] = { 0xf000 },
	[GCC_USB2_PHY_SEC_BCR] = { 0x50018 },
	[GCC_USB3_DP_PHY_SEC_BCR] = { 0x50020 },
	[GCC_USB3PHY_PHY_SEC_BCR] = { 0x5001c },
	[GCC_PCIE_0_BCR] = { 0x6b000 },
	[GCC_PCIE_0_PHY_BCR] = { 0x6c01c },
	[GCC_PCIE_PHY_BCR] = { 0x6f000 },
	[GCC_PCIE_PHY_COM_BCR] = { 0x6f010 },
	[GCC_UFS_PHY_BCR] = { 0x77000 },
	[GCC_USB20_SEC_BCR] = { 0xa6000 },
	[GCC_USB3PHY_PHY_PRIM_SP0_BCR] = { 0x50008 },
	[GCC_USB3_PHY_PRIM_SP0_BCR] = { 0x50000 },
	[GCC_SDCC1_BCR] = { 0x12000 },
	[GCC_SDCC2_BCR] = { 0x14000 }
};

static const struct qcom_power_map qcs615_gdscs[] = {
	[UFS_PHY_GDSC] = { 0x77004 },
	[USB30_PRIM_GDSC] = { 0xf004 },
};

static struct msm_clk_data sa8775_gcc_data = {
	.resets = qcs615_gcc_resets,
	.num_resets = ARRAY_SIZE(qcs615_gcc_resets),
	.clks = qcs615_clks,
	.num_clks = ARRAY_SIZE(qcs615_clks),

	.power_domains = qcs615_gdscs,
	.num_power_domains = ARRAY_SIZE(qcs615_gdscs),

	.enable = qcs615_enable,
	.set_rate = qcs615_set_rate,
};

static const struct udevice_id gcc_qcs615_of_match[] = {
	{
		.compatible = "qcom,qcs615-gcc",
		.data = (ulong)&sa8775_gcc_data,
	},
	{ }
};

U_BOOT_DRIVER(gcc_qcs615) = {
	.name		= "gcc_qcs615",
	.id		= UCLASS_NOP,
	.of_match	= gcc_qcs615_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
