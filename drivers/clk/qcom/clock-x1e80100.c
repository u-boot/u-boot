// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm x1e80100
 *
 * (C) Copyright 2024 Linaro Ltd.
 */

#include <clk-uclass.h>
#include <dm.h>
#include <linux/delay.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/bug.h>
#include <linux/bitops.h>
#include <dt-bindings/clock/qcom,x1e80100-gcc.h>
#include <dt-bindings/clock/qcom,x1e80100-tcsr.h>

#include "clock-qcom.h"

/* On-board TCXO, TOFIX get from DT */
#define TCXO_RATE	38400000

/* bi_tcxo_div2 divided after RPMh output */
#define TCXO_DIV2_RATE	(TCXO_RATE / 2)

static const struct freq_tbl ftbl_gcc_qupv3_wrap0_s4_clk_src[] = {
	F(7372800, CFG_CLK_SRC_GPLL0_EVEN, 1, 384, 15625),
	F(14745600, CFG_CLK_SRC_GPLL0_EVEN, 1, 768, 15625),
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(29491200, CFG_CLK_SRC_GPLL0_EVEN, 1, 1536, 15625),
	F(32000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 8, 75),
	F(48000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 4, 25),
	F(64000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 16, 75),
	F(75000000, CFG_CLK_SRC_GPLL0_EVEN, 4, 0, 0),
	F(80000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 4, 15),
	F(96000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 8, 25),
	F(100000000, CFG_CLK_SRC_GPLL0, 6, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_sdcc2_apps_clk_src[] = {
	F(400000, CFG_CLK_SRC_CXO, 12, 1, 4),
	F(25000000, CFG_CLK_SRC_GPLL0_EVEN, 12, 0, 0),
	F(100000000, CFG_CLK_SRC_GPLL0_EVEN, 3, 0, 0),
	/* TOFIX F(202000000, CFG_CLK_SRC_GPLL9, 4, 0, 0), */
	{ }
};

static const struct freq_tbl ftbl_gcc_usb30_prim_master_clk_src[] = {
	F(66666667, CFG_CLK_SRC_GPLL0_EVEN, 4.5, 0, 0),
	F(133333333, CFG_CLK_SRC_GPLL0, 4.5, 0, 0),
	F(200000000, CFG_CLK_SRC_GPLL0, 3, 0, 0),
	F(240000000, CFG_CLK_SRC_GPLL0, 2.5, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_pcie_0_aux_clk_src[] = {
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_pcie_0_phy_rchng_clk_src[] = {
	F(100000000, CFG_CLK_SRC_GPLL0_EVEN, 3, 0, 0),
	{ }
};

static ulong x1e80100_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);
	const struct freq_tbl *freq;

	switch (clk->id) {
	case GCC_QUPV3_WRAP2_S5_CLK: /* UART21 */
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap0_s4_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, 0x1e500,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_SDCC2_APPS_CLK:
		freq = qcom_find_freq(ftbl_gcc_sdcc2_apps_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, 0x14018,
				     freq->pre_div, freq->m, freq->n, freq->src, 8);
		return freq->freq;
	case GCC_USB30_PRIM_MASTER_CLK:
		freq = qcom_find_freq(ftbl_gcc_usb30_prim_master_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, 0x3902c,
				     freq->pre_div, freq->m, freq->n, freq->src, 8);
		return freq->freq;
	case GCC_USB30_PRIM_MOCK_UTMI_CLK:
		clk_rcg_set_rate(priv->base, 0x39044, 0, 0);
		return TCXO_DIV2_RATE;
	case GCC_PCIE_4_AUX_CLK:
		freq = qcom_find_freq(ftbl_gcc_pcie_0_aux_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, 0x6b080,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_PCIE_4_PHY_RCHNG_CLK:
		freq = qcom_find_freq(ftbl_gcc_pcie_0_phy_rchng_clk_src, rate);
		clk_rcg_set_rate(priv->base, 0x6b064, freq->pre_div, freq->src);
		return freq->freq;
	case GCC_PCIE_6A_AUX_CLK:
		freq = qcom_find_freq(ftbl_gcc_pcie_0_aux_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, 0x3108c,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_PCIE_6A_PHY_RCHNG_CLK:
		freq = qcom_find_freq(ftbl_gcc_pcie_0_phy_rchng_clk_src, rate);
		clk_rcg_set_rate(priv->base, 0x31070, freq->pre_div, freq->src);
		return freq->freq;
	default:
		return 0;
	}
}

static const struct gate_clk x1e80100_clks[] = {
	GATE_CLK(GCC_AGGRE_UFS_PHY_AXI_CLK,		0x770e4, BIT(0)),
	GATE_CLK(GCC_CFG_NOC_USB3_PRIM_AXI_CLK,		0x3908c, BIT(0)),
	GATE_CLK(GCC_CFG_NOC_PCIE_ANOC_SOUTH_AHB_CLK,	0x52000, BIT(20)),
	GATE_CLK(GCC_CFG_NOC_PCIE_ANOC_NORTH_AHB_CLK,	0x52028, BIT(22)),
	GATE_CLK(GCC_CNOC_PCIE_SOUTH_SF_AXI_CLK,	0x52028, BIT(12)),
	GATE_CLK(GCC_CNOC_PCIE_NORTH_SF_AXI_CLK,	0x52008, BIT(6)),
	GATE_CLK(GCC_PCIE_4_AUX_CLK,			0x52008, BIT(3)),
	GATE_CLK(GCC_PCIE_4_CFG_AHB_CLK,		0x52008, BIT(2)),
	GATE_CLK(GCC_PCIE_4_MSTR_AXI_CLK,		0x52008, BIT(1)),
	GATE_CLK(GCC_PCIE_4_PHY_RCHNG_CLK,		0x52000, BIT(22)),
	GATE_CLK(GCC_PCIE_4_PIPE_CLK,			0x52008, BIT(4)),
	GATE_CLK(GCC_PCIE_4_SLV_AXI_CLK,		0x52008, BIT(0)),
	GATE_CLK(GCC_PCIE_4_SLV_Q2A_AXI_CLK,		0x52008, BIT(5)),
	GATE_CLK(GCC_PCIE_6A_AUX_CLK,			0x52018, BIT(24)),
	GATE_CLK(GCC_PCIE_6A_CFG_AHB_CLK,		0x52018, BIT(23)),
	GATE_CLK(GCC_PCIE_6A_MSTR_AXI_CLK,		0x52018, BIT(22)),
	GATE_CLK(GCC_PCIE_6A_PHY_RCHNG_CLK,		0x52018, BIT(27)),
	GATE_CLK(GCC_PCIE_6A_PIPE_CLK,			0x52018, BIT(26)),
	GATE_CLK(GCC_PCIE_6A_SLV_AXI_CLK,		0x52018, BIT(21)),
	GATE_CLK(GCC_PCIE_6A_SLV_Q2A_AXI_CLK,		0x52018, BIT(20)),
	GATE_CLK(GCC_QUPV3_WRAP2_CORE_2X_CLK,		0x52010, BIT(3)),
	GATE_CLK(GCC_QUPV3_WRAP2_CORE_CLK,		0x52010, BIT(0)),
	GATE_CLK(GCC_QUPV3_WRAP2_S0_CLK,		0x52010, BIT(4)),
	GATE_CLK(GCC_QUPV3_WRAP2_S1_CLK,		0x52010, BIT(5)),
	GATE_CLK(GCC_QUPV3_WRAP2_S2_CLK,		0x52010, BIT(6)),
	GATE_CLK(GCC_QUPV3_WRAP2_S3_CLK,		0x52010, BIT(7)),
	GATE_CLK(GCC_QUPV3_WRAP2_S4_CLK,		0x52010, BIT(8)),
	GATE_CLK(GCC_QUPV3_WRAP2_S5_CLK,		0x52010, BIT(9)),
	GATE_CLK(GCC_QUPV3_WRAP2_S6_CLK,		0x52010, BIT(10)),
	GATE_CLK(GCC_QUPV3_WRAP2_S7_CLK,		0x52010, BIT(17)),
	GATE_CLK(GCC_QUPV3_WRAP_2_M_AHB_CLK,		0x52010, BIT(2)),
	GATE_CLK(GCC_QUPV3_WRAP_2_S_AHB_CLK,		0x52010, BIT(1)),
	GATE_CLK(GCC_USB30_PRIM_MASTER_CLK,		0x39018, BIT(0)),
	GATE_CLK(GCC_USB30_PRIM_MOCK_UTMI_CLK,		0x39028, BIT(0)),
	GATE_CLK(GCC_USB30_PRIM_SLEEP_CLK,		0x39024, BIT(0)),
	GATE_CLK(GCC_USB3_PRIM_PHY_AUX_CLK,		0x39060, BIT(0)),
	GATE_CLK(GCC_USB3_PRIM_PHY_COM_AUX_CLK,		0x39064, BIT(0)),
	GATE_CLK(GCC_USB3_PRIM_PHY_PIPE_CLK,		0x39068, BIT(0)),
};

static int x1e80100_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case GCC_AGGRE_USB3_PRIM_AXI_CLK:
		qcom_gate_clk_en(priv, GCC_USB30_PRIM_MASTER_CLK);
		fallthrough;
	case GCC_USB30_PRIM_MASTER_CLK:
		qcom_gate_clk_en(priv, GCC_USB3_PRIM_PHY_AUX_CLK);
		qcom_gate_clk_en(priv, GCC_USB3_PRIM_PHY_COM_AUX_CLK);
		break;
	case GCC_PCIE_4_PIPE_CLK:
		// GCC_PCIE_4_PIPE_CLK_SRC
		clk_phy_mux_enable(priv->base, 0x6b07c, true);
		break;
	case GCC_PCIE_6A_PIPE_CLK:
		// GCC_PCIE_6A_PIPE_CLK_SRC
		clk_phy_mux_enable(priv->base, 0x31088, true);
		break;
	}

	return qcom_gate_clk_en(priv, clk->id);
}

static const struct qcom_reset_map x1e80100_gcc_resets[] = {
	[GCC_AV1E_BCR] = { 0x4a000 },
	[GCC_CAMERA_BCR] = { 0x26000 },
	[GCC_DISPLAY_BCR] = { 0x27000 },
	[GCC_GPU_BCR] = { 0x71000 },
	[GCC_PCIE_0_LINK_DOWN_BCR] = { 0x6c014 },
	[GCC_PCIE_0_NOCSR_COM_PHY_BCR] = { 0x6c020 },
	[GCC_PCIE_0_PHY_BCR] = { 0x6c01c },
	[GCC_PCIE_0_PHY_NOCSR_COM_PHY_BCR] = { 0x6c028 },
	[GCC_PCIE_0_TUNNEL_BCR] = { 0xa0000 },
	[GCC_PCIE_1_LINK_DOWN_BCR] = { 0x8e014 },
	[GCC_PCIE_1_NOCSR_COM_PHY_BCR] = { 0x8e020 },
	[GCC_PCIE_1_PHY_BCR] = { 0x8e01c },
	[GCC_PCIE_1_PHY_NOCSR_COM_PHY_BCR] = { 0x8e024 },
	[GCC_PCIE_1_TUNNEL_BCR] = { 0x2c000 },
	[GCC_PCIE_2_LINK_DOWN_BCR] = { 0xa5014 },
	[GCC_PCIE_2_NOCSR_COM_PHY_BCR] = { 0xa5020 },
	[GCC_PCIE_2_PHY_BCR] = { 0xa501c },
	[GCC_PCIE_2_PHY_NOCSR_COM_PHY_BCR] = { 0xa5028 },
	[GCC_PCIE_2_TUNNEL_BCR] = { 0x13000 },
	[GCC_PCIE_3_BCR] = { 0x58000 },
	[GCC_PCIE_3_LINK_DOWN_BCR] = { 0xab014 },
	[GCC_PCIE_3_NOCSR_COM_PHY_BCR] = { 0xab020 },
	[GCC_PCIE_3_PHY_BCR] = { 0xab01c },
	[GCC_PCIE_3_PHY_NOCSR_COM_PHY_BCR] = { 0xab024 },
	[GCC_PCIE_4_BCR] = { 0x6b000 },
	[GCC_PCIE_4_LINK_DOWN_BCR] = { 0xb3014 },
	[GCC_PCIE_4_NOCSR_COM_PHY_BCR] = { 0xb3020 },
	[GCC_PCIE_4_PHY_BCR] = { 0xb301c },
	[GCC_PCIE_4_PHY_NOCSR_COM_PHY_BCR] = { 0xb3028 },
	[GCC_PCIE_5_BCR] = { 0x2f000 },
	[GCC_PCIE_5_LINK_DOWN_BCR] = { 0xaa014 },
	[GCC_PCIE_5_NOCSR_COM_PHY_BCR] = { 0xaa020 },
	[GCC_PCIE_5_PHY_BCR] = { 0xaa01c },
	[GCC_PCIE_5_PHY_NOCSR_COM_PHY_BCR] = { 0xaa028 },
	[GCC_PCIE_6A_BCR] = { 0x31000 },
	[GCC_PCIE_6A_LINK_DOWN_BCR] = { 0xac014 },
	[GCC_PCIE_6A_NOCSR_COM_PHY_BCR] = { 0xac020 },
	[GCC_PCIE_6A_PHY_BCR] = { 0xac01c },
	[GCC_PCIE_6A_PHY_NOCSR_COM_PHY_BCR] = { 0xac024 },
	[GCC_PCIE_6B_BCR] = { 0x8d000 },
	[GCC_PCIE_6B_LINK_DOWN_BCR] = { 0xb5014 },
	[GCC_PCIE_6B_NOCSR_COM_PHY_BCR] = { 0xb5020 },
	[GCC_PCIE_6B_PHY_BCR] = { 0xb501c },
	[GCC_PCIE_6B_PHY_NOCSR_COM_PHY_BCR] = { 0xb5024 },
	[GCC_PCIE_PHY_BCR] = { 0x6f000 },
	[GCC_PCIE_PHY_CFG_AHB_BCR] = { 0x6f00c },
	[GCC_PCIE_PHY_COM_BCR] = { 0x6f010 },
	[GCC_PCIE_RSCC_BCR] = { 0xa4000 },
	[GCC_PDM_BCR] = { 0x33000 },
	[GCC_QUPV3_WRAPPER_0_BCR] = { 0x42000 },
	[GCC_QUPV3_WRAPPER_1_BCR] = { 0x18000 },
	[GCC_QUPV3_WRAPPER_2_BCR] = { 0x1e000 },
	[GCC_QUSB2PHY_HS0_MP_BCR] = { 0x1200c },
	[GCC_QUSB2PHY_HS1_MP_BCR] = { 0x12010 },
	[GCC_QUSB2PHY_PRIM_BCR] = { 0x12000 },
	[GCC_QUSB2PHY_SEC_BCR] = { 0x12004 },
	[GCC_QUSB2PHY_TERT_BCR] = { 0x12008 },
	[GCC_QUSB2PHY_USB20_HS_BCR] = { 0x12014 },
	[GCC_SDCC2_BCR] = { 0x14000 },
	[GCC_SDCC4_BCR] = { 0x16000 },
	[GCC_UFS_PHY_BCR] = { 0x77000 },
	[GCC_USB20_PRIM_BCR] = { 0x29000 },
	[GCC_USB30_MP_BCR] = { 0x17000 },
	[GCC_USB30_PRIM_BCR] = { 0x39000 },
	[GCC_USB30_SEC_BCR] = { 0xa1000 },
	[GCC_USB30_TERT_BCR] = { 0xa2000 },
	[GCC_USB3_MP_SS0_PHY_BCR] = { 0x19008 },
	[GCC_USB3_MP_SS1_PHY_BCR] = { 0x54008 },
	[GCC_USB3_PHY_PRIM_BCR] = { 0x50000 },
	[GCC_USB3_PHY_SEC_BCR] = { 0x2a000 },
	[GCC_USB3_PHY_TERT_BCR] = { 0xa3000 },
	[GCC_USB3_UNIPHY_MP0_BCR] = { 0x19000 },
	[GCC_USB3_UNIPHY_MP1_BCR] = { 0x54000 },
	[GCC_USB3PHY_PHY_PRIM_BCR] = { 0x50004 },
	[GCC_USB3PHY_PHY_SEC_BCR] = { 0x2a004 },
	[GCC_USB3PHY_PHY_TERT_BCR] = { 0xa3004 },
	[GCC_USB3UNIPHY_PHY_MP0_BCR] = { 0x19004 },
	[GCC_USB3UNIPHY_PHY_MP1_BCR] = { 0x54004 },
	[GCC_USB4_0_BCR] = { 0x9f000 },
	[GCC_USB4_0_DP0_PHY_PRIM_BCR] = { 0x50010 },
	[GCC_USB4_1_DP0_PHY_SEC_BCR] = { 0x2a010 },
	[GCC_USB4_2_DP0_PHY_TERT_BCR] = { 0xa3010 },
	[GCC_USB4_1_BCR] = { 0x2b000 },
	[GCC_USB4_2_BCR] = { 0x11000 },
	[GCC_USB_0_PHY_BCR] = { 0x50020 },
	[GCC_USB_1_PHY_BCR] = { 0x2a020 },
	[GCC_USB_2_PHY_BCR] = { 0xa3020 },
	[GCC_VIDEO_BCR] = { 0x32000 },
};

static const struct qcom_power_map x1e80100_gdscs[] = {
	[GCC_PCIE_0_TUNNEL_GDSC] = { 0xa0004 },
	[GCC_PCIE_1_TUNNEL_GDSC] = { 0x2c004 },
	[GCC_PCIE_2_TUNNEL_GDSC] = { 0x13004 },
	[GCC_PCIE_3_GDSC] = { 0x58004 },
	[GCC_PCIE_3_PHY_GDSC] = { 0x3e000 },
	[GCC_PCIE_4_GDSC] = { 0x6b004 },
	[GCC_PCIE_4_PHY_GDSC] = { 0x6c000 },
	[GCC_PCIE_5_GDSC] = { 0x2f004 },
	[GCC_PCIE_5_PHY_GDSC] = { 0x30000 },
	[GCC_PCIE_6_PHY_GDSC] = { 0x8e000 },
	[GCC_PCIE_6A_GDSC] = { 0x31004 },
	[GCC_PCIE_6B_GDSC] = { 0x8d004 },
	[GCC_UFS_MEM_PHY_GDSC] = { 0x9e000 },
	[GCC_UFS_PHY_GDSC] = { 0x77004 },
	[GCC_USB20_PRIM_GDSC] = { 0x29004 },
	[GCC_USB30_MP_GDSC] = { 0x17004 },
	[GCC_USB30_PRIM_GDSC] = { 0x39004 },
	[GCC_USB30_SEC_GDSC] = { 0xa1004 },
	[GCC_USB30_TERT_GDSC] = { 0xa2004 },
	[GCC_USB3_MP_SS0_PHY_GDSC] = { 0x1900c },
	[GCC_USB3_MP_SS1_PHY_GDSC] = { 0x5400c },
	[GCC_USB4_0_GDSC] = { 0x9f004 },
	[GCC_USB4_1_GDSC] = { 0x2b004 },
	[GCC_USB4_2_GDSC] = { 0x11004 },
	[GCC_USB_0_PHY_GDSC] = { 0x50024 },
	[GCC_USB_1_PHY_GDSC] = { 0x2a024 },
	[GCC_USB_2_PHY_GDSC] = { 0xa3024 },
};

static struct msm_clk_data x1e80100_gcc_data = {
	.resets = x1e80100_gcc_resets,
	.num_resets = ARRAY_SIZE(x1e80100_gcc_resets),
	.clks = x1e80100_clks,
	.num_clks = ARRAY_SIZE(x1e80100_clks),
	.power_domains = x1e80100_gdscs,
	.num_power_domains = ARRAY_SIZE(x1e80100_gdscs),

	.enable = x1e80100_enable,
	.set_rate = x1e80100_set_rate,
};

static const struct udevice_id gcc_x1e80100_of_match[] = {
	{
		.compatible = "qcom,x1e80100-gcc",
		.data = (ulong)&x1e80100_gcc_data,
	},
	{ }
};

U_BOOT_DRIVER(gcc_x1e80100) = {
	.name		= "gcc_x1e80100",
	.id		= UCLASS_NOP,
	.of_match	= gcc_x1e80100_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};

/* TCSRCC */

static const struct gate_clk x1e80100_tcsr_clks[] = {
	GATE_CLK(TCSR_PCIE_2L_4_CLKREF_EN,	0x15100, BIT(0)),
	GATE_CLK(TCSR_PCIE_2L_5_CLKREF_EN,	0x15104, BIT(0)),
	GATE_CLK(TCSR_PCIE_8L_CLKREF_EN,	0x15108, BIT(0)),
	GATE_CLK(TCSR_USB3_MP0_CLKREF_EN,	0x1510c, BIT(0)),
	GATE_CLK(TCSR_USB3_MP1_CLKREF_EN,	0x15110, BIT(0)),
	GATE_CLK(TCSR_USB2_1_CLKREF_EN,		0x15114, BIT(0)),
	GATE_CLK(TCSR_UFS_PHY_CLKREF_EN,	0x15118, BIT(0)),
	GATE_CLK(TCSR_USB4_1_CLKREF_EN,		0x15120, BIT(0)),
	GATE_CLK(TCSR_USB4_2_CLKREF_EN,		0x15124, BIT(0)),
	GATE_CLK(TCSR_USB2_2_CLKREF_EN,		0x15128, BIT(0)),
	GATE_CLK(TCSR_PCIE_4L_CLKREF_EN,	0x1512c, BIT(0)),
	GATE_CLK(TCSR_EDP_CLKREF_EN,		0x15130, BIT(0)),
};

static struct msm_clk_data x1e80100_tcsrcc_data = {
	.clks = x1e80100_tcsr_clks,
	.num_clks = ARRAY_SIZE(x1e80100_tcsr_clks),
};

static int tcsrcc_x1e80100_clk_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	qcom_gate_clk_en(priv, clk->id);

	return 0;
}

static ulong tcsrcc_x1e80100_clk_get_rate(struct clk *clk)
{
	return TCXO_RATE;
}

static int tcsrcc_x1e80100_clk_probe(struct udevice *dev)
{
	struct msm_clk_data *data = (struct msm_clk_data *)dev_get_driver_data(dev);
	struct msm_clk_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->data = data;

	return 0;
}

static struct clk_ops tcsrcc_x1e80100_clk_ops = {
	.enable = tcsrcc_x1e80100_clk_enable,
	.get_rate = tcsrcc_x1e80100_clk_get_rate,
};

static const struct udevice_id tcsrcc_x1e80100_of_match[] = {
	{
		.compatible = "qcom,x1e80100-tcsr",
		.data = (ulong)&x1e80100_tcsrcc_data,
	},
	{ }
};

U_BOOT_DRIVER(tcsrcc_x1e80100) = {
	.name		= "tcsrcc_x1e80100",
	.id		= UCLASS_CLK,
	.of_match	= tcsrcc_x1e80100_of_match,
	.ops		= &tcsrcc_x1e80100_clk_ops,
	.priv_auto	= sizeof(struct msm_clk_priv),
	.probe		= tcsrcc_x1e80100_clk_probe,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
