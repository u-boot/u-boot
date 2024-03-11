// SPDX-License-Identifier: GPL-2.0

#include <common.h>
#include <dm.h>
#include <log.h>
#include <asm/arch-mediatek/reset.h>
#include <asm/io.h>
#include <dt-bindings/clock/mt6735-clk.h>
#include <linux/bitops.h>

#include "clk-mtk.h"

#define MT6735_CLKSQ_STB_CON0		0x20
#define MT6735_PLL_ISO_CON0		0x2C
#define MT6735_PLL_FMAX			(3000UL * MHZ)
#define MT6735_PLL_FMIN			(62UL * MHZ)
#define MT6735_CON0_RST_BAR		BIT(27)

#define MCU_AXI_DIV			0x640
#define AXI_DIV_MSK			GENMASK(4, 0)
#define AXI_DIV_SEL(x)			(x)

/* apmixedsys */
#define PLL(_id, _reg, _pwr_reg, _en_mask, _flags, _pcwbits, _pd_reg,	\
	    _pd_shift, _pcw_reg, _pcw_shift) {				\
		.id = _id,						\
		.reg = _reg,						\
		.pwr_reg = _pwr_reg,					\
		.en_mask = _en_mask,					\
		.rst_bar_mask = MT6735_CON0_RST_BAR,			\
		.fmax = MT6735_PLL_FMAX,				\
		.fmin = MT6735_PLL_FMIN,			\
		.flags = _flags,					\
		.pcwbits = _pcwbits,					\
		.pd_reg = _pd_reg,					\
		.pd_shift = _pd_shift,					\
		.pcw_reg = _pcw_reg,					\
		.pcw_shift = _pcw_shift,				\
	}

static const struct mtk_pll_data apmixed_plls[] = {
	PLL(CLK_APMIXED_ARMPLL, 0x200, 0x20c, 0x00000001, 0,
	    21, 0x204, 24, 0x204, 0),
	PLL(CLK_APMIXED_MAINPLL, 0x210, 0x21c, 0xf0000101, 0,
	    21, 0x210, 24, 0x214, 0),
	PLL(CLK_APMIXED_UNIVPLL, 0x220, 0x22c, 0xfc000001, HAVE_RST_BAR,
	    7, 0x224, 24, 0x224, 14),
	PLL(CLK_APMIXED_MMPLL, 0x230, 0x23c, 0x00000001, 0,
	    21, 0x234, 24, 0x234, 0),
	PLL(CLK_APMIXED_MSDCPLL, 0x240, 0x24c, 0x00000001, 0,
	    21, 0x244, 24, 0x244, 0),
	PLL(CLK_APMIXED_TVDPLL, 0x260, 0x26c, 0x00000001, 0,
	    21, 0x264, 24, 0x264, 0),
	PLL(CLK_APMIXED_VENCPLL, 0x250, 0x25c, 0x00000001, 0,
	    21, 0x254, 24, 0x254, 0),
	PLL(CLK_APMIXED_APLL1, 0x270, 0x280, 0x00000001, 0,
	    31, 0x270, 24, 0x274, 0),
	PLL(CLK_APMIXED_APLL2, 0x284, 0x294, 0x00000001, 0,
	    31, 0x284, 4, 0x288, 0),
};

/* topckgen */
#define FACTOR0(_id, _parent, _mult, _div)			\
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_APMIXED)

#define FACTOR1(_id, _parent, _mult, _div)			\
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_TOPCKGEN)

#define FACTOR2(_id, _parent, _mult, _div)			\
	FACTOR(_id, _parent, _mult, _div, 0)

static const struct mtk_fixed_clk top_fixed_clks[] = {
	FIXED_CLK(CLK_TOP_DPI_CK, CLK_XTAL, 0),
	FIXED_CLK(CLK_TOP_CLKPH_MCK_O, CLK_XTAL, 0),
};

