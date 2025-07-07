// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek clock driver for MT7986 SoC
 *
 * Copyright (C) 2022 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#include <dm.h>
#include <log.h>
#include <asm/arch-mediatek/reset.h>
#include <asm/io.h>
#include <dt-bindings/clock/mt7986-clk.h>
#include <linux/bitops.h>

#include "clk-mtk.h"

#define MT7986_CLK_PDN 0x250
#define MT7986_CLK_PDN_EN_WRITE BIT(31)

#define APMIXED_PARENT(_id) PARENT(_id, CLK_PARENT_APMIXED)
#define INFRA_PARENT(_id) PARENT(_id, CLK_PARENT_INFRASYS)
#define TOP_PARENT(_id) PARENT(_id, CLK_PARENT_TOPCKGEN)
#define VOID_PARENT PARENT(-1, 0)

#define PLL_FACTOR(_id, _name, _parent, _mult, _div)                           \
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_APMIXED)

#define TOP_FACTOR(_id, _name, _parent, _mult, _div)                           \
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_TOPCKGEN)

#define INFRA_FACTOR(_id, _name, _parent, _mult, _div)                         \
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_INFRASYS)

/* FIXED PLLS */
static const struct mtk_fixed_clk fixed_pll_clks[] = {
	FIXED_CLK(CLK_APMIXED_ARMPLL, CLK_XTAL, 2000000000),
	FIXED_CLK(CLK_APMIXED_NET2PLL, CLK_XTAL, 800000000),
	FIXED_CLK(CLK_APMIXED_MMPLL, CLK_XTAL, 1440000000),
	FIXED_CLK(CLK_APMIXED_SGMPLL, CLK_XTAL, 325000000),
	FIXED_CLK(CLK_APMIXED_WEDMCUPLL, CLK_XTAL, 760000000),
	FIXED_CLK(CLK_APMIXED_NET1PLL, CLK_XTAL, 2500000000),
	FIXED_CLK(CLK_APMIXED_MPLL, CLK_XTAL, 416000000),
	FIXED_CLK(CLK_APMIXED_APLL2, CLK_XTAL, 196608000),
};

/* TOPCKGEN FIXED CLK */
static const struct mtk_fixed_clk top_fixed_clks[] = {
	FIXED_CLK(CLK_TOP_XTAL, CLK_XTAL, 40000000),
};

/* TOPCKGEN FIXED DIV */
static const struct mtk_fixed_factor top_fixed_divs[] = {
	/* TOP Factors */
	TOP_FACTOR(CLK_TOP_XTAL_D2, "xtal_d2", CLK_TOP_XTAL,
		   1, 2),
	TOP_FACTOR(CLK_TOP_RTC_32K, "rtc_32k", CLK_TOP_XTAL, 1,
		   1250),
	TOP_FACTOR(CLK_TOP_RTC_32P7K, "rtc_32p7k", CLK_TOP_XTAL, 1,
		   1220),
	/* Not defined upstream and not used */
	/* TOP_FACTOR(CLK_TOP_A_TUNER, "a_tuner", CLK_TOP_A_TUNER_SEL, 2, 1), */
	/* MPLL */
	PLL_FACTOR(CLK_TOP_MPLL_D2, "mpll_d2", CLK_APMIXED_MPLL, 1, 2),
	PLL_FACTOR(CLK_TOP_MPLL_D4, "mpll_d4", CLK_APMIXED_MPLL, 1, 4),
	PLL_FACTOR(CLK_TOP_MPLL_D8, "mpll_d8", CLK_APMIXED_MPLL, 1, 8),
	PLL_FACTOR(CLK_TOP_MPLL_D8_D2, "mpll_d8_d2", CLK_APMIXED_MPLL, 1, 16),
	PLL_FACTOR(CLK_TOP_MPLL_D3_D2, "mpll_d3_d2", CLK_APMIXED_MPLL, 1, 2),
	/* MMPLL */
	PLL_FACTOR(CLK_TOP_MMPLL_D2, "mmpll_d2", CLK_APMIXED_MMPLL, 1, 2),
	PLL_FACTOR(CLK_TOP_MMPLL_D4, "mmpll_d4", CLK_APMIXED_MMPLL, 1, 4),
	PLL_FACTOR(CLK_TOP_MMPLL_D8, "mmpll_d8", CLK_APMIXED_MMPLL, 1, 8),
	PLL_FACTOR(CLK_TOP_MMPLL_D8_D2, "mmpll_d8_d2", CLK_APMIXED_MMPLL, 1, 16),
	PLL_FACTOR(CLK_TOP_MMPLL_D3_D8, "mmpll_d3_d8", CLK_APMIXED_MMPLL, 1, 8),
	PLL_FACTOR(CLK_TOP_MMPLL_U2PHYD, "mmpll_u2phy", CLK_APMIXED_MMPLL, 1, 30),
	/* APLL2 */
	PLL_FACTOR(CLK_TOP_APLL2_D4, "apll2_d4", CLK_APMIXED_APLL2, 1, 4),
	/* NET1PLL */
	PLL_FACTOR(CLK_TOP_NET1PLL_D4, "net1pll_d4", CLK_APMIXED_NET1PLL, 1, 4),
	PLL_FACTOR(CLK_TOP_NET1PLL_D5, "net1pll_d5", CLK_APMIXED_NET1PLL, 1, 5),
	PLL_FACTOR(CLK_TOP_NET1PLL_D5_D2, "net1pll_d5_d2", CLK_APMIXED_NET1PLL, 1, 10),
	PLL_FACTOR(CLK_TOP_NET1PLL_D5_D4, "net1pll_d5_d4", CLK_APMIXED_NET1PLL, 1, 20),
	PLL_FACTOR(CLK_TOP_NET1PLL_D8_D2, "net1pll_d8_d2", CLK_APMIXED_NET1PLL, 1, 16),
	PLL_FACTOR(CLK_TOP_NET1PLL_D8_D4, "net1pll_d8_d4", CLK_APMIXED_NET1PLL, 1, 32),
	/* NET2PLL */
	PLL_FACTOR(CLK_TOP_NET2PLL_D4, "net2pll_d4", CLK_APMIXED_NET2PLL, 1, 4),
	PLL_FACTOR(CLK_TOP_NET2PLL_D4_D2, "net2pll_d4_d2", CLK_APMIXED_NET2PLL, 1, 8),
	PLL_FACTOR(CLK_TOP_NET2PLL_D3_D2, "net2pll_d3_d2", CLK_APMIXED_NET2PLL, 1, 2),
	/* WEDMCUPLL */
	PLL_FACTOR(CLK_TOP_WEDMCUPLL_D5_D2, "wedmcupll_d5_d2", CLK_APMIXED_WEDMCUPLL, 1,
		   10),
};

