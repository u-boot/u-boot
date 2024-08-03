/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#ifndef _DT_BINDINGS_CLK_MT7988_H
#define _DT_BINDINGS_CLK_MT7988_H

/* INFRACFG_AO */
#define GATE_OFFSET 65
/* mtk_mux */
#define CK_INFRA_MUX_UART0_SEL		0 /* Linux CLK ID (0) */
#define CK_INFRA_MUX_UART1_SEL		1 /* Linux CLK ID (1) */
#define CK_INFRA_MUX_UART2_SEL		2 /* Linux CLK ID (2) */
#define CK_INFRA_MUX_SPI0_SEL		3 /* Linux CLK ID (3) */
#define CK_INFRA_MUX_SPI1_SEL		4 /* Linux CLK ID (4) */
#define CK_INFRA_MUX_SPI2_SEL		5 /* Linux CLK ID (5) */
#define CK_INFRA_PWM_SEL		6 /* Linux CLK ID (6) */
#define CK_INFRA_PWM_CK1_SEL		7 /* Linux CLK ID (7) */
#define CK_INFRA_PWM_CK2_SEL		8 /* Linux CLK ID (8) */
#define CK_INFRA_PWM_CK3_SEL		9 /* Linux CLK ID (9) */
#define CK_INFRA_PWM_CK4_SEL		10 /* Linux CLK ID (10) */
#define CK_INFRA_PWM_CK5_SEL		11 /* Linux CLK ID (11) */
#define CK_INFRA_PWM_CK6_SEL		12 /* Linux CLK ID (12) */
#define CK_INFRA_PWM_CK7_SEL		13 /* Linux CLK ID (13) */
#define CK_INFRA_PWM_CK8_SEL		14 /* Linux CLK ID (14) */
#define CK_INFRA_PCIE_GFMUX_TL_O_P0_SEL 15 /* Linux CLK ID (15) */
#define CK_INFRA_PCIE_GFMUX_TL_O_P1_SEL 16 /* Linux CLK ID (16) */
#define CK_INFRA_PCIE_GFMUX_TL_O_P2_SEL 17 /* Linux CLK ID (17) */
#define CK_INFRA_PCIE_GFMUX_TL_O_P3_SEL 18 /* Linux CLK ID (18) */
/* mtk_gate */
#define CK_INFRA_PCIE_PERI_26M_CK_P0 (65 - GATE_OFFSET) /* Linux CLK ID (99) */
#define CK_INFRA_PCIE_PERI_26M_CK_P1                                           \
	(66 - GATE_OFFSET) /* Linux CLK ID (100) */
#define CK_INFRA_PCIE_PERI_26M_CK_P2                                           \
	(67 - GATE_OFFSET) /* Linux CLK ID (101) */
#define CK_INFRA_PCIE_PERI_26M_CK_P3                                           \
	(68 - GATE_OFFSET) /* Linux CLK ID (102) */
