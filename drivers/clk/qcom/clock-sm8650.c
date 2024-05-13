// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm sm8650
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
#include <dt-bindings/clock/qcom,sm8650-gcc.h>
#include <dt-bindings/clock/qcom,sm8650-tcsr.h>

#include "clock-qcom.h"

/* On-board TCXO, TOFIX get from DT */
#define TCXO_RATE	38400000

/* bi_tcxo_div2 divided after RPMh output */
#define TCXO_DIV2_RATE	(TCXO_RATE / 2)

static const struct freq_tbl ftbl_gcc_qupv3_wrap1_s3_clk_src[] = {
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

static ulong sm8650_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);
	const struct freq_tbl *freq;

	switch (clk->id) {
	case GCC_QUPV3_WRAP2_S7_CLK: /* UART15 */
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap1_s3_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, 0x1e898,
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
	case GCC_USB3_PRIM_PHY_AUX_CLK_SRC:
		clk_rcg_set_rate(priv->base, 0x39070, 0, 0);
		return TCXO_DIV2_RATE;
	default:
		return 0;
	}
}

static const struct gate_clk sm8650_clks[] = {
	GATE_CLK(GCC_AGGRE_NOC_PCIE_AXI_CLK,		0x52000, BIT(12)),
	GATE_CLK(GCC_AGGRE_UFS_PHY_AXI_CLK,		0x770e4, BIT(0)),
	GATE_CLK(GCC_AGGRE_UFS_PHY_AXI_HW_CTL_CLK,	0x770e4, BIT(1)),
	GATE_CLK(GCC_AGGRE_USB3_PRIM_AXI_CLK,		0x3908c, BIT(0)),
	GATE_CLK(GCC_CNOC_PCIE_SF_AXI_CLK,		0x52008, BIT(6)),
	GATE_CLK(GCC_DDRSS_GPU_AXI_CLK,			0x71154, BIT(0)),
	GATE_CLK(GCC_DDRSS_PCIE_SF_QTB_CLK,		0x52000, BIT(19)),
	GATE_CLK(GCC_PCIE_0_AUX_CLK,			0x52008, BIT(3)),
	GATE_CLK(GCC_PCIE_0_CFG_AHB_CLK,		0x52008, BIT(2)),
	GATE_CLK(GCC_PCIE_0_MSTR_AXI_CLK,		0x52008, BIT(1)),
	GATE_CLK(GCC_PCIE_0_PHY_RCHNG_CLK,		0x52000, BIT(22)),
	GATE_CLK(GCC_PCIE_0_PIPE_CLK,			0x52008, BIT(4)),
	GATE_CLK(GCC_PCIE_0_SLV_AXI_CLK,		0x52008, BIT(0)),
	GATE_CLK(GCC_PCIE_0_SLV_Q2A_AXI_CLK,		0x52008, BIT(5)),
	GATE_CLK(GCC_PCIE_1_AUX_CLK,			0x52000, BIT(29)),
	GATE_CLK(GCC_PCIE_1_CFG_AHB_CLK,		0x52000, BIT(28)),
	GATE_CLK(GCC_PCIE_1_MSTR_AXI_CLK,		0x52000, BIT(27)),
	GATE_CLK(GCC_PCIE_1_PHY_AUX_CLK,		0x52000, BIT(24)),
	GATE_CLK(GCC_PCIE_1_PHY_RCHNG_CLK,		0x52000, BIT(23)),
	GATE_CLK(GCC_PCIE_1_PIPE_CLK,			0x52000, BIT(30)),
	GATE_CLK(GCC_PCIE_1_SLV_AXI_CLK,		0x52000, BIT(26)),
	GATE_CLK(GCC_PCIE_1_SLV_Q2A_AXI_CLK,		0x52000, BIT(25)),
	GATE_CLK(GCC_QUPV3_I2C_CORE_CLK,		0x52008, BIT(8)),
	GATE_CLK(GCC_QUPV3_I2C_S0_CLK,			0x52008, BIT(10)),
	GATE_CLK(GCC_QUPV3_I2C_S1_CLK,			0x52008, BIT(11)),
	GATE_CLK(GCC_QUPV3_I2C_S2_CLK,			0x52008, BIT(12)),
	GATE_CLK(GCC_QUPV3_I2C_S3_CLK,			0x52008, BIT(13)),
	GATE_CLK(GCC_QUPV3_I2C_S4_CLK,			0x52008, BIT(14)),
	GATE_CLK(GCC_QUPV3_I2C_S5_CLK,			0x52008, BIT(15)),
	GATE_CLK(GCC_QUPV3_I2C_S6_CLK,			0x52008, BIT(16)),
	GATE_CLK(GCC_QUPV3_I2C_S7_CLK,			0x52008, BIT(17)),
	GATE_CLK(GCC_QUPV3_I2C_S8_CLK,			0x52010, BIT(14)),
	GATE_CLK(GCC_QUPV3_I2C_S9_CLK,			0x52010, BIT(15)),
	GATE_CLK(GCC_QUPV3_I2C_S_AHB_CLK,		0x52008, BIT(7)),
	GATE_CLK(GCC_QUPV3_WRAP1_CORE_2X_CLK,		0x52008, BIT(18)),
	GATE_CLK(GCC_QUPV3_WRAP1_CORE_CLK,		0x52008, BIT(19)),
	GATE_CLK(GCC_QUPV3_WRAP1_S0_CLK,		0x52008, BIT(22)),
	GATE_CLK(GCC_QUPV3_WRAP1_S1_CLK,		0x52008, BIT(23)),
	GATE_CLK(GCC_QUPV3_WRAP1_S2_CLK,		0x52008, BIT(24)),
	GATE_CLK(GCC_QUPV3_WRAP1_S3_CLK,		0x52008, BIT(25)),
	GATE_CLK(GCC_QUPV3_WRAP1_S4_CLK,		0x52008, BIT(26)),
	GATE_CLK(GCC_QUPV3_WRAP1_S5_CLK,		0x52008, BIT(27)),
	GATE_CLK(GCC_QUPV3_WRAP1_S6_CLK,		0x52008, BIT(28)),
	GATE_CLK(GCC_QUPV3_WRAP1_S7_CLK,		0x52010, BIT(16)),
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
	GATE_CLK(GCC_QUPV3_WRAP_1_M_AHB_CLK,		0x52008, BIT(20)),
	GATE_CLK(GCC_QUPV3_WRAP_1_S_AHB_CLK,		0x52008, BIT(21)),
	GATE_CLK(GCC_QUPV3_WRAP_2_M_AHB_CLK,		0x52010, BIT(2)),
	GATE_CLK(GCC_QUPV3_WRAP_2_S_AHB_CLK,		0x52010, BIT(1)),
	GATE_CLK(GCC_SDCC2_AHB_CLK,			0x14010, BIT(0)),
	GATE_CLK(GCC_SDCC2_APPS_CLK,			0x14004, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_AHB_CLK,			0x77024, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_AXI_CLK,			0x77018, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_AXI_HW_CTL_CLK,		0x77018, BIT(1)),
	GATE_CLK(GCC_UFS_PHY_ICE_CORE_CLK,		0x77074, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_ICE_CORE_HW_CTL_CLK,	0x77074, BIT(1)),
	GATE_CLK(GCC_UFS_PHY_PHY_AUX_CLK,		0x770b0, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_PHY_AUX_HW_CTL_CLK,	0x770b0, BIT(1)),
	GATE_CLK(GCC_UFS_PHY_RX_SYMBOL_0_CLK,		0x7702c, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_RX_SYMBOL_1_CLK,		0x770cc, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_TX_SYMBOL_0_CLK,		0x77028, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_UNIPRO_CORE_CLK,		0x77068, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_UNIPRO_CORE_HW_CTL_CLK,	0x77068, BIT(1)),
	GATE_CLK(GCC_USB30_PRIM_MASTER_CLK,		0x39018, BIT(0)),
	GATE_CLK(GCC_USB30_PRIM_MOCK_UTMI_CLK,		0x39028, BIT(0)),
	GATE_CLK(GCC_USB30_PRIM_SLEEP_CLK,		0x39024, BIT(0)),
	GATE_CLK(GCC_USB3_PRIM_PHY_AUX_CLK,		0x39060, BIT(0)),
	GATE_CLK(GCC_USB3_PRIM_PHY_COM_AUX_CLK,		0x39064, BIT(0)),
	GATE_CLK(GCC_USB3_PRIM_PHY_PIPE_CLK,		0x39068, BIT(0)),
};