static const struct mtk_fixed_factor top_fixed_divs[] = {
	FACTOR0(CLK_TOP_UNIVPLL_D2, CLK_APMIXED_UNIVPLL, 1, 2),
	FACTOR0(CLK_TOP_UNIVPLL_D3, CLK_APMIXED_UNIVPLL, 1, 3),
	FACTOR0(CLK_TOP_UNIVPLL1_D2, CLK_APMIXED_UNIVPLL, 2, 2),
	FACTOR0(CLK_TOP_AD_APLL1_CK, CLK_APMIXED_APLL1, 1, 1),
	FACTOR0(CLK_TOP_MMPLL_CK, CLK_APMIXED_MMPLL, 1, 1),
    FACTOR0(CLK_TOP_MSDCPLL_CK, CLK_APMIXED_MSDCPLL, 1, 1),
	FACTOR0(CLK_TOP_MSDCPLL_D16, CLK_APMIXED_MSDCPLL, 1, 16),
	FACTOR0(CLK_TOP_MSDCPLL_D2, CLK_APMIXED_MSDCPLL, 1, 2),
	FACTOR0(CLK_TOP_MSDCPLL_D4, CLK_APMIXED_MSDCPLL, 1, 4),
	FACTOR0(CLK_TOP_MSDCPLL_D8, CLK_APMIXED_MSDCPLL, 1, 8),
	FACTOR0(CLK_TOP_SYSPLL_D2, CLK_APMIXED_MAINPLL, 1, 2),
	FACTOR0(CLK_TOP_SYSPLL_D3, CLK_APMIXED_MAINPLL, 1, 3),
	FACTOR0(CLK_TOP_SYSPLL_D5, CLK_APMIXED_MAINPLL, 1, 5),
	FACTOR0(CLK_TOP_SYSPLL1_D16, CLK_APMIXED_MAINPLL, 2, 16),
	FACTOR0(CLK_TOP_SYSPLL1_D2, CLK_APMIXED_MAINPLL, 2, 2),
	FACTOR0(CLK_TOP_SYSPLL1_D4, CLK_APMIXED_MAINPLL, 2, 4),
	FACTOR0(CLK_TOP_SYSPLL1_D8, CLK_APMIXED_MAINPLL, 2, 8),
	FACTOR0(CLK_TOP_SYSPLL2_D2, CLK_APMIXED_MAINPLL, 3, 2),
	FACTOR0(CLK_TOP_SYSPLL2_D4, CLK_APMIXED_MAINPLL, 3, 4),
	FACTOR0(CLK_TOP_SYSPLL3_D2, CLK_APMIXED_MAINPLL, 5, 2),
	FACTOR0(CLK_TOP_SYSPLL3_D4, CLK_APMIXED_MAINPLL, 5, 4),
	FACTOR0(CLK_TOP_SYSPLL4_D2, CLK_APMIXED_MAINPLL, 7, 2),
	FACTOR0(CLK_TOP_SYSPLL4_D2_D8, CLK_APMIXED_MAINPLL, 7, 8),
	FACTOR0(CLK_TOP_SYSPLL4_D4, CLK_APMIXED_MAINPLL, 7, 4),
	FACTOR0(CLK_TOP_TVDPLL_CK, CLK_APMIXED_TVDPLL, 1, 1),
	FACTOR0(CLK_TOP_TVDPLL_D2, CLK_APMIXED_TVDPLL, 1, 2),
	FACTOR0(CLK_TOP_TVDPLL_D4, CLK_APMIXED_TVDPLL, 1, 4),
	FACTOR0(CLK_TOP_UNIVPLL_D26, CLK_APMIXED_UNIVPLL, 1, 26),
	FACTOR0(CLK_TOP_UNIVPLL_D5, CLK_APMIXED_UNIVPLL, 1, 5),
	FACTOR0(CLK_TOP_UNIVPLL1_D4, CLK_APMIXED_UNIVPLL, 2, 4),
	FACTOR0(CLK_TOP_UNIVPLL1_D8, CLK_APMIXED_UNIVPLL, 2, 8),
	FACTOR0(CLK_TOP_UNIVPLL2_D2, CLK_APMIXED_UNIVPLL, 3, 2),
	FACTOR0(CLK_TOP_UNIVPLL2_D4, CLK_APMIXED_UNIVPLL, 3, 4),
	FACTOR0(CLK_TOP_UNIVPLL2_D8, CLK_APMIXED_UNIVPLL, 3, 8),
	FACTOR0(CLK_TOP_UNIVPLL3_D2, CLK_APMIXED_UNIVPLL, 5, 2),
	FACTOR0(CLK_TOP_UNIVPLL3_D4, CLK_APMIXED_UNIVPLL, 5, 4),
	FACTOR0(CLK_TOP_USB_PHY48M, CLK_APMIXED_UNIVPLL, 1, 26),
	FACTOR0(CLK_TOP_VENCPLL_CK, CLK_APMIXED_VENCPLL, 1, 1),
	FACTOR0(CLK_TOP_VENCPLL_D3, CLK_APMIXED_VENCPLL, 1, 3),
	FACTOR1(CLK_TOP_DMPLL_CK, CLK_TOP_CLKPH_MCK_O, 1, 1),
	FACTOR1(CLK_TOP_DMPLL_D2, CLK_TOP_CLKPH_MCK_O, 1, 2),
	FACTOR1(CLK_TOP_DMPLL_D4, CLK_TOP_CLKPH_MCK_O, 1, 4),
	FACTOR1(CLK_TOP_DMPLL_D8, CLK_TOP_CLKPH_MCK_O, 1, 8),
    FACTOR2(CLK_TOP_WHPLL_AUDIO_CK, CLK_XTAL, 1, 1),
	FACTOR2(CLK_TOP_AD_SYS_26M_CK, CLK_XTAL, 1, 1),
	FACTOR2(CLK_TOP_AD_SYS_26M_D2, CLK_XTAL, 1, 2),
};

