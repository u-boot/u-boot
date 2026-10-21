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

#define USB30_PRIM_MOCK_UTMI_CLK_CMD_RCGR	0x1b040
#define USB30_PRIM_MASTER_CLK_CMD_RCGR		0x1b028
#define USB3_PRIM_PHY_AUX_CMD_RCGR		0x1b06c
#define USB20_MASTER_CLK_CMD_RCGR		0x1c028
#define USB20_MOCK_UTMI_CLK_CMD_RCGR		0x1c040

#define UFS_PHY_AXI_CLK_CMD_RCGR		0x8302c
#define UFS_PHY_ICE_CORE_CLK_CMD_RCGR		0x83074
#define UFS_PHY_PHY_AUX_CLK_CMD_RCGR		0x830a8
#define UFS_PHY_UNIPRO_CORE_CLK_CMD_RCGR	0x8308c

#define SDCC1_APPS_CLK_CMD_RCGR			0x20014
#define SDCC1_ICE_CORE_CLK_CMD_RCGR		0x2002c

#define QUPV3_WRAP0_S0_CLK_CMD_RCGR		0x23154
#define QUPV3_WRAP0_S1_CLK_CMD_RCGR		0x23288
#define QUPV3_WRAP0_S2_CLK_CMD_RCGR		0x233bc
#define QUPV3_WRAP0_S3_CLK_CMD_RCGR		0x234f0
#define QUPV3_WRAP0_S4_CLK_CMD_RCGR		0x23624
#define QUPV3_WRAP0_S5_CLK_CMD_RCGR		0x23758
#define QUPV3_WRAP0_S6_CLK_CMD_RCGR		0x2388c
#define QUPV3_WRAP0_S7_CLK_CMD_RCGR		0x239c0
#define QUPV3_WRAP1_S0_CLK_CMD_RCGR		0x24154
#define QUPV3_WRAP1_S1_CLK_CMD_RCGR		0x24288
#define QUPV3_WRAP1_S2_CLK_CMD_RCGR		0x243bc
#define QUPV3_WRAP1_S3_CLK_CMD_RCGR		0x244f0
#define QUPV3_WRAP1_S4_CLK_CMD_RCGR		0x24624
#define QUPV3_WRAP1_S5_CLK_CMD_RCGR		0x24758
#define QUPV3_WRAP1_S6_CLK_CMD_RCGR		0x2488c
#define QUPV3_WRAP1_S7_CLK_CMD_RCGR		0x249c0
#define QUPV3_WRAP3_S0_CLK_CMD_RCGR		0xc4158

#define GP1_CLK_CMD_RCGR			0x70004
#define GP2_CLK_CMD_RCGR			0x71004
#define GP3_CLK_CMD_RCGR			0x62004
#define GP4_CLK_CMD_RCGR			0x1e004
#define GP5_CLK_CMD_RCGR			0x1f004

#define PDM2_CLK_CMD_RCGR			0x3f010

#define EMAC0_PHY_AUX_CLK_CMD_RCGR		0xb6028
#define EMAC0_PTP_CLK_CMD_RCGR			0xb6060
#define EMAC0_RGMII_CLK_CMD_RCGR		0xb6048

#define PCIE_0_AUX_CLK_CMD_RCGR			0xa9078
#define PCIE_0_PHY_RCHNG_CLK_CMD_RCGR		0xa9054
#define PCIE_1_AUX_CLK_CMD_RCGR			0x77078
#define PCIE_1_PHY_RCHNG_CLK_CMD_RCGR		0x77054

#define CFG_CLK_SRC_GPLL4_UFS_ICE_CORE		(3 << 8)
#define CFG_CLK_SRC_GPLL7_EMAC0			(2 << 8)

static struct pll_vote_clk gpll4_vote_clk = {
	.status = 0x4000,
	.status_bit = BIT(31),
	.ena_vote = 0x4b028,
	.vote_bit = BIT(4),
};

static struct pll_vote_clk gpll7_vote_clk = {
	.status = 0x7000,
	.status_bit = BIT(31),
	.ena_vote = 0x4b028,
	.vote_bit = BIT(7),
};

static struct pll_vote_clk gpll9_vote_clk = {
	.status = 0x9000,
	.status_bit = BIT(31),
	.ena_vote = 0x4b028,
	.vote_bit = BIT(9),
};

