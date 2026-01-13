// SPDX-License-Identifier: GPL-2.0
/*
 * Clock drivers for Qualcomm sa8775p
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
#include <dt-bindings/clock/qcom,sa8775p-gcc.h>
#include "clock-qcom.h"

#define USB30_PRIM_MOCK_UTMI_CLK_CMD_RCGR	0x1b040
#define USB30_PRIM_MASTER_CLK_CMD_RCGR		0x1b028
#define USB3_PRIM_PHY_AUX_CMD_RCGR		0x1b06c

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
#define GCC_QUPV3_WRAP1_S6_CLK_ENA_BIT BIT(27)

#define GCC_QUPV3_WRAP2_S0_CLK_ENA_BIT BIT(4)
#define GCC_QUPV3_WRAP2_S1_CLK_ENA_BIT BIT(5)
#define GCC_QUPV3_WRAP2_S2_CLK_ENA_BIT BIT(6)
#define GCC_QUPV3_WRAP2_S3_CLK_ENA_BIT BIT(7)
#define GCC_QUPV3_WRAP2_S4_CLK_ENA_BIT BIT(8)
#define GCC_QUPV3_WRAP2_S5_CLK_ENA_BIT BIT(9)
#define GCC_QUPV3_WRAP2_S6_CLK_ENA_BIT BIT(29)

#define GCC_QUPV3_WRAP3_S0_CLK_ENA_BIT BIT(25)

static ulong sa8775p_set_rate(struct clk *clk, ulong rate)
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

static const struct gate_clk sa8775p_clks[] = {
	GATE_CLK(GCC_CFG_NOC_USB3_PRIM_AXI_CLK, 0x1b088, 1),
	GATE_CLK(GCC_USB30_PRIM_MASTER_CLK, 0x1b018, 1),
	GATE_CLK(GCC_AGGRE_USB3_PRIM_AXI_CLK, 0x1b084, 1),
	GATE_CLK(GCC_USB30_PRIM_SLEEP_CLK, 0x1b020, 1),
	GATE_CLK(GCC_USB30_PRIM_MOCK_UTMI_CLK, 0x1b024, 1),
	GATE_CLK(GCC_USB3_PRIM_PHY_AUX_CLK, 0x1b05c, 1),
	GATE_CLK(GCC_USB3_PRIM_PHY_COM_AUX_CLK, 0x1b060, 1),
	GATE_CLK(GCC_USB3_PRIM_PHY_PIPE_CLK, 0x1b064, 1),

	/* QUP Wrapper 0 clocks */
	GATE_CLK(GCC_QUPV3_WRAP0_S0_CLK, 0x4b008, GCC_QUPV3_WRAP0_S0_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP0_S1_CLK, 0x4b008, GCC_QUPV3_WRAP0_S1_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP0_S2_CLK, 0x4b008, GCC_QUPV3_WRAP0_S2_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP0_S3_CLK, 0x4b008, GCC_QUPV3_WRAP0_S3_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP0_S4_CLK, 0x4b008, GCC_QUPV3_WRAP0_S4_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP0_S5_CLK, 0x4b008, GCC_QUPV3_WRAP0_S5_CLK_ENA_BIT),

	/* QUP Wrapper 1 clocks (includes uart10) */
	GATE_CLK(GCC_QUPV3_WRAP1_S0_CLK, 0x4b008, GCC_QUPV3_WRAP1_S0_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP1_S1_CLK, 0x4b008, GCC_QUPV3_WRAP1_S1_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP1_S2_CLK, 0x4b008, GCC_QUPV3_WRAP1_S2_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP1_S3_CLK, 0x4b008, GCC_QUPV3_WRAP1_S3_CLK_ENA_BIT),  /* uart10 */
	GATE_CLK(GCC_QUPV3_WRAP1_S4_CLK, 0x4b008, GCC_QUPV3_WRAP1_S4_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP1_S5_CLK, 0x4b008, GCC_QUPV3_WRAP1_S5_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP1_S6_CLK, 0x4b018, GCC_QUPV3_WRAP1_S6_CLK_ENA_BIT),

	/* QUP Wrapper 2 clocks */
	GATE_CLK(GCC_QUPV3_WRAP2_S0_CLK, 0x4b010, GCC_QUPV3_WRAP2_S0_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP2_S1_CLK, 0x4b010, GCC_QUPV3_WRAP2_S1_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP2_S2_CLK, 0x4b010, GCC_QUPV3_WRAP2_S2_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP2_S3_CLK, 0x4b010, GCC_QUPV3_WRAP2_S3_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP2_S4_CLK, 0x4b010, GCC_QUPV3_WRAP2_S4_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP2_S5_CLK, 0x4b010, GCC_QUPV3_WRAP2_S5_CLK_ENA_BIT),
	GATE_CLK(GCC_QUPV3_WRAP2_S6_CLK, 0x4b018, GCC_QUPV3_WRAP2_S6_CLK_ENA_BIT),

	/* QUP Wrapper 3 clocks */
	GATE_CLK(GCC_QUPV3_WRAP3_S0_CLK, 0x4b000, GCC_QUPV3_WRAP3_S0_CLK_ENA_BIT),
};