static const int axi_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_SYSPLL_D5,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_UNIVPLL_D5,
	CLK_TOP_UNIVPLL2_D2,
	CLK_TOP_DMPLL_CK,
	CLK_TOP_DMPLL_D2
};

static const int mem_parents[] = {
	CLK_XTAL,
	CLK_TOP_DMPLL_CK
};

static const int ddrphycfg_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D8
};

static const int mm_parents[] = {
	CLK_XTAL,
	CLK_TOP_VENCPLL_CK,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_SYSPLL_D5,
    CLK_TOP_SYSPLL1_D4,
	CLK_TOP_UNIVPLL_D5,
	CLK_TOP_UNIVPLL2_D2,
	CLK_TOP_DMPLL_CK
};

static const int pwm_parents[] = {
	CLK_XTAL,
	CLK_TOP_UNIVPLL2_D4,
	CLK_TOP_UNIVPLL3_D2,
	CLK_TOP_UNIVPLL1_D4
};

static const int vdec_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_SYSPLL_D5,
	CLK_TOP_SYSPLL1_D4,
	CLK_TOP_UNIVPLL_D5,
	CLK_TOP_SYSPLL_D2,
	CLK_TOP_SYSPLL2_D2,
	CLK_TOP_MSDCPLL_D2
};

static const int mfg_parents[] = {
	CLK_XTAL,
	CLK_TOP_MMPLL_CK,
	CLK_XTAL,
    CLK_XTAL,
    CLK_XTAL,
    CLK_XTAL,
    CLK_XTAL,
    CLK_XTAL,
    CLK_XTAL,
    CLK_TOP_SYSPLL1_D2,
	CLK_TOP_SYSPLL_D3,
    CLK_TOP_SYSPLL_D5,
	CLK_TOP_UNIVPLL_D3,
	CLK_TOP_UNIVPLL1_D2
};

