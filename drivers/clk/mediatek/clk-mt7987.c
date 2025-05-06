// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek clock driver for MT7987 SoC
 *
 * Copyright (C) 2024 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#include <dm.h>
#include <log.h>
#include <asm/arch-mediatek/reset.h>
#include <asm/io.h>
#include <dt-bindings/clock/mediatek,mt7987-clk.h>
#include <linux/bitops.h>

#include "clk-mtk.h"

#define MT7987_XTAL_RATE	(40 * MHZ)
#define MT7987_CLK_PDN		0x250
#define MT7987_CLK_PDN_EN_WRITE	BIT(31)

#define XTAL_FACTOR(_id, _name, _parent, _mult, _div)                          \
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_XTAL)

#define PLL_FACTOR(_id, _name, _parent, _mult, _div)                           \
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_APMIXED)

#define TOP_FACTOR(_id, _name, _parent, _mult, _div)                           \
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_TOPCKGEN)

#define INFRA_FACTOR(_id, _name, _parent, _mult, _div)                         \
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_INFRASYS)

/* FIXED PLLS */
static const struct mtk_fixed_clk apmixedsys_mtk_plls[] = {
	FIXED_CLK(CLK_APMIXED_MPLL, CLK_XTAL, 416000000),
	FIXED_CLK(CLK_APMIXED_APLL2, CLK_XTAL, 196608000),
	FIXED_CLK(CLK_APMIXED_NET1PLL, CLK_XTAL, 2500000000),
	FIXED_CLK(CLK_APMIXED_NET2PLL, CLK_XTAL, 800000000),
	FIXED_CLK(CLK_APMIXED_WEDMCUPLL, CLK_XTAL, 208000000),
	FIXED_CLK(CLK_APMIXED_SGMPLL, CLK_XTAL, 325000000),
	FIXED_CLK(CLK_APMIXED_ARM_LL, CLK_XTAL, 2000000000),
	FIXED_CLK(CLK_APMIXED_MSDCPLL, CLK_XTAL, 384000000),
};

static const struct mtk_clk_tree mt7987_fixed_pll_clk_tree = {
	.fdivs_offs = ARRAY_SIZE(apmixedsys_mtk_plls),
	.fclks = apmixedsys_mtk_plls,
	.flags = CLK_APMIXED,
	.xtal_rate = 40 * MHZ,
};

static const struct udevice_id mt7987_fixed_pll_compat[] = {
	{ .compatible = "mediatek,mt7987-fixed-plls" },
	{ .compatible = "mediatek,mt7987-apmixedsys" },
	{}
};

static int mt7987_fixed_pll_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt7987_fixed_pll_clk_tree);
}

U_BOOT_DRIVER(mtk_clk_apmixedsys) = {
	.name = "mt7987-clock-fixed-pll",
	.id = UCLASS_CLK,
	.of_match = mt7987_fixed_pll_compat,
	.probe = mt7987_fixed_pll_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_fixed_pll_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

/* TOPCKGEN FIXED DIV */
static const struct mtk_fixed_factor topckgen_mtk_fixed_factors[] = {
	PLL_FACTOR(CLK_TOP_CB_M_D2, "cb_m_d2", CLK_APMIXED_MPLL, 1, 2),
	PLL_FACTOR(CLK_TOP_CB_M_D3, "cb_m_d3", CLK_APMIXED_MPLL, 1, 3),
	PLL_FACTOR(CLK_TOP_M_D3_D2, "m_d3_d2", CLK_APMIXED_MPLL, 1, 6),
	PLL_FACTOR(CLK_TOP_CB_M_D4, "cb_m_d4", CLK_APMIXED_MPLL, 1, 4),
	PLL_FACTOR(CLK_TOP_CB_M_D8, "cb_m_d8", CLK_APMIXED_MPLL, 1, 8),
	PLL_FACTOR(CLK_TOP_M_D8_D2, "m_d8_d2", CLK_APMIXED_MPLL, 1, 16),
	PLL_FACTOR(CLK_TOP_CB_APLL2_D4, "cb_apll2_d4", CLK_APMIXED_APLL2, 1, 4),
	PLL_FACTOR(CLK_TOP_CB_NET1_D3, "cb_net1_d3", CLK_APMIXED_NET1PLL, 1, 3),
	PLL_FACTOR(CLK_TOP_CB_NET1_D4, "cb_net1_d4", CLK_APMIXED_NET1PLL, 1, 4),
	PLL_FACTOR(CLK_TOP_CB_NET1_D5, "cb_net1_d5", CLK_APMIXED_NET1PLL, 1, 5),
	PLL_FACTOR(CLK_TOP_NET1_D5_D2, "net1_d5_d2", CLK_APMIXED_NET1PLL, 1, 10),
	PLL_FACTOR(CLK_TOP_NET1_D5_D4, "net1_d5_d4", CLK_APMIXED_NET1PLL, 1, 20),
	PLL_FACTOR(CLK_TOP_CB_NET1_D7, "cb_net1_d7", CLK_APMIXED_NET1PLL, 1, 7),
	PLL_FACTOR(CLK_TOP_NET1_D7_D2, "net1_d7_d2", CLK_APMIXED_NET1PLL, 1, 14),
	PLL_FACTOR(CLK_TOP_NET1_D7_D4, "net1_d7_d4", CLK_APMIXED_NET1PLL, 1, 28),
	PLL_FACTOR(CLK_TOP_NET1_D8_D2, "net1_d8_d2", CLK_APMIXED_NET1PLL, 1, 16),
	PLL_FACTOR(CLK_TOP_NET1_D8_D4, "net1_d8_d4", CLK_APMIXED_NET1PLL, 1, 32),
	PLL_FACTOR(CLK_TOP_NET1_D8_D8, "net1_d8_d8", CLK_APMIXED_NET1PLL, 1, 64),
	PLL_FACTOR(CLK_TOP_NET1_D8_D16, "net1_d8_d16", CLK_APMIXED_NET1PLL, 1, 128),
	PLL_FACTOR(CLK_TOP_CB_NET2_D2, "cb_net2_d2", CLK_APMIXED_NET2PLL, 1, 2),
	PLL_FACTOR(CLK_TOP_CB_NET2_D4, "cb_net2_d4", CLK_APMIXED_NET2PLL, 1, 4),
	PLL_FACTOR(CLK_TOP_NET2_D4_D4, "net2_d4_d4", CLK_APMIXED_NET2PLL, 1, 16),
	PLL_FACTOR(CLK_TOP_NET2_D4_D8, "net2_d4_d8", CLK_APMIXED_NET2PLL, 1, 32),
	PLL_FACTOR(CLK_TOP_CB_NET2_D6, "cb_net2_d6", CLK_APMIXED_NET2PLL, 1, 6),
	PLL_FACTOR(CLK_TOP_NET2_D7_D2, "net2_d7_d2", CLK_APMIXED_NET2PLL, 1, 14),
	PLL_FACTOR(CLK_TOP_CB_NET2_D8, "cb_net2_d8", CLK_APMIXED_NET2PLL, 1, 8),
	PLL_FACTOR(CLK_TOP_MSDC_D2, "msdc_d2", CLK_APMIXED_MSDCPLL, 1, 2),
	XTAL_FACTOR(CLK_TOP_CB_CKSQ_40M, "cb_cksq_40m", CLK_XTAL, 1, 1),
	TOP_FACTOR(CLK_TOP_CKSQ_40M_D2, "cksq_40m_d2", CLK_TOP_CB_CKSQ_40M, 1, 2),
	TOP_FACTOR(CLK_TOP_CB_RTC_32K, "cb_rtc_32k", CLK_TOP_CB_CKSQ_40M, 1, 1250),
	TOP_FACTOR(CLK_TOP_CB_RTC_32P7K, "cb_rtc_32p7k", CLK_TOP_CB_CKSQ_40M, 1, 1221),
};

/* TOPCKGEN MUX PARENTS */
#define APMIXED_PARENT(_id) PARENT(_id, CLK_PARENT_APMIXED)
#define TOP_PARENT(_id) PARENT(_id, CLK_PARENT_TOPCKGEN)

/* CLK_TOP_NETSYS_SEL (netsys_sel) in topckgen */
static const struct mtk_parent netsys_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_CB_NET2_D2)
};

