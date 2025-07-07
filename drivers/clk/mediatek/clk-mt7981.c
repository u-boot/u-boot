// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek clock driver for MT7981 SoC
 *
 * Copyright (C) 2022 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#include <dm.h>
#include <log.h>
#include <asm/arch-mediatek/reset.h>
#include <asm/io.h>
#include <dt-bindings/clock/mt7981-clk.h>
#include <linux/bitops.h>

#include "clk-mtk.h"

#define MT7981_CLK_PDN 0x250
#define MT7981_CLK_PDN_EN_WRITE BIT(31)

#define PLL_FACTOR(_id, _name, _parent, _mult, _div)                           \
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_APMIXED)

#define TOP_FACTOR(_id, _name, _parent, _mult, _div)                           \
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_TOPCKGEN)

#define INFRA_FACTOR(_id, _name, _parent, _mult, _div)                         \
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_INFRASYS)

/* FIXED PLLS */
static const struct mtk_fixed_clk fixed_pll_clks[] = {
	FIXED_CLK(CLK_APMIXED_ARMPLL, CLK_XTAL, 1300000000),
	FIXED_CLK(CLK_APMIXED_NET2PLL, CLK_XTAL, 800000000),
	FIXED_CLK(CLK_APMIXED_MMPLL, CLK_XTAL, 720000000),
	FIXED_CLK(CLK_APMIXED_SGMPLL, CLK_XTAL, 325000000),
	FIXED_CLK(CLK_APMIXED_WEDMCUPLL, CLK_XTAL, 208000000),
	FIXED_CLK(CLK_APMIXED_NET1PLL, CLK_XTAL, 2500000000),
	FIXED_CLK(CLK_APMIXED_MPLL, CLK_XTAL, 416000000),
	FIXED_CLK(CLK_APMIXED_APLL2, CLK_XTAL, 196608000),
};

/* TOPCKGEN FIXED CLK */
static const struct mtk_fixed_clk top_fixed_clks[] = {
	FIXED_CLK(CLK_TOP_CB_CKSQ_40M, CLK_XTAL, 40000000),
};