static const struct freq_tbl ftbl_gcc_qupv3_wrap_s0_clk_src[] = {
	F(7372800, CFG_CLK_SRC_GPLL0_EVEN, 1, 384, 15625),
	F(14745600, CFG_CLK_SRC_GPLL0_EVEN, 1, 768, 15625),
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(29491200, CFG_CLK_SRC_GPLL0_EVEN, 1, 1536, 15625),
	F(32000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 8, 75),
	F(48000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 4, 25),
	F(64000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 16, 75),
	F(80000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 4, 15),
	F(96000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 8, 25),
	F(120000000, CFG_CLK_SRC_GPLL0, 5, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_qupv3_wrap_s2_clk_src[] = {
	F(7372800, CFG_CLK_SRC_GPLL0_EVEN, 1, 384, 15625),
	F(14745600, CFG_CLK_SRC_GPLL0_EVEN, 1, 768, 15625),
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(29491200, CFG_CLK_SRC_GPLL0_EVEN, 1, 1536, 15625),
	F(32000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 8, 75),
	F(48000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 4, 25),
	F(64000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 16, 75),
	F(80000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 4, 15),
	F(96000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 8, 25),
	F(100000000, CFG_CLK_SRC_GPLL0, 6, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_qupv3_wrap3_s0_clk_src[] = {
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
	F(403200000, CFG_CLK_SRC_GPLL4, 2, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_gp_clk_src[] = {
	F(100000000, CFG_CLK_SRC_GPLL0, 6, 0, 0),
	F(200000000, CFG_CLK_SRC_GPLL0, 3, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_pdm2_clk_src[] = {
	F(60000000, CFG_CLK_SRC_GPLL0, 10, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_emac0_ptp_clk_src[] = {
	F(125000000, CFG_CLK_SRC_GPLL7_EMAC0, 8, 0, 0),
	F(230400000, CFG_CLK_SRC_GPLL4, 3.5, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_emac0_rgmii_clk_src[] = {
	F(5000000, CFG_CLK_SRC_GPLL0_EVEN, 10, 1, 6),
	F(50000000, CFG_CLK_SRC_GPLL0_EVEN, 6, 0, 0),
	F(125000000, CFG_CLK_SRC_GPLL7_EMAC0, 8, 0, 0),
	F(250000000, CFG_CLK_SRC_GPLL7_EMAC0, 4, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_pcie_0_phy_rchng_clk_src[] = {
	F(100000000, CFG_CLK_SRC_GPLL0, 6, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_sdcc1_apps_clk_src[] = {
	F(144000, CFG_CLK_SRC_CXO, 16, 3, 25),
	F(400000, CFG_CLK_SRC_CXO, 12, 1, 4),
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(20000000, CFG_CLK_SRC_GPLL0_EVEN, 5, 1, 3),
	F(25000000, CFG_CLK_SRC_GPLL0_EVEN, 12, 0, 0),
	F(50000000, CFG_CLK_SRC_GPLL0_EVEN, 6, 0, 0),
	F(100000000, CFG_CLK_SRC_GPLL0_EVEN, 3, 0, 0),
	F(192000000, CFG_CLK_SRC_GPLL9, 4, 0, 0),
	F(384000000, CFG_CLK_SRC_GPLL9, 2, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_sdcc1_ice_core_clk_src[] = {
	F(150000000, CFG_CLK_SRC_GPLL0, 4, 0, 0),
	F(300000000, CFG_CLK_SRC_GPLL0, 2, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_ufs_phy_axi_clk_src[] = {
	F(25000000, CFG_CLK_SRC_GPLL0_EVEN, 12, 0, 0),
	F(75000000, CFG_CLK_SRC_GPLL0_EVEN, 4, 0, 0),
	F(150000000, CFG_CLK_SRC_GPLL0, 4, 0, 0),
	F(300000000, CFG_CLK_SRC_GPLL0, 2, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_ufs_phy_ice_core_clk_src[] = {
	F(75000000, CFG_CLK_SRC_GPLL0_EVEN, 4, 0, 0),
	F(201600000, CFG_CLK_SRC_GPLL4_UFS_ICE_CORE, 4, 0, 0),
	F(403200000, CFG_CLK_SRC_GPLL4_UFS_ICE_CORE, 2, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_ufs_phy_unipro_core_clk_src[] = {
	F(75000000, CFG_CLK_SRC_GPLL0_EVEN, 4, 0, 0),
	F(150000000, CFG_CLK_SRC_GPLL0, 4, 0, 0),
	F(300000000, CFG_CLK_SRC_GPLL0, 2, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_usb20_master_clk_src[] = {
	F(120000000, CFG_CLK_SRC_GPLL0, 5, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_usb30_prim_master_clk_src[] = {
	F(133333333, CFG_CLK_SRC_GPLL0, 4.5, 0, 0),
	F(200000000, CFG_CLK_SRC_GPLL0, 3, 0, 0),
	F(240000000, CFG_CLK_SRC_GPLL0, 2.5, 0, 0),
	{ }
};

static ulong qcs8300_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);
	const struct freq_tbl *freq;

	if (clk->id < priv->data->num_clks)
		debug("%s: %s, requested rate=%ld\n",
		      __func__, priv->data->clks[clk->id].name, rate);

	switch (clk->id) {
	case GCC_USB30_PRIM_MOCK_UTMI_CLK:
		WARN(rate != 19200000, "Unexpected rate for USB30_PRIM_MOCK_UTMI_CLK: %lu\n", rate);
		clk_rcg_set_rate(priv->base, USB30_PRIM_MOCK_UTMI_CLK_CMD_RCGR, 0, CFG_CLK_SRC_CXO);
		return rate;
	case GCC_USB30_PRIM_MASTER_CLK:
		freq = qcom_find_freq(ftbl_gcc_usb30_prim_master_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, USB30_PRIM_MASTER_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 8);
		clk_rcg_set_rate(priv->base, USB3_PRIM_PHY_AUX_CMD_RCGR, 0, 0);
		return freq->freq;
	case GCC_USB20_MASTER_CLK:
		freq = qcom_find_freq(ftbl_gcc_usb20_master_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, USB20_MASTER_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 8);
		return freq->freq;
	case GCC_USB20_MOCK_UTMI_CLK:
		WARN(rate != 19200000, "Unexpected rate for USB20_MOCK_UTMI_CLK: %lu\n", rate);
		clk_rcg_set_rate(priv->base, USB20_MOCK_UTMI_CLK_CMD_RCGR, 0, CFG_CLK_SRC_CXO);
		return rate;
	case GCC_UFS_PHY_AXI_CLK:
		freq = qcom_find_freq(ftbl_gcc_ufs_phy_axi_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, UFS_PHY_AXI_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 8);
		return freq->freq;
	case GCC_UFS_PHY_UNIPRO_CORE_CLK:
		freq = qcom_find_freq(ftbl_gcc_ufs_phy_unipro_core_clk_src, rate);
		clk_rcg_set_rate(priv->base, UFS_PHY_UNIPRO_CORE_CLK_CMD_RCGR,
				 freq->pre_div, freq->src);
		return freq->freq;
	case GCC_UFS_PHY_ICE_CORE_CLK:
		clk_enable_gpll0(priv->base, &gpll4_vote_clk);
		freq = qcom_find_freq(ftbl_gcc_ufs_phy_ice_core_clk_src, rate);
		clk_rcg_set_rate(priv->base, UFS_PHY_ICE_CORE_CLK_CMD_RCGR,
				 freq->pre_div, freq->src);
		return freq->freq;
	case GCC_UFS_PHY_PHY_AUX_CLK:
		clk_rcg_set_rate(priv->base, UFS_PHY_PHY_AUX_CLK_CMD_RCGR, 0, CFG_CLK_SRC_CXO);
		return 19200000;
	case GCC_SDCC1_APPS_CLK:
		clk_enable_gpll0(priv->base, &gpll9_vote_clk);
		freq = qcom_find_freq(ftbl_gcc_sdcc1_apps_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, SDCC1_APPS_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 8);
		return freq->freq;
	case GCC_SDCC1_ICE_CORE_CLK:
		freq = qcom_find_freq(ftbl_gcc_sdcc1_ice_core_clk_src, rate);
		clk_rcg_set_rate(priv->base, SDCC1_ICE_CORE_CLK_CMD_RCGR,
				 freq->pre_div, freq->src);
		return freq->freq;
	case GCC_QUPV3_WRAP0_S0_CLK:
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap_s0_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP0_S0_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_QUPV3_WRAP0_S1_CLK:
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap_s0_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP0_S1_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_QUPV3_WRAP0_S2_CLK:
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap_s2_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP0_S2_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_QUPV3_WRAP0_S3_CLK:
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap_s2_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP0_S3_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_QUPV3_WRAP0_S4_CLK:
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap_s2_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP0_S4_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_QUPV3_WRAP0_S5_CLK:
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap_s2_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP0_S5_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_QUPV3_WRAP0_S6_CLK:
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap_s2_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP0_S6_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_QUPV3_WRAP0_S7_CLK:
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap_s2_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP0_S7_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_QUPV3_WRAP1_S0_CLK:
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap_s0_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP1_S0_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_QUPV3_WRAP1_S1_CLK:
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap_s0_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP1_S1_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_QUPV3_WRAP1_S2_CLK:
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap_s2_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP1_S2_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_QUPV3_WRAP1_S3_CLK:
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap_s2_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP1_S3_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_QUPV3_WRAP1_S4_CLK:
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap_s2_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP1_S4_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_QUPV3_WRAP1_S5_CLK:
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap_s2_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP1_S5_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_QUPV3_WRAP1_S6_CLK:
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap_s2_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP1_S6_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_QUPV3_WRAP1_S7_CLK:
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap_s2_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP1_S7_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_QUPV3_WRAP3_S0_CLK:
		clk_enable_gpll0(priv->base, &gpll4_vote_clk);
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap3_s0_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP3_S0_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_GP1_CLK:
		freq = qcom_find_freq(ftbl_gcc_gp_clk_src, rate);
		clk_rcg_set_rate(priv->base, GP1_CLK_CMD_RCGR, freq->pre_div, freq->src);
		return freq->freq;
	case GCC_GP2_CLK:
		freq = qcom_find_freq(ftbl_gcc_gp_clk_src, rate);
		clk_rcg_set_rate(priv->base, GP2_CLK_CMD_RCGR, freq->pre_div, freq->src);
		return freq->freq;
	case GCC_GP3_CLK:
		freq = qcom_find_freq(ftbl_gcc_gp_clk_src, rate);
		clk_rcg_set_rate(priv->base, GP3_CLK_CMD_RCGR, freq->pre_div, freq->src);
		return freq->freq;
	case GCC_GP4_CLK:
		freq = qcom_find_freq(ftbl_gcc_gp_clk_src, rate);
		clk_rcg_set_rate(priv->base, GP4_CLK_CMD_RCGR, freq->pre_div, freq->src);
		return freq->freq;
	case GCC_GP5_CLK:
		freq = qcom_find_freq(ftbl_gcc_gp_clk_src, rate);
		clk_rcg_set_rate(priv->base, GP5_CLK_CMD_RCGR, freq->pre_div, freq->src);
		return freq->freq;
	case GCC_PDM2_CLK:
		freq = qcom_find_freq(ftbl_gcc_pdm2_clk_src, rate);
		clk_rcg_set_rate(priv->base, PDM2_CLK_CMD_RCGR, freq->pre_div, freq->src);
		return freq->freq;
	case GCC_EMAC0_PHY_AUX_CLK:
		clk_rcg_set_rate(priv->base, EMAC0_PHY_AUX_CLK_CMD_RCGR, 0, CFG_CLK_SRC_CXO);
		return 19200000;
	case GCC_EMAC0_PTP_CLK:
		clk_enable_gpll0(priv->base, &gpll7_vote_clk);
		clk_enable_gpll0(priv->base, &gpll4_vote_clk);
		freq = qcom_find_freq(ftbl_gcc_emac0_ptp_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, EMAC0_PTP_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_EMAC0_RGMII_CLK:
		clk_enable_gpll0(priv->base, &gpll7_vote_clk);
		freq = qcom_find_freq(ftbl_gcc_emac0_rgmii_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, EMAC0_RGMII_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_PCIE_0_AUX_CLK:
		clk_rcg_set_rate(priv->base, PCIE_0_AUX_CLK_CMD_RCGR, 0, CFG_CLK_SRC_CXO);
		return 19200000;
	case GCC_PCIE_0_PHY_RCHNG_CLK:
		freq = qcom_find_freq(ftbl_gcc_pcie_0_phy_rchng_clk_src, rate);
		clk_rcg_set_rate(priv->base, PCIE_0_PHY_RCHNG_CLK_CMD_RCGR,
				 freq->pre_div, freq->src);
		return freq->freq;
	case GCC_PCIE_1_AUX_CLK:
		clk_rcg_set_rate(priv->base, PCIE_1_AUX_CLK_CMD_RCGR, 0, CFG_CLK_SRC_CXO);
		return 19200000;
	case GCC_PCIE_1_PHY_RCHNG_CLK:
		freq = qcom_find_freq(ftbl_gcc_pcie_0_phy_rchng_clk_src, rate);
		clk_rcg_set_rate(priv->base, PCIE_1_PHY_RCHNG_CLK_CMD_RCGR,
				 freq->pre_div, freq->src);
		return freq->freq;
	default:
		return 0;
	}
}

static const struct gate_clk qcs8300_clks[] = {
	GATE_CLK_POLLED(GCC_AGGRE_NOC_QUPV3_AXI_CLK, 0x4b000, BIT(28), 0x8e200),
	GATE_CLK_POLLED(GCC_AGGRE_UFS_PHY_AXI_CLK, 0x830d4, BIT(0), 0x830d4),
	GATE_CLK_POLLED(GCC_AGGRE_USB2_PRIM_AXI_CLK, 0x1c05c, BIT(0), 0x1c05c),
	GATE_CLK_POLLED(GCC_AGGRE_USB3_PRIM_AXI_CLK, 0x1b084, BIT(0), 0x1b084),
	GATE_CLK_POLLED(GCC_AHB2PHY0_CLK, 0x76004, BIT(0), 0x76004),
	GATE_CLK_POLLED(GCC_AHB2PHY2_CLK, 0x76008, BIT(0), 0x76008),
	GATE_CLK_POLLED(GCC_AHB2PHY3_CLK, 0x7600c, BIT(0), 0x7600c),
	GATE_CLK_POLLED(GCC_BOOT_ROM_AHB_CLK, 0x4b000, BIT(10), 0x44004),
	GATE_CLK(GCC_CAMERA_HF_AXI_CLK, 0x32010, BIT(0)),
	GATE_CLK(GCC_CAMERA_SF_AXI_CLK, 0x32018, BIT(0)),
	GATE_CLK_POLLED(GCC_CAMERA_THROTTLE_XO_CLK, 0x32024, BIT(0), 0x32024),
	GATE_CLK_POLLED(GCC_CFG_NOC_USB2_PRIM_AXI_CLK, 0x1c060, BIT(0), 0x1c060),
	GATE_CLK_POLLED(GCC_CFG_NOC_USB3_PRIM_AXI_CLK, 0x1b088, BIT(0), 0x1b088),
	GATE_CLK_POLLED(GCC_DDRSS_GPU_AXI_CLK, 0x7d164, BIT(0), 0x7d164),
	GATE_CLK_POLLED(GCC_DISP_HF_AXI_CLK, 0x33010, BIT(0), 0x33010),
	GATE_CLK(GCC_EDP_REF_CLKREF_EN, 0x97448, BIT(0)),
	GATE_CLK_POLLED(GCC_EMAC0_AXI_CLK, 0xb6018, BIT(0), 0xb6018),
	GATE_CLK_POLLED(GCC_EMAC0_PHY_AUX_CLK, 0xb6024, BIT(0), 0xb6024),
	GATE_CLK_POLLED(GCC_EMAC0_PTP_CLK, 0xb6040, BIT(0), 0xb6040),
	GATE_CLK_POLLED(GCC_EMAC0_RGMII_CLK, 0xb6044, BIT(0), 0xb6044),
	GATE_CLK_POLLED(GCC_EMAC0_SLV_AHB_CLK, 0xb6020, BIT(0), 0xb6020),
	GATE_CLK_POLLED(GCC_GP1_CLK, 0x70000, BIT(0), 0x70000),
	GATE_CLK_POLLED(GCC_GP2_CLK, 0x71000, BIT(0), 0x71000),
	GATE_CLK_POLLED(GCC_GP3_CLK, 0x62000, BIT(0), 0x62000),
	GATE_CLK_POLLED(GCC_GP4_CLK, 0x1e000, BIT(0), 0x1e000),
	GATE_CLK_POLLED(GCC_GP5_CLK, 0x1f000, BIT(0), 0x1f000),
	GATE_CLK(GCC_GPU_GPLL0_CLK_SRC, 0x4b000, BIT(15)),
	GATE_CLK(GCC_GPU_GPLL0_DIV_CLK_SRC, 0x4b000, BIT(16)),
	GATE_CLK_POLLED(GCC_GPU_MEMNOC_GFX_CENTER_PIPELINE_CLK, 0x7d160, BIT(0), 0x7d160),
	GATE_CLK_POLLED(GCC_GPU_MEMNOC_GFX_CLK, 0x7d010, BIT(0), 0x7d010),
	GATE_CLK(GCC_GPU_SNOC_DVM_GFX_CLK, 0x7d01c, BIT(0)),
	GATE_CLK_POLLED(GCC_GPU_TCU_THROTTLE_AHB_CLK, 0x7d008, BIT(0), 0x7d008),
	GATE_CLK_POLLED(GCC_GPU_TCU_THROTTLE_CLK, 0x7d014, BIT(0), 0x7d014),
	GATE_CLK_POLLED(GCC_PCIE_0_AUX_CLK, 0x4b010, BIT(16), 0xa9038),
	GATE_CLK_POLLED(GCC_PCIE_0_CFG_AHB_CLK, 0x4b010, BIT(12), 0xa902c),
	GATE_CLK_POLLED(GCC_PCIE_0_MSTR_AXI_CLK, 0x4b010, BIT(11), 0xa9024),
	GATE_CLK_POLLED(GCC_PCIE_0_PHY_AUX_CLK, 0x4b010, BIT(13), 0xa9030),
	GATE_CLK_POLLED(GCC_PCIE_0_PHY_RCHNG_CLK, 0x4b010, BIT(15), 0xa9050),
	GATE_CLK(GCC_PCIE_0_PIPE_CLK, 0x4b010, BIT(14)),
	GATE_CLK(GCC_PCIE_0_PIPEDIV2_CLK, 0x4b018, BIT(22)),
	GATE_CLK_POLLED(GCC_PCIE_0_SLV_AXI_CLK, 0x4b010, BIT(10), 0xa901c),
	GATE_CLK_POLLED(GCC_PCIE_0_SLV_Q2A_AXI_CLK, 0x4b018, BIT(12), 0xa9018),
	GATE_CLK_POLLED(GCC_PCIE_1_AUX_CLK, 0x4b000, BIT(31), 0x77038),
	GATE_CLK_POLLED(GCC_PCIE_1_CFG_AHB_CLK, 0x4b008, BIT(2), 0x7702c),
	GATE_CLK_POLLED(GCC_PCIE_1_MSTR_AXI_CLK, 0x4b008, BIT(1), 0x77024),
	GATE_CLK_POLLED(GCC_PCIE_1_PHY_AUX_CLK, 0x4b008, BIT(3), 0x77030),
	GATE_CLK_POLLED(GCC_PCIE_1_PHY_RCHNG_CLK, 0x4b000, BIT(22), 0x77050),
	GATE_CLK(GCC_PCIE_1_PIPE_CLK, 0x4b008, BIT(4)),
	GATE_CLK(GCC_PCIE_1_PIPEDIV2_CLK, 0x4b018, BIT(16)),
	GATE_CLK_POLLED(GCC_PCIE_1_SLV_AXI_CLK, 0x4b008, BIT(0), 0x7701c),
	GATE_CLK_POLLED(GCC_PCIE_1_SLV_Q2A_AXI_CLK, 0x4b008, BIT(5), 0x77018),
	GATE_CLK(GCC_PCIE_CLKREF_EN, 0x9746c, BIT(0)),
	GATE_CLK_POLLED(GCC_PCIE_THROTTLE_CFG_CLK, 0x4b020, BIT(15), 0xb2034),
	GATE_CLK_POLLED(GCC_PDM2_CLK, 0x3f00c, BIT(0), 0x3f00c),
	GATE_CLK_POLLED(GCC_PDM_AHB_CLK, 0x3f004, BIT(0), 0x3f004),
	GATE_CLK_POLLED(GCC_PDM_XO4_CLK, 0x3f008, BIT(0), 0x3f008),
	GATE_CLK_POLLED(GCC_QMIP_CAMERA_NRT_AHB_CLK, 0x32008, BIT(0), 0x32008),
	GATE_CLK_POLLED(GCC_QMIP_CAMERA_RT_AHB_CLK, 0x3200c, BIT(0), 0x3200c),
	GATE_CLK_POLLED(GCC_QMIP_DISP_AHB_CLK, 0x33008, BIT(0), 0x33008),
	GATE_CLK_POLLED(GCC_QMIP_DISP_ROT_AHB_CLK, 0x3300c, BIT(0), 0x3300c),
	GATE_CLK_POLLED(GCC_QMIP_VIDEO_CVP_AHB_CLK, 0x34008, BIT(0), 0x34008),
	GATE_CLK_POLLED(GCC_QMIP_VIDEO_VCODEC_AHB_CLK, 0x3400c, BIT(0), 0x3400c),
	GATE_CLK_POLLED(GCC_QMIP_VIDEO_VCPU_AHB_CLK, 0x34010, BIT(0), 0x34010),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP0_CORE_2X_CLK, 0x4b008, BIT(9), 0x23018),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP0_CORE_CLK, 0x4b008, BIT(8), 0x2300c),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP0_S0_CLK, 0x4b008, BIT(10), 0x2314c),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP0_S1_CLK, 0x4b008, BIT(11), 0x23280),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP0_S2_CLK, 0x4b008, BIT(12), 0x233b4),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP0_S3_CLK, 0x4b008, BIT(13), 0x234e8),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP0_S4_CLK, 0x4b008, BIT(14), 0x2361c),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP0_S5_CLK, 0x4b008, BIT(15), 0x23750),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP0_S6_CLK, 0x4b008, BIT(16), 0x23884),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP0_S7_CLK, 0x4b008, BIT(17), 0x239b8),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_CORE_2X_CLK, 0x4b008, BIT(18), 0x24018),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_CORE_CLK, 0x4b008, BIT(19), 0x2400c),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_S0_CLK, 0x4b008, BIT(22), 0x2414c),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_S1_CLK, 0x4b008, BIT(23), 0x24280),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_S2_CLK, 0x4b008, BIT(24), 0x243b4),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_S3_CLK, 0x4b008, BIT(25), 0x244e8),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_S4_CLK, 0x4b008, BIT(26), 0x2461c),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_S5_CLK, 0x4b008, BIT(27), 0x24750),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_S6_CLK, 0x4b018, BIT(27), 0x24884),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP1_S7_CLK, 0x4b018, BIT(28), 0x249b8),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP3_CORE_2X_CLK, 0x4b000, BIT(24), 0xc4018),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP3_CORE_CLK, 0x4b000, BIT(23), 0xc400c),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP3_QSPI_CLK, 0x4b000, BIT(26), 0xc4284),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP3_S0_CLK, 0x4b000, BIT(25), 0xc4150),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP_0_M_AHB_CLK, 0x4b008, BIT(6), 0x23004),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP_0_S_AHB_CLK, 0x4b008, BIT(7), 0x23008),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP_1_M_AHB_CLK, 0x4b008, BIT(20), 0x24004),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP_1_S_AHB_CLK, 0x4b008, BIT(21), 0x24008),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP_3_M_AHB_CLK, 0x4b000, BIT(27), 0xc4004),
	GATE_CLK_POLLED(GCC_QUPV3_WRAP_3_S_AHB_CLK, 0x4b000, BIT(20), 0xc4008),
	GATE_CLK_POLLED(GCC_SDCC1_AHB_CLK, 0x2000c, BIT(0), 0x2000c),
	GATE_CLK_POLLED(GCC_SDCC1_APPS_CLK, 0x20004, BIT(0), 0x20004),
	GATE_CLK_POLLED(GCC_SDCC1_ICE_CORE_CLK, 0x20044, BIT(0), 0x20044),
	GATE_CLK(GCC_SGMI_CLKREF_EN, 0x97034, BIT(0)),
	GATE_CLK_POLLED(GCC_UFS_PHY_AHB_CLK, 0x83020, BIT(0), 0x83020),
	GATE_CLK_POLLED(GCC_UFS_PHY_AXI_CLK, 0x83018, BIT(0), 0x83018),
	GATE_CLK_POLLED(GCC_UFS_PHY_ICE_CORE_CLK, 0x8306c, BIT(0), 0x8306c),
	GATE_CLK_POLLED(GCC_UFS_PHY_PHY_AUX_CLK, 0x830a4, BIT(0), 0x830a4),
	GATE_CLK(GCC_UFS_PHY_RX_SYMBOL_0_CLK, 0x83028, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_RX_SYMBOL_1_CLK, 0x830c0, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_TX_SYMBOL_0_CLK, 0x83024, BIT(0)),
	GATE_CLK_POLLED(GCC_UFS_PHY_UNIPRO_CORE_CLK, 0x83064, BIT(0), 0x83064),
	GATE_CLK_POLLED(GCC_USB20_MASTER_CLK, 0x1c018, BIT(0), 0x1c018),
	GATE_CLK_POLLED(GCC_USB20_MOCK_UTMI_CLK, 0x1c024, BIT(0), 0x1c024),
	GATE_CLK_POLLED(GCC_USB20_SLEEP_CLK, 0x1c020, BIT(0), 0x1c020),
	GATE_CLK_POLLED(GCC_USB30_PRIM_MASTER_CLK, 0x1b018, BIT(0), 0x1b018),
	GATE_CLK_POLLED(GCC_USB30_PRIM_MOCK_UTMI_CLK, 0x1b024, BIT(0), 0x1b024),
	GATE_CLK_POLLED(GCC_USB30_PRIM_SLEEP_CLK, 0x1b020, BIT(0), 0x1b020),
	GATE_CLK_POLLED(GCC_USB3_PRIM_PHY_AUX_CLK, 0x1b05c, BIT(0), 0x1b05c),
	GATE_CLK_POLLED(GCC_USB3_PRIM_PHY_COM_AUX_CLK, 0x1b060, BIT(0), 0x1b060),
	GATE_CLK(GCC_USB3_PRIM_PHY_PIPE_CLK, 0x1b064, BIT(0)),
	GATE_CLK(GCC_USB_CLKREF_EN, 0x97468, BIT(0)),
	GATE_CLK_POLLED(GCC_VIDEO_AXI0_CLK, 0x34014, BIT(0), 0x34014),
	GATE_CLK_POLLED(GCC_VIDEO_AXI1_CLK, 0x3401c, BIT(0), 0x3401c),
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
	case GCC_QUPV3_WRAP0_S0_CLK ... GCC_QUPV3_WRAP0_S7_CLK:
		qcom_gate_clk_en(priv, GCC_QUPV3_WRAP_0_M_AHB_CLK);
		qcom_gate_clk_en(priv, GCC_QUPV3_WRAP_0_S_AHB_CLK);
		qcom_gate_clk_en(priv, GCC_QUPV3_WRAP0_CORE_CLK);
		qcom_gate_clk_en(priv, GCC_QUPV3_WRAP0_CORE_2X_CLK);
		break;
	case GCC_QUPV3_WRAP1_S0_CLK ... GCC_QUPV3_WRAP1_S7_CLK:
		qcom_gate_clk_en(priv, GCC_QUPV3_WRAP_1_M_AHB_CLK);
		qcom_gate_clk_en(priv, GCC_QUPV3_WRAP_1_S_AHB_CLK);
		qcom_gate_clk_en(priv, GCC_QUPV3_WRAP1_CORE_CLK);
		qcom_gate_clk_en(priv, GCC_QUPV3_WRAP1_CORE_2X_CLK);
		break;
	case GCC_QUPV3_WRAP3_S0_CLK:
		qcom_gate_clk_en(priv, GCC_QUPV3_WRAP_3_M_AHB_CLK);
		qcom_gate_clk_en(priv, GCC_QUPV3_WRAP_3_S_AHB_CLK);
		qcom_gate_clk_en(priv, GCC_QUPV3_WRAP3_CORE_CLK);
		qcom_gate_clk_en(priv, GCC_QUPV3_WRAP3_CORE_2X_CLK);
		break;
	}

	return qcom_gate_clk_en(priv, clk->id);
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
	[GCC_EMAC0_GDSC] = { 0xb6004 },
	[GCC_PCIE_0_GDSC] = { 0xa9004 },
	[GCC_PCIE_1_GDSC] = { 0x77004 },
	[GCC_UFS_PHY_GDSC] = { 0x83004 },
	[GCC_USB20_PRIM_GDSC] = { 0x1c004 },
	[GCC_USB30_PRIM_GDSC] = { 0x1b004 },
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
