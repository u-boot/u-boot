/*
 * sun6i clock register definitions
 *
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_CLOCK_SUN6I_H
#define _SUNXI_CLOCK_SUN6I_H

struct sunxi_ccm_reg {
	u32 pll1_cfg;		/* 0x00 pll1 control */
	u32 reserved0;
	u32 pll2_cfg;		/* 0x08 pll2 control */
	u32 reserved1;
	u32 pll3_cfg;		/* 0x10 pll3 control */
	u32 reserved2;
	u32 pll4_cfg;		/* 0x18 pll4 control */
	u32 reserved3;
	u32 pll5_cfg;		/* 0x20 pll5 control */
	u32 reserved4;
	u32 pll6_cfg;		/* 0x28 pll6 control */
	u32 reserved5;
	u32 pll7_cfg;		/* 0x30 pll7 control */
	u32 reserved6;
	u32 pll8_cfg;		/* 0x38 pll8 control */
	u32 reserved7;
	u32 mipi_pll_cfg;	/* 0x40 MIPI pll control */
	u32 pll9_cfg;		/* 0x44 pll9 control */
	u32 pll10_cfg;		/* 0x48 pll10 control */
	u32 reserved8;
	u32 cpu_axi_cfg;	/* 0x50 CPU/AXI divide ratio */
	u32 ahb1_apb1_div;	/* 0x54 AHB1/APB1 divide ratio */
	u32 apb2_div;		/* 0x58 APB2 divide ratio */
	u32 axi_gate;		/* 0x5c axi module clock gating */
	u32 ahb_gate0;		/* 0x60 ahb module clock gating 0 */
	u32 ahb_gate1;		/* 0x64 ahb module clock gating 1 */
	u32 apb1_gate;		/* 0x68 apb1 module clock gating */
	u32 apb2_gate;		/* 0x6c apb2 module clock gating */
	u32 reserved9[4];
	u32 nand0_clk_cfg;	/* 0x80 nand0 clock control */
	u32 nand1_clk_cfg;	/* 0x84 nand1 clock control */
	u32 sd0_clk_cfg;	/* 0x88 sd0 clock control */
	u32 sd1_clk_cfg;	/* 0x8c sd1 clock control */
	u32 sd2_clk_cfg;	/* 0x90 sd2 clock control */
	u32 sd3_clk_cfg;	/* 0x94 sd3 clock control */
	u32 ts_clk_cfg;		/* 0x98 transport stream clock control */
	u32 ss_clk_cfg;		/* 0x9c security system clock control */
	u32 spi0_clk_cfg;	/* 0xa0 spi0 clock control */
	u32 spi1_clk_cfg;	/* 0xa4 spi1 clock control */
	u32 spi2_clk_cfg;	/* 0xa8 spi2 clock control */
	u32 spi3_clk_cfg;	/* 0xac spi3 clock control */
	u32 i2s0_clk_cfg;	/* 0xb0 I2S0 clock control*/
	u32 i2s1_clk_cfg;	/* 0xb4 I2S1 clock control */
	u32 reserved10[2];
	u32 spdif_clk_cfg;	/* 0xc0 SPDIF clock control */
	u32 reserved11[2];
	u32 usb_clk_cfg;	/* 0xcc USB clock control */
	u32 gmac_clk_cfg;	/* 0xd0 GMAC clock control */
	u32 reserved12[7];
	u32 mdfs_clk_cfg;	/* 0xf0 MDFS clock control */
	u32 dram_clk_cfg;	/* 0xf4 DRAM configuration clock control */
	u32 reserved13[2];
	u32 dram_clk_gate;	/* 0x100 DRAM module gating */
	u32 be0_clk_cfg;	/* 0x104 BE0 module clock */
	u32 be1_clk_cfg;	/* 0x108 BE1 module clock */
	u32 fe0_clk_cfg;	/* 0x10c FE0 module clock */
	u32 fe1_clk_cfg;	/* 0x110 FE1 module clock */
	u32 mp_clk_cfg;		/* 0x114 MP module clock */
	u32 lcd0_ch0_clk_cfg;	/* 0x118 LCD0 CH0 module clock */
	u32 lcd1_ch0_clk_cfg;	/* 0x11c LCD1 CH0 module clock */
	u32 reserved14[3];
	u32 lcd0_ch1_clk_cfg;	/* 0x12c LCD0 CH1 module clock */
	u32 lcd1_ch1_clk_cfg;	/* 0x130 LCD1 CH1 module clock */
	u32 csi0_clk_cfg;	/* 0x134 CSI0 module clock */
	u32 csi1_clk_cfg;	/* 0x138 CSI1 module clock */
	u32 ve_clk_cfg;		/* 0x13c VE module clock */
	u32 adda_clk_cfg;	/* 0x140 ADDA module clock */
	u32 avs_clk_cfg;	/* 0x144 AVS module clock */
	u32 dmic_clk_cfg;	/* 0x148 Digital Mic module clock*/
	u32 reserved15;
	u32 hdmi_clk_cfg;	/* 0x150 HDMI module clock */
	u32 ps_clk_cfg;		/* 0x154 PS module clock */
	u32 mtc_clk_cfg;	/* 0x158 MTC module clock */
	u32 mbus0_clk_cfg;	/* 0x15c MBUS0 module clock */
	u32 mbus1_clk_cfg;	/* 0x160 MBUS1 module clock */
	u32 reserved16;
	u32 mipi_dsi_clk_cfg;	/* 0x168 MIPI DSI clock control */
	u32 mipi_csi_clk_cfg;	/* 0x16c MIPI CSI clock control */
	u32 reserved17[4];
	u32 iep_drc0_clk_cfg;	/* 0x180 IEP DRC0 module clock */
	u32 iep_drc1_clk_cfg;	/* 0x184 IEP DRC1 module clock */
	u32 iep_deu0_clk_cfg;	/* 0x188 IEP DEU0 module clock */
	u32 iep_deu1_clk_cfg;	/* 0x18c IEP DEU1 module clock */
	u32 reserved18[4];
	u32 gpu_core_clk_cfg;	/* 0x1a0 GPU core clock config */
	u32 gpu_mem_clk_cfg;	/* 0x1a4 GPU memory clock config */
	u32 gpu_hyd_clk_cfg;	/* 0x1a0 GPU HYD clock config */
	u32 reserved19[21];
	u32 pll_lock;		/* 0x200 PLL Lock Time */
	u32 pll1_lock;		/* 0x204 PLL1 Lock Time */
	u32 reserved20[6];
	u32 pll1_bias_cfg;	/* 0x220 PLL1 Bias config */
	u32 pll2_bias_cfg;	/* 0x224 PLL2 Bias config */
	u32 pll3_bias_cfg;	/* 0x228 PLL3 Bias config */
	u32 pll4_bias_cfg;	/* 0x22c PLL4 Bias config */
	u32 pll5_bias_cfg;	/* 0x230 PLL5 Bias config */
	u32 pll6_bias_cfg;	/* 0x234 PLL6 Bias config */
	u32 pll7_bias_cfg;	/* 0x238 PLL7 Bias config */
	u32 pll8_bias_cfg;	/* 0x23c PLL8 Bias config */
	u32 mipi_bias_cfg;	/* 0x240 MIPI Bias config */
	u32 pll9_bias_cfg;	/* 0x244 PLL9 Bias config */
	u32 pll10_bias_cfg;	/* 0x248 PLL10 Bias config */
	u32 reserved21[13];
	u32 pll1_pattern_cfg;	/* 0x280 PLL1 Pattern config */
	u32 pll2_pattern_cfg;	/* 0x284 PLL2 Pattern config */
	u32 pll3_pattern_cfg;	/* 0x288 PLL3 Pattern config */
	u32 pll4_pattern_cfg;	/* 0x28c PLL4 Pattern config */
	u32 pll5_pattern_cfg;	/* 0x290 PLL5 Pattern config */
	u32 pll6_pattern_cfg;	/* 0x294 PLL6 Pattern config */
	u32 pll7_pattern_cfg;	/* 0x298 PLL7 Pattern config */
	u32 pll8_pattern_cfg;	/* 0x29c PLL8 Pattern config */
	u32 mipi_pattern_cfg;	/* 0x2a0 MIPI Pattern config */
	u32 pll9_pattern_cfg;	/* 0x2a4 PLL9 Pattern config */
	u32 pll10_pattern_cfg;	/* 0x2a8 PLL10 Pattern config */
	u32 reserved22[5];
	u32 ahb_reset0_cfg;	/* 0x2c0 AHB1 Reset 0 config */
	u32 ahb_reset1_cfg;	/* 0x2c4 AHB1 Reset 1 config */
	u32 ahb_reset2_cfg;	/* 0x2c8 AHB1 Reset 2 config */
	u32 reserved23;
	u32 apb1_reset_cfg;	/* 0x2d0 APB1 Reset config */
	u32 reserved24;
	u32 apb2_reset_cfg;	/* 0x2d8 APB2 Reset config */
};

