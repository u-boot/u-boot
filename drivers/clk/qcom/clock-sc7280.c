// SPDX-License-Identifier: GPL-2.0
/*
 * Clock drivers for Qualcomm sc7280
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
#include <dt-bindings/clock/qcom,gcc-sc7280.h>

#include "clock-qcom.h"

#define USB30_PRIM_MASTER_CLK_CMD_RCGR 0xf020
#define USB30_PRIM_MOCK_UTMI_CLK_CMD_RCGR 0xf038
#define USB30_SEC_MASTER_CLK_CMD_RCGR 0x9e020
#define USB30_SEC_MOCK_UTMI_CLK_CMD_RCGR 0x9e038
#define PCIE_1_AUX_CLK_CMD_RCGR 0x8d058
#define PCIE1_PHY_RCHNG_CMD_RCGR 0x8d03c
#define PCIE_1_PIPE_CLK_PHY_MUX 0x8d054

static const struct freq_tbl ftbl_gcc_usb30_prim_master_clk_src[] = {
	F(66666667, CFG_CLK_SRC_GPLL0_EVEN, 4.5, 0, 0),
	F(133333333, CFG_CLK_SRC_GPLL0, 4.5, 0, 0),
	F(200000000, CFG_CLK_SRC_GPLL0_ODD, 1, 0, 0),
	F(240000000, CFG_CLK_SRC_GPLL0, 2.5, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_usb30_sec_master_clk_src[] = {
	F(60000000, CFG_CLK_SRC_GPLL0_EVEN, 5, 0, 0),
	F(120000000, CFG_CLK_SRC_GPLL0_EVEN, 2.5, 0, 0),
	{ }
};

static ulong sc7280_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);
	const struct freq_tbl *freq;

	if (clk->id < priv->data->num_clks)
		debug("%s: %s, requested rate=%ld\n", __func__, priv->data->clks[clk->id].name, rate);

	switch (clk->id) {
	case GCC_USB30_PRIM_MASTER_CLK:
		freq = qcom_find_freq(ftbl_gcc_usb30_prim_master_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, USB30_PRIM_MASTER_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 8);
		return freq->freq;
	case GCC_USB30_PRIM_MOCK_UTMI_CLK:
		clk_rcg_set_rate(priv->base, USB30_PRIM_MOCK_UTMI_CLK_CMD_RCGR, 1, 0);
		return 19200000;
	case GCC_USB3_PRIM_PHY_AUX_CLK_SRC:
		clk_rcg_set_rate(priv->base, USB30_PRIM_MOCK_UTMI_CLK_CMD_RCGR, 1, 0);
		return 19200000;
	case GCC_USB30_SEC_MASTER_CLK:
		freq = qcom_find_freq(ftbl_gcc_usb30_sec_master_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, USB30_SEC_MASTER_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 8);
		return freq->freq;
	case GCC_USB30_SEC_MOCK_UTMI_CLK:
		clk_rcg_set_rate(priv->base, USB30_SEC_MOCK_UTMI_CLK_CMD_RCGR, 1, 0);
		return 19200000;
	case GCC_USB3_SEC_PHY_AUX_CLK_SRC:
		clk_rcg_set_rate(priv->base, USB30_PRIM_MOCK_UTMI_CLK_CMD_RCGR, 1, 0);
		return 19200000;
	case GCC_PCIE1_PHY_RCHNG_CLK:
		clk_rcg_set_rate(priv->base, PCIE1_PHY_RCHNG_CMD_RCGR, 5, CFG_CLK_SRC_GPLL0_EVEN);
		return 100000000;
	default:
		return rate;
	}
}

static const struct gate_clk sc7280_clks[] = {
	GATE_CLK(GCC_CFG_NOC_USB3_PRIM_AXI_CLK, 0xf07c, 1),
	GATE_CLK(GCC_USB30_PRIM_MASTER_CLK, 0xf010, 1),
	GATE_CLK(GCC_AGGRE_USB3_PRIM_AXI_CLK, 0xf080, 1),
	GATE_CLK(GCC_USB30_PRIM_SLEEP_CLK, 0xf018, 1),
	GATE_CLK(GCC_USB30_PRIM_MOCK_UTMI_CLK, 0xf01c, 1),
	GATE_CLK(GCC_USB3_PRIM_PHY_AUX_CLK, 0xf054, 1),
	GATE_CLK(GCC_USB3_PRIM_PHY_COM_AUX_CLK, 0xf058, 1),
	GATE_CLK(GCC_CFG_NOC_USB3_SEC_AXI_CLK, 0x9e07c, 1),
	GATE_CLK(GCC_USB30_SEC_MASTER_CLK, 0x9e010, 1),
	GATE_CLK(GCC_AGGRE_USB3_SEC_AXI_CLK, 0x9e080, 1),
	GATE_CLK(GCC_USB30_SEC_SLEEP_CLK, 0x9e018, 1),
	GATE_CLK(GCC_USB30_SEC_MOCK_UTMI_CLK, 0x9e01c, 1),
	GATE_CLK(GCC_USB3_SEC_PHY_AUX_CLK, 0x9e054, 1),
	GATE_CLK(GCC_USB3_SEC_PHY_COM_AUX_CLK, 0x9e058, 1),
	GATE_CLK(GCC_PCIE_CLKREF_EN, 0x8c004, 1),
	GATE_CLK(GCC_PCIE_1_PIPE_CLK, 0x52000, BIT(30)),
	GATE_CLK(GCC_PCIE_1_AUX_CLK, 0x52000, BIT(29)),
	GATE_CLK(GCC_PCIE_1_CFG_AHB_CLK, 0x52000, BIT(28)),
	GATE_CLK(GCC_PCIE_1_MSTR_AXI_CLK, 0x52000, BIT(27)),
	GATE_CLK(GCC_PCIE_1_SLV_AXI_CLK, 0x52000, BIT(26)),
	GATE_CLK(GCC_PCIE_1_SLV_Q2A_AXI_CLK, 0x52000, BIT(25)),
	GATE_CLK(GCC_PCIE1_PHY_RCHNG_CLK, 0x52000, BIT(23)),
	GATE_CLK(GCC_DDRSS_PCIE_SF_CLK, 0x52000, BIT(19)),
	GATE_CLK(GCC_AGGRE_NOC_PCIE_TBU_CLK, 0x52000, BIT(18)),
	GATE_CLK(GCC_AGGRE_NOC_PCIE_1_AXI_CLK, 0x52000, BIT(11)),
	GATE_CLK(GCC_AGGRE_NOC_PCIE_CENTER_SF_AXI_CLK, 0x52008, BIT(28)),
	GATE_CLK(GCC_QUPV3_WRAP0_S0_CLK, 0x52008, BIT(10)),
	GATE_CLK(GCC_QUPV3_WRAP0_S1_CLK, 0x52008, BIT(11)),
	GATE_CLK(GCC_QUPV3_WRAP0_S3_CLK, 0x52008, BIT(13)),
	GATE_CLK(GCC_UFS_PHY_AXI_CLK, 0x77010, BIT(0)),
	GATE_CLK(GCC_AGGRE_UFS_PHY_AXI_CLK, 0x770cc, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_AHB_CLK, 0x77018, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_UNIPRO_CORE_CLK, 0x7705c, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_PHY_AUX_CLK, 0x7709c, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_TX_SYMBOL_0_CLK, 0x7701c, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_RX_SYMBOL_0_CLK, 0x77020, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_RX_SYMBOL_1_CLK, 0x770b8, BIT(0)),
	GATE_CLK(GCC_UFS_1_CLKREF_EN, 0x8c000, BIT(0)),
	GATE_CLK(GCC_SDCC2_AHB_CLK, 0x14008, BIT(0)),
	GATE_CLK(GCC_SDCC2_APPS_CLK, 0x14004, BIT(0)),
};

static int sc7280_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	if (priv->data->num_clks <= clk->id) {
		debug("%s: unknown clk id %lu\n", __func__, clk->id);
		return 0;
	}

	debug("%s: clk %ld: %s\n", __func__, clk->id, sc7280_clks[clk->id].name);

	switch (clk->id) {
	case GCC_AGGRE_USB3_PRIM_AXI_CLK:
		qcom_gate_clk_en(priv, GCC_USB30_PRIM_MASTER_CLK);
		fallthrough;
	case GCC_USB30_PRIM_MASTER_CLK:
		qcom_gate_clk_en(priv, GCC_USB3_PRIM_PHY_AUX_CLK);
		qcom_gate_clk_en(priv, GCC_USB3_PRIM_PHY_COM_AUX_CLK);
		break;
	case GCC_AGGRE_USB3_SEC_AXI_CLK:
		qcom_gate_clk_en(priv, GCC_USB30_SEC_MASTER_CLK);
		fallthrough;
	case GCC_USB30_SEC_MASTER_CLK:
		qcom_gate_clk_en(priv, GCC_USB3_SEC_PHY_AUX_CLK);
		qcom_gate_clk_en(priv, GCC_USB3_SEC_PHY_COM_AUX_CLK);
		break;
	case GCC_PCIE_1_PIPE_CLK:
		clk_phy_mux_enable(priv->base, PCIE_1_PIPE_CLK_PHY_MUX, true);
		break;
	case GCC_PCIE_1_AUX_CLK:
		clk_rcg_set_rate_mnd(priv->base, PCIE_1_AUX_CLK_CMD_RCGR, 1, 0, 0,
				     CFG_CLK_SRC_CXO, 16);
		break;
	case GCC_QUPV3_WRAP0_S0_CLK:
		clk_rcg_set_rate_mnd(priv->base, 0x17010, 1, 0, 0, CFG_CLK_SRC_CXO, 16);
		break;
	case GCC_QUPV3_WRAP0_S1_CLK:
		clk_rcg_set_rate_mnd(priv->base, 0x17140, 1, 0, 0, CFG_CLK_SRC_CXO, 16);
		break;
	case GCC_QUPV3_WRAP0_S3_CLK:
		clk_rcg_set_rate_mnd(priv->base, 0x173a0, 1, 0, 0, CFG_CLK_SRC_CXO, 16);
		break;
	}

	return qcom_gate_clk_en(priv, clk->id);
}

static const struct qcom_reset_map sc7280_gcc_resets[] = {
	[GCC_PCIE_0_BCR] = { 0x6b000 },
	[GCC_PCIE_0_PHY_BCR] = { 0x6c01c },
	[GCC_PCIE_1_BCR] = { 0x8d000 },
	[GCC_PCIE_1_PHY_BCR] = { 0x8e01c },
	[GCC_QUSB2PHY_PRIM_BCR] = { 0x12000 },
	[GCC_QUSB2PHY_SEC_BCR] = { 0x12004 },
	[GCC_SDCC1_BCR] = { 0x75000 },
	[GCC_SDCC2_BCR] = { 0x14000 },
	[GCC_SDCC4_BCR] = { 0x16000 },
	[GCC_UFS_PHY_BCR] = { 0x77000 },
	[GCC_USB30_PRIM_BCR] = { 0xf000 },
	[GCC_USB30_SEC_BCR] = { 0x9e000 },
	[GCC_USB3_DP_PHY_PRIM_BCR] = { 0x50008 },
	[GCC_USB3_PHY_PRIM_BCR] = { 0x50000 },
	[GCC_USB3PHY_PHY_PRIM_BCR] = { 0x50004 },
	[GCC_USB_PHY_CFG_AHB2PHY_BCR] = { 0x6a000 },
};

static const struct qcom_power_map sc7280_gdscs[] = {
	[GCC_UFS_PHY_GDSC] = { 0x77004 },
	[GCC_USB30_PRIM_GDSC] = { 0xf004 },
	[GCC_USB30_SEC_GDSC] = { 0x9e004 },
	[GCC_PCIE_1_GDSC] = { 0x8d004 },
};

static const phys_addr_t sc7280_rcg_addrs[] = {
	0x10f020, // USB30_PRIM_MASTER_CLK_CMD_RCGR
	0x10f038, // USB30_PRIM_MOCK_UTMI_CLK_CMD_RCGR
	0x18d058, // PCIE_1_AUX_CLK_CMD_RCGR
};

static const char *const sc7280_rcg_names[] = {
	"USB30_PRIM_MASTER_CLK_SRC",
	"USB30_PRIM_MOCK_UTMI_CLK_SRC",
	"GCC_PCIE_1_AUX_CLK_SRC",
};

static struct msm_clk_data sc7280_gcc_data = {
	.resets = sc7280_gcc_resets,
	.num_resets = ARRAY_SIZE(sc7280_gcc_resets),
	.clks = sc7280_clks,
	.num_clks = ARRAY_SIZE(sc7280_clks),

	.power_domains = sc7280_gdscs,
	.num_power_domains = ARRAY_SIZE(sc7280_gdscs),

	.enable = sc7280_enable,
	.set_rate = sc7280_set_rate,

	.dbg_rcg_addrs = sc7280_rcg_addrs,
	.num_rcgs = ARRAY_SIZE(sc7280_rcg_addrs),
	.dbg_rcg_names = sc7280_rcg_names,
};

static const struct udevice_id gcc_sc7280_of_match[] = {
	{
		.compatible = "qcom,gcc-sc7280",
		.data = (ulong)&sc7280_gcc_data,
	},
	{ }
};

U_BOOT_DRIVER(gcc_sc7280) = {
	.name		= "gcc_sc7280",
	.id		= UCLASS_NOP,
	.of_match	= gcc_sc7280_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