static const int camtg_parents[] = {
	CLK_XTAL,
	CLK_TOP_UNIVPLL_D26,
	CLK_TOP_UNIVPLL2_D2,
	CLK_TOP_SYSPLL3_D2,
	CLK_TOP_SYSPLL3_D4,
	CLK_TOP_MSDCPLL_D4
};

static const int uart_parents[] = {
	CLK_XTAL,
	CLK_TOP_UNIVPLL2_D8
};

static const int spi_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL3_D2,
    CLK_TOP_MSDCPLL_D8,
    CLK_TOP_SYSPLL2_D4,
	CLK_TOP_SYSPLL4_D2,
	CLK_TOP_UNIVPLL2_D4,
	CLK_TOP_UNIVPLL1_D8
};

static const int usb20_parents[] = {
	CLK_XTAL,
	CLK_TOP_UNIVPLL1_D8,
	CLK_TOP_UNIVPLL3_D4
};

static const int msdc50_0_parents[] = {
	CLK_XTAL,
	CLK_TOP_SYSPLL1_D2,
	CLK_TOP_SYSPLL2_D2,
	CLK_TOP_SYSPLL4_D2,
	CLK_TOP_UNIVPLL1_D4,
	CLK_TOP_UNIVPLL_D5
};

static const int msdc30_0_parents[] = {
    CLK_XTAL,
    CLK_TOP_MSDCPLL_CK,
    CLK_TOP_MSDCPLL_D2,
    CLK_TOP_MSDCPLL_D4,
    CLK_TOP_SYSPLL2_D2,
    CLK_TOP_SYSPLL1_D4,
    CLK_TOP_UNIVPLL1_D4,
    CLK_TOP_UNIVPLL_D3,
    CLK_TOP_UNIVPLL_D26,
    CLK_TOP_SYSPLL2_D4,
    CLK_TOP_UNIVPLL_D2
};


static const int msdc30_1_parents[] = {
    CLK_XTAL,
    CLK_TOP_UNIVPLL2_D2,
    CLK_TOP_MSDCPLL_D4,
    CLK_TOP_SYSPLL2_D2,
    CLK_TOP_SYSPLL1_D4,
    CLK_TOP_UNIVPLL1_D4,
    CLK_TOP_UNIVPLL_D26,
    CLK_TOP_SYSPLL2_D4
};

static const int msdc30_2_parents[] = {
    CLK_XTAL,
    CLK_TOP_UNIVPLL2_D2,
    CLK_TOP_MSDCPLL_D4,
    CLK_TOP_SYSPLL2_D2,
    CLK_TOP_SYSPLL1_D4,
    CLK_TOP_UNIVPLL1_D4,
    CLK_TOP_UNIVPLL_D26,
    CLK_TOP_SYSPLL2_D4
};

static const int msdc30_3_parents[] = {
    CLK_XTAL,
    CLK_TOP_UNIVPLL2_D2,
    CLK_TOP_MSDCPLL_D4,
    CLK_TOP_SYSPLL2_D2,
    CLK_TOP_SYSPLL1_D4,
    CLK_TOP_UNIVPLL1_D4,
    CLK_TOP_UNIVPLL_D26,
    CLK_TOP_MSDCPLL_D16,
    CLK_TOP_SYSPLL2_D4
};

static const int audio_parents[] = {
    CLK_XTAL,
    CLK_TOP_SYSPLL3_D4,
    CLK_TOP_SYSPLL4_D4,
    CLK_TOP_SYSPLL1_D16
};

static const int aud_intbus_parents[] = {
    CLK_XTAL,
    CLK_TOP_SYSPLL1_D4,
    CLK_TOP_SYSPLL4_D2,
    CLK_TOP_DMPLL_D4
};

static const int pmicspi_parents[] = {
    CLK_XTAL,
    CLK_TOP_SYSPLL1_D8,
    CLK_TOP_SYSPLL3_D4,
    CLK_TOP_SYSPLL1_D16,
    CLK_TOP_UNIVPLL3_D4,
    CLK_TOP_UNIVPLL_D26,
    CLK_TOP_DMPLL_D4,
    CLK_TOP_DMPLL_D8
};

