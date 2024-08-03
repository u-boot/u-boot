// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek clock driver for MT7988 SoC
 *
 * Copyright (C) 2022 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#include <dm.h>
#include <log.h>
#include <asm/arch-mediatek/reset.h>
#include <asm/io.h>
#include <dt-bindings/clock/mt7988-clk.h>
#include <linux/bitops.h>

#include "clk-mtk.h"

#define MT7988_CLK_PDN		0x250
#define MT7988_CLK_PDN_EN_WRITE BIT(31)

#define MT7988_ETHDMA_RST_CTRL_OFS	0x34
#define MT7988_ETHWARP_RST_CTRL_OFS	0x8

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
	FIXED_CLK(CK_APMIXED_NETSYSPLL, CLK_XTAL, 850000000),
	FIXED_CLK(CK_APMIXED_MPLL, CLK_XTAL, 416000000),
	FIXED_CLK(CK_APMIXED_MMPLL, CLK_XTAL, 720000000),
	FIXED_CLK(CK_APMIXED_APLL2, CLK_XTAL, 196608000),
	FIXED_CLK(CK_APMIXED_NET1PLL, CLK_XTAL, 2500000000),
	FIXED_CLK(CK_APMIXED_NET2PLL, CLK_XTAL, 800000000),
	FIXED_CLK(CK_APMIXED_WEDMCUPLL, CLK_XTAL, 208000000),
	FIXED_CLK(CK_APMIXED_SGMPLL, CLK_XTAL, 325000000),
	FIXED_CLK(CK_APMIXED_ARM_B, CLK_XTAL, 1500000000),
	FIXED_CLK(CK_APMIXED_CCIPLL2_B, CLK_XTAL, 960000000),
	FIXED_CLK(CK_APMIXED_USXGMIIPLL, CLK_XTAL, 644533000),
	FIXED_CLK(CK_APMIXED_MSDCPLL, CLK_XTAL, 400000000),
};

/* TOPCKGEN FIXED CLK */
static const struct mtk_fixed_clk topckgen_mtk_fixed_clks[] = {
	FIXED_CLK(CK_TOP_XTAL, CLK_XTAL, 40000000),
};

/* TOPCKGEN FIXED DIV */
static const struct mtk_fixed_factor topckgen_mtk_fixed_factors[] = {
	TOP_FACTOR(CK_TOP_XTAL_D2, "xtal_d2", CK_TOP_XTAL, 1, 2),
	TOP_FACTOR(CK_TOP_RTC_32K, "rtc_32k", CK_TOP_XTAL, 1,
		   1250),
	TOP_FACTOR(CK_TOP_RTC_32P7K, "rtc_32p7k", CK_TOP_XTAL, 1,
		   1220),
	PLL_FACTOR(CK_TOP_MPLL_D2, "mpll_d2", CK_APMIXED_MPLL, 1, 2),
	PLL_FACTOR(CK_TOP_MPLL_D3_D2, "mpll_d3_d2", CK_APMIXED_MPLL, 1, 2),
	PLL_FACTOR(CK_TOP_MPLL_D4, "mpll_d4", CK_APMIXED_MPLL, 1, 4),
	PLL_FACTOR(CK_TOP_MPLL_D8, "mpll_d8", CK_APMIXED_MPLL, 1, 8),
	PLL_FACTOR(CK_TOP_MPLL_D8_D2, "mpll_d8_d2", CK_APMIXED_MPLL, 1, 16),
	PLL_FACTOR(CK_TOP_MMPLL_D2, "mmpll_d2", CK_APMIXED_MMPLL, 1, 2),
	PLL_FACTOR(CK_TOP_MMPLL_D3_D5, "mmpll_d3_d5", CK_APMIXED_MMPLL, 1, 15),
	PLL_FACTOR(CK_TOP_MMPLL_D4, "mmpll_d4", CK_APMIXED_MMPLL, 1, 4),
	PLL_FACTOR(CK_TOP_MMPLL_D6_D2, "mmpll_d6_d2", CK_APMIXED_MMPLL, 1, 12),
	PLL_FACTOR(CK_TOP_MMPLL_D8, "mmpll_d8", CK_APMIXED_MMPLL, 1, 8),
	PLL_FACTOR(CK_TOP_APLL2_D4, "apll2_d4", CK_APMIXED_APLL2, 1, 4),
	PLL_FACTOR(CK_TOP_NET1PLL_D4, "net1pll_d4", CK_APMIXED_NET1PLL, 1, 4),
	PLL_FACTOR(CK_TOP_NET1PLL_D5, "net1pll_d5", CK_APMIXED_NET1PLL, 1, 5),
	PLL_FACTOR(CK_TOP_NET1PLL_D5_D2, "net1pll_d5_d2", CK_APMIXED_NET1PLL, 1, 10),
	PLL_FACTOR(CK_TOP_NET1PLL_D5_D4, "net1pll_d5_d4", CK_APMIXED_NET1PLL, 1, 20),
	PLL_FACTOR(CK_TOP_NET1PLL_D8, "net1pll_d8", CK_APMIXED_NET1PLL, 1, 8),
	PLL_FACTOR(CK_TOP_NET1PLL_D8_D2, "net1pll_d8_d2", CK_APMIXED_NET1PLL, 1, 16),
	PLL_FACTOR(CK_TOP_NET1PLL_D8_D4, "net1pll_d8_d4", CK_APMIXED_NET1PLL, 1, 32),
	PLL_FACTOR(CK_TOP_NET1PLL_D8_D8, "net1pll_d8_d8", CK_APMIXED_NET1PLL, 1, 64),
	PLL_FACTOR(CK_TOP_NET1PLL_D8_D16, "net1pll_d8_d16", CK_APMIXED_NET1PLL, 1,
		   128),
	PLL_FACTOR(CK_TOP_NET2PLL_D2, "net2pll_d2", CK_APMIXED_NET2PLL, 1, 2),
	PLL_FACTOR(CK_TOP_NET2PLL_D4, "net2pll_d4", CK_APMIXED_NET2PLL, 1, 4),
	PLL_FACTOR(CK_TOP_NET2PLL_D4_D4, "net2pll_d4_d4", CK_APMIXED_NET2PLL, 1, 16),
	PLL_FACTOR(CK_TOP_NET2PLL_D4_D8, "net2pll_d4_d8", CK_APMIXED_NET2PLL, 1, 32),
	PLL_FACTOR(CK_TOP_NET2PLL_D6, "net2pll_d6", CK_APMIXED_NET2PLL, 1, 6),
	PLL_FACTOR(CK_TOP_NET2PLL_D8, "net2pll_d8", CK_APMIXED_NET2PLL, 1, 8),
};

/* TOPCKGEN MUX PARENTS */
#define APMIXED_PARENT(_id) PARENT(_id, CLK_PARENT_APMIXED)
#define TOP_PARENT(_id) PARENT(_id, CLK_PARENT_TOPCKGEN)

static const struct mtk_parent netsys_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_NET2PLL_D2),
	TOP_PARENT(CK_TOP_MMPLL_D2),
};

static const struct mtk_parent netsys_500m_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_NET1PLL_D5),
	TOP_PARENT(CK_TOP_NET1PLL_D5_D2),
};

static const struct mtk_parent netsys_2x_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), APMIXED_PARENT(CK_APMIXED_NET2PLL),
	APMIXED_PARENT(CK_APMIXED_MMPLL),
};

static const struct mtk_parent netsys_gsw_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_NET1PLL_D4),
	TOP_PARENT(CK_TOP_NET1PLL_D5),
};

static const struct mtk_parent eth_gmii_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_NET1PLL_D5_D4),
};

static const struct mtk_parent netsys_mcu_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), APMIXED_PARENT(CK_APMIXED_NET2PLL),
	APMIXED_PARENT(CK_APMIXED_MMPLL), TOP_PARENT(CK_TOP_NET1PLL_D4),
	TOP_PARENT(CK_TOP_NET1PLL_D5), APMIXED_PARENT(CK_APMIXED_MPLL),
};

static const struct mtk_parent eip197_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), APMIXED_PARENT(CK_APMIXED_NETSYSPLL),
	APMIXED_PARENT(CK_APMIXED_NET2PLL), APMIXED_PARENT(CK_APMIXED_MMPLL),
	TOP_PARENT(CK_TOP_NET1PLL_D4), TOP_PARENT(CK_TOP_NET1PLL_D5),
};

