// SPDX-License-Identifier: BSD-3-Clause
/*
 * Global Clock Controller driver for Qualcomm SDM630/636/660
 */

#include <clk-uclass.h>
#include <dm.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/bug.h>
#include <dt-bindings/clock/qcom,gcc-sdm660.h>

#include "clock-qcom.h"

#define GCC_BASE				0x00100000

#define GCC_CFG_NOC_USB3_AXI_CBCR		0x5018
#define GCC_USB30_MASTER_CBCR			0xf008
#define GCC_USB30_SLEEP_CBCR			0xf00c
#define GCC_USB30_MOCK_UTMI_CBCR		0xf010
#define GCC_USB30_MASTER_CMD_RCGR		0xf014
#define GCC_AGGRE2_USB3_AXI_CBCR		0xf03c
#define GCC_USB_PHY_CFG_AHB2PHY_CBCR		0x6a004
#define GCC_SDCC1_APPS_CBCR			0x16004
#define GCC_SDCC1_AHB_CBCR			0x16008
#define GCC_SDCC1_ICE_CORE_CBCR			0x1600c
#define GCC_SDCC1_APPS_CLK_CMD_RCGR		0x1602c
#define GCC_SDCC2_APPS_CBCR			0x14004
#define GCC_SDCC2_AHB_CBCR			0x14008
#define GCC_SDCC2_APPS_CLK_CMD_RCGR		0x14010
#define GCC_BLSP1_AHB_CBCR			0x17004
#define GCC_BLSP1_UART2_APPS_CBCR		0x1c004
#define GCC_BLSP1_UART2_APPS_CLK_CMD_RCGR	0x1c00c
#define GCC_USB3_PHY_AUX_CBCR			0x50000
#define GCC_USB3_PHY_PIPE_CBCR			0x50004
#define GCC_USB3_PHY_AUX_CMD_RCGR		0x5000c

/* vote clocks */
#define GCC_APCS_GPLL_ENA_VOTE			0x52000
#define   GPLL6_ENA_BIT 			BIT(6)
#define   GPLL5_ENA_BIT				BIT(5)
#define   GPLL4_ENA_BIT				BIT(4)
#define   GPLL3_ENA_BIT				BIT(3)
#define   GPLL2_ENA_BIT				BIT(2)
#define   GPLL1_ENA_BIT				BIT(1)
#define   GPLL0_ENA_BIT				BIT(0)
#define GCC_APCS_CLOCK_BRANCH_ENA_VOTE		0x52004
#define   BLSP1_AHB_CLK_ENA_BIT			BIT(17)
#define   BLSP1_SLEEP_CLK_ENA_BIT		BIT(16)
#define GCC_USB3_CLKREF_EN			0x8800c
#define   USB3_ENABLE_BIT			BIT(0)
#define GCC_RX1_USB2_CLKREF_EN			0x88014
#define   RX1_USB2_ENABLE_BIT			BIT(0)
#define GCC_RX0_USB2_CLKREF_EN			0x88018
#define   RX0_USB2_ENABLE_BIT			BIT(0)

/* GPLL mode regs - offset relative to GCC base */
#define GCC_GPLL0_MODE				0x0
#define GCC_GPLL1_MODE				0x1000
#define GCC_GPLL2_MODE				0x2000
#define GCC_GPLL3_MODE				0x3000
#define GCC_GPLL4_MODE				0x77000
#define GCC_GPLL5_MODE				0x74000
/*
 * Linux seems to only use GPLLs 0, 1 and 4. Though, if U-Boot is
 * chainloaded, almost every GPLL0-GPLL5 is already enabled, except GPLL3.
 */

/* blsp1_uart{1,2}_apps_clk_src share the same frequency table */
static const struct freq_tbl ftbl_blsp1_uart1_apps_clk_src[] = {
	F(3686400, CFG_CLK_SRC_GPLL0, 1, 96, 15625),
	F(7372800, CFG_CLK_SRC_GPLL0, 1, 192, 15625),
	F(14745600, CFG_CLK_SRC_GPLL0, 1, 384, 15625),
	F(16000000, CFG_CLK_SRC_GPLL0, 5, 2, 15),
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(24000000, CFG_CLK_SRC_GPLL0, 5, 1, 5),
	F(32000000, CFG_CLK_SRC_GPLL0, 1, 4, 75),
	F(40000000, CFG_CLK_SRC_GPLL0, 15, 0, 0),
	F(46400000, CFG_CLK_SRC_GPLL0, 1, 29, 375),
	F(48000000, CFG_CLK_SRC_GPLL0, 12.5, 0, 0),
	F(51200000, CFG_CLK_SRC_GPLL0, 1, 32, 375),
	F(56000000, CFG_CLK_SRC_GPLL0, 1, 7, 75),
	F(58982400, CFG_CLK_SRC_GPLL0, 1, 1536, 15625),
	F(60000000, CFG_CLK_SRC_GPLL0, 10, 0, 0),
	F(63157895, CFG_CLK_SRC_GPLL0, 9.5, 0, 0),
	{ }
};