/* CLK_TOP_NETSYS_500M_SEL (netsys_500m_sel) in topckgen */
static const struct mtk_parent netsys_500m_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_CB_NET1_D5),
	TOP_PARENT(CLK_TOP_NET1_D5_D2)
};

/* CLK_TOP_NETSYS_2X_SEL (netsys_2x_sel) in topckgen */
static const struct mtk_parent netsys_2x_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	APMIXED_PARENT(CLK_APMIXED_NET2PLL)
};

/* CLK_TOP_ETH_GMII_SEL (eth_gmii_sel) in topckgen */
static const struct mtk_parent eth_gmii_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_NET1_D5_D4)
};

/* CLK_TOP_EIP_SEL (eip_sel) in topckgen */
static const struct mtk_parent eip_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_CB_NET1_D3),
	APMIXED_PARENT(CLK_APMIXED_NET2PLL),
	TOP_PARENT(CLK_TOP_CB_NET1_D4),
	TOP_PARENT(CLK_TOP_CB_NET1_D5)
};

/* CLK_TOP_AXI_INFRA_SEL (axi_infra_sel) in topckgen */
static const struct mtk_parent axi_infra_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_NET1_D8_D2)
};

/* CLK_TOP_UART_SEL (uart_sel) in topckgen */
static const struct mtk_parent uart_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_CB_M_D8),
	TOP_PARENT(CLK_TOP_M_D8_D2)
};

/* CLK_TOP_EMMC_250M_SEL (emmc_250m_sel) in topckgen */
static const struct mtk_parent emmc_250m_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_NET1_D5_D2),
	TOP_PARENT(CLK_TOP_NET1_D7_D2)
};

/* CLK_TOP_EMMC_400M_SEL (emmc_400m_sel) in topckgen */
static const struct mtk_parent emmc_400m_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	APMIXED_PARENT(CLK_APMIXED_MSDCPLL),
	TOP_PARENT(CLK_TOP_CB_NET1_D7),
	TOP_PARENT(CLK_TOP_CB_M_D2),
	TOP_PARENT(CLK_TOP_NET1_D7_D2),
	TOP_PARENT(CLK_TOP_CB_NET2_D6)
};

/* CLK_TOP_SPI_SEL (spi_sel) in topckgen */
static const struct mtk_parent spi_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_CB_M_D2),
	TOP_PARENT(CLK_TOP_NET1_D7_D2),
	TOP_PARENT(CLK_TOP_NET1_D8_D2),
	TOP_PARENT(CLK_TOP_CB_NET2_D6),
	TOP_PARENT(CLK_TOP_NET1_D5_D4),
	TOP_PARENT(CLK_TOP_CB_M_D4),
	TOP_PARENT(CLK_TOP_NET1_D8_D4)
};

/* CLK_TOP_NFI_SEL (nfi_sel) in topckgen */
static const struct mtk_parent nfi_parents[] = {
	TOP_PARENT(CLK_TOP_CKSQ_40M_D2),
	TOP_PARENT(CLK_TOP_NET1_D8_D2),
	TOP_PARENT(CLK_TOP_CB_M_D3),
	TOP_PARENT(CLK_TOP_NET1_D5_D4),
	TOP_PARENT(CLK_TOP_CB_M_D4),
	TOP_PARENT(CLK_TOP_NET1_D7_D4),
	TOP_PARENT(CLK_TOP_NET1_D8_D4),
	TOP_PARENT(CLK_TOP_M_D3_D2),
	TOP_PARENT(CLK_TOP_NET2_D7_D2),
	TOP_PARENT(CLK_TOP_CB_M_D8)
};

/* CLK_TOP_PWM_SEL (pwm_sel) in topckgen */
static const struct mtk_parent pwm_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_NET1_D8_D2),
	TOP_PARENT(CLK_TOP_NET1_D5_D4),
	TOP_PARENT(CLK_TOP_CB_M_D4),
	TOP_PARENT(CLK_TOP_M_D8_D2),
	TOP_PARENT(CLK_TOP_CB_RTC_32K)
};

/* CLK_TOP_I2C_SEL (i2c_sel) in topckgen */
static const struct mtk_parent i2c_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_NET1_D5_D4),
	TOP_PARENT(CLK_TOP_CB_M_D4),
	TOP_PARENT(CLK_TOP_NET1_D8_D4)
};

/* CLK_TOP_PCIE_MBIST_250M_SEL (pcie_mbist_250m_sel) in topckgen */
static const struct mtk_parent pcie_mbist_250m_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_NET1_D5_D2)
};

/* CLK_TOP_PEXTP_TL_SEL (pextp_tl_ck_sel) in topckgen */
static const struct mtk_parent pextp_tl_ck_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_CB_NET2_D6),
	TOP_PARENT(CLK_TOP_NET1_D7_D4),
	TOP_PARENT(CLK_TOP_M_D8_D2),
	TOP_PARENT(CLK_TOP_CB_RTC_32K)
};