static int sm8650_enable(struct clk *clk)
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
	}

	qcom_gate_clk_en(priv, clk->id);

	return 0;
}

static const struct qcom_reset_map sm8650_gcc_resets[] = {
	[GCC_CAMERA_BCR] = { 0x26000 },
	[GCC_DISPLAY_BCR] = { 0x27000 },
	[GCC_GPU_BCR] = { 0x71000 },
	[GCC_PCIE_0_BCR] = { 0x6b000 },
	[GCC_PCIE_0_LINK_DOWN_BCR] = { 0x6c014 },
	[GCC_PCIE_0_NOCSR_COM_PHY_BCR] = { 0x6c020 },
	[GCC_PCIE_0_PHY_BCR] = { 0x6c01c },
	[GCC_PCIE_0_PHY_NOCSR_COM_PHY_BCR] = { 0x6c028 },
	[GCC_PCIE_1_BCR] = { 0x8d000 },
	[GCC_PCIE_1_LINK_DOWN_BCR] = { 0x8e014 },
	[GCC_PCIE_1_NOCSR_COM_PHY_BCR] = { 0x8e020 },
	[GCC_PCIE_1_PHY_BCR] = { 0x8e01c },
	[GCC_PCIE_1_PHY_NOCSR_COM_PHY_BCR] = { 0x8e024 },
	[GCC_PCIE_PHY_BCR] = { 0x6f000 },
	[GCC_PCIE_PHY_CFG_AHB_BCR] = { 0x6f00c },
	[GCC_PCIE_PHY_COM_BCR] = { 0x6f010 },
	[GCC_PDM_BCR] = { 0x33000 },
	[GCC_QUPV3_WRAPPER_1_BCR] = { 0x18000 },
	[GCC_QUPV3_WRAPPER_2_BCR] = { 0x1e000 },
	[GCC_QUPV3_WRAPPER_3_BCR] = { 0x19000 },
	[GCC_QUPV3_WRAPPER_I2C_BCR] = { 0x17000 },
	[GCC_QUSB2PHY_PRIM_BCR] = { 0x12000 },
	[GCC_QUSB2PHY_SEC_BCR] = { 0x12004 },
	[GCC_SDCC2_BCR] = { 0x14000 },
	[GCC_SDCC4_BCR] = { 0x16000 },
	[GCC_UFS_PHY_BCR] = { 0x77000 },
	[GCC_USB30_PRIM_BCR] = { 0x39000 },
	[GCC_USB3_DP_PHY_PRIM_BCR] = { 0x50008 },
	[GCC_USB3_DP_PHY_SEC_BCR] = { 0x50014 },
	[GCC_USB3_PHY_PRIM_BCR] = { 0x50000 },
	[GCC_USB3_PHY_SEC_BCR] = { 0x5000c },
	[GCC_USB3PHY_PHY_PRIM_BCR] = { 0x50004 },
	[GCC_USB3PHY_PHY_SEC_BCR] = { 0x50010 },
	[GCC_VIDEO_AXI0_CLK_ARES] = { 0x32018, 2 },
	[GCC_VIDEO_AXI1_CLK_ARES] = { 0x32024, 2 },
	[GCC_VIDEO_BCR] = { 0x32000 },
};