/*
 * In Linux's parent_map for this clock:
static const struct parent_map gcc_parent_map_xo_gpll0_gpll0_early_div_gpll4[] = {
	{ P_XO, 0 },
	{ P_GPLL0, 1 },
	{ P_GPLL0_EARLY_DIV, 2 },
	{ P_GPLL4, 5 },
};
*/
#define CFG_CLK_SRC_GPLL0_EARLY_DIV_SDCC2 (2 << 8)

static const struct freq_tbl ftbl_sdcc2_apps_clk_src[] = {
	F(144000, CFG_CLK_SRC_CXO, 16, 3, 25),
	F(400000, CFG_CLK_SRC_CXO, 12, 1, 4),
	F(20000000, CFG_CLK_SRC_GPLL0_EARLY_DIV_SDCC2, 5, 1, 3),
	F(25000000, CFG_CLK_SRC_GPLL0_EARLY_DIV_SDCC2, 6, 1, 2),
	F(50000000, CFG_CLK_SRC_GPLL0_EARLY_DIV_SDCC2, 6, 0, 0),
	F(100000000, CFG_CLK_SRC_GPLL0, 6, 0, 0),
	F(192000000, CFG_CLK_SRC_GPLL4, 8, 0, 0),
	F(200000000, CFG_CLK_SRC_GPLL0, 3, 0, 0),
	{ }
};

/* In Linux's parent_map for this clock:
static const struct parent_map gcc_parent_map_xo_gpll0_gpll0_early_div[] = {
	{ P_XO, 0 },
	{ P_GPLL0, 1 },
	{ P_GPLL0_EARLY_DIV, 6 },
};
*/
#define CFG_CLK_SRC_GPLL0_EARLY_DIV_USB30_MASTER (6 << 8)

static const struct freq_tbl ftbl_usb30_master_clk_src[] = {
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	F(66666667, CFG_CLK_SRC_GPLL0_EARLY_DIV_USB30_MASTER, 4.5, 0, 0),
	F(120000000, CFG_CLK_SRC_GPLL0, 5, 0, 0),
	F(133333333, CFG_CLK_SRC_GPLL0, 4.5, 0, 0),
	F(150000000, CFG_CLK_SRC_GPLL0, 4, 0, 0),
	F(200000000, CFG_CLK_SRC_GPLL0, 3, 0, 0),
	F(240000000, CFG_CLK_SRC_GPLL0, 2.5, 0, 0),
	{ }
};

static const struct freq_tbl ftbl_usb3_phy_aux_clk_src[] = {
	F(1200000, CFG_CLK_SRC_CXO, 16, 0, 0),
	F(19200000, CFG_CLK_SRC_CXO, 1, 0, 0),
	{ }
};

static const struct pll_vote_clk gpll0_clk = {
	.status = GCC_GPLL0_MODE,
	.status_bit = BIT(31),
	.ena_vote = GCC_APCS_GPLL_ENA_VOTE,
	.vote_bit = GPLL0_ENA_BIT,
};

static const struct pll_vote_clk gpll4_clk = {
	.status = GCC_GPLL4_MODE,
	.status_bit = BIT(31),
	.ena_vote = GCC_APCS_GPLL_ENA_VOTE,
	.vote_bit = GPLL4_ENA_BIT,
};