/* TOPCKGEN FIXED DIV */
static const struct mtk_fixed_factor top_fixed_divs[] = {
	PLL_FACTOR(CLK_TOP_CB_M_416M, "cb_m_416m", CLK_APMIXED_MPLL, 1, 1),
	PLL_FACTOR(CLK_TOP_CB_M_D2, "cb_m_d2", CLK_APMIXED_MPLL, 1, 2),
	PLL_FACTOR(CLK_TOP_CB_M_D3, "cb_m_d3", CLK_APMIXED_MPLL, 1, 3),
	PLL_FACTOR(CLK_TOP_M_D3_D2, "m_d3_d2", CLK_APMIXED_MPLL, 1, 2),
	PLL_FACTOR(CLK_TOP_CB_M_D4, "cb_m_d4", CLK_APMIXED_MPLL, 1, 4),
	PLL_FACTOR(CLK_TOP_CB_M_D8, "cb_m_d8", CLK_APMIXED_MPLL, 1, 8),
	PLL_FACTOR(CLK_TOP_M_D8_D2, "m_d8_d2", CLK_APMIXED_MPLL, 1, 16),
	PLL_FACTOR(CLK_TOP_CB_MM_720M, "cb_mm_720m", CLK_APMIXED_MMPLL, 1, 1),
	PLL_FACTOR(CLK_TOP_CB_MM_D2, "cb_mm_d2", CLK_APMIXED_MMPLL, 1, 2),
	PLL_FACTOR(CLK_TOP_CB_MM_D3, "cb_mm_d3", CLK_APMIXED_MMPLL, 1, 3),
	PLL_FACTOR(CLK_TOP_CB_MM_D3_D5, "cb_mm_d3_d5", CLK_APMIXED_MMPLL, 1, 15),
	PLL_FACTOR(CLK_TOP_CB_MM_D4, "cb_mm_d4", CLK_APMIXED_MMPLL, 1, 4),
	PLL_FACTOR(CLK_TOP_CB_MM_D6, "cb_mm_d6", CLK_APMIXED_MMPLL, 1, 6),
	PLL_FACTOR(CLK_TOP_MM_D6_D2, "mm_d6_d2", CLK_APMIXED_MMPLL, 1, 12),
	PLL_FACTOR(CLK_TOP_CB_MM_D8, "cb_mm_d8", CLK_APMIXED_MMPLL, 1, 8),
	PLL_FACTOR(CLK_TOP_CB_APLL2_196M, "cb_apll2_196m", CLK_APMIXED_APLL2, 1,
		   1),
	PLL_FACTOR(CLK_TOP_APLL2_D2, "apll2_d2", CLK_APMIXED_APLL2, 1, 2),
	PLL_FACTOR(CLK_TOP_APLL2_D4, "apll2_d4", CLK_APMIXED_APLL2, 1, 4),
	PLL_FACTOR(CLK_TOP_NET1_2500M, "net1_2500m", CLK_APMIXED_NET1PLL, 1, 1),
	PLL_FACTOR(CLK_TOP_CB_NET1_D4, "cb_net1_d4", CLK_APMIXED_NET1PLL, 1, 4),
	PLL_FACTOR(CLK_TOP_CB_NET1_D5, "cb_net1_d5", CLK_APMIXED_NET1PLL, 1, 5),
	PLL_FACTOR(CLK_TOP_NET1_D5_D2, "net1_d5_d2", CLK_APMIXED_NET1PLL, 1, 10),
	PLL_FACTOR(CLK_TOP_NET1_D5_D4, "net1_d5_d4", CLK_APMIXED_NET1PLL, 1, 20),
	PLL_FACTOR(CLK_TOP_CB_NET1_D8, "cb_net1_d8", CLK_APMIXED_NET1PLL, 1, 8),
	PLL_FACTOR(CLK_TOP_NET1_D8_D2, "net1_d8_d2", CLK_APMIXED_NET1PLL, 1, 16),
	PLL_FACTOR(CLK_TOP_NET1_D8_D4, "net1_d8_d4", CLK_APMIXED_NET1PLL, 1, 32),
	PLL_FACTOR(CLK_TOP_CB_NET2_800M, "cb_net2_800m", CLK_APMIXED_NET2PLL, 1,
		   1),
	PLL_FACTOR(CLK_TOP_CB_NET2_D2, "cb_net2_d2", CLK_APMIXED_NET2PLL, 1, 2),
	PLL_FACTOR(CLK_TOP_CB_NET2_D4, "cb_net2_d4", CLK_APMIXED_NET2PLL, 1, 4),
	PLL_FACTOR(CLK_TOP_NET2_D4_D2, "net2_d4_d2", CLK_APMIXED_NET2PLL, 1, 8),
	PLL_FACTOR(CLK_TOP_NET2_D4_D4, "net2_d4_d4", CLK_APMIXED_NET2PLL, 1, 16),
	PLL_FACTOR(CLK_TOP_CB_NET2_D6, "cb_net2_d6", CLK_APMIXED_NET2PLL, 1, 6),
	PLL_FACTOR(CLK_TOP_CB_WEDMCU_208M, "cb_wedmcu_208m",
		   CLK_APMIXED_WEDMCUPLL, 1, 1),
	PLL_FACTOR(CLK_TOP_CB_SGM_325M, "cb_sgm_325m", CLK_APMIXED_SGMPLL, 1, 1),
	TOP_FACTOR(CLK_TOP_CKSQ_40M_D2, "cksq_40m_d2", CLK_TOP_CB_CKSQ_40M, 1, 2),
	TOP_FACTOR(CLK_TOP_CB_RTC_32K, "cb_rtc_32k", CLK_TOP_CB_CKSQ_40M, 1,
		   1250),
	TOP_FACTOR(CLK_TOP_CB_RTC_32P7K, "cb_rtc_32p7k", CLK_TOP_CB_CKSQ_40M, 1,
		   1220),
	TOP_FACTOR(CLK_TOP_USB_TX250M, "usb_tx250m", CLK_TOP_CB_CKSQ_40M, 1, 1),
	TOP_FACTOR(CLK_TOP_FAUD, "faud", CLK_TOP_AUD_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_NFI1X, "nfi1x", CLK_TOP_NFI1X_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_USB_EQ_RX250M, "usb_eq_rx250m", CLK_TOP_CB_CKSQ_40M, 1,
		   1),
	TOP_FACTOR(CLK_TOP_USB_CDR_CK, "usb_cdr", CLK_TOP_CB_CKSQ_40M, 1, 1),
	TOP_FACTOR(CLK_TOP_USB_LN0_CK, "usb_ln0", CLK_TOP_CB_CKSQ_40M, 1, 1),
	TOP_FACTOR(CLK_TOP_SPINFI_BCK, "spinfi_bck", CLK_TOP_SPINFI_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_SPI, "spi", CLK_TOP_SPI_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_SPIM_MST, "spim_mst", CLK_TOP_SPIM_MST_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_UART_BCK, "uart_bck", CLK_TOP_UART_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_PWM_BCK, "pwm_bck", CLK_TOP_PWM_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_I2C_BCK, "i2c_bck", CLK_TOP_I2C_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_PEXTP_TL, "pextp_tl", CLK_TOP_PEXTP_TL_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_EMMC_208M, "emmc_208m", CLK_TOP_EMMC_208M_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_EMMC_400M, "emmc_400m", CLK_TOP_EMMC_400M_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_DRAMC_REF, "dramc_ref", CLK_TOP_DRAMC_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_DRAMC_MD32, "dramc_md32", CLK_TOP_DRAMC_MD32_SEL, 1,
		   1),
	TOP_FACTOR(CLK_TOP_SYSAXI, "sysaxi", CLK_TOP_SYSAXI_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_SYSAPB, "sysapb", CLK_TOP_SYSAPB_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_ARM_DB_MAIN, "arm_db_main", CLK_TOP_ARM_DB_MAIN_SEL, 1,
		   1),
	TOP_FACTOR(CLK_TOP_AP2CNN_HOST, "ap2cnn_host", CLK_TOP_AP2CNN_HOST_SEL, 1,
		   1),
	TOP_FACTOR(CLK_TOP_NETSYS, "netsys", CLK_TOP_NETSYS_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_NETSYS_500M, "netsys_500m", CLK_TOP_NETSYS_500M_SEL, 1,
		   1),
	TOP_FACTOR(CLK_TOP_NETSYS_WED_MCU, "netsys_wed_mcu",
		   CLK_TOP_NETSYS_MCU_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_NETSYS_2X, "netsys_2x", CLK_TOP_NETSYS_2X_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_SGM_325M, "sgm_325m", CLK_TOP_SGM_325M_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_SGM_REG, "sgm_reg", CLK_TOP_SGM_REG_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_F26M, "csw_f26m", CLK_TOP_F26M_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_EIP97B, "eip97b", CLK_TOP_EIP97B_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_USB3_PHY, "usb3_phy", CLK_TOP_USB3_PHY_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_AUD, "aud", CLK_TOP_FAUD, 1, 1),
	TOP_FACTOR(CLK_TOP_A1SYS, "a1sys", CLK_TOP_A1SYS_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_AUD_L, "aud_l", CLK_TOP_AUD_L_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_A_TUNER, "a_tuner", CLK_TOP_A_TUNER_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_U2U3_REF, "u2u3_ref", CLK_TOP_U2U3_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_U2U3_SYS, "u2u3_sys", CLK_TOP_U2U3_SYS_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_U2U3_XHCI, "u2u3_xhci", CLK_TOP_U2U3_XHCI_SEL, 1, 1),
	TOP_FACTOR(CLK_TOP_USB_FRMCNT, "usb_frmcnt", CLK_TOP_USB_FRMCNT_SEL, 1,
		   1),
};