/* CLK_TOP_AUD_SEL (aud_sel) in topckgen */
static const struct mtk_parent aud_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	APMIXED_PARENT(CLK_APMIXED_APLL2)
};

/* CLK_TOP_A1SYS_SEL (a1sys_sel) in topckgen */
static const struct mtk_parent a1sys_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_CB_APLL2_D4)
};

/* CLK_TOP_AUD_L_SEL (aud_l_sel) in topckgen */
static const struct mtk_parent aud_l_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	APMIXED_PARENT(CLK_APMIXED_APLL2),
	TOP_PARENT(CLK_TOP_M_D8_D2)
};

/* CLK_TOP_USB_PHY_SEL (usb_phy_sel) in topckgen */
static const struct mtk_parent usb_phy_parents[] = {
	TOP_PARENT(CLK_TOP_CKSQ_40M_D2),
	TOP_PARENT(CLK_TOP_M_D8_D2)
};

/* CLK_TOP_SGM_0_SEL (sgm_0_sel) in topckgen */
static const struct mtk_parent sgm_0_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	APMIXED_PARENT(CLK_APMIXED_SGMPLL)
};

/* CLK_TOP_SGM_SBUS_0_SEL (sgm_sbus_0_sel) in topckgen */
static const struct mtk_parent sgm_sbus_0_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_NET1_D8_D4)
};

/* CLK_TOP_SYSAPB_SEL (sysapb_sel) in topckgen */
static const struct mtk_parent sysapb_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_M_D3_D2)
};

/* CLK_TOP_ETH_REFCK_50M_SEL (eth_refck_50m_sel) in topckgen */
static const struct mtk_parent eth_refck_50m_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_NET2_D4_D4)
};

/* CLK_TOP_ETH_SYS_200M_SEL (eth_sys_200m_sel) in topckgen */
static const struct mtk_parent eth_sys_200m_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_CB_NET2_D4)
};

/* CLK_TOP_ETH_XGMII_SEL (eth_xgmii_sel) in topckgen */
static const struct mtk_parent eth_xgmii_parents[] = {
	TOP_PARENT(CLK_TOP_CKSQ_40M_D2),
	TOP_PARENT(CLK_TOP_NET1_D8_D8),
	TOP_PARENT(CLK_TOP_NET1_D8_D16)
};

/* CLK_TOP_DRAMC_MD32_SEL (dramc_md32_sel) in topckgen */
static const struct mtk_parent dramc_md32_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_CB_M_D2),
	APMIXED_PARENT(CLK_APMIXED_WEDMCUPLL)
};

/* CLK_TOP_DA_XTP_GLB_P0_SEL (da_xtp_glb_p0_sel) in topckgen */
static const struct mtk_parent da_xtp_glb_p0_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_CB_NET2_D8)
};

/* CLK_TOP_DA_CKM_XTAL_SEL (da_ckm_xtal_sel) in topckgen */
static const struct mtk_parent da_ckm_xtal_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_M_D8_D2)
};

/* CLK_TOP_ETH_MII_SEL (eth_mii_sel) in topckgen */
static const struct mtk_parent eth_mii_parents[] = {
	TOP_PARENT(CLK_TOP_CKSQ_40M_D2),
	TOP_PARENT(CLK_TOP_NET2_D4_D8)
};

/* CLK_TOP_EMMC_200M_SEL (emmc_200m_sel) in topckgen */
static const struct mtk_parent emmc_200m_parents[] = {
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
	TOP_PARENT(CLK_TOP_MSDC_D2),
	TOP_PARENT(CLK_TOP_NET1_D7_D2),
	TOP_PARENT(CLK_TOP_CB_NET2_D6),
	TOP_PARENT(CLK_TOP_NET1_D7_D4)
};

#define TOP_MUX(_id, _name, _parents, _mux_ofs, _mux_set_ofs, _mux_clr_ofs,    \
		_shift, _width, _gate, _upd_ofs, _upd)                         \
	{                                                                      \
		.id = (_id), .mux_reg = (_mux_ofs),                            \
		.mux_set_reg = (_mux_set_ofs), .mux_clr_reg = (_mux_clr_ofs),  \
		.upd_reg = (_upd_ofs), .upd_shift = (_upd),                    \
		.mux_shift = (_shift), .mux_mask = BIT(_width) - 1,            \
		.gate_reg = (_mux_ofs), .gate_shift = (_gate),                 \
		.parent_flags = (_parents),                                    \
		.num_parents = ARRAY_SIZE(_parents),                           \
		.flags = CLK_MUX_SETCLR_UPD | CLK_PARENT_MIXED,                \
	}