static const int scp_parents[] = {
    CLK_XTAL,
    CLK_TOP_SYSPLL1_D8,
    CLK_TOP_DMPLL_D2,
    CLK_TOP_DMPLL_D4
};

static const int atb_parents[] = {
    CLK_XTAL,
    CLK_TOP_SYSPLL1_D2,
    CLK_TOP_SYSPLL_D5,
    CLK_TOP_DMPLL_CK
};

static const int dpi0_parents[] = {
    CLK_XTAL,
    CLK_TOP_TVDPLL_CK,
    CLK_TOP_TVDPLL_D2,
    CLK_TOP_TVDPLL_D4,
    CLK_TOP_DPI_CK
};

static const int scam_parents[] = {
    CLK_XTAL,
    CLK_TOP_SYSPLL3_D2,
    CLK_TOP_UNIVPLL2_D4,
    CLK_TOP_VENCPLL_D3
};

static const int mfg13m_parents[] = {
    CLK_XTAL,
    CLK_TOP_AD_SYS_26M_D2
};

static const int aud_1_parents[] = {
    CLK_XTAL,
    CLK_TOP_AD_APLL1_CK
};

static const int aud_2_parents[] = {
    CLK_XTAL,
    CLK_TOP_WHPLL_AUDIO_CK
};

static const int irda_parents[] = {
    CLK_XTAL,
    CLK_TOP_UNIVPLL2_D4
};

static const int irtx_parents[] = {
    CLK_XTAL,
    CLK_TOP_AD_SYS_26M_CK
};

static const int disppwm_parents[] = {
    CLK_XTAL,
    CLK_TOP_UNIVPLL2_D4,
    CLK_TOP_SYSPLL4_D2_D8,
    CLK_TOP_AD_SYS_26M_CK
};