/* TOPCKGEN MUX PARENTS */
static const int nfi1x_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_CB_MM_D4,
				     CLK_TOP_NET1_D8_D2,  CLK_TOP_CB_NET2_D6,
				     CLK_TOP_CB_M_D4,     CLK_TOP_CB_MM_D8,
				     CLK_TOP_NET1_D8_D4,  CLK_TOP_CB_M_D8 };

static const int spinfi_parents[] = { CLK_TOP_CKSQ_40M_D2, CLK_TOP_CB_CKSQ_40M,
				      CLK_TOP_NET1_D5_D4,  CLK_TOP_CB_M_D4,
				      CLK_TOP_CB_MM_D8,    CLK_TOP_NET1_D8_D4,
				      CLK_TOP_MM_D6_D2,    CLK_TOP_CB_M_D8 };

static const int spi_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_CB_M_D2,
				   CLK_TOP_CB_MM_D4,    CLK_TOP_NET1_D8_D2,
				   CLK_TOP_CB_NET2_D6,  CLK_TOP_NET1_D5_D4,
				   CLK_TOP_CB_M_D4,     CLK_TOP_NET1_D8_D4 };

static const int uart_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_CB_M_D8,
				    CLK_TOP_M_D8_D2 };

static const int pwm_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_NET1_D8_D2,
				   CLK_TOP_NET1_D5_D4,  CLK_TOP_CB_M_D4,
				   CLK_TOP_M_D8_D2,     CLK_TOP_CB_RTC_32K };

static const int i2c_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_NET1_D5_D4,
				   CLK_TOP_CB_M_D4, CLK_TOP_NET1_D8_D4 };

static const int pextp_tl_ck_parents[] = { CLK_TOP_CB_CKSQ_40M,
					   CLK_TOP_NET1_D5_D4, CLK_TOP_CB_M_D4,
					   CLK_TOP_CB_RTC_32K };

static const int emmc_208m_parents[] = {
	CLK_TOP_CB_CKSQ_40M,   CLK_TOP_CB_M_D2,  CLK_TOP_CB_NET2_D4,
	CLK_TOP_CB_APLL2_196M, CLK_TOP_CB_MM_D4, CLK_TOP_NET1_D8_D2,
	CLK_TOP_CB_MM_D6
};

static const int emmc_400m_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_CB_NET2_D2,
					 CLK_TOP_CB_MM_D2, CLK_TOP_CB_NET2_D2 };

static const int csw_f26m_parents[] = { CLK_TOP_CKSQ_40M_D2, CLK_TOP_M_D8_D2 };

static const int dramc_md32_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_CB_M_D2,
					  CLK_TOP_CB_WEDMCU_208M };

static const int sysaxi_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_NET1_D8_D2 };

static const int sysapb_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_M_D3_D2 };

static const int arm_db_main_parents[] = { CLK_TOP_CB_CKSQ_40M,
					   CLK_TOP_CB_NET2_D6 };

static const int ap2cnn_host_parents[] = { CLK_TOP_CB_CKSQ_40M,
					   CLK_TOP_NET1_D8_D4 };

static const int netsys_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_CB_MM_D2 };

static const int netsys_500m_parents[] = { CLK_TOP_CB_CKSQ_40M,
					   CLK_TOP_CB_NET1_D5 };

static const int netsys_mcu_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_CB_MM_720M,
					  CLK_TOP_CB_NET1_D4, CLK_TOP_CB_NET1_D5,
					  CLK_TOP_CB_M_416M };

static const int netsys_2x_parents[] = { CLK_TOP_CB_CKSQ_40M,
					 CLK_TOP_CB_NET2_800M,
					 CLK_TOP_CB_MM_720M };

static const int sgm_325m_parents[] = { CLK_TOP_CB_CKSQ_40M,
					CLK_TOP_CB_SGM_325M };

static const int sgm_reg_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_CB_NET2_D4 };

static const int eip97b_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_CB_NET1_D5,
				      CLK_TOP_CB_M_416M, CLK_TOP_CB_MM_D2,
				      CLK_TOP_NET1_D5_D2 };

static const int aud_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_CB_APLL2_196M };