/* TOPCKGEN MUX_GATE */
static const struct mtk_composite topckgen_mtk_muxes[] = {
	TOP_MUX(CLK_TOP_NETSYS_SEL, "netsys_sel", netsys_parents,
		0x000, 0x004, 0x008, 0, 1, 7, 0x1C0, 0),
	TOP_MUX(CLK_TOP_NETSYS_500M_SEL, "netsys_500m_sel",
		netsys_500m_parents, 0x000, 0x004, 0x008, 8, 2, 15, 0x1C0, 1),
	TOP_MUX(CLK_TOP_NETSYS_2X_SEL, "netsys_2x_sel", netsys_2x_parents,
		0x000, 0x004, 0x008, 16, 1, 23, 0x1C0, 2),
	TOP_MUX(CLK_TOP_ETH_GMII_SEL, "eth_gmii_sel", eth_gmii_parents,
		0x000, 0x004, 0x008, 24, 1, 31, 0x1C0, 3),
	TOP_MUX(CLK_TOP_EIP_SEL, "eip_sel", eip_parents,
		0x010, 0x014, 0x018, 0, 3, 7, 0x1C0, 4),
	TOP_MUX(CLK_TOP_AXI_INFRA_SEL, "axi_infra_sel", axi_infra_parents,
		0x010, 0x014, 0x018, 8, 1, 15, 0x1C0, 5),
	TOP_MUX(CLK_TOP_UART_SEL, "uart_sel", uart_parents,
		0x010, 0x014, 0x018, 16, 2, 23, 0x1C0, 6),
	TOP_MUX(CLK_TOP_EMMC_250M_SEL, "emmc_250m_sel", emmc_250m_parents,
		0x010, 0x014, 0x018, 24, 2, 31, 0x1C0, 7),
	TOP_MUX(CLK_TOP_EMMC_400M_SEL, "emmc_400m_sel", emmc_400m_parents,
		0x020, 0x024, 0x028, 0, 3, 7, 0x1C0, 8),
	TOP_MUX(CLK_TOP_SPI_SEL, "spi_sel", spi_parents,
		0x020, 0x024, 0x028, 8, 3, 15, 0x1C0, 9),
	TOP_MUX(CLK_TOP_SPIM_MST_SEL, "spim_mst_sel", spi_parents,
		0x020, 0x024, 0x028, 16, 3, 23, 0x1C0, 10),
	TOP_MUX(CLK_TOP_NFI_SEL, "nfi_sel", nfi_parents,
		0x020, 0x024, 0x028, 24, 4, 31, 0x1C0, 11),
	TOP_MUX(CLK_TOP_PWM_SEL, "pwm_sel", pwm_parents,
		0x030, 0x034, 0x038, 0, 3, 7, 0x1C0, 12),
	TOP_MUX(CLK_TOP_I2C_SEL, "i2c_sel", i2c_parents,
		0x030, 0x034, 0x038, 8, 2, 15, 0x1C0, 13),
	TOP_MUX(CLK_TOP_PCIE_MBIST_250M_SEL, "pcie_mbist_250m_sel",
		pcie_mbist_250m_parents, 0x030, 0x034, 0x038, 16, 1, 23, 0x1C0, 14),
	TOP_MUX(CLK_TOP_PEXTP_TL_SEL, "pextp_tl_ck_sel", pextp_tl_ck_parents,
		0x030, 0x034, 0x038, 24, 3, 31, 0x1C0, 15),
	TOP_MUX(CLK_TOP_PEXTP_TL_P1_SEL, "pextp_tl_ck_p1_sel",
		pextp_tl_ck_parents, 0x040, 0x044, 0x048, 0, 3, 7, 0x1C0, 16),
	TOP_MUX(CLK_TOP_USB_SYS_P1_SEL, "usb_sys_p1_sel", eth_gmii_parents,
		0x040, 0x044, 0x048, 8, 1, 15, 0x1C0, 17),
	TOP_MUX(CLK_TOP_USB_XHCI_P1_SEL, "usb_xhci_p1_sel", eth_gmii_parents,
		0x040, 0x044, 0x048, 16, 1, 23, 0x1C0, 18),
	TOP_MUX(CLK_TOP_AUD_SEL, "aud_sel", aud_parents,
		0x040, 0x044, 0x048, 24, 1, 31, 0x1C0, 19),
	TOP_MUX(CLK_TOP_A1SYS_SEL, "a1sys_sel", a1sys_parents,
		0x050, 0x054, 0x058, 0, 1, 7, 0x1C0, 20),
	TOP_MUX(CLK_TOP_AUD_L_SEL, "aud_l_sel", aud_l_parents,
		0x050, 0x054, 0x058, 8, 2, 15, 0x1C0, 21),
	TOP_MUX(CLK_TOP_A_TUNER_SEL, "a_tuner_sel", a1sys_parents,
		0x050, 0x054, 0x058, 16, 1, 23, 0x1C0, 22),
	TOP_MUX(CLK_TOP_USB_PHY_SEL, "usb_phy_sel", usb_phy_parents,
		0x050, 0x054, 0x058, 24, 1, 31, 0x1C0, 23),
	TOP_MUX(CLK_TOP_SGM_0_SEL, "sgm_0_sel", sgm_0_parents,
		0x060, 0x064, 0x068, 0, 1, 7, 0x1C0, 24),
	TOP_MUX(CLK_TOP_SGM_SBUS_0_SEL, "sgm_sbus_0_sel", sgm_sbus_0_parents,
		0x060, 0x064, 0x068, 8, 1, 15, 0x1C0, 25),
	TOP_MUX(CLK_TOP_SGM_1_SEL, "sgm_1_sel", sgm_0_parents,
		0x060, 0x064, 0x068, 16, 1, 23, 0x1C0, 26),
	TOP_MUX(CLK_TOP_SGM_SBUS_1_SEL, "sgm_sbus_1_sel", sgm_sbus_0_parents,
		0x060, 0x064, 0x068, 24, 1, 31, 0x1C0, 27),
	TOP_MUX(CLK_TOP_SYSAXI_SEL, "sysaxi_sel", axi_infra_parents,
		0x070, 0x074, 0x078, 0, 1, 7, 0x1C0, 28),
	TOP_MUX(CLK_TOP_SYSAPB_SEL, "sysapb_sel", sysapb_parents,
		0x070, 0x074, 0x078, 8, 1, 15, 0x1C0, 29),
	TOP_MUX(CLK_TOP_ETH_REFCK_50M_SEL, "eth_refck_50m_sel",
		eth_refck_50m_parents, 0x070, 0x074, 0x078, 16, 1, 23, 0x1C0, 30),
	TOP_MUX(CLK_TOP_ETH_SYS_200M_SEL, "eth_sys_200m_sel",
		eth_sys_200m_parents, 0x070, 0x074, 0x078, 24, 1, 31, 0x1C4, 0),
	TOP_MUX(CLK_TOP_ETH_SYS_SEL, "eth_sys_sel", pcie_mbist_250m_parents,
		0x080, 0x084, 0x088, 0, 1, 7, 0x1C4, 1),
	TOP_MUX(CLK_TOP_ETH_XGMII_SEL, "eth_xgmii_sel", eth_xgmii_parents,
		0x080, 0x084, 0x088, 8, 2, 15, 0x1C4, 2),
	TOP_MUX(CLK_TOP_DRAMC_SEL, "dramc_sel", usb_phy_parents,
		0x080, 0x084, 0x088, 16, 1, 23, 0x1C4, 3),
	TOP_MUX(CLK_TOP_DRAMC_MD32_SEL, "dramc_md32_sel", dramc_md32_parents,
		0x080, 0x084, 0x088, 24, 2, 31, 0x1C4, 4),
	TOP_MUX(CLK_TOP_INFRA_F26M_SEL, "csw_infra_f26m_sel",
		usb_phy_parents, 0x090, 0x094, 0x098, 0, 1, 7, 0x1C4, 5),
	TOP_MUX(CLK_TOP_PEXTP_P0_SEL, "pextp_p0_sel", usb_phy_parents,
		0x090, 0x094, 0x098, 8, 1, 15, 0x1C4, 6),
	TOP_MUX(CLK_TOP_PEXTP_P1_SEL, "pextp_p1_sel", usb_phy_parents,
		0x090, 0x094, 0x098, 16, 1, 23, 0x1C4, 7),
	TOP_MUX(CLK_TOP_DA_XTP_GLB_P0_SEL, "da_xtp_glb_p0_sel",
		da_xtp_glb_p0_parents, 0x090, 0x094, 0x098, 24, 1, 31, 0x1C4, 8),
	TOP_MUX(CLK_TOP_DA_XTP_GLB_P1_SEL, "da_xtp_glb_p1_sel",
		da_xtp_glb_p0_parents, 0x0A0, 0x0A4, 0x0A8, 0, 1, 7, 0x1C4, 9),
	TOP_MUX(CLK_TOP_CKM_SEL, "ckm_sel", usb_phy_parents,
		0x0A0, 0x0A4, 0x0A8, 8, 1, 15, 0x1C4, 10),
	TOP_MUX(CLK_TOP_DA_CKM_XTAL_SEL, "da_ckm_xtal_sel",
		da_ckm_xtal_parents, 0x0A0, 0x0A4, 0x0A8, 16, 1, 23, 0x1C4, 11),
	TOP_MUX(CLK_TOP_PEXTP_SEL, "pextp_sel", usb_phy_parents,
		0x0A0, 0x0A4, 0x0A8, 24, 1, 31, 0x1C4, 12),
	TOP_MUX(CLK_TOP_ETH_MII_SEL, "eth_mii_sel", eth_mii_parents,
		0x0B0, 0x0B4, 0x0B8, 0, 1, 7, 0x1C4, 13),
	TOP_MUX(CLK_TOP_EMMC_200M_SEL, "emmc_200m_sel", emmc_200m_parents,
		0x0B0, 0x0B4, 0x0B8, 8, 3, 15, 0x1C4, 14),
};