static const struct mtk_parent axi_infra_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_NET1PLL_D8_D2),
};

static const struct mtk_parent uart_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_MPLL_D8),
	TOP_PARENT(CK_TOP_MPLL_D8_D2),
};

static const struct mtk_parent emmc_250m_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_NET1PLL_D5_D2),
	TOP_PARENT(CK_TOP_MMPLL_D4),
};

static const struct mtk_parent emmc_400m_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), APMIXED_PARENT(CK_APMIXED_MSDCPLL),
	TOP_PARENT(CK_TOP_MMPLL_D2), TOP_PARENT(CK_TOP_MPLL_D2),
	TOP_PARENT(CK_TOP_MMPLL_D4), TOP_PARENT(CK_TOP_NET1PLL_D8_D2),
};

static const struct mtk_parent spi_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_MPLL_D2),
	TOP_PARENT(CK_TOP_MMPLL_D4), TOP_PARENT(CK_TOP_NET1PLL_D8_D2),
	TOP_PARENT(CK_TOP_NET2PLL_D6), TOP_PARENT(CK_TOP_NET1PLL_D5_D4),
	TOP_PARENT(CK_TOP_MPLL_D4), TOP_PARENT(CK_TOP_NET1PLL_D8_D4),
};

static const struct mtk_parent nfi1x_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_MMPLL_D4),
	TOP_PARENT(CK_TOP_NET1PLL_D8_D2), TOP_PARENT(CK_TOP_NET2PLL_D6),
	TOP_PARENT(CK_TOP_MPLL_D4), TOP_PARENT(CK_TOP_MMPLL_D8),
	TOP_PARENT(CK_TOP_NET1PLL_D8_D4), TOP_PARENT(CK_TOP_MPLL_D8),
};

static const struct mtk_parent spinfi_parents[] = {
	TOP_PARENT(CK_TOP_XTAL_D2), TOP_PARENT(CK_TOP_XTAL),
	TOP_PARENT(CK_TOP_NET1PLL_D5_D4), TOP_PARENT(CK_TOP_MPLL_D4),
	TOP_PARENT(CK_TOP_MMPLL_D8), TOP_PARENT(CK_TOP_NET1PLL_D8_D4),
	TOP_PARENT(CK_TOP_MMPLL_D6_D2), TOP_PARENT(CK_TOP_MPLL_D8),
};

static const struct mtk_parent pwm_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_NET1PLL_D8_D2),
	TOP_PARENT(CK_TOP_NET1PLL_D5_D4), TOP_PARENT(CK_TOP_MPLL_D4),
	TOP_PARENT(CK_TOP_MPLL_D8_D2), TOP_PARENT(CK_TOP_RTC_32K),
};

static const struct mtk_parent i2c_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_NET1PLL_D5_D4),
	TOP_PARENT(CK_TOP_MPLL_D4), TOP_PARENT(CK_TOP_NET1PLL_D8_D4),
};

static const struct mtk_parent pcie_mbist_250m_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_NET1PLL_D5_D2),
};

static const struct mtk_parent pextp_tl_ck_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_NET2PLL_D6),
	TOP_PARENT(CK_TOP_MMPLL_D8), TOP_PARENT(CK_TOP_MPLL_D8_D2),
	TOP_PARENT(CK_TOP_RTC_32K),
};

static const struct mtk_parent usb_frmcnt_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_MMPLL_D3_D5),
};

static const struct mtk_parent aud_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), APMIXED_PARENT(CK_APMIXED_APLL2),
};

static const struct mtk_parent a1sys_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_APLL2_D4),
};

static const struct mtk_parent aud_l_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), APMIXED_PARENT(CK_APMIXED_APLL2),
	TOP_PARENT(CK_TOP_MPLL_D8_D2),
};

static const struct mtk_parent sspxtp_parents[] = {
	TOP_PARENT(CK_TOP_XTAL_D2), TOP_PARENT(CK_TOP_MPLL_D8_D2),
};

static const struct mtk_parent usxgmii_sbus_0_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_NET1PLL_D8_D4),
};

static const struct mtk_parent sgm_0_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), APMIXED_PARENT(CK_APMIXED_SGMPLL),
};

static const struct mtk_parent sysapb_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_MPLL_D3_D2),
};

static const struct mtk_parent eth_refck_50m_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_NET2PLL_D4_D4),
};

static const struct mtk_parent eth_sys_200m_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_NET2PLL_D4),
};

static const struct mtk_parent eth_xgmii_parents[] = {
	TOP_PARENT(CK_TOP_XTAL_D2), TOP_PARENT(CK_TOP_NET1PLL_D8_D8),
	TOP_PARENT(CK_TOP_NET1PLL_D8_D16),
};

static const struct mtk_parent bus_tops_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_NET1PLL_D5),
	TOP_PARENT(CK_TOP_NET2PLL_D2),
};

static const struct mtk_parent npu_tops_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), APMIXED_PARENT(CK_APMIXED_NET2PLL),
};

static const struct mtk_parent dramc_md32_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_MPLL_D2),
	APMIXED_PARENT(CK_APMIXED_WEDMCUPLL),
};

static const struct mtk_parent da_xtp_glb_p0_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_NET2PLL_D8),
};

static const struct mtk_parent mcusys_backup_625m_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_NET1PLL_D4),
};

static const struct mtk_parent macsec_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), APMIXED_PARENT(CK_APMIXED_SGMPLL),
	TOP_PARENT(CK_TOP_NET1PLL_D8),
};

static const struct mtk_parent netsys_tops_400m_parents[] = {
	TOP_PARENT(CK_TOP_XTAL), TOP_PARENT(CK_TOP_NET2PLL_D2),
};

static const struct mtk_parent eth_mii_parents[] = {
	TOP_PARENT(CK_TOP_XTAL_D2), TOP_PARENT(CK_TOP_NET2PLL_D4_D8),
};

#define TOP_MUX(_id, _name, _parents, _mux_ofs, _mux_set_ofs, _mux_clr_ofs,    \
		_shift, _width, _gate, _upd_ofs, _upd)                         \
	{                                                                      \
		.id = _id, .mux_reg = _mux_ofs, .mux_set_reg = _mux_set_ofs,   \
		.mux_clr_reg = _mux_clr_ofs, .upd_reg = _upd_ofs,              \
		.upd_shift = _upd, .mux_shift = _shift,                        \
		.mux_mask = BIT(_width) - 1, .gate_reg = _mux_ofs,             \
		.gate_shift = _gate, .parent_flags = _parents,                 \
		.num_parents = ARRAY_SIZE(_parents),                           \
		.flags = CLK_MUX_SETCLR_UPD | CLK_PARENT_MIXED,                \
	}