static const struct mtk_composite top_muxes[] = {
	MUX_GATE(CLK_TOP_MUX_AXI, axi_parents, 0x40, 0, 3, 32),
	MUX_GATE(CLK_TOP_MUX_MEM, mem_parents, 0x40, 8, 1, 32),
	MUX_GATE(CLK_TOP_MUX_DDRPHY, ddrphycfg_parents, 0x40, 16, 1, 32),
	MUX_GATE_FLAGS(CLK_TOP_MUX_MM, mm_parents, 0x40, 24, 3, 31, CLK_DOMAIN_SCPSYS),
	MUX_GATE(CLK_TOP_MUX_PWM, pwm_parents, 0x50, 0, 2, 7),
	MUX_GATE_FLAGS(CLK_TOP_MUX_VDEC, vdec_parents, 0x50, 8, 3, 15, CLK_DOMAIN_SCPSYS),
	MUX_GATE_FLAGS(CLK_TOP_MUX_MFG, mfg_parents, 0x50, 16, 4, 23,CLK_DOMAIN_SCPSYS),
	MUX_GATE_FLAGS(CLK_TOP_MUX_CAMTG, camtg_parents, 0x50, 24, 3, 31, CLK_DOMAIN_SCPSYS),
	MUX_GATE(CLK_TOP_MUX_UART, uart_parents, 0x60, 0, 1, 7),
	MUX_GATE(CLK_TOP_MUX_SPI, spi_parents, 0x60, 8, 3, 15),
	MUX_GATE(CLK_TOP_MUX_USB20, usb20_parents, 0x60, 16, 2, 23),
	MUX_GATE(CLK_TOP_MUX_MSDC50_0, msdc50_0_parents, 0x60, 24, 3, 31),
	MUX_GATE(CLK_TOP_MUX_MSDC30_0, msdc30_0_parents, 0x70, 0, 4, 7),
	MUX_GATE(CLK_TOP_MUX_MSDC30_1, msdc30_1_parents, 0x70, 8, 3, 15),
	MUX_GATE(CLK_TOP_MUX_MSDC30_2, msdc30_2_parents, 0x70, 16, 3, 23),
	MUX_GATE(CLK_TOP_MUX_MSDC30_3, msdc30_3_parents, 0x70, 24, 4, 31),
	MUX_GATE(CLK_TOP_MUX_AUDIO, audio_parents, 0x80, 0, 2, 7),
	MUX_GATE(CLK_TOP_MUX_AUDINTBUS, aud_intbus_parents, 0x80, 8, 2, 15),
	MUX_GATE(CLK_TOP_MUX_PMICSPI, pmicspi_parents, 0x80, 16, 3, 32),
	MUX_GATE(CLK_TOP_MUX_SCP, scp_parents, 0x80, 24, 2, 31),
	MUX_GATE(CLK_TOP_MUX_ATB, atb_parents, 0x90, 0, 2, 7),
	MUX_GATE(CLK_TOP_MUX_DPI0, dpi0_parents, 0x90, 8, 3, 15),
	MUX_GATE_FLAGS(CLK_TOP_MUX_SCAM, scam_parents, 0x90, 16, 2, 23, CLK_DOMAIN_SCPSYS),
	MUX_GATE(CLK_TOP_MUX_MFG13M, mfg13m_parents, 0x90, 24, 1, 31),
	MUX_GATE(CLK_TOP_MUX_AUD1, aud_1_parents, 0xa0, 0, 1, 7),
	MUX_GATE(CLK_TOP_MUX_AUD2, aud_2_parents, 0xa0, 8, 1, 15),
	MUX_GATE(CLK_TOP_MUX_IRDA, irda_parents, 0xa0, 16, 1, 23),
	MUX_GATE(CLK_TOP_MUX_IRTX, irtx_parents, 0xa0, 24, 1, 31),
	MUX_GATE_FLAGS(CLK_TOP_MUX_DISPPWM, disppwm_parents, 0xb0, 0, 2, 7, CLK_DOMAIN_SCPSYS)
};

/* infracfg */
static const struct mtk_gate_regs infra_cg_regs = {
	.set_ofs = 0x40,
	.clr_ofs = 0x44,
	.sta_ofs = 0x48,
};

#define GATE_INFRA(_id, _parent, _shift) {			\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &infra_cg_regs,				\
		.shift = _shift,				\
		.flags = CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN,	\
	}

static const struct mtk_gate infra_cgs[] = {
	GATE_INFRA(CLK_INFRA_DBGCLK, CLK_TOP_MUX_AXI, 0),
	GATE_INFRA(CLK_INFRA_GCE, CLK_TOP_MUX_AXI, 1),
	GATE_INFRA(CLK_INFRA_TRBG, CLK_TOP_MUX_AXI, 2),
	GATE_INFRA(CLK_INFRA_CPUM, CLK_TOP_MUX_AXI, 3),
	GATE_INFRA(CLK_INFRA_DEVAPC, CLK_TOP_MUX_AXI, 4),
	GATE_INFRA(CLK_INFRA_AUDIO, CLK_TOP_MUX_AUDINTBUS, 5),
	GATE_INFRA(CLK_INFRA_GCPU, CLK_TOP_MUX_AXI, 6),
	GATE_INFRA(CLK_INFRA_L2C_SRAM, CLK_TOP_MUX_AXI, 7),
	GATE_INFRA(CLK_INFRA_M4U, CLK_TOP_MUX_AXI, 8),
	GATE_INFRA(CLK_INFRA_CLDMA, CLK_TOP_MUX_AXI, 12),
	GATE_INFRA(CLK_INFRA_CONNMCU_BUS, CLK_TOP_MUX_AXI, 15),
	GATE_INFRA(CLK_INFRA_KP, CLK_TOP_MUX_AXI, 16),
	GATE_INFRA(CLK_INFRA_APXGPT, CLK_TOP_MUX_AXI, 18),
	GATE_INFRA(CLK_INFRA_SEJ, CLK_TOP_MUX_AXI, 19),
	GATE_INFRA(CLK_INFRA_CCIF0_AP, CLK_TOP_MUX_AXI, 20),
	GATE_INFRA(CLK_INFRA_CCIF1_AP, CLK_TOP_MUX_AXI, 21),
	GATE_INFRA(CLK_INFRA_PMIC_SPI, CLK_TOP_MUX_PMICSPI, 22),
	GATE_INFRA(CLK_INFRA_PMIC_WRAP, CLK_TOP_MUX_AXI, 23),
};