/* TOPCKGEN MUX PARENTS */
static const struct mtk_parent nfi1x_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_MMPLL_D8),
	TOP_PARENT(CLK_TOP_NET1PLL_D8_D2), TOP_PARENT(CLK_TOP_NET2PLL_D3_D2),
	TOP_PARENT(CLK_TOP_MPLL_D4), TOP_PARENT(CLK_TOP_MMPLL_D8_D2),
	TOP_PARENT(CLK_TOP_WEDMCUPLL_D5_D2), TOP_PARENT(CLK_TOP_MPLL_D8),
};

static const struct mtk_parent spinfi_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL_D2), TOP_PARENT(CLK_TOP_XTAL),
	TOP_PARENT(CLK_TOP_NET1PLL_D5_D4), TOP_PARENT(CLK_TOP_MPLL_D4),
	TOP_PARENT(CLK_TOP_MMPLL_D8_D2), TOP_PARENT(CLK_TOP_WEDMCUPLL_D5_D2),
	TOP_PARENT(CLK_TOP_MMPLL_D3_D8), TOP_PARENT(CLK_TOP_MPLL_D8),
};

static const struct mtk_parent spi_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_MPLL_D2),
	TOP_PARENT(CLK_TOP_MMPLL_D8), TOP_PARENT(CLK_TOP_NET1PLL_D8_D2),
	TOP_PARENT(CLK_TOP_NET2PLL_D3_D2), TOP_PARENT(CLK_TOP_NET1PLL_D5_D4),
	TOP_PARENT(CLK_TOP_MPLL_D4), TOP_PARENT(CLK_TOP_WEDMCUPLL_D5_D2),
};

static const struct mtk_parent uart_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_MPLL_D8),
	TOP_PARENT(CLK_TOP_MPLL_D8_D2),
};

static const struct mtk_parent pwm_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_NET1PLL_D8_D2),
	TOP_PARENT(CLK_TOP_NET1PLL_D5_D4), TOP_PARENT(CLK_TOP_MPLL_D4),
};

static const struct mtk_parent i2c_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_NET1PLL_D5_D4),
	TOP_PARENT(CLK_TOP_MPLL_D4), TOP_PARENT(CLK_TOP_NET1PLL_D8_D4),
};

static const struct mtk_parent pextp_tl_ck_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_NET1PLL_D5_D4),
	TOP_PARENT(CLK_TOP_NET2PLL_D4_D2), TOP_PARENT(CLK_TOP_RTC_32K),
};

static const struct mtk_parent emmc_250m_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_NET1PLL_D5_D2),
};

static const struct mtk_parent emmc_416m_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), APMIXED_PARENT(CLK_APMIXED_MPLL),
};