static const int a1sys_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_APLL2_D4 };

static const int aud_l_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_CB_APLL2_196M,
				     CLK_TOP_M_D8_D2 };

static const int a_tuner_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_APLL2_D4,
				       CLK_TOP_M_D8_D2 };

static const int u2u3_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_M_D8_D2 };

static const int u2u3_sys_parents[] = { CLK_TOP_CB_CKSQ_40M, CLK_TOP_NET1_D5_D4 };

static const int usb_frmcnt_parents[] = { CLK_TOP_CB_CKSQ_40M,
					  CLK_TOP_CB_MM_D3_D5 };

#define TOP_MUX(_id, _name, _parents, _mux_ofs, _mux_set_ofs, _mux_clr_ofs,    \
		_shift, _width, _gate, _upd_ofs, _upd)                         \
	{                                                                      \
		.id = _id, .mux_reg = _mux_ofs, .mux_set_reg = _mux_set_ofs,   \
		.mux_clr_reg = _mux_clr_ofs, .upd_reg = _upd_ofs,              \
		.upd_shift = _upd, .mux_shift = _shift,                        \
		.mux_mask = BIT(_width) - 1, .gate_reg = _mux_ofs,             \
		.gate_shift = _gate, .parent = _parents,                       \
		.num_parents = ARRAY_SIZE(_parents),                           \
		.flags = CLK_MUX_SETCLR_UPD,                                   \
	}

/* TOPCKGEN MUX_GATE */
static const struct mtk_composite top_muxes[] = {
	TOP_MUX(CLK_TOP_NFI1X_SEL, "nfi1x_sel", nfi1x_parents, 0x0, 0x4, 0x8, 0,
		3, 7, 0x1c0, 0),
	TOP_MUX(CLK_TOP_SPINFI_SEL, "spinfi_sel", spinfi_parents, 0x0, 0x4, 0x8,
		8, 3, 15, 0x1c0, 1),
	TOP_MUX(CLK_TOP_SPI_SEL, "spi_sel", spi_parents, 0x0, 0x4, 0x8, 16, 3,
		23, 0x1c0, 2),
	TOP_MUX(CLK_TOP_SPIM_MST_SEL, "spim_mst_sel", spi_parents, 0x0, 0x4, 0x8,
		24, 3, 31, 0x1c0, 3),
	TOP_MUX(CLK_TOP_UART_SEL, "uart_sel", uart_parents, 0x10, 0x14, 0x18, 0,
		2, 7, 0x1c0, 4),
	TOP_MUX(CLK_TOP_PWM_SEL, "pwm_sel", pwm_parents, 0x10, 0x14, 0x18, 8, 3,
		15, 0x1c0, 5),
	TOP_MUX(CLK_TOP_I2C_SEL, "i2c_sel", i2c_parents, 0x10, 0x14, 0x18, 16, 2,
		23, 0x1c0, 6),
	TOP_MUX(CLK_TOP_PEXTP_TL_SEL, "pextp_tl_ck_sel", pextp_tl_ck_parents,
		0x10, 0x14, 0x18, 24, 2, 31, 0x1c0, 7),
	TOP_MUX(CLK_TOP_EMMC_208M_SEL, "emmc_208m_sel", emmc_208m_parents, 0x20,
		0x24, 0x28, 0, 3, 7, 0x1c0, 8),
	TOP_MUX(CLK_TOP_EMMC_400M_SEL, "emmc_400m_sel", emmc_400m_parents, 0x20,
		0x24, 0x28, 8, 2, 15, 0x1c0, 9),
	TOP_MUX(CLK_TOP_F26M_SEL, "csw_f26m_sel", csw_f26m_parents, 0x20, 0x24,
		0x28, 16, 1, 23, 0x1c0, 10),
	TOP_MUX(CLK_TOP_DRAMC_SEL, "dramc_sel", csw_f26m_parents, 0x20, 0x24,
		0x28, 24, 1, 31, 0x1c0, 11),
	TOP_MUX(CLK_TOP_DRAMC_MD32_SEL, "dramc_md32_sel", dramc_md32_parents,
		0x30, 0x34, 0x38, 0, 2, 7, 0x1c0, 12),
	TOP_MUX(CLK_TOP_SYSAXI_SEL, "sysaxi_sel", sysaxi_parents, 0x30, 0x34,
		0x38, 8, 1, 15, 0x1c0, 13),
	TOP_MUX(CLK_TOP_SYSAPB_SEL, "sysapb_sel", sysapb_parents, 0x30, 0x34,
		0x38, 16, 1, 23, 0x1c0, 14),
	TOP_MUX(CLK_TOP_ARM_DB_MAIN_SEL, "arm_db_main_sel", arm_db_main_parents,
		0x30, 0x34, 0x38, 24, 1, 31, 0x1c0, 15),
	TOP_MUX(CLK_TOP_AP2CNN_HOST_SEL, "ap2cnn_host_sel", ap2cnn_host_parents,
		0x40, 0x44, 0x48, 0, 1, 7, 0x1c0, 16),
	TOP_MUX(CLK_TOP_NETSYS_SEL, "netsys_sel", netsys_parents, 0x40, 0x44,
		0x48, 8, 1, 15, 0x1c0, 17),
	TOP_MUX(CLK_TOP_NETSYS_500M_SEL, "netsys_500m_sel", netsys_500m_parents,
		0x40, 0x44, 0x48, 16, 1, 23, 0x1c0, 18),
	TOP_MUX(CLK_TOP_NETSYS_MCU_SEL, "netsys_mcu_sel", netsys_mcu_parents,
		0x40, 0x44, 0x48, 24, 3, 31, 0x1c0, 19),
	TOP_MUX(CLK_TOP_NETSYS_2X_SEL, "netsys_2x_sel", netsys_2x_parents, 0x50,
		0x54, 0x58, 0, 2, 7, 0x1c0, 20),
	TOP_MUX(CLK_TOP_SGM_325M_SEL, "sgm_325m_sel", sgm_325m_parents, 0x50,
		0x54, 0x58, 8, 1, 15, 0x1c0, 21),
	TOP_MUX(CLK_TOP_SGM_REG_SEL, "sgm_reg_sel", sgm_reg_parents, 0x50, 0x54,
		0x58, 16, 1, 23, 0x1c0, 22),
	TOP_MUX(CLK_TOP_EIP97B_SEL, "eip97b_sel", eip97b_parents, 0x50, 0x54,
		0x58, 24, 3, 31, 0x1c0, 23),
	TOP_MUX(CLK_TOP_USB3_PHY_SEL, "usb3_phy_sel", csw_f26m_parents, 0x60,
		0x64, 0x68, 0, 1, 7, 0x1c0, 24),
	TOP_MUX(CLK_TOP_AUD_SEL, "aud_sel", aud_parents, 0x60, 0x64, 0x68, 8, 1,
		15, 0x1c0, 25),
	TOP_MUX(CLK_TOP_A1SYS_SEL, "a1sys_sel", a1sys_parents, 0x60, 0x64, 0x68,
		16, 1, 23, 0x1c0, 26),
	TOP_MUX(CLK_TOP_AUD_L_SEL, "aud_l_sel", aud_l_parents, 0x60, 0x64, 0x68,
		24, 2, 31, 0x1c0, 27),
	TOP_MUX(CLK_TOP_A_TUNER_SEL, "a_tuner_sel", a_tuner_parents, 0x70, 0x74,
		0x78, 0, 2, 7, 0x1c0, 28),
	TOP_MUX(CLK_TOP_U2U3_SEL, "u2u3_sel", u2u3_parents, 0x70, 0x74, 0x78, 8,
		1, 15, 0x1c0, 29),
	TOP_MUX(CLK_TOP_U2U3_SYS_SEL, "u2u3_sys_sel", u2u3_sys_parents, 0x70,
		0x74, 0x78, 16, 1, 23, 0x1c0, 30),
	TOP_MUX(CLK_TOP_U2U3_XHCI_SEL, "u2u3_xhci_sel", u2u3_sys_parents, 0x70,
		0x74, 0x78, 24, 1, 31, 0x1c4, 0),
	TOP_MUX(CLK_TOP_USB_FRMCNT_SEL, "usb_frmcnt_sel", usb_frmcnt_parents,
		0x80, 0x84, 0x88, 0, 1, 7, 0x1c4, 1),
};