/* TOPCKGEN MUX_GATE */
static const struct mtk_composite topckgen_mtk_muxes[] = {
	TOP_MUX(CK_TOP_NETSYS_SEL, "netsys_sel", netsys_parents, 0x0, 0x4, 0x8,
		0, 2, 7, 0x1c0, 0),
	TOP_MUX(CK_TOP_NETSYS_500M_SEL, "netsys_500m_sel", netsys_500m_parents,
		0x0, 0x4, 0x8, 8, 2, 15, 0x1c0, 1),
	TOP_MUX(CK_TOP_NETSYS_2X_SEL, "netsys_2x_sel", netsys_2x_parents, 0x0,
		0x4, 0x8, 16, 2, 23, 0x1c0, 2),
	TOP_MUX(CK_TOP_NETSYS_GSW_SEL, "netsys_gsw_sel", netsys_gsw_parents,
		0x0, 0x4, 0x8, 24, 2, 31, 0x1c0, 3),
	TOP_MUX(CK_TOP_ETH_GMII_SEL, "eth_gmii_sel", eth_gmii_parents, 0x10,
		0x14, 0x18, 0, 1, 7, 0x1c0, 4),
	TOP_MUX(CK_TOP_NETSYS_MCU_SEL, "netsys_mcu_sel", netsys_mcu_parents,
		0x10, 0x14, 0x18, 8, 3, 15, 0x1c0, 5),
	TOP_MUX(CK_TOP_NETSYS_PAO_2X_SEL, "netsys_pao_2x_sel",
		netsys_mcu_parents, 0x10, 0x14, 0x18, 16, 3, 23, 0x1c0, 6),
	TOP_MUX(CK_TOP_EIP197_SEL, "eip197_sel", eip197_parents, 0x10, 0x14,
		0x18, 24, 3, 31, 0x1c0, 7),
	TOP_MUX(CK_TOP_AXI_INFRA_SEL, "axi_infra_sel", axi_infra_parents, 0x20,
		0x24, 0x28, 0, 1, 7, 0x1c0, 8),
	TOP_MUX(CK_TOP_UART_SEL, "uart_sel", uart_parents, 0x20, 0x24, 0x28, 8,
		2, 15, 0x1c0, 9),
	TOP_MUX(CK_TOP_EMMC_250M_SEL, "emmc_250m_sel", emmc_250m_parents, 0x20,
		0x24, 0x28, 16, 2, 23, 0x1c0, 10),
	TOP_MUX(CK_TOP_EMMC_400M_SEL, "emmc_400m_sel", emmc_400m_parents, 0x20,
		0x24, 0x28, 24, 3, 31, 0x1c0, 11),
	TOP_MUX(CK_TOP_SPI_SEL, "spi_sel", spi_parents, 0x30, 0x34, 0x38, 0, 3,
		7, 0x1c0, 12),
	TOP_MUX(CK_TOP_SPIM_MST_SEL, "spim_mst_sel", spi_parents, 0x30, 0x34,
		0x38, 8, 3, 15, 0x1c0, 13),
	TOP_MUX(CK_TOP_NFI1X_SEL, "nfi1x_sel", nfi1x_parents, 0x30, 0x34, 0x38,
		16, 3, 23, 0x1c0, 14),
	TOP_MUX(CK_TOP_SPINFI_SEL, "spinfi_sel", spinfi_parents, 0x30, 0x34,
		0x38, 24, 3, 31, 0x1c0, 15),
	TOP_MUX(CK_TOP_PWM_SEL, "pwm_sel", pwm_parents, 0x40, 0x44, 0x48, 0, 3,
		7, 0x1c0, 16),
	TOP_MUX(CK_TOP_I2C_SEL, "i2c_sel", i2c_parents, 0x40, 0x44, 0x48, 8, 2,
		15, 0x1c0, 17),
	TOP_MUX(CK_TOP_PCIE_MBIST_250M_SEL, "pcie_mbist_250m_sel",
		pcie_mbist_250m_parents, 0x40, 0x44, 0x48, 16, 1, 23, 0x1c0,
		18),
	TOP_MUX(CK_TOP_PEXTP_TL_SEL, "pextp_tl_ck_sel", pextp_tl_ck_parents,
		0x40, 0x44, 0x48, 24, 3, 31, 0x1c0, 19),
	TOP_MUX(CK_TOP_PEXTP_TL_P1_SEL, "pextp_tl_ck_p1_sel",
		pextp_tl_ck_parents, 0x50, 0x54, 0x58, 0, 3, 7, 0x1c0, 20),
	TOP_MUX(CK_TOP_PEXTP_TL_P2_SEL, "pextp_tl_ck_p2_sel",
		pextp_tl_ck_parents, 0x50, 0x54, 0x58, 8, 3, 15, 0x1c0, 21),
	TOP_MUX(CK_TOP_PEXTP_TL_P3_SEL, "pextp_tl_ck_p3_sel",
		pextp_tl_ck_parents, 0x50, 0x54, 0x58, 16, 3, 23, 0x1c0, 22),
	TOP_MUX(CK_TOP_USB_SYS_SEL, "usb_sys_sel", eth_gmii_parents, 0x50, 0x54,
		0x58, 24, 1, 31, 0x1c0, 23),
	TOP_MUX(CK_TOP_USB_SYS_P1_SEL, "usb_sys_p1_sel", eth_gmii_parents, 0x60,
		0x64, 0x68, 0, 1, 7, 0x1c0, 24),
	TOP_MUX(CK_TOP_USB_XHCI_SEL, "usb_xhci_sel", eth_gmii_parents, 0x60,
		0x64, 0x68, 8, 1, 15, 0x1c0, 25),
	TOP_MUX(CK_TOP_USB_XHCI_P1_SEL, "usb_xhci_p1_sel", eth_gmii_parents,
		0x60, 0x64, 0x68, 16, 1, 23, 0x1c0, 26),
	TOP_MUX(CK_TOP_USB_FRMCNT_SEL, "usb_frmcnt_sel", usb_frmcnt_parents,
		0x60, 0x64, 0x68, 24, 1, 31, 0x1c0, 27),
	TOP_MUX(CK_TOP_USB_FRMCNT_P1_SEL, "usb_frmcnt_p1_sel",
		usb_frmcnt_parents, 0x70, 0x74, 0x78, 0, 1, 7, 0x1c0, 28),
	TOP_MUX(CK_TOP_AUD_SEL, "aud_sel", aud_parents, 0x70, 0x74, 0x78, 8, 1,
		15, 0x1c0, 29),
	TOP_MUX(CK_TOP_A1SYS_SEL, "a1sys_sel", a1sys_parents, 0x70, 0x74, 0x78,
		16, 1, 23, 0x1c0, 30),
	TOP_MUX(CK_TOP_AUD_L_SEL, "aud_l_sel", aud_l_parents, 0x70, 0x74, 0x78,
		24, 2, 31, 0x1c4, 0),
	TOP_MUX(CK_TOP_A_TUNER_SEL, "a_tuner_sel", a1sys_parents, 0x80, 0x84,
		0x88, 0, 1, 7, 0x1c4, 1),
	TOP_MUX(CK_TOP_SSPXTP_SEL, "sspxtp_sel", sspxtp_parents, 0x80, 0x84,
		0x88, 8, 1, 15, 0x1c4, 2),
	TOP_MUX(CK_TOP_USB_PHY_SEL, "usb_phy_sel", sspxtp_parents, 0x80, 0x84,
		0x88, 16, 1, 23, 0x1c4, 3),
	TOP_MUX(CK_TOP_USXGMII_SBUS_0_SEL, "usxgmii_sbus_0_sel",
		usxgmii_sbus_0_parents, 0x80, 0x84, 0x88, 24, 1, 31, 0x1c4, 4),
	TOP_MUX(CK_TOP_USXGMII_SBUS_1_SEL, "usxgmii_sbus_1_sel",
		usxgmii_sbus_0_parents, 0x90, 0x94, 0x98, 0, 1, 7, 0x1c4, 5),
	TOP_MUX(CK_TOP_SGM_0_SEL, "sgm_0_sel", sgm_0_parents, 0x90, 0x94, 0x98,
		8, 1, 15, 0x1c4, 6),
	TOP_MUX(CK_TOP_SGM_SBUS_0_SEL, "sgm_sbus_0_sel", usxgmii_sbus_0_parents,
		0x90, 0x94, 0x98, 16, 1, 23, 0x1c4, 7),
	TOP_MUX(CK_TOP_SGM_1_SEL, "sgm_1_sel", sgm_0_parents, 0x90, 0x94, 0x98,
		24, 1, 31, 0x1c4, 8),
	TOP_MUX(CK_TOP_SGM_SBUS_1_SEL, "sgm_sbus_1_sel", usxgmii_sbus_0_parents,
		0xa0, 0xa4, 0xa8, 0, 1, 7, 0x1c4, 9),
	TOP_MUX(CK_TOP_XFI_PHY_0_XTAL_SEL, "xfi_phy_0_xtal_sel", sspxtp_parents,
		0xa0, 0xa4, 0xa8, 8, 1, 15, 0x1c4, 10),
	TOP_MUX(CK_TOP_XFI_PHY_1_XTAL_SEL, "xfi_phy_1_xtal_sel", sspxtp_parents,
		0xa0, 0xa4, 0xa8, 16, 1, 23, 0x1c4, 11),
	TOP_MUX(CK_TOP_SYSAXI_SEL, "sysaxi_sel", axi_infra_parents, 0xa0, 0xa4,
		0xa8, 24, 1, 31, 0x1c4, 12),
	TOP_MUX(CK_TOP_SYSAPB_SEL, "sysapb_sel", sysapb_parents, 0xb0, 0xb4,
		0xb8, 0, 1, 7, 0x1c4, 13),
	TOP_MUX(CK_TOP_ETH_REFCK_50M_SEL, "eth_refck_50m_sel",
		eth_refck_50m_parents, 0xb0, 0xb4, 0xb8, 8, 1, 15, 0x1c4, 14),
	TOP_MUX(CK_TOP_ETH_SYS_200M_SEL, "eth_sys_200m_sel",
		eth_sys_200m_parents, 0xb0, 0xb4, 0xb8, 16, 1, 23, 0x1c4, 15),
	TOP_MUX(CK_TOP_ETH_SYS_SEL, "eth_sys_sel", pcie_mbist_250m_parents,
		0xb0, 0xb4, 0xb8, 24, 1, 31, 0x1c4, 16),
	TOP_MUX(CK_TOP_ETH_XGMII_SEL, "eth_xgmii_sel", eth_xgmii_parents, 0xc0,
		0xc4, 0xc8, 0, 2, 7, 0x1c4, 17),
	TOP_MUX(CK_TOP_BUS_TOPS_SEL, "bus_tops_sel", bus_tops_parents, 0xc0,
		0xc4, 0xc8, 8, 2, 15, 0x1c4, 18),
	TOP_MUX(CK_TOP_NPU_TOPS_SEL, "npu_tops_sel", npu_tops_parents, 0xc0,
		0xc4, 0xc8, 16, 1, 23, 0x1c4, 19),
	TOP_MUX(CK_TOP_DRAMC_SEL, "dramc_sel", sspxtp_parents, 0xc0, 0xc4, 0xc8,
		24, 1, 31, 0x1c4, 20),
	TOP_MUX(CK_TOP_DRAMC_MD32_SEL, "dramc_md32_sel", dramc_md32_parents,
		0xd0, 0xd4, 0xd8, 0, 2, 7, 0x1c4, 21),
	TOP_MUX(CK_TOP_INFRA_F26M_SEL, "csw_infra_f26m_sel", sspxtp_parents,
		0xd0, 0xd4, 0xd8, 8, 1, 15, 0x1c4, 22),
	TOP_MUX(CK_TOP_PEXTP_P0_SEL, "pextp_p0_sel", sspxtp_parents, 0xd0, 0xd4,
		0xd8, 16, 1, 23, 0x1c4, 23),
	TOP_MUX(CK_TOP_PEXTP_P1_SEL, "pextp_p1_sel", sspxtp_parents, 0xd0, 0xd4,
		0xd8, 24, 1, 31, 0x1c4, 24),
	TOP_MUX(CK_TOP_PEXTP_P2_SEL, "pextp_p2_sel", sspxtp_parents, 0xe0, 0xe4,
		0xe8, 0, 1, 7, 0x1c4, 25),
	TOP_MUX(CK_TOP_PEXTP_P3_SEL, "pextp_p3_sel", sspxtp_parents, 0xe0, 0xe4,
		0xe8, 8, 1, 15, 0x1c4, 26),
	TOP_MUX(CK_TOP_DA_XTP_GLB_P0_SEL, "da_xtp_glb_p0_sel",
		da_xtp_glb_p0_parents, 0xe0, 0xe4, 0xe8, 16, 1, 23, 0x1c4, 27),
	TOP_MUX(CK_TOP_DA_XTP_GLB_P1_SEL, "da_xtp_glb_p1_sel",
		da_xtp_glb_p0_parents, 0xe0, 0xe4, 0xe8, 24, 1, 31, 0x1c4, 28),
	TOP_MUX(CK_TOP_DA_XTP_GLB_P2_SEL, "da_xtp_glb_p2_sel",
		da_xtp_glb_p0_parents, 0xf0, 0xf4, 0xf8, 0, 1, 7, 0x1c4, 29),
	TOP_MUX(CK_TOP_DA_XTP_GLB_P3_SEL, "da_xtp_glb_p3_sel",
		da_xtp_glb_p0_parents, 0xf0, 0xf4, 0xf8, 8, 1, 15, 0x1c4, 30),
	TOP_MUX(CK_TOP_CKM_SEL, "ckm_sel", sspxtp_parents, 0xf0, 0xf4, 0xf8, 16,
		1, 23, 0x1c8, 0),
	TOP_MUX(CK_TOP_DA_SEL, "da_sel", sspxtp_parents,
		0xf0, 0xf4, 0xf8, 24, 1, 31, 0x1c8, 1),
	TOP_MUX(CK_TOP_PEXTP_SEL, "pextp_sel", sspxtp_parents, 0x100, 0x104,
		0x108, 0, 1, 7, 0x1c8, 2),
	TOP_MUX(CK_TOP_TOPS_P2_26M_SEL, "tops_p2_26m_sel", sspxtp_parents,
		0x100, 0x104, 0x108, 8, 1, 15, 0x1c8, 3),
	TOP_MUX(CK_TOP_MCUSYS_BACKUP_625M_SEL, "mcusys_backup_625m_sel",
		mcusys_backup_625m_parents, 0x100, 0x104, 0x108, 16, 1, 23,
		0x1c8, 4),
	TOP_MUX(CK_TOP_NETSYS_SYNC_250M_SEL, "netsys_sync_250m_sel",
		pcie_mbist_250m_parents, 0x100, 0x104, 0x108, 24, 1, 31, 0x1c8,
		5),
	TOP_MUX(CK_TOP_MACSEC_SEL, "macsec_sel", macsec_parents, 0x110, 0x114,
		0x118, 0, 2, 7, 0x1c8, 6),
	TOP_MUX(CK_TOP_NETSYS_TOPS_400M_SEL, "netsys_tops_400m_sel",
		netsys_tops_400m_parents, 0x110, 0x114, 0x118, 8, 1, 15, 0x1c8,
		7),
	TOP_MUX(CK_TOP_NETSYS_PPEFB_250M_SEL, "netsys_ppefb_250m_sel",
		pcie_mbist_250m_parents, 0x110, 0x114, 0x118, 16, 1, 23, 0x1c8,
		8),
	TOP_MUX(CK_TOP_NETSYS_WARP_SEL, "netsys_warp_sel", netsys_parents,
		0x110, 0x114, 0x118, 24, 2, 31, 0x1c8, 9),
	TOP_MUX(CK_TOP_ETH_MII_SEL, "eth_mii_sel", eth_mii_parents, 0x120,
		0x124, 0x128, 0, 1, 7, 0x1c8, 10),
	TOP_MUX(CK_TOP_NPU_SEL, "ck_npu_sel",
		netsys_2x_parents, 0x120, 0x124, 0x128, 8, 2, 15, 0x1c8, 11),
};