/* apb2 bit field */
#define APB2_CLK_SRC_LOSC		(0x0 << 24)
#define APB2_CLK_SRC_OSC24M		(0x1 << 24)
#define APB2_CLK_SRC_PLL6		(0x2 << 24)
#define APB2_CLK_SRC_MASK		(0x3 << 24)
#define APB2_CLK_RATE_N_1		(0x0 << 16)
#define APB2_CLK_RATE_N_2		(0x1 << 16)
#define APB2_CLK_RATE_N_4		(0x2 << 16)
#define APB2_CLK_RATE_N_8		(0x3 << 16)
#define APB2_CLK_RATE_N_MASK		(3 << 16)
#define APB2_CLK_RATE_M(m)		(((m)-1) << 0)
#define APB2_CLK_RATE_M_MASK            (0x1f << 0)

/* apb2 gate field */
#define APB2_GATE_UART_SHIFT	(16)
#define APB2_GATE_UART_MASK		(0xff << APB2_GATE_UART_SHIFT)
#define APB2_GATE_TWI_SHIFT	(0)
#define APB2_GATE_TWI_MASK		(0xf << APB2_GATE_TWI_SHIFT)

/* cpu_axi_cfg bits */
#define AXI_DIV_SHIFT			0
#define ATB_DIV_SHIFT			8
#define CPU_CLK_SRC_SHIFT		16