/* INFRA FIXED DIV */
static const struct mtk_fixed_factor infra_fixed_divs[] = {
	TOP_FACTOR(CLK_INFRA_66M_MCK, "infra_66m_mck", CLK_TOP_SYSAXI_SEL, 1, 2),
};

/* INFRASYS MUX PARENTS */
#define INFRA_PARENT(_id) PARENT(_id, CLK_PARENT_INFRASYS)
#define TOP_PARENT(_id) PARENT(_id, CLK_PARENT_TOPCKGEN)
#define VOID_PARENT PARENT(-1, 0)

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
	TOP_PARENT(CLK_TOP_SPIM_MST_SEL)
};

static const struct mtk_parent infra_pwm1_parents[] = {
	VOID_PARENT,
	TOP_PARENT(CLK_TOP_PWM_SEL)
};

static const struct mtk_parent infra_pwm_bsel_parents[] = {
	TOP_PARENT(CLK_TOP_CB_RTC_32P7K),
	TOP_PARENT(CLK_TOP_F26M_SEL),
	INFRA_PARENT(CLK_INFRA_66M_MCK),
	TOP_PARENT(CLK_TOP_PWM_SEL)
};

static const struct mtk_parent infra_pcie_parents[] = {
	TOP_PARENT(CLK_TOP_CB_RTC_32P7K),
	TOP_PARENT(CLK_TOP_F26M_SEL),
	TOP_PARENT(CLK_TOP_CB_CKSQ_40M),
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
	INFRA_MUX(CLK_INFRA_SPI2_SEL, "infra_spi2_sel", infra_spi0_parents, 0x10,
		  6, 1),
	INFRA_MUX(CLK_INFRA_PWM1_SEL, "infra_pwm1_sel", infra_pwm1_parents, 0x10,
		  9, 1),
	INFRA_MUX(CLK_INFRA_PWM2_SEL, "infra_pwm2_sel", infra_pwm1_parents, 0x10,
		  11, 1),
	INFRA_MUX(CLK_INFRA_PWM3_SEL, "infra_pwm3_sel", infra_pwm1_parents, 0x10,
		  15, 1),
	INFRA_MUX(CLK_INFRA_PWM_BSEL, "infra_pwm_bsel", infra_pwm_bsel_parents,
		  0x10, 13, 2),
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
	GATE_INFRA0_INFRA(CLK_INFRA_GPT_STA, "infra_gpt_sta", CLK_INFRA_66M_MCK, 0),
	GATE_INFRA0_INFRA(CLK_INFRA_PWM_HCK, "infra_pwm_hck", CLK_INFRA_66M_MCK, 1),
	GATE_INFRA0_INFRA(CLK_INFRA_PWM_STA, "infra_pwm_sta", CLK_INFRA_PWM_BSEL, 2),
	GATE_INFRA0_INFRA(CLK_INFRA_PWM1_CK, "infra_pwm1", CLK_INFRA_PWM1_SEL, 3),
	GATE_INFRA0_INFRA(CLK_INFRA_PWM2_CK, "infra_pwm2", CLK_INFRA_PWM2_SEL, 4),
	GATE_INFRA0_INFRA(CLK_INFRA_PWM3_CK, "infra_pwm3", CLK_INFRA_PWM3_SEL, 27),
	GATE_INFRA0_TOP(CLK_INFRA_CQ_DMA_CK, "infra_cq_dma", CLK_TOP_SYSAXI, 6),
	GATE_INFRA0_TOP(CLK_INFRA_AUD_BUS_CK, "infra_aud_bus", CLK_TOP_SYSAXI, 8),
	GATE_INFRA0_TOP(CLK_INFRA_AUD_26M_CK, "infra_aud_26m", CLK_TOP_F26M_SEL, 9),
	GATE_INFRA0_TOP(CLK_INFRA_AUD_L_CK, "infra_aud_l", CLK_TOP_AUD_L, 10),
	GATE_INFRA0_TOP(CLK_INFRA_AUD_AUD_CK, "infra_aud_aud", CLK_TOP_A1SYS,
			11),
	GATE_INFRA0_TOP(CLK_INFRA_AUD_EG2_CK, "infra_aud_eg2", CLK_TOP_A_TUNER,
			13),
	GATE_INFRA0_TOP(CLK_INFRA_DRAMC_26M_CK, "infra_dramc_26m", CLK_TOP_F26M_SEL,
			14),
	GATE_INFRA0_INFRA(CLK_INFRA_DBG_CK, "infra_dbg", CLK_INFRA_66M_MCK, 15),
	GATE_INFRA0_INFRA(CLK_INFRA_AP_DMA_CK, "infra_ap_dma", CLK_INFRA_66M_MCK, 16),
	GATE_INFRA0_INFRA(CLK_INFRA_SEJ_CK, "infra_sej", CLK_INFRA_66M_MCK, 24),
	GATE_INFRA0_TOP(CLK_INFRA_SEJ_13M_CK, "infra_sej_13m", CLK_TOP_F26M_SEL, 25),
	GATE_INFRA1_TOP(CLK_INFRA_THERM_CK, "infra_therm", CLK_TOP_F26M_SEL, 0),
	GATE_INFRA1_TOP(CLK_INFRA_I2C0_CK, "infra_i2c0", CLK_TOP_I2C_BCK, 1),
	GATE_INFRA1_INFRA(CLK_INFRA_UART0_CK, "infra_uart0", CLK_INFRA_UART0_SEL, 2),
	GATE_INFRA1_INFRA(CLK_INFRA_UART1_CK, "infra_uart1", CLK_INFRA_UART1_SEL, 3),
	GATE_INFRA1_INFRA(CLK_INFRA_UART2_CK, "infra_uart2", CLK_INFRA_UART2_SEL, 4),
	GATE_INFRA1_INFRA(CLK_INFRA_SPI2_CK, "infra_spi2", CLK_INFRA_SPI2_SEL, 6),
	GATE_INFRA1_INFRA(CLK_INFRA_SPI2_HCK_CK, "infra_spi2_hck", CLK_INFRA_66M_MCK,
			  7),
	GATE_INFRA1_TOP(CLK_INFRA_NFI1_CK, "infra_nfi1", CLK_TOP_NFI1X, 8),
	GATE_INFRA1_TOP(CLK_INFRA_SPINFI1_CK, "infra_spinfi1", CLK_TOP_SPINFI_BCK,
		    9),
	GATE_INFRA1_INFRA(CLK_INFRA_NFI_HCK_CK, "infra_nfi_hck", CLK_INFRA_66M_MCK, 10),
	GATE_INFRA1_INFRA(CLK_INFRA_SPI0_CK, "infra_spi0", CLK_INFRA_SPI0_SEL, 11),
	GATE_INFRA1_INFRA(CLK_INFRA_SPI1_CK, "infra_spi1", CLK_INFRA_SPI1_SEL, 12),
	GATE_INFRA1_INFRA(CLK_INFRA_SPI0_HCK_CK, "infra_spi0_hck", CLK_INFRA_66M_MCK,
			  13),
	GATE_INFRA1_INFRA(CLK_INFRA_SPI1_HCK_CK, "infra_spi1_hck", CLK_INFRA_66M_MCK,
			  14),
	GATE_INFRA1_TOP(CLK_INFRA_FRTC_CK, "infra_frtc", CLK_TOP_CB_RTC_32K, 15),
	GATE_INFRA1_TOP(CLK_INFRA_MSDC_CK, "infra_msdc", CLK_TOP_EMMC_400M, 16),
	GATE_INFRA1_TOP(CLK_INFRA_MSDC_HCK_CK, "infra_msdc_hck",
			CLK_TOP_EMMC_208M, 17),
	GATE_INFRA1_TOP(CLK_INFRA_MSDC_133M_CK, "infra_msdc_133m",
			CLK_TOP_SYSAXI, 18),
	GATE_INFRA1_TOP(CLK_INFRA_MSDC_66M_CK, "infra_msdc_66m", CLK_TOP_SYSAXI,
			19),
	GATE_INFRA1_INFRA(CLK_INFRA_ADC_26M_CK, "infra_adc_26m", CLK_INFRA_ADC_FRC_CK, 20),
	GATE_INFRA1_TOP(CLK_INFRA_ADC_FRC_CK, "infra_adc_frc", CLK_TOP_F26M, 21),
	GATE_INFRA1_TOP(CLK_INFRA_FBIST2FPC_CK, "infra_fbist2fpc", CLK_TOP_NFI1X,
			23),
	GATE_INFRA1_TOP(CLK_INFRA_I2C_MCK_CK, "infra_i2c_mck", CLK_TOP_SYSAXI,
			25),
	GATE_INFRA1_INFRA(CLK_INFRA_I2C_PCK_CK, "infra_i2c_pck", CLK_INFRA_66M_MCK, 26),
	GATE_INFRA2_TOP(CLK_INFRA_IUSB_133_CK, "infra_iusb_133", CLK_TOP_SYSAXI,
			0),
	GATE_INFRA2_TOP(CLK_INFRA_IUSB_66M_CK, "infra_iusb_66m", CLK_TOP_SYSAXI,
			1),
	GATE_INFRA2_TOP(CLK_INFRA_IUSB_SYS_CK, "infra_iusb_sys", CLK_TOP_U2U3_SYS,
			2),
	GATE_INFRA2_TOP(CLK_INFRA_IUSB_CK, "infra_iusb", CLK_TOP_U2U3_REF, 3),
	GATE_INFRA2_TOP(CLK_INFRA_IPCIE_CK, "infra_ipcie", CLK_TOP_PEXTP_TL, 12),
	GATE_INFRA2_TOP(CLK_INFRA_IPCIE_PIPE_CK, "infra_ipcie_pipe", CLK_TOP_CB_CKSQ_40M, 13),
	GATE_INFRA2_TOP(CLK_INFRA_IPCIER_CK, "infra_ipcier", CLK_TOP_F26M, 14),
	GATE_INFRA2_TOP(CLK_INFRA_IPCIEB_CK, "infra_ipcieb", CLK_TOP_SYSAXI, 15),
};