static const struct mtk_parent f_26m_adc_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_MPLL_D8_D2),
};

static const struct mtk_parent dramc_md32_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_MPLL_D2),
};

static const struct mtk_parent sysaxi_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_NET1PLL_D8_D2),
	TOP_PARENT(CLK_TOP_NET2PLL_D4),
};

static const struct mtk_parent sysapb_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_MPLL_D3_D2),
	TOP_PARENT(CLK_TOP_NET2PLL_D4_D2),
};

static const struct mtk_parent arm_db_main_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_NET2PLL_D3_D2),
};

static const struct mtk_parent arm_db_jtsel_parents[] = {
	VOID_PARENT, TOP_PARENT(CLK_TOP_XTAL),
};

static const struct mtk_parent netsys_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_MMPLL_D4),
};

static const struct mtk_parent netsys_500m_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_NET1PLL_D5),
};

static const struct mtk_parent netsys_mcu_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), APMIXED_PARENT(CLK_APMIXED_WEDMCUPLL),
	TOP_PARENT(CLK_TOP_MMPLL_D2), TOP_PARENT(CLK_TOP_NET1PLL_D4),
	TOP_PARENT(CLK_TOP_NET1PLL_D5),
};

static const struct mtk_parent netsys_2x_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_APMIXED_NET2PLL),
	APMIXED_PARENT(CLK_APMIXED_WEDMCUPLL), TOP_PARENT(CLK_TOP_MMPLL_D2),
};

static const struct mtk_parent sgm_325m_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), APMIXED_PARENT(CLK_APMIXED_SGMPLL),
};

static const struct mtk_parent sgm_reg_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_NET1PLL_D8_D4),
};

static const struct mtk_parent a1sys_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_APLL2_D4),
};

static const struct mtk_parent conn_mcusys_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_MMPLL_D2),
};

static const struct mtk_parent eip_b_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), APMIXED_PARENT(CLK_APMIXED_NET2PLL),
};

static const struct mtk_parent aud_l_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), APMIXED_PARENT(CLK_APMIXED_APLL2),
	TOP_PARENT(CLK_TOP_MPLL_D8_D2),
};

static const struct mtk_parent a_tuner_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_APLL2_D4),
	TOP_PARENT(CLK_TOP_MPLL_D8_D2),
};

static const struct mtk_parent u2u3_sys_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_NET1PLL_D5_D4),
};