/* pericfg */
static const struct mtk_gate_regs peri0_cg_regs = {
	.set_ofs = 0x08,
	.clr_ofs = 0x10,
	.sta_ofs = 0x18,
};

#define GATE_PERI0(_id, _parent, _shift) {			\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &peri0_cg_regs,				\
		.shift = _shift,				\
		.flags = CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN,	\
	}

static const struct mtk_gate peri_cgs[] = {
	GATE_PERI0(CLK_PERI_DISP_PWM, CLK_TOP_MUX_DISPPWM, 0),
	GATE_PERI0(CLK_PERI_THERM, CLK_TOP_MUX_AXI, 1),
	GATE_PERI0(CLK_PERI_PWM1, CLK_TOP_MUX_AXI, 2),
	GATE_PERI0(CLK_PERI_PWM2, CLK_TOP_MUX_AXI, 3),
	GATE_PERI0(CLK_PERI_PWM3, CLK_TOP_MUX_AXI, 4),
	GATE_PERI0(CLK_PERI_PWM4, CLK_TOP_MUX_AXI, 5),
	GATE_PERI0(CLK_PERI_PWM5, CLK_TOP_MUX_AXI, 6),
	GATE_PERI0(CLK_PERI_PWM6, CLK_TOP_MUX_AXI, 7),
	GATE_PERI0(CLK_PERI_PWM7, CLK_TOP_MUX_AXI, 8),
	GATE_PERI0(CLK_PERI_PWM, CLK_TOP_MUX_AXI, 9),
	GATE_PERI0(CLK_PERI_USB0, CLK_TOP_MUX_USB20, 10),
	GATE_PERI0(CLK_PERI_IRDA, CLK_TOP_MUX_IRDA, 11),
	GATE_PERI0(CLK_PERI_APDMA, CLK_TOP_MUX_AXI, 12),
	GATE_PERI0(CLK_PERI_MSDC30_0, CLK_TOP_MUX_MSDC30_0, 13),
	GATE_PERI0(CLK_PERI_MSDC30_1, CLK_TOP_MUX_MSDC30_1, 14),
	GATE_PERI0(CLK_PERI_MSDC30_2, CLK_TOP_MUX_MSDC30_2, 15),
	GATE_PERI0(CLK_PERI_MSDC30_3, CLK_TOP_MUX_MSDC30_3, 16),
	GATE_PERI0(CLK_PERI_UART0, CLK_TOP_MUX_UART, 17),
	GATE_PERI0(CLK_PERI_UART1, CLK_TOP_MUX_UART, 18),
	GATE_PERI0(CLK_PERI_UART2, CLK_TOP_MUX_UART, 19),
	GATE_PERI0(CLK_PERI_UART3, CLK_TOP_MUX_UART, 20),
	GATE_PERI0(CLK_PERI_UART4, CLK_TOP_MUX_UART, 21),
	GATE_PERI0(CLK_PERI_BTIF, CLK_TOP_MUX_AXI, 22),
	GATE_PERI0(CLK_PERI_I2C0, CLK_TOP_MUX_AXI, 23),
	GATE_PERI0(CLK_PERI_I2C1, CLK_TOP_MUX_AXI, 24),
	GATE_PERI0(CLK_PERI_I2C2, CLK_TOP_MUX_AXI, 25),
	GATE_PERI0(CLK_PERI_I2C3, CLK_TOP_MUX_AXI, 26),
	GATE_PERI0(CLK_PERI_AUXADC, CLK_TOP_MUX_AXI, 27),
	GATE_PERI0(CLK_PERI_SPI0, CLK_TOP_MUX_SPI, 28),
	GATE_PERI0(CLK_PERI_IRTX, CLK_TOP_MUX_IRTX, 29),
};