static const struct mtk_clk_tree mt7981_fixed_pll_clk_tree = {
	.fdivs_offs = CLK_APMIXED_NR_CLK,
	.xtal_rate = 40 * MHZ,
	.fclks = fixed_pll_clks,
};

static const struct mtk_clk_tree mt7981_topckgen_clk_tree = {
	.fdivs_offs = CLK_TOP_CB_M_416M,
	.muxes_offs = CLK_TOP_NFI1X_SEL,
	.fclks = top_fixed_clks,
	.fdivs = top_fixed_divs,
	.muxes = top_muxes,
	.flags = CLK_BYPASS_XTAL | CLK_TOPCKGEN,
};

static const struct mtk_clk_tree mt7981_infracfg_clk_tree = {
	.fdivs_offs = CLK_INFRA_66M_MCK,
	.muxes_offs = CLK_INFRA_UART0_SEL,
	.gates_offs = CLK_INFRA_GPT_STA,
	.fdivs = infra_fixed_divs,
	.muxes = infra_muxes,
	.gates = infracfg_gates,
	.flags = CLK_INFRASYS,
};

static const struct udevice_id mt7981_fixed_pll_compat[] = {
	{ .compatible = "mediatek,mt7981-fixed-plls" },
	{ .compatible = "mediatek,mt7981-apmixedsys" },
	{}
};