static const struct mtk_parent da_u2_refsel_parents[] = {
	TOP_PARENT(CLK_TOP_XTAL), TOP_PARENT(CLK_TOP_MMPLL_U2PHYD),
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
static const struct mtk_composite top_muxes[] = {
	/* CLK_CFG_0 */
	TOP_MUX(CLK_TOP_NFI1X_SEL, "nfi1x_sel", nfi1x_parents, 0x000, 0x004,
		0x008, 0, 3, 7, 0x1C0, 0),
	TOP_MUX(CLK_TOP_SPINFI_SEL, "spinfi_sel", spinfi_parents, 0x000, 0x004,
		0x008, 8, 3, 15, 0x1C0, 1),
	TOP_MUX(CLK_TOP_SPI_SEL, "spi_sel", spi_parents, 0x000, 0x004, 0x008, 16,
		3, 23, 0x1C0, 2),
	TOP_MUX(CLK_TOP_SPIM_MST_SEL, "spim_mst_sel", spi_parents, 0x000, 0x004,
		0x008, 24, 3, 31, 0x1C0, 3),
	/* CLK_CFG_1 */
	TOP_MUX(CLK_TOP_UART_SEL, "uart_sel", uart_parents, 0x010, 0x014, 0x018,
		0, 2, 7, 0x1C0, 4),
	TOP_MUX(CLK_TOP_PWM_SEL, "pwm_sel", pwm_parents, 0x010, 0x014, 0x018, 8,
		2, 15, 0x1C0, 5),
	TOP_MUX(CLK_TOP_I2C_SEL, "i2c_sel", i2c_parents, 0x010, 0x014, 0x018, 16,
		2, 23, 0x1C0, 6),
	TOP_MUX(CLK_TOP_PEXTP_TL_SEL, "pextp_tl_ck_sel", pextp_tl_ck_parents,
		0x010, 0x014, 0x018, 24, 2, 31, 0x1C0, 7),
	/* CLK_CFG_2 */
	TOP_MUX(CLK_TOP_EMMC_250M_SEL, "emmc_250m_sel", emmc_250m_parents, 0x020,
		0x024, 0x028, 0, 1, 7, 0x1C0, 8),
	TOP_MUX(CLK_TOP_EMMC_416M_SEL, "emmc_416m_sel", emmc_416m_parents, 0x020,
		0x024, 0x028, 8, 1, 15, 0x1C0, 9),
	TOP_MUX(CLK_TOP_F_26M_ADC_SEL, "f_26m_adc_sel", f_26m_adc_parents, 0x020,
		0x024, 0x028, 16, 1, 23, 0x1C0, 10),
	TOP_MUX(CLK_TOP_DRAMC_SEL, "dramc_sel", f_26m_adc_parents, 0x020, 0x024,
		0x028, 24, 1, 31, 0x1C0, 11),
	/* CLK_CFG_3 */
	TOP_MUX(CLK_TOP_DRAMC_MD32_SEL, "dramc_md32_sel", dramc_md32_parents,
		0x030, 0x034, 0x038, 0, 1, 7, 0x1C0, 12),
	TOP_MUX(CLK_TOP_SYSAXI_SEL, "sysaxi_sel", sysaxi_parents, 0x030, 0x034,
		0x038, 8, 2, 15, 0x1C0, 13),
	TOP_MUX(CLK_TOP_SYSAPB_SEL, "sysapb_sel", sysapb_parents, 0x030, 0x034,
		0x038, 16, 2, 23, 0x1C0, 14),
	TOP_MUX(CLK_TOP_ARM_DB_MAIN_SEL, "arm_db_main_sel", arm_db_main_parents,
		0x030, 0x034, 0x038, 24, 1, 31, 0x1C0, 15),
	/* CLK_CFG_4 */
	TOP_MUX(CLK_TOP_ARM_DB_JTSEL, "arm_db_jtsel", arm_db_jtsel_parents,
		0x040, 0x044, 0x048, 0, 1, 7, 0x1C0, 16),
	TOP_MUX(CLK_TOP_NETSYS_SEL, "netsys_sel", netsys_parents, 0x040, 0x044,
		0x048, 8, 1, 15, 0x1C0, 17),
	TOP_MUX(CLK_TOP_NETSYS_500M_SEL, "netsys_500m_sel", netsys_500m_parents,
		0x040, 0x044, 0x048, 16, 1, 23, 0x1C0, 18),
	TOP_MUX(CLK_TOP_NETSYS_MCU_SEL, "netsys_mcu_sel", netsys_mcu_parents,
		0x040, 0x044, 0x048, 24, 3, 31, 0x1C0, 19),
	/* CLK_CFG_5 */
	TOP_MUX(CLK_TOP_NETSYS_2X_SEL, "netsys_2x_sel", netsys_2x_parents, 0x050,
		0x054, 0x058, 0, 2, 7, 0x1C0, 20),
	TOP_MUX(CLK_TOP_SGM_325M_SEL, "sgm_325m_sel", sgm_325m_parents, 0x050,
		0x054, 0x058, 8, 1, 15, 0x1C0, 21),
	TOP_MUX(CLK_TOP_SGM_REG_SEL, "sgm_reg_sel", sgm_reg_parents, 0x050,
		0x054, 0x058, 16, 1, 23, 0x1C0, 22),
	TOP_MUX(CLK_TOP_A1SYS_SEL, "a1sys_sel", a1sys_parents, 0x050, 0x054,
		0x058, 24, 1, 31, 0x1C0, 23),
	/* CLK_CFG_6 */
	TOP_MUX(CLK_TOP_CONN_MCUSYS_SEL, "conn_mcusys_sel", conn_mcusys_parents,
		0x060, 0x064, 0x068, 0, 1, 7, 0x1C0, 24),
	TOP_MUX(CLK_TOP_EIP_B_SEL, "eip_b_sel", eip_b_parents, 0x060, 0x064,
		0x068, 8, 1, 15, 0x1C0, 25),
	TOP_MUX(CLK_TOP_PCIE_PHY_SEL, "pcie_phy_sel", f_26m_adc_parents, 0x060,
		0x064, 0x068, 16, 1, 23, 0x1C0, 26),
	TOP_MUX(CLK_TOP_USB3_PHY_SEL, "usb3_phy_sel", f_26m_adc_parents, 0x060,
		0x064, 0x068, 24, 1, 31, 0x1C0, 27),
	/* CLK_CFG_7 */
	TOP_MUX(CLK_TOP_F26M_SEL, "csw_f26m_sel", f_26m_adc_parents, 0x070,
		0x074, 0x078, 0, 1, 7, 0x1C0, 28),
	TOP_MUX(CLK_TOP_AUD_L_SEL, "aud_l_sel", aud_l_parents, 0x070, 0x074,
		0x078, 8, 2, 15, 0x1C0, 29),
	TOP_MUX(CLK_TOP_A_TUNER_SEL, "a_tuner_sel", a_tuner_parents, 0x070,
		0x074, 0x078, 16, 2, 23, 0x1C0, 30),
	TOP_MUX(CLK_TOP_U2U3_SEL, "u2u3_sel", f_26m_adc_parents, 0x070, 0x074,
		0x078, 24, 1, 31, 0x1C4, 0),
	/* CLK_CFG_8 */
	TOP_MUX(CLK_TOP_U2U3_SYS_SEL, "u2u3_sys_sel", u2u3_sys_parents, 0x080,
		0x084, 0x088, 0, 1, 7, 0x1C4, 1),
	TOP_MUX(CLK_TOP_U2U3_XHCI_SEL, "u2u3_xhci_sel", u2u3_sys_parents, 0x080,
		0x084, 0x088, 8, 1, 15, 0x1C4, 2),
	TOP_MUX(CLK_TOP_DA_U2_REFSEL, "da_u2_refsel", da_u2_refsel_parents,
		0x080, 0x084, 0x088, 16, 1, 23, 0x1C4, 3),
	TOP_MUX(CLK_TOP_DA_U2_CK_1P_SEL, "da_u2_ck_1p_sel", da_u2_refsel_parents,
		0x080, 0x084, 0x088, 24, 1, 31, 0x1C4, 4),
	/* CLK_CFG_9 */
	TOP_MUX(CLK_TOP_AP2CNN_HOST_SEL, "ap2cnn_host_sel", sgm_reg_parents,
		0x090, 0x094, 0x098, 0, 1, 7, 0x1C4, 5),
};

/* INFRA FIXED DIV */
static const struct mtk_fixed_factor infra_fixed_divs[] = {
	TOP_FACTOR(CLK_INFRA_SYSAXI_D2, "infra_sysaxi_d2", CLK_TOP_SYSAXI_SEL, 1, 2),
};

/* INFRASYS MUX PARENTS */


static const struct mtk_parent infra_uart0_parents[] = {
	TOP_PARENT(CLK_TOP_F26M_SEL),
	TOP_PARENT(CLK_TOP_UART_SEL)
};

static const struct mtk_parent infra_spi0_parents[] = {
	TOP_PARENT(CLK_TOP_I2C_SEL),
	TOP_PARENT(CLK_TOP_SPI_SEL)
};

static const struct mtk_parent infra_spi1_parents[] = {
	TOP_PARENT(CLK_TOP_I2C_SEL),
	TOP_PARENT(CLK_TOP_SPINFI_SEL)
};

static const struct mtk_parent infra_pwm_bsel_parents[] = {
	TOP_PARENT(CLK_TOP_RTC_32P7K),
	TOP_PARENT(CLK_TOP_F26M_SEL),
	INFRA_PARENT(CLK_INFRA_SYSAXI_D2),
	TOP_PARENT(CLK_TOP_PWM_SEL)
};

static const struct mtk_parent infra_pcie_parents[] = {
	TOP_PARENT(CLK_TOP_RTC_32P7K),
	TOP_PARENT(CLK_TOP_F26M_SEL),
	TOP_PARENT(CLK_TOP_XTAL),
	TOP_PARENT(CLK_TOP_PEXTP_TL_SEL)
};

#define INFRA_MUX(_id, _name, _parents, _reg, _shift, _width)                  \
	{                                                                      \
		.id = _id, .mux_reg = (_reg) + 0x8,                            \
		.mux_set_reg = (_reg) + 0x0, .mux_clr_reg = (_reg) + 0x4,      \
		.mux_shift = _shift, .mux_mask = BIT(_width) - 1,              \
		.gate_shift = -1, .upd_shift = -1,			       \
		.parent_flags = _parents, .num_parents = ARRAY_SIZE(_parents), \
		.flags = CLK_MUX_SETCLR_UPD | CLK_PARENT_MIXED,                \
	}

/* INFRA MUX */

static const struct mtk_composite infra_muxes[] = {
	/* MODULE_CLK_SEL_0 */
	INFRA_MUX(CLK_INFRA_UART0_SEL, "infra_uart0_sel", infra_uart0_parents,
		  0x10, 0, 1),
	INFRA_MUX(CLK_INFRA_UART1_SEL, "infra_uart1_sel", infra_uart0_parents,
		  0x10, 1, 1),
	INFRA_MUX(CLK_INFRA_UART2_SEL, "infra_uart2_sel", infra_uart0_parents,
		  0x10, 2, 1),
	INFRA_MUX(CLK_INFRA_SPI0_SEL, "infra_spi0_sel", infra_spi0_parents, 0x10,
		  4, 1),
	INFRA_MUX(CLK_INFRA_SPI1_SEL, "infra_spi1_sel", infra_spi1_parents, 0x10,
		  5, 1),
	INFRA_MUX(CLK_INFRA_PWM1_SEL, "infra_pwm1_sel", infra_pwm_bsel_parents,
		  0x10, 9, 2),
	INFRA_MUX(CLK_INFRA_PWM2_SEL, "infra_pwm2_sel", infra_pwm_bsel_parents,
		  0x10, 11, 2),
	INFRA_MUX(CLK_INFRA_PWM_BSEL, "infra_pwm_bsel", infra_pwm_bsel_parents,
		  0x10, 13, 2),
	/* MODULE_CLK_SEL_1 */
	INFRA_MUX(CLK_INFRA_PCIE_SEL, "infra_pcie_sel", infra_pcie_parents, 0x20,
		  0, 2),
};

static const struct mtk_gate_regs infra_0_cg_regs = {
	.set_ofs = 0x40,
	.clr_ofs = 0x44,
	.sta_ofs = 0x48,
};

static const struct mtk_gate_regs infra_1_cg_regs = {
	.set_ofs = 0x50,
	.clr_ofs = 0x54,
	.sta_ofs = 0x58,
};

static const struct mtk_gate_regs infra_2_cg_regs = {
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

/* INFRA GATE */

static const struct mtk_gate infracfg_gates[] = {
	/* INFRA0 */
	GATE_INFRA0_INFRA(CLK_INFRA_GPT_STA, "infra_gpt_sta", CLK_INFRA_SYSAXI_D2, 0),
	GATE_INFRA0_INFRA(CLK_INFRA_PWM_HCK, "infra_pwm_hck", CLK_INFRA_SYSAXI_D2, 1),
	GATE_INFRA0_INFRA(CLK_INFRA_PWM_STA, "infra_pwm_sta", CLK_INFRA_PWM_BSEL, 2),
	GATE_INFRA0_INFRA(CLK_INFRA_PWM1_CK, "infra_pwm1", CLK_INFRA_PWM1_SEL, 3),
	GATE_INFRA0_INFRA(CLK_INFRA_PWM2_CK, "infra_pwm2", CLK_INFRA_PWM2_SEL, 4),
	GATE_INFRA0_TOP(CLK_INFRA_CQ_DMA_CK, "infra_cq_dma", CLK_TOP_SYSAXI_SEL, 6),
	GATE_INFRA0_TOP(CLK_INFRA_EIP97_CK, "infra_eip97", CLK_TOP_EIP_B_SEL, 7),
	GATE_INFRA0_TOP(CLK_INFRA_AUD_BUS_CK, "infra_aud_bus", CLK_TOP_SYSAXI_SEL, 8),
	GATE_INFRA0_TOP(CLK_INFRA_AUD_26M_CK, "infra_aud_26m", CLK_TOP_F26M_SEL, 9),
	GATE_INFRA0_TOP(CLK_INFRA_AUD_L_CK, "infra_aud_l", CLK_TOP_AUD_L_SEL, 10),
	GATE_INFRA0_TOP(CLK_INFRA_AUD_AUD_CK, "infra_aud_aud", CLK_TOP_A1SYS_SEL,
			11),
	GATE_INFRA0_TOP(CLK_INFRA_AUD_EG2_CK, "infra_aud_eg2", CLK_TOP_A_TUNER_SEL,
			13),
	GATE_INFRA0_TOP(CLK_INFRA_DRAMC_26M_CK, "infra_dramc_26m", CLK_TOP_F26M_SEL,
			14),
	GATE_INFRA0_INFRA(CLK_INFRA_DBG_CK, "infra_dbg", CLK_INFRA_SYSAXI_D2, 15),
	GATE_INFRA0_INFRA(CLK_INFRA_AP_DMA_CK, "infra_ap_dma", CLK_INFRA_SYSAXI_D2, 16),
	GATE_INFRA0_INFRA(CLK_INFRA_SEJ_CK, "infra_sej", CLK_INFRA_SYSAXI_D2, 24),
	GATE_INFRA0_TOP(CLK_INFRA_SEJ_13M_CK, "infra_sej_13m", CLK_TOP_F26M_SEL, 25),
	/* INFRA1 */
	GATE_INFRA1_TOP(CLK_INFRA_THERM_CK, "infra_therm", CLK_TOP_F26M_SEL, 0),
	GATE_INFRA1_TOP(CLK_INFRA_I2C0_CK, "infra_i2co", CLK_TOP_I2C_SEL, 1),
	GATE_INFRA1_INFRA(CLK_INFRA_UART0_CK, "infra_uart0", CLK_INFRA_UART0_SEL, 2),
	GATE_INFRA1_INFRA(CLK_INFRA_UART1_CK, "infra_uart1", CLK_INFRA_UART1_SEL, 3),
	GATE_INFRA1_INFRA(CLK_INFRA_UART2_CK, "infra_uart2", CLK_INFRA_UART2_SEL, 4),
	GATE_INFRA1_TOP(CLK_INFRA_NFI1_CK, "infra_nfi1", CLK_TOP_NFI1X_SEL, 8),
	GATE_INFRA1_TOP(CLK_INFRA_SPINFI1_CK, "infra_spinfi1", CLK_TOP_SPINFI_SEL,
			9),
	GATE_INFRA1_INFRA(CLK_INFRA_NFI_HCK_CK, "infra_nfi_hck", CLK_INFRA_SYSAXI_D2, 10),
	GATE_INFRA1_INFRA(CLK_INFRA_SPI0_CK, "infra_spi0", CLK_INFRA_SPI0_SEL, 11),
	GATE_INFRA1_INFRA(CLK_INFRA_SPI1_CK, "infra_spi1", CLK_INFRA_SPI1_SEL, 12),
	GATE_INFRA1_INFRA(CLK_INFRA_SPI0_HCK_CK, "infra_spi0_hck", CLK_INFRA_SYSAXI_D2,
			  13),
	GATE_INFRA1_INFRA(CLK_INFRA_SPI1_HCK_CK, "infra_spi1_hck", CLK_INFRA_SYSAXI_D2,
			  14),
	GATE_INFRA1_TOP(CLK_INFRA_FRTC_CK, "infra_frtc", CLK_TOP_RTC_32K, 15),
	GATE_INFRA1_TOP(CLK_INFRA_MSDC_CK, "infra_msdc", CLK_TOP_EMMC_416M_SEL, 16),
	GATE_INFRA1_TOP(CLK_INFRA_MSDC_HCK_CK, "infra_msdc_hck",
			CLK_TOP_EMMC_250M_SEL, 17),
	GATE_INFRA1_TOP(CLK_INFRA_MSDC_133M_CK, "infra_msdc_133m",
			CLK_TOP_SYSAXI_SEL, 18),
	GATE_INFRA1_INFRA(CLK_INFRA_MSDC_66M_CK, "infra_msdc_66m", CLK_INFRA_SYSAXI_D2,
			  19),
	GATE_INFRA1_INFRA(CLK_INFRA_ADC_26M_CK, "infra_adc_26m", CLK_INFRA_ADC_FRC_CK, 20),
	GATE_INFRA1_TOP(CLK_INFRA_ADC_FRC_CK, "infra_adc_frc", CLK_TOP_F26M_SEL, 21),
	GATE_INFRA1_TOP(CLK_INFRA_FBIST2FPC_CK, "infra_fbist2fpc", CLK_TOP_NFI1X_SEL,
			23),
	/* INFRA2 */
	GATE_INFRA2_TOP(CLK_INFRA_IUSB_133_CK, "infra_iusb_133", CLK_TOP_SYSAXI_SEL,
			0),
	GATE_INFRA2_INFRA(CLK_INFRA_IUSB_66M_CK, "infra_iusb_66m", CLK_INFRA_SYSAXI_D2,
			  1),
	GATE_INFRA2_TOP(CLK_INFRA_IUSB_SYS_CK, "infra_iusb_sys", CLK_TOP_U2U3_SYS_SEL,
			2),
	GATE_INFRA2_TOP(CLK_INFRA_IUSB_CK, "infra_iusb", CLK_TOP_U2U3_SEL, 3),
	GATE_INFRA2_TOP(CLK_INFRA_IPCIE_CK, "infra_ipcie", CLK_TOP_PEXTP_TL_SEL, 12),
	GATE_INFRA2_TOP(CLK_INFRA_IPCIE_PIPE_CK, "infra_ipcie_pipe", CLK_TOP_XTAL, 13),
	GATE_INFRA2_TOP(CLK_INFRA_IPCIER_CK, "infra_ipcier", CLK_TOP_F26M_SEL, 14),
	GATE_INFRA2_TOP(CLK_INFRA_IPCIEB_CK, "infra_ipcieb", CLK_TOP_SYSAXI_SEL, 15),
	/* upstream linux unordered */
	GATE_INFRA0_TOP(CLK_INFRA_TRNG_CK, "infra_trng", CLK_TOP_SYSAXI_SEL, 26),
};

static const struct mtk_clk_tree mt7986_fixed_pll_clk_tree = {
	.fdivs_offs = CLK_APMIXED_NR_CLK,
	.xtal_rate = 40 * MHZ,
	.fclks = fixed_pll_clks,
	.flags = CLK_APMIXED,
};

static const struct mtk_clk_tree mt7986_topckgen_clk_tree = {
	.fdivs_offs = CLK_TOP_XTAL_D2,
	.muxes_offs = CLK_TOP_NFI1X_SEL,
	.fclks = top_fixed_clks,
	.fdivs = top_fixed_divs,
	.muxes = top_muxes,
	.flags = CLK_BYPASS_XTAL | CLK_TOPCKGEN,
};

static const struct mtk_clk_tree mt7986_infracfg_clk_tree = {
	.fdivs_offs = CLK_INFRA_SYSAXI_D2,
	.muxes_offs = CLK_INFRA_UART0_SEL,
	.gates_offs = CLK_INFRA_GPT_STA,
	.fdivs = infra_fixed_divs,
	.muxes = infra_muxes,
	.gates = infracfg_gates,
	.flags = CLK_INFRASYS,
};

static const struct udevice_id mt7986_fixed_pll_compat[] = {
	{ .compatible = "mediatek,mt7986-fixed-plls" },
	{ .compatible = "mediatek,mt7986-apmixedsys" },
	{}
};

static const struct udevice_id mt7986_topckgen_compat[] = {
	{ .compatible = "mediatek,mt7986-topckgen" },
	{}
};

static int mt7986_fixed_pll_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt7986_fixed_pll_clk_tree);
}