static const struct gate_clk sdm660_clks[] = {
	GATE_CLK(GCC_AGGRE2_USB3_AXI_CLK, GCC_AGGRE2_USB3_AXI_CBCR, BIT(0)),
	GATE_CLK(GCC_BLSP1_AHB_CLK, GCC_APCS_CLOCK_BRANCH_ENA_VOTE, BLSP1_AHB_CLK_ENA_BIT),
	GATE_CLK(GCC_BLSP1_UART2_APPS_CLK, GCC_BLSP1_UART2_APPS_CBCR, BIT(0)),
	GATE_CLK(GCC_CFG_NOC_USB3_AXI_CLK, GCC_CFG_NOC_USB3_AXI_CBCR, BIT(0)),
	GATE_CLK(GCC_SDCC1_AHB_CLK, GCC_SDCC1_AHB_CBCR, BIT(0)),
	GATE_CLK(GCC_SDCC1_APPS_CLK, GCC_SDCC1_APPS_CBCR, BIT(0)),
	GATE_CLK(GCC_SDCC1_ICE_CORE_CLK, GCC_SDCC1_ICE_CORE_CBCR, BIT(0)),
	GATE_CLK(GCC_SDCC2_AHB_CLK, GCC_SDCC2_AHB_CBCR, BIT(0)),
	GATE_CLK(GCC_SDCC2_APPS_CLK, GCC_SDCC2_APPS_CBCR, BIT(0)),
	GATE_CLK(GCC_USB30_MASTER_CLK, GCC_USB30_MASTER_CBCR, BIT(0)),
	GATE_CLK(GCC_USB30_MOCK_UTMI_CLK, GCC_USB30_MOCK_UTMI_CBCR, BIT(0)),
	GATE_CLK(GCC_USB30_SLEEP_CLK, GCC_USB30_SLEEP_CBCR, BIT(0)),
	GATE_CLK(GCC_USB3_CLKREF_CLK, GCC_USB3_CLKREF_EN, USB3_ENABLE_BIT),
	GATE_CLK(GCC_USB3_PHY_AUX_CLK, GCC_USB3_PHY_AUX_CBCR, BIT(0)),
	GATE_CLK(GCC_USB3_PHY_PIPE_CLK, GCC_USB3_PHY_PIPE_CBCR, BIT(0)),
	GATE_CLK(GCC_USB_PHY_CFG_AHB2PHY_CLK, GCC_USB_PHY_CFG_AHB2PHY_CBCR, BIT(0)),
	GATE_CLK(GCC_RX0_USB2_CLKREF_CLK, GCC_RX0_USB2_CLKREF_EN, RX0_USB2_ENABLE_BIT),
	GATE_CLK(GCC_RX1_USB2_CLKREF_CLK, GCC_RX1_USB2_CLKREF_EN, RX1_USB2_ENABLE_BIT),
};

static ulong sdm660_gcc_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);
	const struct freq_tbl *ftbl_entry;

	debug("%s: clk %s rate %lu\n", __func__, sdm660_clks[clk->id].name, rate);

	switch (clk->id) {
	case GCC_BLSP1_UART2_APPS_CLK:
		ftbl_entry = qcom_find_freq(ftbl_blsp1_uart1_apps_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, GCC_BLSP1_UART2_APPS_CLK_CMD_RCGR,
				     ftbl_entry->pre_div, ftbl_entry->m, ftbl_entry->n,
				     ftbl_entry->src, 8);
		return ftbl_entry->freq;
	case GCC_SDCC2_APPS_CLK:
		ftbl_entry = qcom_find_freq(ftbl_sdcc2_apps_clk_src, rate);
		/* we probably should enable source PLL for the selected frequency */
		switch (ftbl_entry->src) {
		case CFG_CLK_SRC_GPLL0:
		case CFG_CLK_SRC_GPLL0_EARLY_DIV_SDCC2:
			clk_enable_gpll0(priv->base, &gpll0_clk);
			break;
		case CFG_CLK_SRC_GPLL4:
			clk_enable_gpll0(priv->base, &gpll4_clk);
			break;
		}
		clk_rcg_set_rate_mnd(priv->base, GCC_SDCC2_APPS_CLK_CMD_RCGR,
				     ftbl_entry->pre_div, ftbl_entry->m, ftbl_entry->n,
				     ftbl_entry->src, 8);
		return ftbl_entry->freq;
	case GCC_SDCC1_APPS_CLK:
		/* The firmware turns this on and sets it to 384 MHz, sourced from GPLL4 */
		return 384000000;
	case GCC_USB30_MASTER_CLK:
		ftbl_entry = qcom_find_freq(ftbl_usb30_master_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, GCC_USB30_MASTER_CMD_RCGR,
			ftbl_entry->pre_div, ftbl_entry->m, ftbl_entry->n,
			ftbl_entry->src, 8);
		return ftbl_entry->freq;
	case GCC_USB30_MOCK_UTMI_CLK:
		return rate;
	case GCC_USB3_PHY_AUX_CLK:
		ftbl_entry = qcom_find_freq(ftbl_usb3_phy_aux_clk_src, rate);
		clk_rcg_set_rate_mnd(priv->base, GCC_USB3_PHY_AUX_CMD_RCGR,
			ftbl_entry->pre_div, ftbl_entry->m, ftbl_entry->n,
			ftbl_entry->src, 8);
		return ftbl_entry->freq;
	default:
		debug("clock-sdm660: can't set rate for unknown clock ID %lu\n", clk->id);
		return 0;
	}
}