static const struct udevice_id mt7981_topckgen_compat[] = {
	{ .compatible = "mediatek,mt7981-topckgen" },
	{}
};

static int mt7981_fixed_pll_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt7981_fixed_pll_clk_tree);
}

static int mt7981_topckgen_probe(struct udevice *dev)
{
	struct mtk_clk_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	writel(MT7981_CLK_PDN_EN_WRITE, priv->base + MT7981_CLK_PDN);

	return mtk_common_clk_init(dev, &mt7981_topckgen_clk_tree);
}

U_BOOT_DRIVER(mtk_clk_apmixedsys) = {
	.name = "mt7981-clock-fixed-pll",
	.id = UCLASS_CLK,
	.of_match = mt7981_fixed_pll_compat,
	.probe = mt7981_fixed_pll_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_fixed_pll_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_topckgen) = {
	.name = "mt7981-clock-topckgen",
	.id = UCLASS_CLK,
	.of_match = mt7981_topckgen_compat,
	.probe = mt7981_topckgen_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_topckgen_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

static const struct udevice_id mt7981_infracfg_compat[] = {
	{ .compatible = "mediatek,mt7981-infracfg" },
	{}
};

static int mt7981_infracfg_probe(struct udevice *dev)
{
	return mtk_common_clk_infrasys_init(dev, &mt7981_infracfg_clk_tree);
}

U_BOOT_DRIVER(mtk_clk_infracfg) = {
	.name = "mt7981-clock-infracfg",
	.id = UCLASS_CLK,
	.of_match = mt7981_infracfg_compat,
	.probe = mt7981_infracfg_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_infrasys_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

/* sgmiisys */
static const struct mtk_gate_regs sgmii_cg_regs = {
	.set_ofs = 0xe4,
	.clr_ofs = 0xe4,
	.sta_ofs = 0xe4,
};

#define GATE_SGMII(_id, _name, _parent, _shift)                                \
	{                                                                      \
		.id = _id, .parent = _parent, .regs = &sgmii_cg_regs,          \
		.shift = _shift,                                               \
		.flags = CLK_GATE_NO_SETCLR_INV | CLK_PARENT_TOPCKGEN,         \
	}

static const struct mtk_gate sgmii0_cgs[] = {
	GATE_SGMII(CLK_SGM0_TX_EN, "sgm0_tx_en", CLK_TOP_USB_TX250M, 2),
	GATE_SGMII(CLK_SGM0_RX_EN, "sgm0_rx_en", CLK_TOP_USB_EQ_RX250M, 3),
	GATE_SGMII(CLK_SGM0_CK0_EN, "sgm0_ck0_en", CLK_TOP_USB_LN0_CK, 4),
	GATE_SGMII(CLK_SGM0_CDR_CK0_EN, "sgm0_cdr_ck0_en", CLK_TOP_USB_CDR_CK, 5),
};

static int mt7981_sgmii0sys_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7981_topckgen_clk_tree,
					sgmii0_cgs);
}