static const struct qcom_power_map sm8650_gdscs[] = {
	[PCIE_0_GDSC] = { 0x6b004 },
	[PCIE_0_PHY_GDSC] = { 0x6c000 },
	[PCIE_1_GDSC] = { 0x8d004 },
	[PCIE_1_PHY_GDSC] = { 0x8e000 },
	[UFS_PHY_GDSC] = { 0x77004 },
	[UFS_MEM_PHY_GDSC] = { 0x9e000 },
	[USB30_PRIM_GDSC] = { 0x39004 },
	[USB3_PHY_GDSC] = { 0x50018 },
};

static struct msm_clk_data sm8650_gcc_data = {
	.resets = sm8650_gcc_resets,
	.num_resets = ARRAY_SIZE(sm8650_gcc_resets),
	.clks = sm8650_clks,
	.num_clks = ARRAY_SIZE(sm8650_clks),
	.power_domains = sm8650_gdscs,
	.num_power_domains = ARRAY_SIZE(sm8650_gdscs),

	.enable = sm8650_enable,
	.set_rate = sm8650_set_rate,
};

static const struct udevice_id gcc_sm8650_of_match[] = {
	{
		.compatible = "qcom,sm8650-gcc",
		.data = (ulong)&sm8650_gcc_data,
	},
	{ }
};

