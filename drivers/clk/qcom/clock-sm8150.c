// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm SM8150
 *
 * Volodymyr Babchuk <volodymyr_babchuk@epam.com>
 * Copyright (c) 2024 EPAM Systems.
 *
 * (C) Copyright 2024 Julius Lehmann <lehmanju@devpi.de>
 *
 * Based on U-Boot driver for SM8250. Constants are taken from the Linux driver.
 */

#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <dt-bindings/clock/qcom,gcc-sm8150.h>

#include "clock-qcom.h"

#define EMAC_RGMII_CLK_CMD_RCGR 0x601c
#define QUPV3_WRAP0_S0_CLK_CMD_RCGR 0x18148
#define USB30_PRIM_MASTER_CLK_CMD_RCGR 0xf01c
#define USB30_PRIM_MOCK_UTMI_CLK_CMD_RCGR 0xf034
#define USB30_PRIM_PHY_AUX_CLK_CMD_RCGR 0xf060
#define USB30_SEC_MASTER_CLK_CMD_RCGR 0x1001c
#define USB30_SEC_MOCK_UTMI_CLK_CMD_RCGR 0x10034
#define USB30_SEC_PHY_AUX_CLK_CMD_RCGR 0x10060
#define SDCC2_APPS_CLK_CMD_RCGR 0x1400c

static struct pll_vote_clk gpll7_vote_clk = {
	.status = 0x1a000,
	.status_bit = BIT(31),
	.ena_vote = 0x52000,
	.vote_bit = BIT(7),
};

static const struct freq_tbl ftbl_gcc_qupv3_wrap0_s0_clk_src[] = {
	F(7372800, CFG_CLK_SRC_GPLL0_EVEN, 1, 384, 15625),
	F(14745600, CFG_CLK_SRC_GPLL0_EVEN, 1, 768, 15625),
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(29491200, CFG_CLK_SRC_GPLL0_EVEN, 1, 1536, 15625),
	F(32000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 8, 75),
	F(48000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 4, 25),
	F(64000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 16, 75),
	F(80000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 4, 15),
	F(96000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 8, 25),
	F(100000000, CFG_CLK_SRC_GPLL0_EVEN, 3, 0, 0),
	F(102400000, CFG_CLK_SRC_GPLL0_EVEN, 1, 128, 375),
	F(112000000, CFG_CLK_SRC_GPLL0_EVEN, 1, 28, 75),
	F(117964800, CFG_CLK_SRC_GPLL0_EVEN, 1, 6144, 15625),
	F(120000000, CFG_CLK_SRC_GPLL0_EVEN, 2.5, 0, 0),
	F(128000000, CFG_CLK_SRC_GPLL0, 1, 16, 75),
	{ }
};