static const struct udevice_id mt7981_sgmii0sys_compat[] = {
	{ .compatible = "mediatek,mt7981-sgmiisys_0", },
	{}
};

U_BOOT_DRIVER(mtk_clk_sgmii0sys) = {
	.name = "mt7981-clock-sgmii0sys",
	.id = UCLASS_CLK,
	.of_match = mt7981_sgmii0sys_compat,
	.probe = mt7981_sgmii0sys_probe,
	.priv_auto = sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
};

static const struct mtk_gate sgmii1_cgs[] = {
	GATE_SGMII(CLK_SGM1_TX_EN, "sgm1_tx_en", CLK_TOP_USB_TX250M, 2),
	GATE_SGMII(CLK_SGM1_RX_EN, "sgm1_rx_en", CLK_TOP_USB_EQ_RX250M, 3),
	GATE_SGMII(CLK_SGM1_CK1_EN, "sgm1_ck1_en", CLK_TOP_USB_LN0_CK, 4),
	GATE_SGMII(CLK_SGM1_CDR_CK1_EN, "sgm1_cdr_ck1_en", CLK_TOP_USB_CDR_CK, 5),
};

static int mt7981_sgmii1sys_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7981_topckgen_clk_tree,
					sgmii1_cgs);
}

static const struct udevice_id mt7981_sgmii1sys_compat[] = {
	{ .compatible = "mediatek,mt7981-sgmiisys_1", },
	{}
};

U_BOOT_DRIVER(mtk_clk_sgmii1sys) = {
	.name = "mt7981-clock-sgmii1sys",
	.id = UCLASS_CLK,
	.of_match = mt7981_sgmii1sys_compat,
	.probe = mt7981_sgmii1sys_probe,
	.priv_auto = sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
};

/* ethsys */
static const struct mtk_gate_regs eth_cg_regs = {
	.set_ofs = 0x30,
	.clr_ofs = 0x30,
	.sta_ofs = 0x30,
};

#define GATE_ETH(_id, _name, _parent, _shift)                                  \
	{                                                                      \
		.id = _id, .parent = _parent, .regs = &eth_cg_regs,            \
		.shift = _shift,                                               \
		.flags = CLK_GATE_NO_SETCLR_INV | CLK_PARENT_TOPCKGEN,         \
	}

static const struct mtk_gate eth_cgs[] = {
	GATE_ETH(CLK_ETH_FE_EN, "eth_fe_en", CLK_TOP_NETSYS_2X, 6),
	GATE_ETH(CLK_ETH_GP2_EN, "eth_gp2_en", CLK_TOP_SGM_325M, 7),
	GATE_ETH(CLK_ETH_GP1_EN, "eth_gp1_en", CLK_TOP_SGM_325M, 8),
	GATE_ETH(CLK_ETH_WOCPU0_EN, "eth_wocpu0_en", CLK_TOP_NETSYS_WED_MCU, 15),
};

static int mt7981_ethsys_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7981_topckgen_clk_tree,
					eth_cgs);
}

static int mt7981_ethsys_bind(struct udevice *dev)
{
	int ret = 0;

	if (CONFIG_IS_ENABLED(RESET_MEDIATEK)) {
		ret = mediatek_reset_bind(dev, ETHSYS_HIFSYS_RST_CTRL_OFS, 1);
		if (ret)
			debug("Warning: failed to bind reset controller\n");
	}

	return ret;
}

static const struct udevice_id mt7981_ethsys_compat[] = {
	{ .compatible = "mediatek,mt7981-ethsys", },
	{}
};

U_BOOT_DRIVER(mtk_clk_ethsys) = {
	.name = "mt7981-clock-ethsys",
	.id = UCLASS_CLK,
	.of_match = mt7981_ethsys_compat,
	.probe = mt7981_ethsys_probe,
	.bind = mt7981_ethsys_bind,
	.priv_auto = sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
};