#define CK_INFRA_66M_GPT_BCK	     (69 - GATE_OFFSET) /* Linux CLK ID (19) */
#define CK_INFRA_66M_PWM_HCK	     (70 - GATE_OFFSET) /* Linux CLK ID (20) */
#define CK_INFRA_66M_PWM_BCK	     (71 - GATE_OFFSET) /* Linux CLK ID (21) */
#define CK_INFRA_66M_PWM_CK1	     (72 - GATE_OFFSET) /* Linux CLK ID (22) */
#define CK_INFRA_66M_PWM_CK2	     (73 - GATE_OFFSET) /* Linux CLK ID (23) */
#define CK_INFRA_66M_PWM_CK3	     (74 - GATE_OFFSET) /* Linux CLK ID (24) */
#define CK_INFRA_66M_PWM_CK4	     (75 - GATE_OFFSET) /* Linux CLK ID (25) */
#define CK_INFRA_66M_PWM_CK5	     (76 - GATE_OFFSET) /* Linux CLK ID (26) */
#define CK_INFRA_66M_PWM_CK6	     (77 - GATE_OFFSET) /* Linux CLK ID (27) */
#define CK_INFRA_66M_PWM_CK7	     (78 - GATE_OFFSET) /* Linux CLK ID (28) */
#define CK_INFRA_66M_PWM_CK8	     (79 - GATE_OFFSET) /* Linux CLK ID (29) */
#define CK_INFRA_133M_CQDMA_BCK	     (80 - GATE_OFFSET) /* Linux CLK ID (30) */
#define CK_INFRA_66M_AUD_SLV_BCK     (81 - GATE_OFFSET) /* Linux CLK ID (31) */
#define CK_INFRA_AUD_26M	     (82 - GATE_OFFSET) /* Linux CLK ID (32) */
#define CK_INFRA_AUD_L		     (83 - GATE_OFFSET) /* Linux CLK ID (33) */
#define CK_INFRA_AUD_AUD	     (84 - GATE_OFFSET) /* Linux CLK ID (34) */
#define CK_INFRA_AUD_EG2	     (85 - GATE_OFFSET) /* Linux CLK ID (35) */
#define CK_INFRA_DRAMC_F26M	     (86 - GATE_OFFSET) /* Linux CLK ID (36) */
#define CK_INFRA_133M_DBG_ACKM	     (87 - GATE_OFFSET) /* Linux CLK ID (37) */
#define CK_INFRA_66M_AP_DMA_BCK	     (88 - GATE_OFFSET) /* Linux CLK ID (38) */
#define CK_INFRA_66M_SEJ_BCK	     (89 - GATE_OFFSET) /* Linux CLK ID (39) */
#define CK_INFRA_PRE_CK_SEJ_F13M     (90 - GATE_OFFSET) /* Linux CLK ID (40) */
#define CK_INFRA_66M_TRNG	     (91 - GATE_OFFSET) /* Linux CLK ID (41) */
#define CK_INFRA_26M_THERM_SYSTEM    (92 - GATE_OFFSET) /* Linux CLK ID (42) */
#define CK_INFRA_I2C_BCK	     (93 - GATE_OFFSET) /* Linux CLK ID (43) */
#define CK_INFRA_66M_UART0_PCK	     (94 - GATE_OFFSET) /* Linux CLK ID (44) */
#define CK_INFRA_66M_UART1_PCK	     (95 - GATE_OFFSET) /* Linux CLK ID (45) */
#define CK_INFRA_66M_UART2_PCK	     (96 - GATE_OFFSET) /* Linux CLK ID (46) */
#define CK_INFRA_52M_UART0_CK	     (97 - GATE_OFFSET) /* Linux CLK ID (47) */
#define CK_INFRA_52M_UART1_CK	     (98 - GATE_OFFSET) /* Linux CLK ID (48) */
#define CK_INFRA_52M_UART2_CK	     (99 - GATE_OFFSET) /* Linux CLK ID (49) */
#define CK_INFRA_NFI		     (100 - GATE_OFFSET) /* Linux CLK ID (50) */
#define CK_INFRA_SPINFI		     (101 - GATE_OFFSET) /* Linux CLK ID (51) */
#define CK_INFRA_66M_NFI_HCK	     (102 - GATE_OFFSET) /* Linux CLK ID (52) */
#define CK_INFRA_104M_SPI0	     (103 - GATE_OFFSET) /* Linux CLK ID (53) */
#define CK_INFRA_104M_SPI1	     (104 - GATE_OFFSET) /* Linux CLK ID (54) */
#define CK_INFRA_104M_SPI2_BCK	     (105 - GATE_OFFSET) /* Linux CLK ID (55) */
#define CK_INFRA_66M_SPI0_HCK	     (106 - GATE_OFFSET) /* Linux CLK ID (56) */
#define CK_INFRA_66M_SPI1_HCK	     (107 - GATE_OFFSET) /* Linux CLK ID (57) */
#define CK_INFRA_66M_SPI2_HCK	     (108 - GATE_OFFSET) /* Linux CLK ID (58) */
#define CK_INFRA_66M_FLASHIF_AXI     (109 - GATE_OFFSET) /* Linux CLK ID (59) */
#define CK_INFRA_RTC		     (110 - GATE_OFFSET) /* Linux CLK ID (60) */
#define CK_INFRA_26M_ADC_BCK	     (111 - GATE_OFFSET) /* Linux CLK ID (61) */
#define CK_INFRA_RC_ADC		     (112 - GATE_OFFSET) /* Linux CLK ID (62) */
#define CK_INFRA_MSDC400	     (113 - GATE_OFFSET) /* Linux CLK ID (63) */
#define CK_INFRA_MSDC2_HCK	     (114 - GATE_OFFSET) /* Linux CLK ID (64) */
#define CK_INFRA_133M_MSDC_0_HCK     (115 - GATE_OFFSET) /* Linux CLK ID (65) */
#define CK_INFRA_66M_MSDC_0_HCK	     (116 - GATE_OFFSET) /* Linux CLK ID (66) */
#define CK_INFRA_133M_CPUM_BCK	     (117 - GATE_OFFSET) /* Linux CLK ID (67) */
#define CK_INFRA_BIST2FPC	     (118 - GATE_OFFSET) /* Linux CLK ID (68) */
#define CK_INFRA_I2C_X16W_MCK_CK_P1  (119 - GATE_OFFSET) /* Linux CLK ID (69) */
#define CK_INFRA_I2C_X16W_PCK_CK_P1  (120 - GATE_OFFSET) /* Linux CLK ID (70) */
#define CK_INFRA_133M_USB_HCK	     (121 - GATE_OFFSET) /* Linux CLK ID (71) */
#define CK_INFRA_133M_USB_HCK_CK_P1  (122 - GATE_OFFSET) /* Linux CLK ID (72) */
#define CK_INFRA_66M_USB_HCK	     (123 - GATE_OFFSET) /* Linux CLK ID (73) */
#define CK_INFRA_66M_USB_HCK_CK_P1   (124 - GATE_OFFSET) /* Linux CLK ID (74) */
#define CK_INFRA_USB_SYS	     (125 - GATE_OFFSET) /* Linux CLK ID (75) */
#define CK_INFRA_USB_SYS_CK_P1	     (126 - GATE_OFFSET) /* Linux CLK ID (76) */
#define CK_INFRA_USB_REF	     (127 - GATE_OFFSET) /* Linux CLK ID (77) */
#define CK_INFRA_USB_CK_P1	     (128 - GATE_OFFSET) /* Linux CLK ID (78) */
#define CK_INFRA_USB_FRMCNT	     (129 - GATE_OFFSET) /* Linux CLK ID (79) */
#define CK_INFRA_USB_FRMCNT_CK_P1    (130 - GATE_OFFSET) /* Linux CLK ID (80) */
#define CK_INFRA_USB_PIPE	     (131 - GATE_OFFSET) /* Linux CLK ID (81) */
#define CK_INFRA_USB_PIPE_CK_P1	     (132 - GATE_OFFSET) /* Linux CLK ID (82) */
#define CK_INFRA_USB_UTMI	     (133 - GATE_OFFSET) /* Linux CLK ID (83) */
#define CK_INFRA_USB_UTMI_CK_P1	     (134 - GATE_OFFSET) /* Linux CLK ID (84) */
#define CK_INFRA_USB_XHCI	     (135 - GATE_OFFSET) /* Linux CLK ID (85) */
#define CK_INFRA_USB_XHCI_CK_P1	     (136 - GATE_OFFSET) /* Linux CLK ID (86) */
#define CK_INFRA_PCIE_GFMUX_TL_P0    (137 - GATE_OFFSET) /* Linux CLK ID (87) */
#define CK_INFRA_PCIE_GFMUX_TL_P1    (138 - GATE_OFFSET) /* Linux CLK ID (88) */
#define CK_INFRA_PCIE_GFMUX_TL_P2    (139 - GATE_OFFSET) /* Linux CLK ID (89) */
#define CK_INFRA_PCIE_GFMUX_TL_P3    (140 - GATE_OFFSET) /* Linux CLK ID (90) */
#define CK_INFRA_PCIE_PIPE_P0	     (141 - GATE_OFFSET) /* Linux CLK ID (91) */
#define CK_INFRA_PCIE_PIPE_P1	     (142 - GATE_OFFSET) /* Linux CLK ID (92) */
#define CK_INFRA_PCIE_PIPE_P2	     (143 - GATE_OFFSET) /* Linux CLK ID (93) */
#define CK_INFRA_PCIE_PIPE_P3	     (144 - GATE_OFFSET) /* Linux CLK ID (94) */
#define CK_INFRA_133M_PCIE_CK_P0     (145 - GATE_OFFSET) /* Linux CLK ID (95) */
#define CK_INFRA_133M_PCIE_CK_P1     (146 - GATE_OFFSET) /* Linux CLK ID (96) */
#define CK_INFRA_133M_PCIE_CK_P2     (147 - GATE_OFFSET) /* Linux CLK ID (97) */
#define CK_INFRA_133M_PCIE_CK_P3     (148 - GATE_OFFSET) /* Linux CLK ID (98) */