static const struct freq_tbl ftbl_gcc_emac_rgmii_clk_src[] = {
	F(2500000, CFG_CLK_SRC_CXO, 1, 25, 192),
	F(5000000, CFG_CLK_SRC_CXO, 1, 25, 96),
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(25000000, CFG_CLK_SRC_GPLL0_EVEN, 12, 0, 0),
	F(50000000, CFG_CLK_SRC_GPLL0_EVEN, 6, 0, 0),
	F(125000000, CFG_CLK_SRC_GPLL7, 4, 0, 0),
	F(250000000, CFG_CLK_SRC_GPLL7, 2, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_usb30_prim_master_clk_src[] = {
	F(33333333, CFG_CLK_SRC_GPLL0_EVEN, 9, 0, 0),
	F(66666667, CFG_CLK_SRC_GPLL0_EVEN, 4.5, 0, 0),
	F(133333333, CFG_CLK_SRC_GPLL0, 4.5, 0, 0),
	F(200000000, CFG_CLK_SRC_GPLL0, 3, 0, 0),
	F(240000000, CFG_CLK_SRC_GPLL0, 2.5, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_usb30_prim_mock_utmi_clk_src[] = {
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(20000000, CFG_CLK_SRC_GPLL0_EVEN, 15, 0, 0),
	F(60000000, CFG_CLK_SRC_GPLL0_EVEN, 5, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_sdcc2_apps_clk_src[] = {
	F(400000, CFG_CLK_SRC_CXO, 12, 1, 4),
	F(9600000, CFG_CLK_SRC_CXO, 2, 0, 0),
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(25000000, CFG_CLK_SRC_GPLL0, 12, 1, 2),
	F(50000000, CFG_CLK_SRC_GPLL0, 12, 0, 0),
	F(100000000, CFG_CLK_SRC_GPLL0, 6, 0, 0),
	F(202000000, CFG_CLK_SRC_GPLL0, 4, 0, 0),
	{ }
};

static ulong sm8150_clk_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);
	const struct freq_tbl *freq;

	switch (clk->id) {
	case GCC_QUPV3_WRAP1_S4_CLK: /* UART2 aka debug-uart */
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap0_s0_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, QUPV3_WRAP0_S0_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
		return freq->freq;
	case GCC_EMAC_RGMII_CLK:
		freq = qcom_find_freq(ftbl_gcc_emac_rgmii_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, EMAC_RGMII_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 8);
		return freq->freq;
	case GCC_USB30_PRIM_MASTER_CLK:
		freq = qcom_find_freq(ftbl_gcc_usb30_prim_master_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, USB30_PRIM_MASTER_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 8);
		return freq->freq;
	case GCC_USB30_PRIM_MOCK_UTMI_CLK:
		freq = qcom_find_freq(ftbl_gcc_usb30_prim_mock_utmi_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, USB30_PRIM_MOCK_UTMI_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 0);
		return freq->freq;
	case GCC_USB3_PRIM_PHY_AUX_CLK_SRC:
		freq = qcom_find_freq(ftbl_gcc_usb30_prim_mock_utmi_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, USB30_PRIM_PHY_AUX_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 0);
		return freq->freq;
	case GCC_USB30_SEC_MASTER_CLK:
		freq = qcom_find_freq(ftbl_gcc_usb30_prim_master_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, USB30_SEC_MASTER_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 8);
		return freq->freq;
	case GCC_USB30_SEC_MOCK_UTMI_CLK:
		freq = qcom_find_freq(ftbl_gcc_usb30_prim_mock_utmi_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, USB30_SEC_MOCK_UTMI_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 0);
		return freq->freq;
	case GCC_USB3_SEC_PHY_AUX_CLK_SRC:
		freq = qcom_find_freq(ftbl_gcc_usb30_prim_mock_utmi_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, USB30_SEC_PHY_AUX_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 0);
		return freq->freq;
	case GCC_SDCC2_APPS_CLK:
		freq = qcom_find_freq(ftbl_gcc_sdcc2_apps_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, SDCC2_APPS_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 8);
		return freq->freq;
	default:
		return 0;
	}
}

static const struct gate_clk sm8150_clks[] = {
	GATE_CLK(GCC_AGGRE_UFS_CARD_AXI_CLK,		0x750c0, 0x00000001),
	GATE_CLK(GCC_AGGRE_UFS_PHY_AXI_CLK,			0x770c0, 0x00000001),
	GATE_CLK(GCC_AGGRE_USB3_PRIM_AXI_CLK,		0xf07c, 0x00000001),
	GATE_CLK(GCC_AGGRE_USB3_SEC_AXI_CLK,		0x1007c, 0x00000001),
	GATE_CLK(GCC_CFG_NOC_USB3_PRIM_AXI_CLK,		0xf078, 0x00000001),
	GATE_CLK(GCC_CFG_NOC_USB3_SEC_AXI_CLK,		0x10078, 0x00000001),
	GATE_CLK(GCC_QUPV3_WRAP0_S0_CLK,		0x5200c, 0x00000400),
	GATE_CLK(GCC_QUPV3_WRAP0_S1_CLK,		0x5200c, 0x00000800),
	GATE_CLK(GCC_QUPV3_WRAP0_S2_CLK,		0x5200c, 0x00001000),
	GATE_CLK(GCC_QUPV3_WRAP0_S3_CLK,		0x5200c, 0x00002000),
	GATE_CLK(GCC_QUPV3_WRAP0_S4_CLK,		0x5200c, 0x00004000),
	GATE_CLK(GCC_QUPV3_WRAP0_S5_CLK,		0x5200c, 0x00008000),
	GATE_CLK(GCC_QUPV3_WRAP1_S0_CLK,		0x5200c, 0x00400000),
	GATE_CLK(GCC_QUPV3_WRAP1_S1_CLK,		0x5200c, 0x00800000),
	GATE_CLK(GCC_QUPV3_WRAP1_S3_CLK,		0x5200c, 0x02000000),
	GATE_CLK(GCC_QUPV3_WRAP1_S4_CLK,		0x5200c, 0x04000000),
	GATE_CLK(GCC_QUPV3_WRAP1_S5_CLK,		0x5200c, 0x08000000),
	GATE_CLK(GCC_QUPV3_WRAP_0_M_AHB_CLK,		0x5200c, 0x00000040),
	GATE_CLK(GCC_QUPV3_WRAP_0_S_AHB_CLK,		0x5200c, 0x00000080),
	GATE_CLK(GCC_QUPV3_WRAP_1_M_AHB_CLK,		0x5200c, 0x00100000),
	GATE_CLK(GCC_QUPV3_WRAP_1_S_AHB_CLK,		0x5200c, 0x00200000),
	GATE_CLK(GCC_SDCC2_AHB_CLK,			0x14008, 0x00000001),
	GATE_CLK(GCC_SDCC2_APPS_CLK,			0x14004, 0x00000001),
	GATE_CLK(GCC_SDCC4_AHB_CLK,			0x16008, 0x00000001),
	GATE_CLK(GCC_SDCC4_APPS_CLK,			0x16004, 0x00000001),
	GATE_CLK(GCC_UFS_CARD_AHB_CLK,			0x75014, 0x00000001),
	GATE_CLK(GCC_UFS_CARD_AXI_CLK,			0x75010, 0x00000001),
	GATE_CLK(GCC_UFS_CARD_CLKREF_CLK,		0x8c004, 0x00000001),
	GATE_CLK(GCC_UFS_CARD_ICE_CORE_CLK,		0x7505c, 0x00000001),
	GATE_CLK(GCC_UFS_CARD_PHY_AUX_CLK,		0x75090, 0x00000001),
	GATE_CLK(GCC_UFS_CARD_RX_SYMBOL_0_CLK,		0x7501c, 0x00000001),
	GATE_CLK(GCC_UFS_CARD_RX_SYMBOL_1_CLK,		0x750ac, 0x00000001),
	GATE_CLK(GCC_UFS_CARD_TX_SYMBOL_0_CLK,		0x75018, 0x00000001),
	GATE_CLK(GCC_UFS_CARD_UNIPRO_CORE_CLK,		0x75058, 0x00000001),
	GATE_CLK(GCC_UFS_MEM_CLKREF_CLK,		0x8c000, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_AHB_CLK,			0x77014, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_AXI_CLK,			0x77010, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_ICE_CORE_CLK,		0x7705c, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_PHY_AUX_CLK,		0x77090, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_RX_SYMBOL_0_CLK,		0x7701c, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_RX_SYMBOL_1_CLK,		0x770ac, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_TX_SYMBOL_0_CLK,		0x77018, 0x00000001),
	GATE_CLK(GCC_UFS_PHY_UNIPRO_CORE_CLK,		0x77058, 0x00000001),
	GATE_CLK(GCC_USB30_PRIM_MASTER_CLK,		0x0f00c, 0x00000001),
	GATE_CLK(GCC_USB30_PRIM_MOCK_UTMI_CLK,		0x0f014, 0x00000001),
	GATE_CLK(GCC_USB30_PRIM_SLEEP_CLK,		0x0f010, 0x00000001),
	GATE_CLK(GCC_USB30_SEC_MASTER_CLK,		0x1000c, 0x00000001),
	GATE_CLK(GCC_USB30_SEC_MOCK_UTMI_CLK,		0x10014, 0x00000001),
	GATE_CLK(GCC_USB30_SEC_SLEEP_CLK,		0x10010, 0x00000001),
	GATE_CLK(GCC_USB3_PRIM_CLKREF_CLK,		0x8c008, 0x00000001),
	GATE_CLK(GCC_USB3_PRIM_PHY_AUX_CLK,		0x0f04c, 0x00000001),
	GATE_CLK(GCC_USB3_PRIM_PHY_COM_AUX_CLK,		0x0f050, 0x00000001),
	GATE_CLK(GCC_USB3_PRIM_PHY_PIPE_CLK,		0x0f054, 0x00000001),
	GATE_CLK(GCC_USB3_SEC_CLKREF_CLK,		0x8c028, 0x00000001),
	GATE_CLK(GCC_USB3_SEC_PHY_AUX_CLK,		0x1004c, 0x00000001),
	GATE_CLK(GCC_USB3_SEC_PHY_PIPE_CLK,		0x10054, 0x00000001),
	GATE_CLK(GCC_USB3_SEC_PHY_COM_AUX_CLK,		0x10050, 0x00000001),
	GATE_CLK(GCC_EMAC_AXI_CLK,			0x06010, 0x00000001),
	GATE_CLK(GCC_EMAC_SLV_AHB_CLK,			0x06014, 0x00000001),
	GATE_CLK(GCC_EMAC_PTP_CLK,			0x06034, 0x00000001),
	GATE_CLK(GCC_EMAC_RGMII_CLK,			0x06018, 0x00000001),
};

static int sm8150_clk_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	if (priv->data->num_clks <= clk->id) {
		debug("%s: unknown clk id %lu\n", __func__, clk->id);
		return 0;
	}

	debug("%s: clk %s\n", __func__, sm8150_clks[clk->id].name);

	switch (clk->id) {
	case GCC_EMAC_RGMII_CLK:
		clk_enable_gpll0(priv->base, &gpll7_vote_clk);
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
	};

	return qcom_gate_clk_en(priv, clk->id);
}

static const struct qcom_reset_map sm8150_gcc_resets[] = {
	[GCC_EMAC_BCR] = { 0x6000 },
	[GCC_GPU_BCR] = { 0x71000 },
	[GCC_MMSS_BCR] = { 0xb000 },
	[GCC_NPU_BCR] = { 0x4d000 },
	[GCC_PCIE_0_BCR] = { 0x6b000 },
	[GCC_PCIE_0_PHY_BCR] = { 0x6c01c },
	[GCC_PCIE_1_BCR] = { 0x8d000 },
	[GCC_PCIE_1_PHY_BCR] = { 0x8e01c },
	[GCC_PCIE_PHY_BCR] = { 0x6f000 },
	[GCC_PDM_BCR] = { 0x33000 },
	[GCC_PRNG_BCR] = { 0x34000 },
	[GCC_QSPI_BCR] = { 0x24008 },
	[GCC_QUPV3_WRAPPER_0_BCR] = { 0x17000 },
	[GCC_QUPV3_WRAPPER_1_BCR] = { 0x18000 },
	[GCC_QUPV3_WRAPPER_2_BCR] = { 0x1e000 },
	[GCC_QUSB2PHY_PRIM_BCR] = { 0x12000 },
	[GCC_QUSB2PHY_SEC_BCR] = { 0x12004 },
	[GCC_USB3_PHY_PRIM_BCR] = { 0x50000 },
	[GCC_USB3_DP_PHY_PRIM_BCR] = { 0x50008 },
	[GCC_USB3_PHY_SEC_BCR] = { 0x5000c },
	[GCC_USB3PHY_PHY_SEC_BCR] = { 0x50010 },
	[GCC_SDCC2_BCR] = { 0x14000 },
	[GCC_SDCC4_BCR] = { 0x16000 },
	[GCC_TSIF_BCR] = { 0x36000 },
	[GCC_UFS_CARD_BCR] = { 0x75000 },
	[GCC_UFS_PHY_BCR] = { 0x77000 },
	[GCC_USB30_PRIM_BCR] = { 0xf000 },
	[GCC_USB30_SEC_BCR] = { 0x10000 },
	[GCC_USB_PHY_CFG_AHB2PHY_BCR] = { 0x6a000 },
};

static const struct qcom_power_map sm8150_gcc_power_domains[] = {
	[EMAC_GDSC] = { 0x6004 },
	[PCIE_0_GDSC] = { 0x6b004 },
	[PCIE_1_GDSC] = { 0x8d004 },
	[UFS_CARD_GDSC] = { 0x75004 },
	[UFS_PHY_GDSC] = { 0x77004 },
	[USB30_PRIM_GDSC] = { 0xf004 },
	[USB30_SEC_GDSC] = { 0x10004 },
};

static struct msm_clk_data sm8150_clk_data = {
	.resets = sm8150_gcc_resets,
	.num_resets = ARRAY_SIZE(sm8150_gcc_resets),
	.clks = sm8150_clks,
	.num_clks = ARRAY_SIZE(sm8150_clks),
	.power_domains = sm8150_gcc_power_domains,
	.num_power_domains = ARRAY_SIZE(sm8150_gcc_power_domains),

	.enable = sm8150_clk_enable,
	.set_rate = sm8150_clk_set_rate,
};

static const struct udevice_id gcc_sm8150_of_match[] = {
	{
		.compatible = "qcom,gcc-sm8150",
		.data = (ulong)&sm8150_clk_data,
	},
	{ }
};

U_BOOT_DRIVER(gcc_sm8150) = {
	.name		= "gcc_sm8150",
	.id             = UCLASS_NOP,
	.of_match	= gcc_sm8150_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC,
};