static int mt7986_topckgen_probe(struct udevice *dev)
{
	struct mtk_clk_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	writel(MT7986_CLK_PDN_EN_WRITE, priv->base + MT7986_CLK_PDN);

	return mtk_common_clk_init(dev, &mt7986_topckgen_clk_tree);
}

U_BOOT_DRIVER(mtk_clk_apmixedsys) = {
	.name = "mt7986-clock-fixed-pll",
	.id = UCLASS_CLK,
	.of_match = mt7986_fixed_pll_compat,
	.probe = mt7986_fixed_pll_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_fixed_pll_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_topckgen) = {
	.name = "mt7986-clock-topckgen",
	.id = UCLASS_CLK,
	.of_match = mt7986_topckgen_compat,
	.probe = mt7986_topckgen_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_topckgen_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

static const struct udevice_id mt7986_infracfg_compat[] = {
	{ .compatible = "mediatek,mt7986-infracfg" },
	{}
};

static int mt7986_infracfg_probe(struct udevice *dev)
{
	return mtk_common_clk_infrasys_init(dev, &mt7986_infracfg_clk_tree);
}

U_BOOT_DRIVER(mtk_clk_infracfg) = {
	.name = "mt7986-clock-infracfg",
	.id = UCLASS_CLK,
	.of_match = mt7986_infracfg_compat,
	.probe = mt7986_infracfg_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_infrasys_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

/* ethsys */
static const struct mtk_gate_regs eth_cg_regs = {
	.sta_ofs = 0x30,
};

#define GATE_ETH(_id, _name, _parent, _shift)                                  \
	{                                                                      \
		.id = _id, .parent = _parent, .regs = &eth_cg_regs,            \
		.shift = _shift,                                               \
		.flags = CLK_GATE_NO_SETCLR_INV | CLK_PARENT_TOPCKGEN,         \
	}

static const struct mtk_gate eth_cgs[] = {
	GATE_ETH(CLK_ETH_FE_EN, "eth_fe_en", CLK_TOP_NETSYS_2X_SEL, 7),
	GATE_ETH(CLK_ETH_GP2_EN, "eth_gp2_en", CLK_TOP_SGM_325M_SEL, 8),
	GATE_ETH(CLK_ETH_GP1_EN, "eth_gp1_en", CLK_TOP_SGM_325M_SEL, 8),
	GATE_ETH(CLK_ETH_WOCPU1_EN, "eth_wocpu1_en", CLK_TOP_NETSYS_MCU_SEL, 14),
	GATE_ETH(CLK_ETH_WOCPU0_EN, "eth_wocpu0_en", CLK_TOP_NETSYS_MCU_SEL, 15),
};

static int mt7986_ethsys_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7986_topckgen_clk_tree,
					eth_cgs);
}

static int mt7986_ethsys_bind(struct udevice *dev)
{
	int ret = 0;

	if (CONFIG_IS_ENABLED(RESET_MEDIATEK)) {
		ret = mediatek_reset_bind(dev, ETHSYS_HIFSYS_RST_CTRL_OFS, 1);
		if (ret)
			debug("Warning: failed to bind reset controller\n");
	}

	return ret;
}

static const struct udevice_id mt7986_ethsys_compat[] = {
	{ .compatible = "mediatek,mt7986-ethsys" },
	{ }
};

U_BOOT_DRIVER(mtk_clk_ethsys) = {
	.name = "mt7986-clock-ethsys",
	.id = UCLASS_CLK,
	.of_match = mt7986_ethsys_compat,
	.probe = mt7986_ethsys_probe,
	.bind = mt7986_ethsys_bind,
	.priv_auto = sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
};