/* INFRASYS MUX PARENTS */
static const int infra_mux_uart0_parents[] = { CK_TOP_INFRA_F26M_SEL,
					       CK_TOP_UART_SEL };

static const int infra_mux_uart1_parents[] = { CK_TOP_INFRA_F26M_SEL,
					       CK_TOP_UART_SEL };

static const int infra_mux_uart2_parents[] = { CK_TOP_INFRA_F26M_SEL,
					       CK_TOP_UART_SEL };

static const int infra_mux_spi0_parents[] = { CK_TOP_I2C_SEL, CK_TOP_SPI_SEL };

static const int infra_mux_spi1_parents[] = { CK_TOP_I2C_SEL, CK_TOP_SPIM_MST_SEL };

static const int infra_pwm_bck_parents[] = { CK_TOP_RTC_32P7K,
					     CK_TOP_INFRA_F26M_SEL, CK_TOP_SYSAXI_SEL,
					     CK_TOP_PWM_SEL };

static const int infra_pcie_gfmux_tl_ck_o_p0_parents[] = {
	CK_TOP_RTC_32P7K, CK_TOP_INFRA_F26M_SEL, CK_TOP_INFRA_F26M_SEL,
	CK_TOP_PEXTP_TL_SEL
};

static const int infra_pcie_gfmux_tl_ck_o_p1_parents[] = {
	CK_TOP_RTC_32P7K, CK_TOP_INFRA_F26M_SEL, CK_TOP_INFRA_F26M_SEL,
	CK_TOP_PEXTP_TL_P1_SEL
};

static const int infra_pcie_gfmux_tl_ck_o_p2_parents[] = {
	CK_TOP_RTC_32P7K, CK_TOP_INFRA_F26M_SEL, CK_TOP_INFRA_F26M_SEL,
	CK_TOP_PEXTP_TL_P2_SEL
};

static const int infra_pcie_gfmux_tl_ck_o_p3_parents[] = {
	CK_TOP_RTC_32P7K, CK_TOP_INFRA_F26M_SEL, CK_TOP_INFRA_F26M_SEL,
	CK_TOP_PEXTP_TL_P3_SEL
};

