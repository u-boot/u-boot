// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm SM7150
 *
 * (C) Copyright 2025 Danila Tikhonov <danila@jiaxyga.com>
 * (C) Copyright 2025 Jens Reidel <adrian@mainlining.org>
 *
 * Based on Linux Kernel driver
 */

#include <clk-uclass.h>
#include <dm.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/bug.h>
#include <linux/bitops.h>
#include <dt-bindings/clock/qcom,sm7150-gcc.h>

#include "clock-qcom.h"

#define USB30_PRIM_MASTER_CLK_CMD_RCGR		0xf01c
#define USB30_PRIM_MOCK_UTMI_CLK_CMD_RCGR	0xf034
#define USB3_PRIM_PHY_AUX_CLK_CMD_RCGR		0xf060

#define SE8_UART_APPS_CMD_RCGR			0x18278
#define GCC_SDCC2_APPS_CLK_SRC_REG		0x1400c

#define APCS_GPLL7_STATUS			0x27000
#define APCS_GPLLX_ENA_REG			0x52000

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

static const struct freq_tbl ftbl_gcc_sdcc2_apps_clk_src[] = {
	F(400000, CFG_CLK_SRC_CXO, 12, 1, 4),
	F(9600000, CFG_CLK_SRC_CXO, 2, 0, 0),
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(25000000, CFG_CLK_SRC_GPLL0_EVEN, 12, 0, 0),
	F(50000000, CFG_CLK_SRC_GPLL0_EVEN, 6, 0, 0),
	F(100000000, CFG_CLK_SRC_GPLL0, 6, 0, 0),
	F(208000000, CFG_CLK_SRC_GPLL7, 4, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_usb30_prim_master_clk_src[] = {
	F(66666667, CFG_CLK_SRC_GPLL0, 4.5, 0, 0),
	F(133333333, CFG_CLK_SRC_GPLL0_EVEN, 4.5, 0, 0),
	F(200000000, CFG_CLK_SRC_GPLL0, 3, 0, 0),
	F(240000000, CFG_CLK_SRC_GPLL0, 2.5, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_usb30_prim_mock_utmi_clk_src[] = {
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(20000000, CFG_CLK_SRC_GPLL0_EVEN, 15, 0, 0),
	F(40000000, CFG_CLK_SRC_GPLL0_EVEN, 7.5, 0, 0),
	F(60000000, CFG_CLK_SRC_GPLL0, 10, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_gcc_usb3_prim_phy_aux_clk_src[] = {
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	{ }
};

static ulong sm7150_clk_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);
	const struct freq_tbl *freq;

	if (clk->id < priv->data->num_clks)
		debug("%s: %s, requested rate=%ld\n", __func__, priv->data->clks[clk->id].name, rate);

	switch (clk->id) {
	case GCC_QUPV3_WRAP1_S2_CLK: /* UART8 */
		freq = qcom_find_freq(ftbl_gcc_qupv3_wrap0_s0_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, SE8_UART_APPS_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 16);
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
		freq = qcom_find_freq(ftbl_gcc_usb3_prim_phy_aux_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, USB3_PRIM_PHY_AUX_CLK_CMD_RCGR,
				     freq->pre_div, freq->m, freq->n, freq->src, 0);
		return freq->freq;
	case GCC_SDCC2_APPS_CLK:
		freq = qcom_find_freq(ftbl_gcc_sdcc2_apps_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, GCC_SDCC2_APPS_CLK_SRC_REG,
				     freq->pre_div, freq->m, freq->n, freq->src, 8);
		return freq->freq;
	default:
		return 0;
	}
}

static const struct gate_clk sm7150_clks[] = {
	GATE_CLK(GCC_AGGRE_UFS_PHY_AXI_CLK,	0x82024, BIT(0)),
	GATE_CLK(GCC_AGGRE_USB3_PRIM_AXI_CLK,	0x8201c, BIT(0)),
	GATE_CLK(GCC_CFG_NOC_USB3_PRIM_AXI_CLK,	0x502c,  BIT(0)),
	GATE_CLK(GCC_QUPV3_WRAP0_S0_CLK,	0x5200c, BIT(10)),
	GATE_CLK(GCC_QUPV3_WRAP0_S1_CLK,	0x5200c, BIT(11)),
	GATE_CLK(GCC_QUPV3_WRAP0_S2_CLK,	0x5200c, BIT(12)),
	GATE_CLK(GCC_QUPV3_WRAP0_S3_CLK,	0x5200c, BIT(13)),
	GATE_CLK(GCC_QUPV3_WRAP0_S4_CLK,	0x5200c, BIT(14)),
	GATE_CLK(GCC_QUPV3_WRAP0_S5_CLK,	0x5200c, BIT(15)),
	GATE_CLK(GCC_QUPV3_WRAP0_S6_CLK,	0x5200c, BIT(16)),
	GATE_CLK(GCC_QUPV3_WRAP0_S7_CLK,	0x5200c, BIT(17)),
	GATE_CLK(GCC_QUPV3_WRAP1_S0_CLK,	0x5200c, BIT(22)),
	GATE_CLK(GCC_QUPV3_WRAP1_S1_CLK,	0x5200c, BIT(23)),
	GATE_CLK(GCC_QUPV3_WRAP1_S2_CLK,	0x5200c, BIT(24)),
	GATE_CLK(GCC_QUPV3_WRAP1_S3_CLK,	0x5200c, BIT(25)),
	GATE_CLK(GCC_QUPV3_WRAP1_S4_CLK,	0x5200c, BIT(26)),
	GATE_CLK(GCC_QUPV3_WRAP1_S5_CLK,	0x5200c, BIT(27)),
	GATE_CLK(GCC_QUPV3_WRAP1_S6_CLK,	0x5200c, BIT(28)),
	GATE_CLK(GCC_QUPV3_WRAP1_S7_CLK,	0x5200c, BIT(29)),
	GATE_CLK(GCC_QUPV3_WRAP_0_M_AHB_CLK,	0x5200c, BIT(6)),
	GATE_CLK(GCC_QUPV3_WRAP_0_S_AHB_CLK,	0x5200c, BIT(7)),
	GATE_CLK(GCC_QUPV3_WRAP_1_M_AHB_CLK,	0x5200c, BIT(20)),
	GATE_CLK(GCC_QUPV3_WRAP_1_S_AHB_CLK,	0x5200c, BIT(21)),
	GATE_CLK(GCC_SDCC1_AHB_CLK,		0x12008, BIT(0)),
	GATE_CLK(GCC_SDCC1_APPS_CLK,		0x1200c, BIT(0)),
	GATE_CLK(GCC_SDCC1_ICE_CORE_CLK,	0x12040, BIT(0)),
	GATE_CLK(GCC_SDCC2_AHB_CLK,		0x14008, BIT(0)),
	GATE_CLK(GCC_SDCC2_APPS_CLK,		0x14004, BIT(0)),
	GATE_CLK(GCC_SDCC4_AHB_CLK,		0x16008, BIT(0)),
	GATE_CLK(GCC_SDCC4_APPS_CLK,		0x16004, BIT(0)),
	GATE_CLK(GCC_UFS_MEM_CLKREF_CLK,	0x8c000, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_AHB_CLK,		0x77014, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_AXI_CLK,		0x77038, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_ICE_CORE_CLK,	0x77090, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_PHY_AUX_CLK,	0x77094, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_RX_SYMBOL_0_CLK,	0x7701c, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_TX_SYMBOL_0_CLK,	0x77018, BIT(0)),
	GATE_CLK(GCC_UFS_PHY_UNIPRO_CORE_CLK,	0x7708c, BIT(0)),
	GATE_CLK(GCC_USB30_PRIM_MASTER_CLK,	0x0f010, BIT(0)),
	GATE_CLK(GCC_USB30_PRIM_MOCK_UTMI_CLK,	0x0f018, BIT(0)),
	GATE_CLK(GCC_USB30_PRIM_SLEEP_CLK,	0x0f014, BIT(0)),
	GATE_CLK(GCC_USB3_PRIM_CLKREF_CLK,	0x8c010, BIT(0)),
	GATE_CLK(GCC_USB3_PRIM_PHY_AUX_CLK,	0x0f050, BIT(0)),
	GATE_CLK(GCC_USB3_PRIM_PHY_COM_AUX_CLK,	0x0f054, BIT(0)),
	GATE_CLK(GCC_USB3_PRIM_PHY_PIPE_CLK,	0x0f058, BIT(0)),
	GATE_CLK(GCC_USB_PHY_CFG_AHB2PHY_CLK,	0x6a004, BIT(0)),
};

static int sm7150_clk_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	if (priv->data->num_clks < clk->id) {
		debug("%s: unknown clk id %lu\n", __func__, clk->id);
		return 0;
	}

	debug("%s: clk %s\n", __func__, sm7150_clks[clk->id].name);

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

static const struct qcom_reset_map sm7150_gcc_resets[] = {
	[GCC_UFS_PHY_BCR] = { 0x77000 },
	[GCC_USB30_PRIM_BCR] = { 0xf000 },
	[GCC_USB3_DP_PHY_PRIM_BCR] = { 0x50008 },
	[GCC_USB3_DP_PHY_SEC_BCR] = { 0x50014 },
	[GCC_USB3_PHY_PRIM_BCR] = { 0x50000 },
	[GCC_USB3_PHY_SEC_BCR] = { 0x5000c },
	[GCC_QUSB2PHY_PRIM_BCR] = { 0x26000 },
};

static const struct qcom_power_map sm7150_gdscs[] = {
	[PCIE_0_GDSC] = { 0x6b004 },
	[UFS_PHY_GDSC] = { 0x77004 },
	[USB30_PRIM_GDSC] = { 0xf004 },
};

static const phys_addr_t sm7150_rcg_addrs[] = {
	0x10f01c, // USB30_PRIM_MASTER_CLK_CMD_RCGR
	0x10f034, // USB30_PRIM_MOCK_UTMI_CLK_CMD_RCGR
	0x10f060, // USB3_PRIM_PHY_AUX_CLK_CMD_RCGR,
};

static const char *const sm7150_rcg_names[] = {
	"USB30_PRIM_MASTER_CLK",
	"USB30_PRIM_MOCK_UTMI_CLK",
	"USB3_PRIM_PHY_AUX_CLK",
};

static struct msm_clk_data sm7150_gcc_data = {
	.resets = sm7150_gcc_resets,
	.num_resets = ARRAY_SIZE(sm7150_gcc_resets),
	.clks = sm7150_clks,
	.num_clks = ARRAY_SIZE(sm7150_clks),

	.power_domains = sm7150_gdscs,
	.num_power_domains = ARRAY_SIZE(sm7150_gdscs),

	.enable = sm7150_clk_enable,
	.set_rate = sm7150_clk_set_rate,

	.dbg_rcg_addrs = sm7150_rcg_addrs,
	.num_rcgs = ARRAY_SIZE(sm7150_rcg_addrs),
	.dbg_rcg_names = sm7150_rcg_names,
};

static const struct udevice_id gcc_sm7150_of_match[] = {
	{ .compatible = "qcom,sm7150-gcc", .data = (ulong)&sm7150_gcc_data, },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(gcc_sm7150) = {
	.name		= "gcc_sm7150",
	.id		= UCLASS_NOP,
	.of_match	= gcc_sm7150_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