static int sa8775p_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	if (priv->data->num_clks < clk->id) {
		debug("%s: unknown clk id %lu\n", __func__, clk->id);
		return 0;
	}

	debug("%s: clk %ld: %s\n", __func__, clk->id, sa8775p_clks[clk->id].name);

	switch (clk->id) {
	case GCC_AGGRE_USB3_PRIM_AXI_CLK:
		qcom_gate_clk_en(priv, GCC_USB30_PRIM_MASTER_CLK);
		fallthrough;
	case GCC_USB30_PRIM_MASTER_CLK:
		qcom_gate_clk_en(priv, GCC_USB3_PRIM_PHY_AUX_CLK);
		qcom_gate_clk_en(priv, GCC_USB3_PRIM_PHY_COM_AUX_CLK);
		break;
	}

	return qcom_gate_clk_en(priv, clk->id);
}

static const struct qcom_reset_map sa8775p_gcc_resets[] = {
	[GCC_CAMERA_BCR] = { 0x32000 },
	[GCC_DISPLAY1_BCR] = { 0xC7000 },
	[GCC_DISPLAY_BCR] = { 0x33000 },
	[GCC_EMAC0_BCR] = { 0xB6000 },
	[GCC_EMAC1_BCR] = { 0xB4000 },
	[GCC_GPU_BCR] = { 0x7D000 },
	[GCC_MMSS_BCR] = { 0x17000 },
	[GCC_PCIE_0_BCR] = { 0xa9000 },
	[GCC_PCIE_0_LINK_DOWN_BCR] = { 0xBF000 },
	[GCC_PCIE_0_NOCSR_COM_PHY_BCR] = { 0xBF008 },
	[GCC_PCIE_0_PHY_BCR] = { 0xAD144 },
	[GCC_PCIE_0_PHY_NOCSR_COM_PHY_BCR] = { 0xBF00C },
	[GCC_PCIE_1_BCR] = { 0x77000 },
	[GCC_PCIE_1_LINK_DOWN_BCR] = { 0xAE084 },
	[GCC_PCIE_1_NOCSR_COM_PHY_BCR] = { 0xAE090 },
	[GCC_PCIE_1_PHY_BCR] = { 0xAE08C },
	[GCC_PCIE_1_PHY_NOCSR_COM_PHY_BCR] = { 0xAE094 },
	[GCC_PDM_BCR] = { 0x3F000 },
	[GCC_QUPV3_WRAPPER_0_BCR] = { 0x23000 },
	[GCC_QUPV3_WRAPPER_1_BCR] = { 0x24000 },
	[GCC_QUPV3_WRAPPER_2_BCR] = { 0x2A000 },
	[GCC_QUPV3_WRAPPER_3_BCR] = { 0xC4000 },
	[GCC_SDCC1_BCR] = { 0x20000 },
	[GCC_TSCSS_BCR] = { 0x21000 },
	[GCC_UFS_CARD_BCR] = { 0x81000 },
	[GCC_UFS_PHY_BCR] = { 0x83000 },
	[GCC_USB20_PRIM_BCR] = {0x1c000},
	[GCC_USB2_PHY_PRIM_BCR] = {0x5c028},
	[GCC_USB2_PHY_SEC_BCR] = {0x5c02c},
	[GCC_USB30_PRIM_BCR] = {0x1b000},
	[GCC_USB30_SEC_BCR] = {0x2f000},
	[GCC_USB3_DP_PHY_PRIM_BCR] = {0x5c008},
	[GCC_USB3_DP_PHY_SEC_BCR] = {0x5c014},
	[GCC_USB3_PHY_PRIM_BCR] = {0x5c000},
	[GCC_USB3_PHY_SEC_BCR] = {0x5c00c},
	[GCC_USB3_PHY_TERT_BCR] = {0x5c030},
	[GCC_USB3_UNIPHY_MP0_BCR] = {0x5c018},
	[GCC_USB3_UNIPHY_MP1_BCR] = {0x5c01c},
	[GCC_USB3PHY_PHY_PRIM_BCR] = {0x5c004},
	[GCC_USB3PHY_PHY_SEC_BCR] = {0x5c010},
	[GCC_USB3UNIPHY_PHY_MP0_BCR] = {0x5c020},
	[GCC_USB3UNIPHY_PHY_MP1_BCR] = {0x5c024},
	[GCC_USB_PHY_CFG_AHB2PHY_BCR] = {0x76000},
	[GCC_VIDEO_BCR] = {0x34000}
};

static const struct qcom_power_map sa8775p_gdscs[] = {
	[UFS_PHY_GDSC] = { 0x83004 },
	[USB30_PRIM_GDSC] = { 0x1B004 },
};

static struct msm_clk_data sa8775_gcc_data = {
	.resets = sa8775p_gcc_resets,
	.num_resets = ARRAY_SIZE(sa8775p_gcc_resets),
	.clks = sa8775p_clks,
	.num_clks = ARRAY_SIZE(sa8775p_clks),

	.power_domains = sa8775p_gdscs,
	.num_power_domains = ARRAY_SIZE(sa8775p_gdscs),

	.enable = sa8775p_enable,
	.set_rate = sa8775p_set_rate,
};

static const struct udevice_id gcc_sa8775p_of_match[] = {
	{
		.compatible = "qcom,sa8775p-gcc",
		.data = (ulong)&sa8775_gcc_data,
	},
	{ }
};

U_BOOT_DRIVER(gcc_sa8775p) = {
	.name		= "gcc_sa8775p",
	.id		= UCLASS_NOP,
	.of_match	= gcc_sa8775p_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