/* TOPCKGEN */
/* mtk_fixed_clk */
#define CK_TOP_XTAL		      0 /* Linux CLK ID (109) */
/* mtk_fixed_factor */
#define CK_TOP_XTAL_D2		      1 /* Linux CLK ID (109) */
#define CK_TOP_RTC_32K		      2 /* Linux CLK ID (110) */
#define CK_TOP_RTC_32P7K	      3 /* Linux CLK ID (111) */
#define CK_TOP_MPLL_D2		      4 /* Linux CLK ID (76) */
#define CK_TOP_MPLL_D3_D2	      5 /* Linux CLK ID (77) */
#define CK_TOP_MPLL_D4		      6 /* Linux CLK ID (78) */
#define CK_TOP_MPLL_D8		      7 /* Linux CLK ID (79) */
#define CK_TOP_MPLL_D8_D2	      8 /* Linux CLK ID (80) */
#define CK_TOP_MMPLL_D2		      9 /* Linux CLK ID (82) */
#define CK_TOP_MMPLL_D3_D5	      10 /* Linux CLK ID (83) */
#define CK_TOP_MMPLL_D4		      11 /* Linux CLK ID (84) */
#define CK_TOP_MMPLL_D6_D2	      12 /* Linux CLK ID (85) */
#define CK_TOP_MMPLL_D8		      13 /* Linux CLK ID (86) */
#define CK_TOP_APLL2_D4		      14 /* Linux CLK ID (88) */
#define CK_TOP_NET1PLL_D4	      15 /* Linux CLK ID (89) */
#define CK_TOP_NET1PLL_D5	      16 /* Linux CLK ID (90) */
#define CK_TOP_NET1PLL_D5_D2	      17 /* Linux CLK ID (91) */
#define CK_TOP_NET1PLL_D5_D4	      18 /* Linux CLK ID (92) */
#define CK_TOP_NET1PLL_D8	      19 /* Linux CLK ID (93) */
#define CK_TOP_NET1PLL_D8_D2	      20 /* Linux CLK ID (94) */
#define CK_TOP_NET1PLL_D8_D4	      21 /* Linux CLK ID (95) */
#define CK_TOP_NET1PLL_D8_D8	      22 /* Linux CLK ID (96) */
#define CK_TOP_NET1PLL_D8_D16	      23 /* Linux CLK ID (97) */
#define CK_TOP_NET2PLL_D2	      24 /* Linux CLK ID (99) */
#define CK_TOP_NET2PLL_D4	      25 /* Linux CLK ID (100) */
#define CK_TOP_NET2PLL_D4_D4	      26 /* Linux CLK ID (101) */
#define CK_TOP_NET2PLL_D4_D8	      27 /* Linux CLK ID (102) */
#define CK_TOP_NET2PLL_D6	      28 /* Linux CLK ID (103) */
#define CK_TOP_NET2PLL_D8	      29 /* Linux CLK ID (104) */
/* mtk_mux */
#define CK_TOP_NETSYS_SEL	      30 /* Linux CLK ID (0) */
#define CK_TOP_NETSYS_500M_SEL	      31 /* Linux CLK ID (1) */
#define CK_TOP_NETSYS_2X_SEL	      32 /* Linux CLK ID (2) */
#define CK_TOP_NETSYS_GSW_SEL	      33 /* Linux CLK ID (3) */
#define CK_TOP_ETH_GMII_SEL	      34 /* Linux CLK ID (4) */
#define CK_TOP_NETSYS_MCU_SEL	      35 /* Linux CLK ID (5) */
#define CK_TOP_NETSYS_PAO_2X_SEL      36 /* Linux CLK ID (6) */
#define CK_TOP_EIP197_SEL	      37 /* Linux CLK ID (7) */
#define CK_TOP_AXI_INFRA_SEL	      38 /* Linux CLK ID (8) */
#define CK_TOP_UART_SEL		      39 /* Linux CLK ID (9) */
#define CK_TOP_EMMC_250M_SEL	      40 /* Linux CLK ID (10) */
#define CK_TOP_EMMC_400M_SEL	      41 /* Linux CLK ID (11) */
#define CK_TOP_SPI_SEL		      42 /* Linux CLK ID (12) */
#define CK_TOP_SPIM_MST_SEL	      43 /* Linux CLK ID (13) */
#define CK_TOP_NFI1X_SEL	      44 /* Linux CLK ID (14) */
#define CK_TOP_SPINFI_SEL	      45 /* Linux CLK ID (15) */
#define CK_TOP_PWM_SEL		      46 /* Linux CLK ID (16) */
#define CK_TOP_I2C_SEL		      47 /* Linux CLK ID (17) */
#define CK_TOP_PCIE_MBIST_250M_SEL    48 /* Linux CLK ID (18) */
#define CK_TOP_PEXTP_TL_SEL	      49 /* Linux CLK ID (19) */
#define CK_TOP_PEXTP_TL_P1_SEL	      50 /* Linux CLK ID (20) */
#define CK_TOP_PEXTP_TL_P2_SEL	      51 /* Linux CLK ID (21) */
#define CK_TOP_PEXTP_TL_P3_SEL	      52 /* Linux CLK ID (22) */
#define CK_TOP_USB_SYS_SEL	      53 /* Linux CLK ID (23) */
#define CK_TOP_USB_SYS_P1_SEL	      54 /* Linux CLK ID (24) */
#define CK_TOP_USB_XHCI_SEL	      55 /* Linux CLK ID (25) */
#define CK_TOP_USB_XHCI_P1_SEL	      56 /* Linux CLK ID (26) */
#define CK_TOP_USB_FRMCNT_SEL	      57 /* Linux CLK ID (27) */
#define CK_TOP_USB_FRMCNT_P1_SEL      58 /* Linux CLK ID (28) */
#define CK_TOP_AUD_SEL		      59 /* Linux CLK ID (29) */
#define CK_TOP_A1SYS_SEL	      60 /* Linux CLK ID (30) */
#define CK_TOP_AUD_L_SEL	      61 /* Linux CLK ID (31) */
#define CK_TOP_A_TUNER_SEL	      62 /* Linux CLK ID (32) */
#define CK_TOP_SSPXTP_SEL	      63 /* Linux CLK ID (33) */
#define CK_TOP_USB_PHY_SEL	      64 /* Linux CLK ID (34) */
#define CK_TOP_USXGMII_SBUS_0_SEL     65 /* Linux CLK ID (35) */
#define CK_TOP_USXGMII_SBUS_1_SEL     66 /* Linux CLK ID (36) */
#define CK_TOP_SGM_0_SEL	      67 /* Linux CLK ID (37) */
#define CK_TOP_SGM_SBUS_0_SEL	      68 /* Linux CLK ID (38) */
#define CK_TOP_SGM_1_SEL	      69 /* Linux CLK ID (39) */
#define CK_TOP_SGM_SBUS_1_SEL	      70 /* Linux CLK ID (40) */
#define CK_TOP_XFI_PHY_0_XTAL_SEL     71 /* Linux CLK ID (41) */
#define CK_TOP_XFI_PHY_1_XTAL_SEL     72 /* Linux CLK ID (42) */
#define CK_TOP_SYSAXI_SEL	      73 /* Linux CLK ID (43) */
#define CK_TOP_SYSAPB_SEL	      74 /* Linux CLK ID (44) */
#define CK_TOP_ETH_REFCK_50M_SEL      75 /* Linux CLK ID (45) */
#define CK_TOP_ETH_SYS_200M_SEL	      76 /* Linux CLK ID (46) */
#define CK_TOP_ETH_SYS_SEL	      77 /* Linux CLK ID (47) */
#define CK_TOP_ETH_XGMII_SEL	      78 /* Linux CLK ID (48) */
#define CK_TOP_BUS_TOPS_SEL	      79 /* Linux CLK ID (49) */
#define CK_TOP_NPU_TOPS_SEL	      80 /* Linux CLK ID (50) */
#define CK_TOP_DRAMC_SEL	      81 /* Linux CLK ID (51) */
#define CK_TOP_DRAMC_MD32_SEL	      82 /* Linux CLK ID (52) */
#define CK_TOP_INFRA_F26M_SEL	      83 /* Linux CLK ID (53) */
#define CK_TOP_PEXTP_P0_SEL	      84 /* Linux CLK ID (54) */
#define CK_TOP_PEXTP_P1_SEL	      85 /* Linux CLK ID (55) */
#define CK_TOP_PEXTP_P2_SEL	      86 /* Linux CLK ID (56) */
#define CK_TOP_PEXTP_P3_SEL	      87 /* Linux CLK ID (57) */
#define CK_TOP_DA_XTP_GLB_P0_SEL      88 /* Linux CLK ID (58) */
#define CK_TOP_DA_XTP_GLB_P1_SEL      89 /* Linux CLK ID (59) */
#define CK_TOP_DA_XTP_GLB_P2_SEL      90 /* Linux CLK ID (60) */
#define CK_TOP_DA_XTP_GLB_P3_SEL      91 /* Linux CLK ID (61) */
#define CK_TOP_CKM_SEL		      92 /* Linux CLK ID (62) */
#define CK_TOP_DA_SEL		      93 /* Linux CLK ID (63) */
#define CK_TOP_PEXTP_SEL	      94 /* Linux CLK ID (64) */
#define CK_TOP_TOPS_P2_26M_SEL	      95 /* Linux CLK ID (65) */
#define CK_TOP_MCUSYS_BACKUP_625M_SEL 96 /* Linux CLK ID (66) */
#define CK_TOP_NETSYS_SYNC_250M_SEL   97 /* Linux CLK ID (67) */
#define CK_TOP_MACSEC_SEL	      98 /* Linux CLK ID (68) */
#define CK_TOP_NETSYS_TOPS_400M_SEL   99 /* Linux CLK ID (69) */
#define CK_TOP_NETSYS_PPEFB_250M_SEL  100 /* Linux CLK ID (70) */
#define CK_TOP_NETSYS_WARP_SEL	      101 /* Linux CLK ID (71) */
#define CK_TOP_ETH_MII_SEL	      102 /* Linux CLK ID (72) */
#define CK_TOP_NPU_SEL		      103 /* Linux CLK ID (73) */