#define INFRA_MUX(_id, _name, _parents, _reg, _shift, _width)                  \
	{                                                                      \
		.id = _id, .mux_reg = _reg + 0x8, .mux_set_reg = _reg + 0x0,   \
		.mux_clr_reg = _reg + 0x4, .mux_shift = _shift,                \
		.mux_mask = BIT(_width) - 1, .parent = _parents,               \
		.num_parents = ARRAY_SIZE(_parents),                           \
		.flags = CLK_MUX_SETCLR_UPD | CLK_PARENT_TOPCKGEN,             \
	}

/* INFRA MUX */
static const struct mtk_composite infracfg_mtk_mux[] = {
	INFRA_MUX(CK_INFRA_MUX_UART0_SEL, "infra_mux_uart0_sel",
		  infra_mux_uart0_parents, 0x10, 0, 1),
	INFRA_MUX(CK_INFRA_MUX_UART1_SEL, "infra_mux_uart1_sel",
		  infra_mux_uart1_parents, 0x10, 1, 1),
	INFRA_MUX(CK_INFRA_MUX_UART2_SEL, "infra_mux_uart2_sel",
		  infra_mux_uart2_parents, 0x10, 2, 1),
	INFRA_MUX(CK_INFRA_MUX_SPI0_SEL, "infra_mux_spi0_sel",
		  infra_mux_spi0_parents, 0x10, 4, 1),
	INFRA_MUX(CK_INFRA_MUX_SPI1_SEL, "infra_mux_spi1_sel",
		  infra_mux_spi1_parents, 0x10, 5, 1),
	INFRA_MUX(CK_INFRA_MUX_SPI2_SEL, "infra_mux_spi2_sel",
		  infra_mux_spi0_parents, 0x10, 6, 1),
	INFRA_MUX(CK_INFRA_PWM_SEL, "infra_pwm_sel", infra_pwm_bck_parents,
		  0x10, 14, 2),
	INFRA_MUX(CK_INFRA_PWM_CK1_SEL, "infra_pwm_ck1_sel",
		  infra_pwm_bck_parents, 0x10, 16, 2),
	INFRA_MUX(CK_INFRA_PWM_CK2_SEL, "infra_pwm_ck2_sel",
		  infra_pwm_bck_parents, 0x10, 18, 2),
	INFRA_MUX(CK_INFRA_PWM_CK3_SEL, "infra_pwm_ck3_sel",
		  infra_pwm_bck_parents, 0x10, 20, 2),
	INFRA_MUX(CK_INFRA_PWM_CK4_SEL, "infra_pwm_ck4_sel",
		  infra_pwm_bck_parents, 0x10, 22, 2),
	INFRA_MUX(CK_INFRA_PWM_CK5_SEL, "infra_pwm_ck5_sel",
		  infra_pwm_bck_parents, 0x10, 24, 2),
	INFRA_MUX(CK_INFRA_PWM_CK6_SEL, "infra_pwm_ck6_sel",
		  infra_pwm_bck_parents, 0x10, 26, 2),
	INFRA_MUX(CK_INFRA_PWM_CK7_SEL, "infra_pwm_ck7_sel",
		  infra_pwm_bck_parents, 0x10, 28, 2),
	INFRA_MUX(CK_INFRA_PWM_CK8_SEL, "infra_pwm_ck8_sel",
		  infra_pwm_bck_parents, 0x10, 30, 2),
	INFRA_MUX(CK_INFRA_PCIE_GFMUX_TL_O_P0_SEL,
		  "infra_pcie_gfmux_tl_o_p0_sel",
		  infra_pcie_gfmux_tl_ck_o_p0_parents, 0x20, 0, 2),
	INFRA_MUX(CK_INFRA_PCIE_GFMUX_TL_O_P1_SEL,
		  "infra_pcie_gfmux_tl_o_p1_sel",
		  infra_pcie_gfmux_tl_ck_o_p1_parents, 0x20, 2, 2),
	INFRA_MUX(CK_INFRA_PCIE_GFMUX_TL_O_P2_SEL,
		  "infra_pcie_gfmux_tl_o_p2_sel",
		  infra_pcie_gfmux_tl_ck_o_p2_parents, 0x20, 4, 2),
	INFRA_MUX(CK_INFRA_PCIE_GFMUX_TL_O_P3_SEL,
		  "infra_pcie_gfmux_tl_o_p3_sel",
		  infra_pcie_gfmux_tl_ck_o_p3_parents, 0x20, 6, 2),
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
		.id = _id, .parent = _parent, .regs = &infra_0_cg_regs,        \
		.shift = _shift,                                               \
		.flags = _flags,                                               \
	}
#define GATE_INFRA0_INFRA(_id, _name, _parent, _shift) \
	GATE_INFRA0(_id, _name, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_INFRASYS)
#define GATE_INFRA0_TOP(_id, _name, _parent, _shift) \
	GATE_INFRA0(_id, _name, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN)

#define GATE_INFRA1(_id, _name, _parent, _shift, _flags)                       \
	{                                                                      \
		.id = _id, .parent = _parent, .regs = &infra_1_cg_regs,        \
		.shift = _shift,                                               \
		.flags = _flags,                                               \
	}
#define GATE_INFRA1_INFRA(_id, _name, _parent, _shift) \
	GATE_INFRA1(_id, _name, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_INFRASYS)
#define GATE_INFRA1_TOP(_id, _name, _parent, _shift) \
	GATE_INFRA1(_id, _name, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN)

#define GATE_INFRA2(_id, _name, _parent, _shift, _flags)                       \
	{                                                                      \
		.id = _id, .parent = _parent, .regs = &infra_2_cg_regs,        \
		.shift = _shift,                                               \
		.flags = _flags,                                               \
	}
#define GATE_INFRA2_INFRA(_id, _name, _parent, _shift) \
	GATE_INFRA2(_id, _name, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_INFRASYS)
#define GATE_INFRA2_TOP(_id, _name, _parent, _shift) \
	GATE_INFRA2(_id, _name, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN)

#define GATE_INFRA3(_id, _name, _parent, _shift, _flags)                       \
	{                                                                      \
		.id = _id, .parent = _parent, .regs = &infra_3_cg_regs,        \
		.shift = _shift,                                               \
		.flags = _flags,                                               \
	}
#define GATE_INFRA3_INFRA(_id, _name, _parent, _shift) \
	GATE_INFRA3(_id, _name, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_INFRASYS)
#define GATE_INFRA3_TOP(_id, _name, _parent, _shift) \
	GATE_INFRA3(_id, _name, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN)
#define GATE_INFRA3_XTAL(_id, _name, _parent, _shift) \
	GATE_INFRA3(_id, _name, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_XTAL)