static const struct mtk_clk_tree mt7987_topckgen_clk_tree = {
	.muxes_offs = CLK_TOP_NETSYS_SEL,
	.fdivs = topckgen_mtk_fixed_factors,
	.muxes = topckgen_mtk_muxes,
	.flags = CLK_BYPASS_XTAL | CLK_TOPCKGEN,
	.xtal_rate = MT7987_XTAL_RATE,
};

static const struct udevice_id mt7987_topckgen_compat[] = {
	{ .compatible = "mediatek,mt7987-topckgen" },
	{}
};

static int mt7987_topckgen_probe(struct udevice *dev)
{
	struct mtk_clk_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -ENOENT;

	writel(MT7987_CLK_PDN_EN_WRITE, priv->base + MT7987_CLK_PDN);
	return mtk_common_clk_init(dev, &mt7987_topckgen_clk_tree);
}

U_BOOT_DRIVER(mtk_clk_topckgen) = {
	.name = "mt7987-clock-topckgen",
	.id = UCLASS_CLK,
	.of_match = mt7987_topckgen_compat,
	.probe = mt7987_topckgen_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_topckgen_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

/* INFRASYS MUX PARENTS */

/* CLK_INFRA_MUX_UART0_SEL (infra_mux_uart0_sel) in infracfg */
static const int infra_mux_uart0_parents[] = {
	CLK_TOP_INFRA_F26M_SEL,
	CLK_TOP_UART_SEL
};

/* CLK_INFRA_MUX_UART1_SEL (infra_mux_uart1_sel) in infracfg */
static const int infra_mux_uart1_parents[] = {
	CLK_TOP_INFRA_F26M_SEL,
	CLK_TOP_UART_SEL
};

/* CLK_INFRA_MUX_UART2_SEL (infra_mux_uart2_sel) in infracfg */
static const int infra_mux_uart2_parents[] = {
	CLK_TOP_INFRA_F26M_SEL,
	CLK_TOP_UART_SEL
};

/* CLK_INFRA_MUX_SPI0_SEL (infra_mux_spi0_sel) in infracfg */
static const int infra_mux_spi0_parents[] = {
	CLK_TOP_I2C_SEL,
	CLK_TOP_SPI_SEL
};

/* CLK_INFRA_MUX_SPI1_SEL (infra_mux_spi1_sel) in infracfg */
static const int infra_mux_spi1_parents[] = {
	CLK_TOP_I2C_SEL,
	CLK_TOP_SPIM_MST_SEL
};

/* CLK_INFRA_MUX_SPI2_BCK_SEL (infra_mux_spi2_bck_sel) in infracfg */
static const int infra_mux_spi2_bck_parents[] = {
	CLK_TOP_I2C_SEL,
	CLK_TOP_SPI_SEL
};

/* CLK_INFRA_PWM_BCK_SEL (infra_pwm_bck_sel) in infracfg */
static const int infra_pwm_bck_parents[] = {
	CLK_TOP_CB_RTC_32P7K,
	CLK_TOP_INFRA_F26M_SEL,
	CLK_TOP_SYSAXI_SEL,
	CLK_TOP_PWM_SEL
};

/* CLK_INFRA_PCIE_GFMUX_TL_O_P0_SEL (infra_pcie_gfmux_tl_ck_o_p0_sel) in infracfg */
static const int infra_pcie_gfmux_tl_ck_o_p0_parents[] = {
	CLK_TOP_CB_RTC_32P7K,
	CLK_TOP_INFRA_F26M_SEL,
	CLK_TOP_INFRA_F26M_SEL,
	CLK_TOP_PEXTP_TL_SEL
};

/* CLK_INFRA_PCIE_GFMUX_TL_O_P1_SEL (infra_pcie_gfmux_tl_ck_o_p1_sel) in infracfg */
static const int infra_pcie_gfmux_tl_ck_o_p1_parents[] = {
	CLK_TOP_CB_RTC_32P7K,
	CLK_TOP_INFRA_F26M_SEL,
	CLK_TOP_INFRA_F26M_SEL,
	CLK_TOP_PEXTP_TL_P1_SEL
};

#define INFRA_MUX(_id, _name, _parents, _reg, _shift, _width)                  \
	{                                                                      \
		.id = (_id), .mux_reg = (_reg) + 0x8,                          \
		.mux_clr_reg = (_reg) + 0x4, .mux_set_reg = (_reg) + 0x0,      \
		.mux_shift = (_shift), .mux_mask = BIT(_width) - 1,            \
		.gate_shift = -1, .upd_shift = -1,                             \
		.parent = (_parents), .num_parents = ARRAY_SIZE(_parents),     \
		.flags = CLK_MUX_SETCLR_UPD | CLK_PARENT_TOPCKGEN,             \
	}

/* INFRA MUX */
static const struct mtk_composite infracfg_mtk_mux[] = {
	INFRA_MUX(CLK_INFRA_MUX_UART0_SEL, "infra_mux_uart0_sel",
		  infra_mux_uart0_parents, 0x0010, 0, 1),
	INFRA_MUX(CLK_INFRA_MUX_UART1_SEL, "infra_mux_uart1_sel",
		  infra_mux_uart1_parents, 0x0010, 1, 1),
	INFRA_MUX(CLK_INFRA_MUX_UART2_SEL, "infra_mux_uart2_sel",
		  infra_mux_uart2_parents, 0x0010, 2, 1),
	INFRA_MUX(CLK_INFRA_MUX_SPI0_SEL, "infra_mux_spi0_sel",
		  infra_mux_spi0_parents, 0x0010, 4, 1),
	INFRA_MUX(CLK_INFRA_MUX_SPI1_SEL, "infra_mux_spi1_sel",
		  infra_mux_spi1_parents, 0x0010, 5, 1),
	INFRA_MUX(CLK_INFRA_MUX_SPI2_BCK_SEL, "infra_mux_spi2_bck_sel",
		  infra_mux_spi2_bck_parents, 0x0010, 6, 1),
	INFRA_MUX(CLK_INFRA_PWM_BCK_SEL, "infra_pwm_bck_sel",
		  infra_pwm_bck_parents, 0x0010, 14, 2),
	INFRA_MUX(CLK_INFRA_PCIE_GFMUX_TL_O_P0_SEL, "infra_pcie_gfmux_tl_ck_o_p0_sel",
		  infra_pcie_gfmux_tl_ck_o_p0_parents, 0x0020, 0, 2),
	INFRA_MUX(CLK_INFRA_PCIE_GFMUX_TL_O_P1_SEL, "infra_pcie_gfmux_tl_ck_o_p1_sel",
		  infra_pcie_gfmux_tl_ck_o_p1_parents, 0x0020, 2, 2),
};

static const struct mtk_gate_regs infra_0_cg_regs = {
	.set_ofs = 0x10,
	.clr_ofs = 0x14,
	.sta_ofs = 0x18,
};

static const struct mtk_gate_regs infra_1_cg_regs = {
	.set_ofs = 0x40,
	.clr_ofs = 0x44,
	.sta_ofs = 0x48,
};

static const struct mtk_gate_regs infra_2_cg_regs = {
	.set_ofs = 0x50,
	.clr_ofs = 0x54,
	.sta_ofs = 0x58,
};

static const struct mtk_gate_regs infra_3_cg_regs = {
	.set_ofs = 0x60,
	.clr_ofs = 0x64,
	.sta_ofs = 0x68,
};

#define GATE_INFRA0(_id, _name, _parent, _shift, _flags)                       \
	{                                                                      \
		.id = (_id), .parent = (_parent), .regs = &infra_0_cg_regs,    \
		.shift = (_shift),                                             \
		.flags = (_flags),                                             \
	}
#define GATE_INFRA0_INFRA(_id, _name, _parent, _shift) \
	GATE_INFRA0(_id, _name, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_INFRASYS)
#define GATE_INFRA0_TOP(_id, _name, _parent, _shift) \
	GATE_INFRA0(_id, _name, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN)

#define GATE_INFRA1(_id, _name, _parent, _shift, _flags)                       \
	{                                                                      \
		.id = (_id), .parent = (_parent), .regs = &infra_1_cg_regs,    \
		.shift = (_shift),                                             \
		.flags = (_flags),                                             \
	}
#define GATE_INFRA1_INFRA(_id, _name, _parent, _shift) \
	GATE_INFRA1(_id, _name, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_INFRASYS)
#define GATE_INFRA1_TOP(_id, _name, _parent, _shift) \
	GATE_INFRA1(_id, _name, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN)

#define GATE_INFRA2(_id, _name, _parent, _shift, _flags)                       \
	{                                                                      \
		.id = (_id), .parent = (_parent), .regs = &infra_2_cg_regs,    \
		.shift = (_shift),                                             \
		.flags = (_flags),                                             \
	}
#define GATE_INFRA2_INFRA(_id, _name, _parent, _shift) \
	GATE_INFRA2(_id, _name, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_INFRASYS)
#define GATE_INFRA2_TOP(_id, _name, _parent, _shift) \
	GATE_INFRA2(_id, _name, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN)

#define GATE_INFRA3(_id, _name, _parent, _shift, _flags)                       \
	{                                                                      \
		.id = (_id), .parent = (_parent), .regs = &infra_3_cg_regs,    \
		.shift = (_shift),                                             \
		.flags = (_flags),                                             \
	}
#define GATE_INFRA3_INFRA(_id, _name, _parent, _shift) \
	GATE_INFRA3(_id, _name, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_INFRASYS)
#define GATE_INFRA3_TOP(_id, _name, _parent, _shift) \
	GATE_INFRA3(_id, _name, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN)
#define GATE_INFRA3_XTAL(_id, _name, _parent, _shift) \
	GATE_INFRA3(_id, _name, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_XTAL)

/* INFRA GATE */
static const struct mtk_gate infracfg_mtk_gates[] = {
	GATE_INFRA1_TOP(CLK_INFRA_66M_GPT_BCK,
			"infra_hf_66m_gpt_bck", CLK_TOP_SYSAXI_SEL, 0),
	GATE_INFRA1_TOP(CLK_INFRA_66M_PWM_HCK,
			"infra_hf_66m_pwm_hck", CLK_TOP_SYSAXI_SEL, 1),
	GATE_INFRA1_INFRA(CLK_INFRA_66M_PWM_BCK,
			  "infra_hf_66m_pwm_bck", CLK_INFRA_PWM_BCK_SEL, 2),
	GATE_INFRA1_TOP(CLK_INFRA_133M_CQDMA_BCK,
			"infra_hf_133m_cqdma_bck", CLK_TOP_SYSAXI_SEL, 12),
	GATE_INFRA1_TOP(CLK_INFRA_66M_AUD_SLV_BCK,
			"infra_66m_aud_slv_bck", CLK_TOP_SYSAXI_SEL, 13),
	GATE_INFRA1_TOP(CLK_INFRA_AUD_26M, "infra_f_faud_26m",
			CLK_TOP_INFRA_F26M_SEL, 14),
	GATE_INFRA1_TOP(CLK_INFRA_AUD_L, "infra_f_faud_l", CLK_TOP_AUD_L_SEL, 15),
	GATE_INFRA1_TOP(CLK_INFRA_AUD_AUD, "infra_f_aud_aud", CLK_TOP_A1SYS_SEL, 16),
	GATE_INFRA1_TOP(CLK_INFRA_AUD_EG2, "infra_f_faud_eg2",
			CLK_TOP_A_TUNER_SEL, 18),
	GATE_INFRA1_TOP(CLK_INFRA_DRAMC_F26M, "infra_dramc_f26m",
			CLK_TOP_INFRA_F26M_SEL, 19),
	GATE_INFRA1_TOP(CLK_INFRA_133M_DBG_ACKM,
			"infra_hf_133m_dbg_ackm", CLK_TOP_SYSAXI_SEL, 20),
	GATE_INFRA1_TOP(CLK_INFRA_66M_AP_DMA_BCK,
			"infra_66m_ap_dma_bck", CLK_TOP_SYSAXI_SEL, 21),
	GATE_INFRA1_TOP(CLK_INFRA_MSDC200_SRC, "infra_f_fmsdc200_src",
			CLK_TOP_EMMC_200M_SEL, 28),
	GATE_INFRA1_TOP(CLK_INFRA_66M_SEJ_BCK,
			"infra_hf_66m_sej_bck", CLK_TOP_SYSAXI_SEL, 29),
	GATE_INFRA1_TOP(CLK_INFRA_PRE_CK_SEJ_F13M,
			"infra_pre_ck_sej_f13m", CLK_TOP_INFRA_F26M_SEL, 30),
	GATE_INFRA1_TOP(CLK_INFRA_66M_TRNG, "infra_hf_66m_trng",
			CLK_TOP_SYSAXI_SEL, 31),
	GATE_INFRA2_TOP(CLK_INFRA_26M_THERM_SYSTEM,
			"infra_hf_26m_therm_system", CLK_TOP_INFRA_F26M_SEL, 0),
	GATE_INFRA2_TOP(CLK_INFRA_I2C_BCK, "infra_i2c_bck", CLK_TOP_I2C_SEL, 1),
	GATE_INFRA2_TOP(CLK_INFRA_66M_UART0_PCK,
			"infra_hf_66m_uart0_pck", CLK_TOP_SYSAXI_SEL, 3),
	GATE_INFRA2_TOP(CLK_INFRA_66M_UART1_PCK,
			"infra_hf_66m_uart1_pck", CLK_TOP_SYSAXI_SEL, 4),
	GATE_INFRA2_TOP(CLK_INFRA_66M_UART2_PCK,
			"infra_hf_66m_uart2_pck", CLK_TOP_SYSAXI_SEL, 5),
	GATE_INFRA2_INFRA(CLK_INFRA_52M_UART0_CK,
			  "infra_f_52m_uart0", CLK_INFRA_MUX_UART0_SEL, 3),
	GATE_INFRA2_INFRA(CLK_INFRA_52M_UART1_CK,
			  "infra_f_52m_uart1", CLK_INFRA_MUX_UART1_SEL, 4),
	GATE_INFRA2_INFRA(CLK_INFRA_52M_UART2_CK,
			  "infra_f_52m_uart2", CLK_INFRA_MUX_UART2_SEL, 5),
	GATE_INFRA2_TOP(CLK_INFRA_NFI, "infra_f_fnfi", CLK_TOP_NFI_SEL, 9),
	GATE_INFRA2_TOP(CLK_INFRA_66M_NFI_HCK,
			"infra_hf_66m_nfi_hck", CLK_TOP_SYSAXI_SEL, 11),
	GATE_INFRA2_INFRA(CLK_INFRA_104M_SPI0, "infra_hf_104m_spi0",
			  CLK_INFRA_MUX_SPI0_SEL, 12),
	GATE_INFRA2_INFRA(CLK_INFRA_104M_SPI1, "infra_hf_104m_spi1",
			  CLK_INFRA_MUX_SPI1_SEL, 13),
	GATE_INFRA2_INFRA(CLK_INFRA_104M_SPI2_BCK,
			  "infra_hf_104m_spi2_bck", CLK_INFRA_MUX_SPI2_BCK_SEL, 14),
	GATE_INFRA2_TOP(CLK_INFRA_66M_SPI0_HCK,
			"infra_hf_66m_spi0_hck", CLK_TOP_SYSAXI_SEL, 15),
	GATE_INFRA2_TOP(CLK_INFRA_66M_SPI1_HCK,
			"infra_hf_66m_spi1_hck", CLK_TOP_SYSAXI_SEL, 16),
	GATE_INFRA2_TOP(CLK_INFRA_66M_SPI2_HCK,
			"infra_hf_66m_spi2_hck", CLK_TOP_SYSAXI_SEL, 17),
	GATE_INFRA2_TOP(CLK_INFRA_66M_FLASHIF_AXI,
			"infra_hf_66m_flashif_axi", CLK_TOP_SYSAXI_SEL, 18),
	GATE_INFRA2_TOP(CLK_INFRA_RTC, "infra_f_frtc", CLK_TOP_CB_RTC_32K, 19),
	GATE_INFRA2_TOP(CLK_INFRA_26M_ADC_BCK, "infra_f_26m_adc_bck",
			CLK_TOP_INFRA_F26M_SEL, 20),
	GATE_INFRA2_INFRA(CLK_INFRA_RC_ADC, "infra_f_frc_adc",
			  CLK_INFRA_26M_ADC_BCK, 21),
	GATE_INFRA2_TOP(CLK_INFRA_MSDC400, "infra_f_fmsdc400",
			CLK_TOP_EMMC_400M_SEL, 22),
	GATE_INFRA2_TOP(CLK_INFRA_MSDC2_HCK, "infra_f_fmsdc2_hck",
			CLK_TOP_EMMC_250M_SEL, 23),
	GATE_INFRA2_TOP(CLK_INFRA_133M_MSDC_0_HCK,
			"infra_hf_133m_msdc_0_hck", CLK_TOP_SYSAXI_SEL, 24),
	GATE_INFRA2_TOP(CLK_INFRA_66M_MSDC_0_HCK,
			"infra_66m_msdc_0_hck", CLK_TOP_SYSAXI_SEL, 25),
	GATE_INFRA2_TOP(CLK_INFRA_133M_CPUM_BCK,
			"infra_hf_133m_cpum_bck", CLK_TOP_SYSAXI_SEL, 26),
	GATE_INFRA2_TOP(CLK_INFRA_BIST2FPC, "infra_hf_fbist2fpc",
			CLK_TOP_NFI_SEL, 27),
	GATE_INFRA2_TOP(CLK_INFRA_I2C_X16W_MCK_CK_P1,
			"infra_hf_i2c_x16w_mck_ck_p1", CLK_TOP_SYSAXI_SEL, 29),
	GATE_INFRA2_TOP(CLK_INFRA_I2C_X16W_PCK_CK_P1,
			"infra_hf_i2c_x16w_pck_ck_p1", CLK_TOP_SYSAXI_SEL, 31),
	GATE_INFRA3_TOP(CLK_INFRA_133M_USB_HCK,
			"infra_133m_usb_hck", CLK_TOP_SYSAXI_SEL, 0),
	GATE_INFRA3_TOP(CLK_INFRA_133M_USB_HCK_CK_P1,
			"infra_133m_usb_hck_ck_p1", CLK_TOP_SYSAXI_SEL, 1),
	GATE_INFRA3_TOP(CLK_INFRA_66M_USB_HCK, "infra_66m_usb_hck",
			CLK_TOP_SYSAXI_SEL, 2),
	GATE_INFRA3_TOP(CLK_INFRA_66M_USB_HCK_CK_P1,
			"infra_66m_usb_hck_ck_p1", CLK_TOP_SYSAXI_SEL, 3),
	GATE_INFRA3_TOP(CLK_INFRA_USB_SYS_CK_P1,
			"infra_usb_sys_ck_p1", CLK_TOP_USB_SYS_P1_SEL, 5),
	GATE_INFRA3_TOP(CLK_INFRA_USB_CK_P1, "infra_usb_ck_p1",
			CLK_TOP_CB_CKSQ_40M, 7),
	GATE_INFRA3_TOP(CLK_INFRA_USB_FRMCNT_CK_P1,
			"infra_usb_frmcnt_ck_p1", CLK_TOP_CKSQ_40M_D2, 9),
	GATE_INFRA3_XTAL(CLK_INFRA_USB_PIPE_CK_P1,
			 "infra_usb_pipe_ck_p1", CLK_XTAL, 11),
	GATE_INFRA3_XTAL(CLK_INFRA_USB_UTMI_CK_P1,
			 "infra_usb_utmi_ck_p1", CLK_XTAL, 13),
	GATE_INFRA3_TOP(CLK_INFRA_USB_XHCI_CK_P1,
			"infra_usb_xhci_ck_p1", CLK_TOP_USB_XHCI_P1_SEL, 15),
	GATE_INFRA3_INFRA(CLK_INFRA_PCIE_GFMUX_TL_P0,
			  "infra_pcie_gfmux_tl_ck_p0", CLK_INFRA_PCIE_GFMUX_TL_O_P0_SEL, 20),
	GATE_INFRA3_INFRA(CLK_INFRA_PCIE_GFMUX_TL_P1,
			  "infra_pcie_gfmux_tl_ck_p1", CLK_INFRA_PCIE_GFMUX_TL_O_P1_SEL, 21),
	GATE_INFRA3_XTAL(CLK_INFRA_PCIE_PIPE_P0,
			 "infra_pcie_pipe_ck_p0", CLK_XTAL, 24),
	GATE_INFRA3_XTAL(CLK_INFRA_PCIE_PIPE_P1,
			 "infra_pcie_pipe_ck_p1", CLK_XTAL, 25),
	GATE_INFRA3_TOP(CLK_INFRA_133M_PCIE_CK_P0,
			"infra_133m_pcie_ck_p0", CLK_TOP_SYSAXI_SEL, 28),
	GATE_INFRA3_TOP(CLK_INFRA_133M_PCIE_CK_P1,
			"infra_133m_pcie_ck_p1", CLK_TOP_SYSAXI_SEL, 29),
	GATE_INFRA0_TOP(CLK_INFRA_PCIE_PERI_26M_CK_P0,
			"infra_pcie_peri_ck_26m_ck_p0", CLK_TOP_INFRA_F26M_SEL, 7),
	GATE_INFRA0_TOP(CLK_INFRA_PCIE_PERI_26M_CK_P1,
			"infra_pcie_peri_ck_26m_ck_p1", CLK_TOP_INFRA_F26M_SEL, 8),
};

static const struct mtk_clk_tree mt7987_infracfg_clk_tree = {
	.muxes_offs = CLK_INFRA_MUX_UART0_SEL,
	.gates_offs = CLK_INFRA_66M_GPT_BCK,
	.muxes = infracfg_mtk_mux,
	.gates = infracfg_mtk_gates,
	.flags = CLK_BYPASS_XTAL,
	.xtal_rate = MT7987_XTAL_RATE,
};

static const struct udevice_id mt7987_infracfg_compat[] = {
	{ .compatible = "mediatek,mt7987-infracfg_ao" },
	{ .compatible = "mediatek,mt7987-infracfg" },
	{}
};

static int mt7987_infracfg_probe(struct udevice *dev)
{
	return mtk_common_clk_infrasys_init(dev, &mt7987_infracfg_clk_tree);
}

U_BOOT_DRIVER(mtk_clk_infracfg) = {
	.name = "mt7987-clock-infracfg",
	.id = UCLASS_CLK,
	.of_match = mt7987_infracfg_compat,
	.probe = mt7987_infracfg_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_infrasys_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

/* ethsys */
static const struct mtk_gate_regs eth_cg_regs = {
	.set_ofs = 0x30,
	.clr_ofs = 0x30,
	.sta_ofs = 0x30,
};

#define GATE_ETH_TOP(_id, _name, _parent, _shift)                              \
	{                                                                      \
		.id = (_id), .parent = (_parent), .regs = &eth_cg_regs,        \
		.shift = (_shift),                                             \
		.flags = CLK_GATE_NO_SETCLR_INV | CLK_PARENT_TOPCKGEN,         \
	}

static const struct mtk_gate eth_cgs[] = {
	GATE_ETH_TOP(CLK_ETHDMA_FE_EN, "ethdma_fe_en", CLK_TOP_NETSYS_2X_SEL, 6),
	GATE_ETH_TOP(CLK_ETHDMA_GP2_EN, "ethdma_gp2_en", CLK_TOP_NETSYS_500M_SEL, 7),
	GATE_ETH_TOP(CLK_ETHDMA_GP1_EN, "ethdma_gp1_en", CLK_TOP_NETSYS_500M_SEL, 8),
	GATE_ETH_TOP(CLK_ETHDMA_GP3_EN, "ethdma_gp3_en", CLK_TOP_NETSYS_500M_SEL, 10),
};

static int mt7987_ethsys_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7987_topckgen_clk_tree,
					eth_cgs);
}

static int mt7987_ethsys_bind(struct udevice *dev)
{
	int ret = 0;

	if (CONFIG_IS_ENABLED(RESET_MEDIATEK)) {
		ret = mediatek_reset_bind(dev, ETHSYS_HIFSYS_RST_CTRL_OFS, 1);
		if (ret)
			debug("Warning: failed to bind reset controller\n");
	}

	return ret;
}

static const struct udevice_id mt7987_ethsys_compat[] = {
	{
		.compatible = "mediatek,mt7987-ethsys",
	},
	{}
};

U_BOOT_DRIVER(mtk_clk_ethsys) = {
	.name = "mt7987-clock-ethsys",
	.id = UCLASS_CLK,
	.of_match = mt7987_ethsys_compat,
	.probe = mt7987_ethsys_probe,
	.bind = mt7987_ethsys_bind,
	.priv_auto = sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
};