#define AXI_DIV_1			0
#define AXI_DIV_2			1
#define AXI_DIV_3			2
#define AXI_DIV_4			3
#define ATB_DIV_1			0
#define ATB_DIV_2			1
#define ATB_DIV_4			2
#define CPU_CLK_SRC_OSC24M		1
#define CPU_CLK_SRC_PLL1		2

#define CCM_PLL1_CTRL_M(n)		((((n) - 1) & 0x3) << 0)
#define CCM_PLL1_CTRL_K(n)		((((n) - 1) & 0x3) << 4)
#define CCM_PLL1_CTRL_N(n)		((((n) - 1) & 0x1f) << 8)
#define CCM_PLL1_CTRL_MAGIC		(0x1 << 16)
#define CCM_PLL1_CTRL_EN		(0x1 << 31)

#define CCM_PLL3_CTRL_M(n)		((((n) - 1) & 0xf) << 0)
#define CCM_PLL3_CTRL_N(n)		((((n) - 1) & 0x7f) << 8)
#define CCM_PLL3_CTRL_INTEGER_MODE	(0x1 << 24)
#define CCM_PLL3_CTRL_EN		(0x1 << 31)

#define CCM_PLL5_CTRL_M(n)		((((n) - 1) & 0x3) << 0)
#define CCM_PLL5_CTRL_K(n)		((((n) - 1) & 0x3) << 4)
#define CCM_PLL5_CTRL_N(n)		((((n) - 1) & 0x1f) << 8)
#define CCM_PLL5_CTRL_UPD		(0x1 << 20)
#define CCM_PLL5_CTRL_EN		(0x1 << 31)

#define PLL6_CFG_DEFAULT		0x90041811 /* 600 MHz */

#define CCM_PLL6_CTRL_N_SHIFT		8
#define CCM_PLL6_CTRL_N_MASK		(0x1f << CCM_PLL6_CTRL_N_SHIFT)
#define CCM_PLL6_CTRL_K_SHIFT		4
#define CCM_PLL6_CTRL_K_MASK		(0x3 << CCM_PLL6_CTRL_K_SHIFT)

#define AHB1_ABP1_DIV_DEFAULT		0x00002020

#define AXI_GATE_OFFSET_DRAM		0

/* ahb_gate0 offsets */
#define AHB_GATE_OFFSET_USB_OHCI1	30
#define AHB_GATE_OFFSET_USB_OHCI0	29
#define AHB_GATE_OFFSET_USB_EHCI1	27
#define AHB_GATE_OFFSET_USB_EHCI0	26
#define AHB_GATE_OFFSET_MCTL		14
#define AHB_GATE_OFFSET_GMAC		17
#define AHB_GATE_OFFSET_MMC3		11
#define AHB_GATE_OFFSET_MMC2		10
#define AHB_GATE_OFFSET_MMC1		9
#define AHB_GATE_OFFSET_MMC0		8
#define AHB_GATE_OFFSET_MMC(n)		(AHB_GATE_OFFSET_MMC0 + (n))

/* ahb_gate1 offsets */
#define AHB_GATE_OFFSET_DRC0		25
#define AHB_GATE_OFFSET_DE_BE0		12
#define AHB_GATE_OFFSET_HDMI		11
#define AHB_GATE_OFFSET_LCD1		5
#define AHB_GATE_OFFSET_LCD0		4

#define CCM_MMC_CTRL_OSCM24 (0x0 << 24)
#define CCM_MMC_CTRL_PLL6   (0x1 << 24)

#define CCM_MMC_CTRL_ENABLE (0x1 << 31)

#define CCM_USB_CTRL_PHY1_RST (0x1 << 1)
#define CCM_USB_CTRL_PHY2_RST (0x1 << 2)
/* There is no global phy clk gate on sun6i, define as 0 */
#define CCM_USB_CTRL_PHYGATE 0
#define CCM_USB_CTRL_PHY1_CLK (0x1 << 9)
#define CCM_USB_CTRL_PHY2_CLK (0x1 << 10)