/* INFRA GATE */
static const struct mtk_gate infracfg_mtk_gates[] = {
	GATE_INFRA0_TOP(CK_INFRA_PCIE_PERI_26M_CK_P0,
			"infra_pcie_peri_ck_26m_ck_p0", CK_TOP_INFRA_F26M_SEL, 7),
	GATE_INFRA0_TOP(CK_INFRA_PCIE_PERI_26M_CK_P1,
			"infra_pcie_peri_ck_26m_ck_p1", CK_TOP_INFRA_F26M_SEL, 8),
	GATE_INFRA0_INFRA(CK_INFRA_PCIE_PERI_26M_CK_P2,
			  "infra_pcie_peri_ck_26m_ck_p2", CK_INFRA_PCIE_PERI_26M_CK_P3, 9),
	GATE_INFRA0_TOP(CK_INFRA_PCIE_PERI_26M_CK_P3,
			"infra_pcie_peri_ck_26m_ck_p3", CK_TOP_INFRA_F26M_SEL, 10),
	GATE_INFRA1_TOP(CK_INFRA_66M_GPT_BCK, "infra_hf_66m_gpt_bck",
			CK_TOP_SYSAXI_SEL, 0),
	GATE_INFRA1_TOP(CK_INFRA_66M_PWM_HCK, "infra_hf_66m_pwm_hck",
			CK_TOP_SYSAXI_SEL, 1),
	GATE_INFRA1_INFRA(CK_INFRA_66M_PWM_BCK, "infra_hf_66m_pwm_bck",
			  CK_INFRA_PWM_SEL, 2),
	GATE_INFRA1_INFRA(CK_INFRA_66M_PWM_CK1, "infra_hf_66m_pwm_ck1",
			  CK_INFRA_PWM_CK1_SEL, 3),
	GATE_INFRA1_INFRA(CK_INFRA_66M_PWM_CK2, "infra_hf_66m_pwm_ck2",
			  CK_INFRA_PWM_CK2_SEL, 4),
	GATE_INFRA1_INFRA(CK_INFRA_66M_PWM_CK3, "infra_hf_66m_pwm_ck3",
			  CK_INFRA_PWM_CK3_SEL, 5),
	GATE_INFRA1_INFRA(CK_INFRA_66M_PWM_CK4, "infra_hf_66m_pwm_ck4",
			  CK_INFRA_PWM_CK4_SEL, 6),
	GATE_INFRA1_INFRA(CK_INFRA_66M_PWM_CK5, "infra_hf_66m_pwm_ck5",
			  CK_INFRA_PWM_CK5_SEL, 7),
	GATE_INFRA1_INFRA(CK_INFRA_66M_PWM_CK6, "infra_hf_66m_pwm_ck6",
			  CK_INFRA_PWM_CK6_SEL, 8),
	GATE_INFRA1_INFRA(CK_INFRA_66M_PWM_CK7, "infra_hf_66m_pwm_ck7",
			  CK_INFRA_PWM_CK7_SEL, 9),
	GATE_INFRA1_INFRA(CK_INFRA_66M_PWM_CK8, "infra_hf_66m_pwm_ck8",
			  CK_INFRA_PWM_CK8_SEL, 10),
	GATE_INFRA1_TOP(CK_INFRA_133M_CQDMA_BCK, "infra_hf_133m_cqdma_bck",
			CK_TOP_SYSAXI_SEL, 12),
	GATE_INFRA1_TOP(CK_INFRA_66M_AUD_SLV_BCK, "infra_66m_aud_slv_bck",
			CK_TOP_SYSAXI_SEL, 13),
	GATE_INFRA1_TOP(CK_INFRA_AUD_26M, "infra_f_faud_26m", CK_TOP_INFRA_F26M_SEL, 14),
	GATE_INFRA1_TOP(CK_INFRA_AUD_L, "infra_f_faud_l", CK_TOP_AUD_L_SEL, 15),
	GATE_INFRA1_TOP(CK_INFRA_AUD_AUD, "infra_f_aud_aud", CK_TOP_A1SYS_SEL,
			16),
	GATE_INFRA1_TOP(CK_INFRA_AUD_EG2, "infra_f_faud_eg2", CK_TOP_A_TUNER_SEL,
			18),
	GATE_INFRA1_TOP(CK_INFRA_DRAMC_F26M, "infra_dramc_f26m", CK_TOP_INFRA_F26M_SEL,
			19),
	GATE_INFRA1_TOP(CK_INFRA_133M_DBG_ACKM, "infra_hf_133m_dbg_ackm",
			CK_TOP_SYSAXI_SEL, 20),
	GATE_INFRA1_TOP(CK_INFRA_66M_AP_DMA_BCK, "infra_66m_ap_dma_bck",
			CK_TOP_SYSAXI_SEL, 21),
	GATE_INFRA1_TOP(CK_INFRA_66M_SEJ_BCK, "infra_hf_66m_sej_bck",
			CK_TOP_SYSAXI_SEL, 29),
	GATE_INFRA1_TOP(CK_INFRA_PRE_CK_SEJ_F13M, "infra_pre_ck_sej_f13m",
			CK_TOP_INFRA_F26M_SEL, 30),
	/* GATE_INFRA1_TOP(CK_INFRA_66M_TRNG, "infra_hf_66m_trng", CK_TOP_SYSAXI_SEL,
			   31), */
	GATE_INFRA2_TOP(CK_INFRA_26M_THERM_SYSTEM, "infra_hf_26m_therm_system",
			CK_TOP_INFRA_F26M_SEL, 0),
	GATE_INFRA2_TOP(CK_INFRA_I2C_BCK, "infra_i2c_bck", CK_TOP_I2C_SEL, 1),
	/* GATE_INFRA2_TOP(CK_INFRA_66M_UART0_PCK, "infra_hf_66m_uart0_pck",
			   CK_TOP_SYSAXI_SEL, 3), */
	/* GATE_INFRA2_TOP(CK_INFRA_66M_UART1_PCK, "infra_hf_66m_uart1_pck",
			   CK_TOP_SYSAXI_SEL, 4), */
	/* GATE_INFRA2_TOP(CK_INFRA_66M_UART2_PCK, "infra_hf_66m_uart2_pck",
			   CK_TOP_SYSAXI_SEL, 5), */
	GATE_INFRA2_INFRA(CK_INFRA_52M_UART0_CK, "infra_f_52m_uart0",
			  CK_INFRA_MUX_UART0_SEL, 3),
	GATE_INFRA2_INFRA(CK_INFRA_52M_UART1_CK, "infra_f_52m_uart1",
			  CK_INFRA_MUX_UART1_SEL, 4),
	GATE_INFRA2_INFRA(CK_INFRA_52M_UART2_CK, "infra_f_52m_uart2",
			  CK_INFRA_MUX_UART2_SEL, 5),
	GATE_INFRA2_TOP(CK_INFRA_NFI, "infra_f_fnfi", CK_TOP_NFI1X_SEL, 9),
	GATE_INFRA2_TOP(CK_INFRA_SPINFI, "infra_f_fspinfi", CK_TOP_SPINFI_SEL, 10),
	GATE_INFRA2_TOP(CK_INFRA_66M_NFI_HCK, "infra_hf_66m_nfi_hck",
			CK_TOP_SYSAXI_SEL, 11),
	GATE_INFRA2_INFRA(CK_INFRA_104M_SPI0, "infra_hf_104m_spi0",
			  CK_INFRA_MUX_SPI0_SEL, 12),
	GATE_INFRA2_INFRA(CK_INFRA_104M_SPI1, "infra_hf_104m_spi1",
			  CK_INFRA_MUX_SPI1_SEL, 13),
	GATE_INFRA2_INFRA(CK_INFRA_104M_SPI2_BCK, "infra_hf_104m_spi2_bck",
			  CK_INFRA_MUX_SPI2_SEL, 14),
	GATE_INFRA2_TOP(CK_INFRA_66M_SPI0_HCK, "infra_hf_66m_spi0_hck",
			CK_TOP_SYSAXI_SEL, 15),
	GATE_INFRA2_TOP(CK_INFRA_66M_SPI1_HCK, "infra_hf_66m_spi1_hck",
			CK_TOP_SYSAXI_SEL, 16),
	GATE_INFRA2_TOP(CK_INFRA_66M_SPI2_HCK, "infra_hf_66m_spi2_hck",
			CK_TOP_SYSAXI_SEL, 17),
	GATE_INFRA2_TOP(CK_INFRA_66M_FLASHIF_AXI, "infra_hf_66m_flashif_axi",
			CK_TOP_SYSAXI_SEL, 18),
	GATE_INFRA2_TOP(CK_INFRA_RTC, "infra_f_frtc", CK_TOP_RTC_32K, 19),
	GATE_INFRA2_TOP(CK_INFRA_26M_ADC_BCK, "infra_f_26m_adc_bck",
			CK_TOP_INFRA_F26M_SEL, 20),
	GATE_INFRA2_INFRA(CK_INFRA_RC_ADC, "infra_f_frc_adc", CK_INFRA_26M_ADC_BCK,
			  21),
	GATE_INFRA2_TOP(CK_INFRA_MSDC400, "infra_f_fmsdc400", CK_TOP_EMMC_400M_SEL,
			22),
	GATE_INFRA2_TOP(CK_INFRA_MSDC2_HCK, "infra_f_fmsdc2_hck",
			CK_TOP_EMMC_250M_SEL, 23),
	GATE_INFRA2_TOP(CK_INFRA_133M_MSDC_0_HCK, "infra_hf_133m_msdc_0_hck",
			CK_TOP_SYSAXI_SEL, 24),
	GATE_INFRA2_TOP(CK_INFRA_66M_MSDC_0_HCK, "infra_66m_msdc_0_hck",
			CK_TOP_SYSAXI_SEL, 25),
	GATE_INFRA2_TOP(CK_INFRA_133M_CPUM_BCK, "infra_hf_133m_cpum_bck",
			CK_TOP_SYSAXI_SEL, 26),
	GATE_INFRA2_TOP(CK_INFRA_BIST2FPC, "infra_hf_fbist2fpc", CK_TOP_NFI1X_SEL,
			27),
	GATE_INFRA2_TOP(CK_INFRA_I2C_X16W_MCK_CK_P1, "infra_hf_i2c_x16w_mck_ck_p1",
			CK_TOP_SYSAXI_SEL, 29),
	GATE_INFRA2_TOP(CK_INFRA_I2C_X16W_PCK_CK_P1, "infra_hf_i2c_x16w_pck_ck_p1",
			CK_TOP_SYSAXI_SEL, 31),
	GATE_INFRA3_TOP(CK_INFRA_133M_USB_HCK, "infra_133m_usb_hck",
			CK_TOP_SYSAXI_SEL, 0),
	GATE_INFRA3_TOP(CK_INFRA_133M_USB_HCK_CK_P1, "infra_133m_usb_hck_ck_p1",
			CK_TOP_SYSAXI_SEL, 1),
	GATE_INFRA3_TOP(CK_INFRA_66M_USB_HCK, "infra_66m_usb_hck",
			CK_TOP_SYSAXI_SEL, 2),
	GATE_INFRA3_TOP(CK_INFRA_66M_USB_HCK_CK_P1, "infra_66m_usb_hck_ck_p1",
			CK_TOP_SYSAXI_SEL, 3),
	GATE_INFRA3_TOP(CK_INFRA_USB_SYS, "infra_usb_sys", CK_TOP_USB_SYS_SEL, 4),
	GATE_INFRA3_TOP(CK_INFRA_USB_SYS_CK_P1, "infra_usb_sys_ck_p1",
			CK_TOP_USB_SYS_P1_SEL, 5),
	GATE_INFRA3_XTAL(CK_INFRA_USB_REF, "infra_usb_ref", CLK_XTAL, 6),
	GATE_INFRA3_XTAL(CK_INFRA_USB_CK_P1, "infra_usb_ck_p1", CLK_XTAL,
			 7),
	GATE_INFRA3_TOP(CK_INFRA_USB_FRMCNT, "infra_usb_frmcnt",
			CK_TOP_USB_FRMCNT_SEL, 8),
	GATE_INFRA3_TOP(CK_INFRA_USB_FRMCNT_CK_P1, "infra_usb_frmcnt_ck_p1",
			CK_TOP_USB_FRMCNT_P1_SEL, 9),
	GATE_INFRA3_XTAL(CK_INFRA_USB_PIPE, "infra_usb_pipe", CLK_XTAL,
			 10),
	GATE_INFRA3_XTAL(CK_INFRA_USB_PIPE_CK_P1, "infra_usb_pipe_ck_p1",
			 CLK_XTAL, 11),
	GATE_INFRA3_XTAL(CK_INFRA_USB_UTMI, "infra_usb_utmi", CLK_XTAL,
			 12),
	GATE_INFRA3_XTAL(CK_INFRA_USB_UTMI_CK_P1, "infra_usb_utmi_ck_p1",
			 CLK_XTAL, 13),
	GATE_INFRA3_TOP(CK_INFRA_USB_XHCI, "infra_usb_xhci", CK_TOP_USB_XHCI_SEL,
			14),
	GATE_INFRA3_TOP(CK_INFRA_USB_XHCI_CK_P1, "infra_usb_xhci_ck_p1",
			CK_TOP_USB_XHCI_P1_SEL, 15),
	GATE_INFRA3_INFRA(CK_INFRA_PCIE_GFMUX_TL_P0, "infra_pcie_gfmux_tl_ck_p0",
			  CK_INFRA_PCIE_GFMUX_TL_O_P0_SEL, 20),
	GATE_INFRA3_INFRA(CK_INFRA_PCIE_GFMUX_TL_P1, "infra_pcie_gfmux_tl_ck_p1",
			  CK_INFRA_PCIE_GFMUX_TL_O_P1_SEL, 21),
	GATE_INFRA3_INFRA(CK_INFRA_PCIE_GFMUX_TL_P2, "infra_pcie_gfmux_tl_ck_p2",
			  CK_INFRA_PCIE_GFMUX_TL_O_P2_SEL, 22),
	GATE_INFRA3_INFRA(CK_INFRA_PCIE_GFMUX_TL_P3, "infra_pcie_gfmux_tl_ck_p3",
			  CK_INFRA_PCIE_GFMUX_TL_O_P3_SEL, 23),
	GATE_INFRA3_XTAL(CK_INFRA_PCIE_PIPE_P0, "infra_pcie_pipe_ck_p0",
			 CLK_XTAL, 24),
	GATE_INFRA3_XTAL(CK_INFRA_PCIE_PIPE_P1, "infra_pcie_pipe_ck_p1",
			 CLK_XTAL, 25),
	GATE_INFRA3_XTAL(CK_INFRA_PCIE_PIPE_P2, "infra_pcie_pipe_ck_p2",
			 CLK_XTAL, 26),
	GATE_INFRA3_XTAL(CK_INFRA_PCIE_PIPE_P3, "infra_pcie_pipe_ck_p3",
			 CLK_XTAL, 27),
	GATE_INFRA3_TOP(CK_INFRA_133M_PCIE_CK_P0, "infra_133m_pcie_ck_p0",
			CK_TOP_SYSAXI_SEL, 28),
	GATE_INFRA3_TOP(CK_INFRA_133M_PCIE_CK_P1, "infra_133m_pcie_ck_p1",
			CK_TOP_SYSAXI_SEL, 29),
	GATE_INFRA3_TOP(CK_INFRA_133M_PCIE_CK_P2, "infra_133m_pcie_ck_p2",
			CK_TOP_SYSAXI_SEL, 30),
	GATE_INFRA3_TOP(CK_INFRA_133M_PCIE_CK_P3, "infra_133m_pcie_ck_p3",
			CK_TOP_SYSAXI_SEL, 31),
};