static const struct mtk_clk_tree mt6735_clk_tree = {
	.xtal_rate = 26 * MHZ,
	.xtal2_rate = 26 * MHZ,
	.fdivs_offs = CLK_TOP_UNIVPLL_D2,
	.muxes_offs = CLK_TOP_MUX_AXI,
	.plls = apmixed_plls,
	.fclks = top_fixed_clks,
	.fdivs = top_fixed_divs,
	.muxes = top_muxes,
};

static int mt6735_apmixedsys_probe(struct udevice *dev)
{
	struct mtk_clk_priv *priv = dev_get_priv(dev);
	int ret;

	ret = mtk_common_clk_init(dev, &mt6735_clk_tree);
	if (ret)
		return ret;

	/* reduce clock square disable time */
	writel(0x98940501, priv->base + MT6735_CLKSQ_STB_CON0);

	/* extend pwr/iso control timing to 1us */
	writel(0x80008, priv->base + MT6735_PLL_ISO_CON0);

	return 0;
}

static int mt6735_mcucfg_probe(struct udevice *dev)
{
    void __iomem *base;

    base = dev_read_addr_ptr(dev);
    if (!base)
        return -ENOENT;

    writel(0x12, base + 0x640);

    return 0;
}

static int mt6735_topckgen_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt6735_clk_tree);
}

static int mt6735_infracfg_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt6735_clk_tree, infra_cgs);
}

static int mt6735_pericfg_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt6735_clk_tree, peri_cgs);
}

static const struct udevice_id mt6735_apmixed_compat[] = {
	{ .compatible = "mediatek,mt6735-apmixedsys" },
	{ }
};

static const struct udevice_id mt6735_topckgen_compat[] = {
	{ .compatible = "mediatek,mt6735-topckgen" },
	{ }
};

static const struct udevice_id mt6735_infracfg_compat[] = {
	{ .compatible = "mediatek,mt6735-infracfg", },
	{ }
};

static const struct udevice_id mt6735_pericfg_compat[] = {
	{ .compatible = "mediatek,mt6735-pericfg", },
	{ }
};

static const struct udevice_id mt6735_mcucfg_compat[] = {
    { .compatible = "mediatek,mt6735-mcucfg" },
    { }
};

U_BOOT_DRIVER(mtk_mcucfg) = {
    .name = "mt6735-mcucfg",
    .id = UCLASS_SYSCON,
    .of_match = mt6735_mcucfg_compat,
    .probe = mt6735_mcucfg_probe,
    .flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_apmixedsys) = {
	.name = "mt6735-clock-apmixedsys",
	.id = UCLASS_CLK,
	.of_match = mt6735_apmixed_compat,
	.probe = mt6735_apmixedsys_probe,
	.priv_auto	= sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_apmixedsys_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_topckgen) = {
	.name = "mt6735-clock-topckgen",
	.id = UCLASS_CLK,
	.of_match = mt6735_topckgen_compat,
	.probe = mt6735_topckgen_probe,
	.priv_auto	= sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_topckgen_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_infracfg) = {
	.name = "mt6735-infracfg",
	.id = UCLASS_CLK,
	.of_match = mt6735_infracfg_compat,
	.probe = mt6735_infracfg_probe,
	.priv_auto	= sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_pericfg) = {
	.name = "mt6735-pericfg",
	.id = UCLASS_CLK,
	.of_match = mt6735_pericfg_compat,
	.probe = mt6735_pericfg_probe,
	.priv_auto	= sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