U_BOOT_DRIVER(gcc_sm8650) = {
	.name		= "gcc_sm8650",
	.id		= UCLASS_NOP,
	.of_match	= gcc_sm8650_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};

/* TCSRCC */

static const struct gate_clk sm8650_tcsr_clks[] = {
	GATE_CLK(TCSR_PCIE_0_CLKREF_EN,		0x31100, BIT(0)),
	GATE_CLK(TCSR_PCIE_1_CLKREF_EN,		0x31114, BIT(0)),
	GATE_CLK(TCSR_UFS_CLKREF_EN,		0x31110, BIT(0)),
	GATE_CLK(TCSR_UFS_PAD_CLKREF_EN,	0x31104, BIT(0)),
	GATE_CLK(TCSR_USB2_CLKREF_EN,		0x31118, BIT(0)),
	GATE_CLK(TCSR_USB3_CLKREF_EN,		0x31108, BIT(0)),
};

static struct msm_clk_data sm8650_tcsrcc_data = {
	.clks = sm8650_tcsr_clks,
	.num_clks = ARRAY_SIZE(sm8650_tcsr_clks),
};

static int tcsrcc_sm8650_clk_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	qcom_gate_clk_en(priv, clk->id);

	return 0;
}

static ulong tcsrcc_sm8650_clk_get_rate(struct clk *clk)
{
	return TCXO_RATE;
}

static int tcsrcc_sm8650_clk_probe(struct udevice *dev)
{
	struct msm_clk_data *data = (struct msm_clk_data *)dev_get_driver_data(dev);
	struct msm_clk_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->data = data;

	return 0;
}

static struct clk_ops tcsrcc_sm8650_clk_ops = {
	.enable = tcsrcc_sm8650_clk_enable,
	.get_rate = tcsrcc_sm8650_clk_get_rate,
};

static const struct udevice_id tcsrcc_sm8650_of_match[] = {
	{
		.compatible = "qcom,sm8650-tcsr",
		.data = (ulong)&sm8650_tcsrcc_data,
	},
	{ }
};

U_BOOT_DRIVER(tcsrcc_sm8650) = {
	.name		= "tcsrcc_sm8650",
	.id		= UCLASS_CLK,
	.of_match	= tcsrcc_sm8650_of_match,
	.ops		= &tcsrcc_sm8650_clk_ops,
	.priv_auto	= sizeof(struct msm_clk_priv),
	.probe		= tcsrcc_sm8650_clk_probe,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