static const struct mtk_clk_tree mt7988_fixed_pll_clk_tree = {
	.fdivs_offs = ARRAY_SIZE(apmixedsys_mtk_plls),
	.fclks = apmixedsys_mtk_plls,
	.flags = CLK_APMIXED,
	.xtal_rate = 40 * MHZ,
};

static const struct mtk_clk_tree mt7988_topckgen_clk_tree = {
	.fdivs_offs = CK_TOP_XTAL_D2,
	.muxes_offs = CK_TOP_NETSYS_SEL,
	.fclks = topckgen_mtk_fixed_clks,
	.fdivs = topckgen_mtk_fixed_factors,
	.muxes = topckgen_mtk_muxes,
	.flags = CLK_BYPASS_XTAL | CLK_TOPCKGEN,
	.xtal_rate = 40 * MHZ,
};

static const struct mtk_clk_tree mt7988_infracfg_clk_tree = {
	.muxes_offs = CK_INFRA_MUX_UART0_SEL,
	.gates_offs = CK_INFRA_PCIE_PERI_26M_CK_P0,
	.muxes = infracfg_mtk_mux,
	.flags = CLK_BYPASS_XTAL,
	.xtal_rate = 40 * MHZ,
};

static const struct udevice_id mt7988_fixed_pll_compat[] = {
	{ .compatible = "mediatek,mt7988-fixed-plls" },
	{ .compatible = "mediatek,mt7988-apmixedsys" },
	{}
};

static const struct udevice_id mt7988_topckgen_compat[] = {
	{ .compatible = "mediatek,mt7988-topckgen" },
	{}
};

static int mt7988_fixed_pll_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt7988_fixed_pll_clk_tree);
}

static int mt7988_topckgen_probe(struct udevice *dev)
{
	struct mtk_clk_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -ENOENT;

	writel(MT7988_CLK_PDN_EN_WRITE, priv->base + MT7988_CLK_PDN);
	return mtk_common_clk_init(dev, &mt7988_topckgen_clk_tree);
}

U_BOOT_DRIVER(mtk_clk_apmixedsys) = {
	.name = "mt7988-clock-fixed-pll",
	.id = UCLASS_CLK,
	.of_match = mt7988_fixed_pll_compat,
	.probe = mt7988_fixed_pll_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_topckgen_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_topckgen) = {
	.name = "mt7988-clock-topckgen",
	.id = UCLASS_CLK,
	.of_match = mt7988_topckgen_compat,
	.probe = mt7988_topckgen_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_topckgen_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

static const struct udevice_id mt7988_infracfg_compat[] = {
	{ .compatible = "mediatek,mt7988-infracfg" },
	{}
};

static const struct udevice_id mt7988_infracfg_ao_cgs_compat[] = {
	{ .compatible = "mediatek,mt7988-infracfg_ao_cgs" },
	{}
};

static int mt7988_infracfg_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt7988_infracfg_clk_tree);
}