static int sdm660_gcc_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	if (priv->data->num_clks < clk->id) {
		debug("unknown clk id %lu\n", clk->id);
		return 0;
	}

	debug("enable clk %s\n", sdm660_clks[clk->id].name);

	return qcom_gate_clk_en(priv, clk->id);
}

static const struct qcom_reset_map sdm660_gcc_resets[] = {
	[GCC_QUSB2PHY_PRIM_BCR] = { 0x12000 },
	[GCC_QUSB2PHY_SEC_BCR] = { 0x12004 },
	[GCC_UFS_BCR] = { 0x75000 },
	[GCC_USB3_DP_PHY_BCR] = { 0x50028 },
	[GCC_USB3_PHY_BCR] = { 0x50020 },
	[GCC_USB3PHY_PHY_BCR] = { 0x50024 },
	[GCC_USB_20_BCR] = { 0x2f000 },
	[GCC_USB_30_BCR] = { 0xf000 },
	[GCC_USB_PHY_CFG_AHB2PHY_BCR] = { 0x6a000 },
	/*
	 * Linux dt-bindings (and sdm630 dtsi) don't have the defines for
	 * (or uses of) GCC_SDCCn_BCR resets, so this won't compile,
	 * but the hardware exists and their offsets are:
	 */
	/* [GCC_SDCC1_BCR] = { 0x16000 }, */
	/* [GCC_SDCC2_BCR] = { 0x14000 }, */
};

static const struct qcom_power_map sdm660_gdscs[] = {
	[USB_30_GDSC] = { .reg = 0xf004, .flags = VOTABLE },
};

static const phys_addr_t __maybe_unused sdm660_gpll_addrs[] = {
	GCC_BASE + GCC_GPLL0_MODE, GCC_BASE + GCC_GPLL1_MODE,
	GCC_BASE + GCC_GPLL2_MODE, GCC_BASE + GCC_GPLL3_MODE,
	GCC_BASE + GCC_GPLL4_MODE, GCC_BASE + GCC_GPLL5_MODE,
};

static const phys_addr_t sdm660_rcg_addrs[] = {
	GCC_BASE + GCC_USB30_MASTER_CMD_RCGR,
	GCC_BASE + GCC_USB3_PHY_AUX_CMD_RCGR,
	GCC_BASE + GCC_SDCC1_APPS_CLK_CMD_RCGR,
	GCC_BASE + GCC_SDCC2_APPS_CLK_CMD_RCGR,
	GCC_BASE + GCC_BLSP1_UART2_APPS_CLK_CMD_RCGR,
};

static const char *const sdm660_rcg_names[] = {
	"GCC_USB30_MASTER_CLK",
	"GCC_USB3_PHY_AUX_CLK",
	"GCC_SDCC1_APPS_CLK",
	"GCC_SDCC2_APPS_CLK",
	"GCC_BLSP1_UART2_APPS_CLK",
};

static struct msm_clk_data sdm660_gcc_data = {
	.resets = sdm660_gcc_resets,
	.num_resets = ARRAY_SIZE(sdm660_gcc_resets),
	.clks = sdm660_clks,
	.num_clks = ARRAY_SIZE(sdm660_clks),
	.power_domains = sdm660_gdscs,
	.num_power_domains = ARRAY_SIZE(sdm660_gdscs),

	.enable = sdm660_gcc_enable,
	.set_rate = sdm660_gcc_set_rate,

	/* some data for debugging, `clk dump` command */
	//.dbg_pll_addrs = sdm660_gpll_addrs,
	//.num_plls = ARRAY_SIZE(sdm660_gpll_addrs),
	/* Sadly, dump_gplls() in clock-qcom.c is not suitable for our PLL type. */
	.dbg_rcg_addrs = sdm660_rcg_addrs,
	.dbg_rcg_names = sdm660_rcg_names,
	.num_rcgs = ARRAY_SIZE(sdm660_rcg_addrs),
};

static const struct udevice_id gcc_sdm660_of_match[] = {
	{
		.compatible = "qcom,gcc-sdm660",
		.data = (ulong)&sdm660_gcc_data,
	},
	{}
};

U_BOOT_DRIVER(gcc_sdm660) = {
	.name = "gcc_sdm660",
	.id = UCLASS_NOP,
	.of_match = gcc_sdm660_of_match,
	.bind = qcom_cc_bind,
	.flags = DM_FLAG_PRE_RELOC,
};