/* APMIXEDSYS */
/* mtk_pll_data */
#define CK_APMIXED_NETSYSPLL  0
#define CK_APMIXED_MPLL	      1
#define CK_APMIXED_MMPLL      2
#define CK_APMIXED_APLL2      3
#define CK_APMIXED_NET1PLL    4
#define CK_APMIXED_NET2PLL    5
#define CK_APMIXED_WEDMCUPLL  6
#define CK_APMIXED_SGMPLL     7
#define CK_APMIXED_ARM_B      8
#define CK_APMIXED_CCIPLL2_B  9
#define CK_APMIXED_USXGMIIPLL 10
#define CK_APMIXED_MSDCPLL    11

/* ETHSYS ETH DMA  */
/* mtk_gate */
#define CK_ETHDMA_FE_EN 0

/* SGMIISYS_0 */
/* mtk_gate */
#define CK_SGM0_TX_EN 0
#define CK_SGM0_RX_EN 1

/* SGMIISYS_1 */
/* mtk_gate */
#define CK_SGM1_TX_EN 0
#define CK_SGM1_RX_EN 1

/* ETHWARP */
/* mtk_gate */
#define CK_ETHWARP_WOCPU2_EN 0
#define CK_ETHWARP_WOCPU1_EN 1
#define CK_ETHWARP_WOCPU0_EN 2

#endif /* _DT_BINDINGS_CLK_MT7988_H */