#define CCM_GMAC_CTRL_TX_CLK_SRC_MII	0x0
#define CCM_GMAC_CTRL_TX_CLK_SRC_EXT_RGMII 0x1
#define CCM_GMAC_CTRL_TX_CLK_SRC_INT_RGMII 0x2
#define CCM_GMAC_CTRL_GPIT_MII		(0x0 << 2)
#define CCM_GMAC_CTRL_GPIT_RGMII	(0x1 << 2)

#define MDFS_CLK_DEFAULT		0x81000002 /* PLL6 / 3 */

#define CCM_DRAMCLK_CFG_DIV0(x)		((x - 1) << 8)
#define CCM_DRAMCLK_CFG_DIV0_MASK	(0xf << 8)
#define CCM_DRAMCLK_CFG_UPD		(0x1 << 16)
#define CCM_DRAMCLK_CFG_RST		(0x1 << 31)

#define CCM_DRAM_GATE_OFFSET_DE_BE0	26

#define CCM_LCD_CH0_CTRL_PLL3		(0 << 24)
#define CCM_LCD_CH0_CTRL_PLL7		(1 << 24)
#define CCM_LCD_CH0_CTRL_PLL3_2X	(2 << 24)
#define CCM_LCD_CH0_CTRL_PLL7_2X	(3 << 24)
#define CCM_LCD_CH0_CTRL_MIPI_PLL	(4 << 24)
#define CCM_LCD_CH0_CTRL_GATE		(0x1 << 31)

#define CCM_LCD_CH1_CTRL_M(n)		((((n) - 1) & 0xf) << 0)
#define CCM_LCD_CH1_CTRL_PLL3		(0 << 24)
#define CCM_LCD_CH1_CTRL_PLL7		(1 << 24)
#define CCM_LCD_CH1_CTRL_PLL3_2X	(2 << 24)
#define CCM_LCD_CH1_CTRL_PLL7_2X	(3 << 24)
#define CCM_LCD_CH1_CTRL_GATE		(0x1 << 31)

#define CCM_HDMI_CTRL_M(n)		((((n) - 1) & 0xf) << 0)
#define CCM_HDMI_CTRL_PLL_MASK		(3 << 24)
#define CCM_HDMI_CTRL_PLL3		(0 << 24)
#define CCM_HDMI_CTRL_PLL7		(1 << 24)
#define CCM_HDMI_CTRL_PLL3_2X		(2 << 24)
#define CCM_HDMI_CTRL_PLL7_2X		(3 << 24)
#define CCM_HDMI_CTRL_DDC_GATE		(0x1 << 30)
#define CCM_HDMI_CTRL_GATE		(0x1 << 31)

#define MBUS_CLK_DEFAULT		0x81000001 /* PLL6 / 2 */

/* ahb_reset0 offsets */
#define AHB_RESET_OFFSET_GMAC		17
#define AHB_RESET_OFFSET_MCTL		14
#define AHB_RESET_OFFSET_MMC3		11
#define AHB_RESET_OFFSET_MMC2		10
#define AHB_RESET_OFFSET_MMC1		9
#define AHB_RESET_OFFSET_MMC0		8
#define AHB_RESET_OFFSET_MMC(n)		(AHB_RESET_OFFSET_MMC0 + (n))

/* ahb_reset0 offsets */
#define AHB_RESET_OFFSET_DRC0		25
#define AHB_RESET_OFFSET_DE_BE0		12
#define AHB_RESET_OFFSET_HDMI		11
#define AHB_RESET_OFFSET_LCD1		5
#define AHB_RESET_OFFSET_LCD0		4

/* apb2 reset */
#define APB2_RESET_UART_SHIFT		(16)
#define APB2_RESET_UART_MASK		(0xff << APB2_RESET_UART_SHIFT)
#define APB2_RESET_TWI_SHIFT		(0)
#define APB2_RESET_TWI_MASK		(0xf << APB2_RESET_TWI_SHIFT)

/* CCM bits common to all Display Engine (and IEP) clock ctrl regs */
#define CCM_DE_CTRL_M(n)		((((n) - 1) & 0xf) << 0)
#define CCM_DE_CTRL_PLL_MASK		(0xf << 24)
#define CCM_DE_CTRL_PLL3		(0 << 24)
#define CCM_DE_CTRL_PLL7		(1 << 24)
#define CCM_DE_CTRL_PLL6_2X		(2 << 24)
#define CCM_DE_CTRL_PLL8		(3 << 24)
#define CCM_DE_CTRL_PLL9		(4 << 24)
#define CCM_DE_CTRL_PLL10		(5 << 24)
#define CCM_DE_CTRL_GATE		(1 << 31)

#endif /* _SUNXI_CLOCK_SUN6I_H */