static int mt7988_infracfg_ao_cgs_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7988_infracfg_clk_tree,
					infracfg_mtk_gates);
}

U_BOOT_DRIVER(mtk_clk_infracfg) = {
	.name = "mt7988-clock-infracfg",
	.id = UCLASS_CLK,
	.of_match = mt7988_infracfg_compat,
	.probe = mt7988_infracfg_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_infrasys_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_infracfg_ao_cgs) = {
	.name = "mt7988-clock-infracfg_ao_cgs",
	.id = UCLASS_CLK,
	.of_match = mt7988_infracfg_ao_cgs_compat,
	.probe = mt7988_infracfg_ao_cgs_probe,
	.priv_auto = sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

/* ETHDMA */

static const struct mtk_gate_regs ethdma_cg_regs = {
	.set_ofs = 0x30,
	.clr_ofs = 0x30,
	.sta_ofs = 0x30,
};

#define GATE_ETHDMA(_id, _name, _parent, _shift)                               \
	{                                                                      \
		.id = _id, .parent = _parent, .regs = &ethdma_cg_regs,         \
		.shift = _shift,                                               \
		.flags = CLK_GATE_NO_SETCLR_INV | CLK_PARENT_TOPCKGEN,         \
	}

static const struct mtk_gate ethdma_mtk_gate[] = {
	GATE_ETHDMA(CK_ETHDMA_FE_EN, "ethdma_fe_en", CK_TOP_NETSYS_2X_SEL, 6),
};

static int mt7988_ethdma_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7988_topckgen_clk_tree,
					ethdma_mtk_gate);
}

static int mt7988_ethdma_bind(struct udevice *dev)
{
	int ret = 0;

	if (CONFIG_IS_ENABLED(RESET_MEDIATEK)) {
		ret = mediatek_reset_bind(dev, MT7988_ETHDMA_RST_CTRL_OFS, 1);
		if (ret)
			debug("Warning: failed to bind reset controller\n");
	}

	return ret;
}

static const struct udevice_id mt7988_ethdma_compat[] = {
	{
		.compatible = "mediatek,mt7988-ethdma",
	},
	{}
};

U_BOOT_DRIVER(mtk_clk_ethdma) = {
	.name = "mt7988-clock-ethdma",
	.id = UCLASS_CLK,
	.of_match = mt7988_ethdma_compat,
	.probe = mt7988_ethdma_probe,
	.bind = mt7988_ethdma_bind,
	.priv_auto = sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
};

/* SGMIISYS_0 */

static const struct mtk_gate_regs sgmii0_cg_regs = {
	.set_ofs = 0xE4,
	.clr_ofs = 0xE4,
	.sta_ofs = 0xE4,
};

#define GATE_SGMII0(_id, _name, _parent, _shift)                               \
	{                                                                      \
		.id = _id, .parent = _parent, .regs = &sgmii0_cg_regs,         \
		.shift = _shift,                                               \
		.flags = CLK_GATE_NO_SETCLR_INV | CLK_PARENT_TOPCKGEN,         \
	}

static const struct mtk_gate sgmiisys_0_mtk_gate[] = {
	/* connect to fake clock, so use CK_TOP_XTAL as the clock parent */
	GATE_SGMII0(CK_SGM0_TX_EN, "sgm0_tx_en", CK_TOP_XTAL, 2),
	/* connect to fake clock, so use CK_TOP_XTAL as the clock parent */
	GATE_SGMII0(CK_SGM0_RX_EN, "sgm0_rx_en", CK_TOP_XTAL, 3),
};

static int mt7988_sgmiisys_0_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7988_topckgen_clk_tree,
					sgmiisys_0_mtk_gate);
}

static const struct udevice_id mt7988_sgmiisys_0_compat[] = {
	{
		.compatible = "mediatek,mt7988-sgmiisys_0",
	},
	{}
};

U_BOOT_DRIVER(mtk_clk_sgmiisys_0) = {
	.name = "mt7988-clock-sgmiisys_0",
	.id = UCLASS_CLK,
	.of_match = mt7988_sgmiisys_0_compat,
	.probe = mt7988_sgmiisys_0_probe,
	.priv_auto = sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
};

/* SGMIISYS_1 */

static const struct mtk_gate_regs sgmii1_cg_regs = {
	.set_ofs = 0xE4,
	.clr_ofs = 0xE4,
	.sta_ofs = 0xE4,
};

#define GATE_SGMII1(_id, _name, _parent, _shift)                               \
	{                                                                      \
		.id = _id, .parent = _parent, .regs = &sgmii1_cg_regs,         \
		.shift = _shift,                                               \
		.flags = CLK_GATE_NO_SETCLR_INV | CLK_PARENT_TOPCKGEN,         \
	}

static const struct mtk_gate sgmiisys_1_mtk_gate[] = {
	/* connect to fake clock, so use CK_TOP_XTAL as the clock parent */
	GATE_SGMII1(CK_SGM1_TX_EN, "sgm1_tx_en", CK_TOP_XTAL, 2),
	/* connect to fake clock, so use CK_TOP_XTAL as the clock parent */
	GATE_SGMII1(CK_SGM1_RX_EN, "sgm1_rx_en", CK_TOP_XTAL, 3),
};

static int mt7988_sgmiisys_1_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7988_topckgen_clk_tree,
					sgmiisys_1_mtk_gate);
}

static const struct udevice_id mt7988_sgmiisys_1_compat[] = {
	{
		.compatible = "mediatek,mt7988-sgmiisys_1",
	},
	{}
};

U_BOOT_DRIVER(mtk_clk_sgmiisys_1) = {
	.name = "mt7988-clock-sgmiisys_1",
	.id = UCLASS_CLK,
	.of_match = mt7988_sgmiisys_1_compat,
	.probe = mt7988_sgmiisys_1_probe,
	.priv_auto = sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
};

/* ETHWARP */

static const struct mtk_gate_regs ethwarp_cg_regs = {
	.set_ofs = 0x14,
	.clr_ofs = 0x14,
	.sta_ofs = 0x14,
};

#define GATE_ETHWARP(_id, _name, _parent, _shift)                              \
	{                                                                      \
		.id = _id, .parent = _parent, .regs = &ethwarp_cg_regs,        \
		.shift = _shift,                                               \
		.flags = CLK_GATE_NO_SETCLR_INV | CLK_PARENT_TOPCKGEN,         \
	}

static const struct mtk_gate ethwarp_mtk_gate[] = {
	GATE_ETHWARP(CK_ETHWARP_WOCPU2_EN, "ethwarp_wocpu2_en",
		     CK_TOP_NETSYS_MCU_SEL, 13),
	GATE_ETHWARP(CK_ETHWARP_WOCPU1_EN, "ethwarp_wocpu1_en",
		     CK_TOP_NETSYS_MCU_SEL, 14),
	GATE_ETHWARP(CK_ETHWARP_WOCPU0_EN, "ethwarp_wocpu0_en",
		     CK_TOP_NETSYS_MCU_SEL, 15),
};

static int mt7988_ethwarp_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7988_topckgen_clk_tree,
					ethwarp_mtk_gate);
}

static int mt7988_ethwarp_bind(struct udevice *dev)
{
	int ret = 0;

	if (CONFIG_IS_ENABLED(RESET_MEDIATEK)) {
		ret = mediatek_reset_bind(dev, MT7988_ETHWARP_RST_CTRL_OFS, 2);
		if (ret)
			debug("Warning: failed to bind reset controller\n");
	}

	return ret;
}

static const struct udevice_id mt7988_ethwarp_compat[] = {
	{
		.compatible = "mediatek,mt7988-ethwarp",
	},
	{}
};

U_BOOT_DRIVER(mtk_clk_ethwarp) = {
	.name = "mt7988-clock-ethwarp",
	.id = UCLASS_CLK,
	.of_match = mt7988_ethwarp_compat,
	.probe = mt7988_ethwarp_probe,
	.bind = mt7988_ethwarp_bind,
	.priv_auto = sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
};
