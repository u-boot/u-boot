// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2026 MediaTek Inc.
 * Copyright (C) 2026 BayLibre, SAS.
 * Authors: Chris-QJ Chen <chris-qj.chen@mediatek.com>
 *          Julien Stephan <jstephan@baylibre.com>
 */

#include <dm.h>
#include <dt-bindings/power/mediatek,mt8188-power.h>
#include <linux/bitops.h>

#include "mtk-power-domain.h"

/* Infra bus protection register offsets */
#define MT8188_TOP_AXI_PROT_EN_SET			0x2A0
#define MT8188_TOP_AXI_PROT_EN_CLR			0x2A4
#define MT8188_TOP_AXI_PROT_EN_STA			0x228
#define MT8188_TOP_AXI_PROT_EN_1_SET			0x2A8
#define MT8188_TOP_AXI_PROT_EN_1_CLR			0x2AC
#define MT8188_TOP_AXI_PROT_EN_1_STA			0x258
#define MT8188_TOP_AXI_PROT_EN_2_SET			0x714
#define MT8188_TOP_AXI_PROT_EN_2_CLR			0x718
#define MT8188_TOP_AXI_PROT_EN_2_STA			0x724
#define MT8188_TOP_AXI_PROT_EN_MM_SET			0x2D4
#define MT8188_TOP_AXI_PROT_EN_MM_CLR			0x2D8
#define MT8188_TOP_AXI_PROT_EN_MM_STA			0x2EC
#define MT8188_TOP_AXI_PROT_EN_MM_2_SET			0xDCC
#define MT8188_TOP_AXI_PROT_EN_MM_2_CLR			0xDD0
#define MT8188_TOP_AXI_PROT_EN_MM_2_STA			0xDD8
#define MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_SET		0xB84
#define MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_CLR		0xB88
#define MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_STA		0xB90
#define MT8188_TOP_AXI_PROT_EN_SUB_INFRA_VDNR_SET	0xBCC
#define MT8188_TOP_AXI_PROT_EN_SUB_INFRA_VDNR_CLR	0xBD0
#define MT8188_TOP_AXI_PROT_EN_SUB_INFRA_VDNR_STA	0xBD8

/* MFG1 bus protection bits */
#define MT8188_TOP_AXI_PROT_EN_MFG1_STEP1			BIT(11)
#define MT8188_TOP_AXI_PROT_EN_2_MFG1_STEP2			BIT(7)
#define MT8188_TOP_AXI_PROT_EN_1_MFG1_STEP3			BIT(19)
#define MT8188_TOP_AXI_PROT_EN_2_MFG1_STEP4			BIT(5)
#define MT8188_TOP_AXI_PROT_EN_MFG1_STEP5			GENMASK(22, 21)
#define MT8188_TOP_AXI_PROT_EN_SUB_INFRA_VDNR_MFG1_STEP6	BIT(17)

/* PCIe bus protection bits */
#define MT8188_TOP_AXI_PROT_EN_PEXTP_MAC_P0_STEP1		BIT(2)
#define MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_PEXTP_MAC_P0_STEP2	(BIT(8) | BIT(18) | BIT(30))

/* Ethernet / HDMI / DP / eDP bus protection bits */
#define MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_ETHER_STEP1		BIT(24)
#define MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_HDMI_TX_STEP1		BIT(20)
#define MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_DP_TX_STEP1		BIT(23)
#define MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_EDP_TX_STEP1		BIT(22)

/* ADSP bus protection bits */
#define MT8188_TOP_AXI_PROT_EN_2_ADSP_AO_STEP1			GENMASK(31, 29)
#define MT8188_TOP_AXI_PROT_EN_2_ADSP_AO_STEP2			(GENMASK(4, 3) | BIT(28))
#define MT8188_TOP_AXI_PROT_EN_2_ADSP_INFRA_STEP1		(GENMASK(16, 14) | BIT(23) | BIT(27))
#define MT8188_TOP_AXI_PROT_EN_2_ADSP_INFRA_STEP2		(GENMASK(19, 17) | GENMASK(26, 25))
#define MT8188_TOP_AXI_PROT_EN_2_ADSP_STEP1			GENMASK(11, 8)
#define MT8188_TOP_AXI_PROT_EN_2_ADSP_STEP2			GENMASK(22, 21)

/* Audio bus protection bits */
#define MT8188_TOP_AXI_PROT_EN_2_AUDIO_STEP1			BIT(20)
#define MT8188_TOP_AXI_PROT_EN_2_AUDIO_STEP2			BIT(12)
#define MT8188_TOP_AXI_PROT_EN_2_AUDIO_ASRC_STEP1		BIT(24)
#define MT8188_TOP_AXI_PROT_EN_2_AUDIO_ASRC_STEP2		BIT(13)

/* Video / display bus protection bits */
#define MT8188_TOP_AXI_PROT_EN_VPPSYS0_STEP1			BIT(10)
#define MT8188_TOP_AXI_PROT_EN_MM_2_VPPSYS0_STEP2		GENMASK(9, 8)
#define MT8188_TOP_AXI_PROT_EN_VPPSYS0_STEP3			BIT(23)
#define MT8188_TOP_AXI_PROT_EN_MM_2_VPPSYS0_STEP4		(BIT(1) | BIT(4) | BIT(11))
#define MT8188_TOP_AXI_PROT_EN_SUB_INFRA_VDNR_VPPSYS0_STEP5	BIT(20)
#define MT8188_TOP_AXI_PROT_EN_MM_VDOSYS0_STEP1			(GENMASK(18, 17) | GENMASK(21, 20))
#define MT8188_TOP_AXI_PROT_EN_VDOSYS0_STEP2			BIT(6)
#define MT8188_TOP_AXI_PROT_EN_SUB_INFRA_VDNR_VDOSYS0_STEP3	BIT(21)
#define MT8188_TOP_AXI_PROT_EN_MM_VDOSYS1_STEP1			GENMASK(31, 30)
#define MT8188_TOP_AXI_PROT_EN_MM_VDOSYS1_STEP2			BIT(22)
#define MT8188_TOP_AXI_PROT_EN_MM_2_VDOSYS1_STEP3		BIT(10)
#define MT8188_TOP_AXI_PROT_EN_MM_VPPSYS1_STEP1			GENMASK(6, 5)
#define MT8188_TOP_AXI_PROT_EN_MM_VPPSYS1_STEP2			BIT(23)
#define MT8188_TOP_AXI_PROT_EN_MM_2_VPPSYS1_STEP3		BIT(18)
#define MT8188_TOP_AXI_PROT_EN_MM_2_WPE_STEP1			BIT(23)
#define MT8188_TOP_AXI_PROT_EN_MM_2_WPE_STEP2			BIT(21)
#define MT8188_TOP_AXI_PROT_EN_MM_VDEC0_STEP1			BIT(13)
#define MT8188_TOP_AXI_PROT_EN_MM_2_VDEC0_STEP2			BIT(13)
#define MT8188_TOP_AXI_PROT_EN_MM_VDEC1_STEP1			BIT(14)
#define MT8188_TOP_AXI_PROT_EN_MM_VDEC1_STEP2			BIT(29)
#define MT8188_TOP_AXI_PROT_EN_MM_VENC_STEP1			(BIT(9) | BIT(11))
#define MT8188_TOP_AXI_PROT_EN_MM_VENC_STEP2			BIT(26)
#define MT8188_TOP_AXI_PROT_EN_MM_2_VENC_STEP3			BIT(2)
#define MT8188_TOP_AXI_PROT_EN_MM_IMG_VCORE_STEP1		(BIT(1) | BIT(3))
#define MT8188_TOP_AXI_PROT_EN_MM_IMG_VCORE_STEP2		BIT(25)
#define MT8188_TOP_AXI_PROT_EN_MM_2_IMG_VCORE_STEP3		BIT(16)
#define MT8188_TOP_AXI_PROT_EN_MM_2_IMG_MAIN_STEP1		GENMASK(27, 26)
#define MT8188_TOP_AXI_PROT_EN_MM_2_IMG_MAIN_STEP2		GENMASK(25, 24)
#define MT8188_TOP_AXI_PROT_EN_MM_CAM_VCORE_STEP1		(BIT(2) | BIT(4))
#define MT8188_TOP_AXI_PROT_EN_2_CAM_VCORE_STEP2		BIT(0)
#define MT8188_TOP_AXI_PROT_EN_1_CAM_VCORE_STEP3		BIT(22)
#define MT8188_TOP_AXI_PROT_EN_MM_CAM_VCORE_STEP4		BIT(24)
#define MT8188_TOP_AXI_PROT_EN_MM_2_CAM_VCORE_STEP5		BIT(17)
#define MT8188_TOP_AXI_PROT_EN_MM_2_CAM_MAIN_STEP1		GENMASK(31, 30)
#define MT8188_TOP_AXI_PROT_EN_2_CAM_MAIN_STEP2			BIT(2)
#define MT8188_TOP_AXI_PROT_EN_MM_2_CAM_MAIN_STEP3		GENMASK(29, 28)
#define MT8188_TOP_AXI_PROT_EN_2_CAM_MAIN_STEP4			BIT(1)

static const struct mtk_scp_domain_data mt8188_scp_domain[] = {
	[MT8188_POWER_DOMAIN_MFG0] = {
		.sta_mask = BIT(1),
		.ctl_offs = 0x300,
		.pwr_sta_offs = 0x174,
		.pwr_sta2nd_offs = 0x178,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8188_POWER_DOMAIN_MFG1] = {
		.sta_mask = BIT(2),
		.ctl_offs = 0x304,
		.pwr_sta_offs = 0x174,
		.pwr_sta2nd_offs = 0x178,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MFG1_STEP1,
				    MT8188_TOP_AXI_PROT_EN_SET,
				    MT8188_TOP_AXI_PROT_EN_CLR,
				    MT8188_TOP_AXI_PROT_EN_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_2_MFG1_STEP2,
				    MT8188_TOP_AXI_PROT_EN_2_SET,
				    MT8188_TOP_AXI_PROT_EN_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_2_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_1_MFG1_STEP3,
				    MT8188_TOP_AXI_PROT_EN_1_SET,
				    MT8188_TOP_AXI_PROT_EN_1_CLR,
				    MT8188_TOP_AXI_PROT_EN_1_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_2_MFG1_STEP4,
				    MT8188_TOP_AXI_PROT_EN_2_SET,
				    MT8188_TOP_AXI_PROT_EN_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_2_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MFG1_STEP5,
				    MT8188_TOP_AXI_PROT_EN_SET,
				    MT8188_TOP_AXI_PROT_EN_CLR,
				    MT8188_TOP_AXI_PROT_EN_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_SUB_INFRA_VDNR_MFG1_STEP6,
				    MT8188_TOP_AXI_PROT_EN_SUB_INFRA_VDNR_SET,
				    MT8188_TOP_AXI_PROT_EN_SUB_INFRA_VDNR_CLR,
				    MT8188_TOP_AXI_PROT_EN_SUB_INFRA_VDNR_STA),
		},
	},
	[MT8188_POWER_DOMAIN_MFG2] = {
		.sta_mask = BIT(3),
		.ctl_offs = 0x308,
		.pwr_sta_offs = 0x174,
		.pwr_sta2nd_offs = 0x178,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8188_POWER_DOMAIN_MFG3] = {
		.sta_mask = BIT(4),
		.ctl_offs = 0x30C,
		.pwr_sta_offs = 0x174,
		.pwr_sta2nd_offs = 0x178,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8188_POWER_DOMAIN_MFG4] = {
		.sta_mask = BIT(5),
		.ctl_offs = 0x310,
		.pwr_sta_offs = 0x174,
		.pwr_sta2nd_offs = 0x178,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8188_POWER_DOMAIN_PEXTP_MAC_P0] = {
		.sta_mask = BIT(10),
		.ctl_offs = 0x324,
		.pwr_sta_offs = 0x174,
		.pwr_sta2nd_offs = 0x178,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_PEXTP_MAC_P0_STEP1,
				    MT8188_TOP_AXI_PROT_EN_SET,
				    MT8188_TOP_AXI_PROT_EN_CLR,
				    MT8188_TOP_AXI_PROT_EN_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_PEXTP_MAC_P0_STEP2,
				    MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_SET,
				    MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_CLR,
				    MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_STA),
		},
	},
	[MT8188_POWER_DOMAIN_PEXTP_PHY_TOP] = {
		.sta_mask = BIT(12),
		.ctl_offs = 0x328,
		.pwr_sta_offs = 0x174,
		.pwr_sta2nd_offs = 0x178,
	},
	[MT8188_POWER_DOMAIN_CSIRX_TOP] = {
		.sta_mask = BIT(17),
		.ctl_offs = 0x3C4,
		.pwr_sta_offs = 0x174,
		.pwr_sta2nd_offs = 0x178,
	},
	[MT8188_POWER_DOMAIN_ETHER] = {
		.sta_mask = BIT(1),
		.ctl_offs = 0x338,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_ETHER_STEP1,
				    MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_SET,
				    MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_CLR,
				    MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_STA),
		},
	},
	[MT8188_POWER_DOMAIN_HDMI_TX] = {
		.sta_mask = BIT(18),
		.ctl_offs = 0x37C,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_HDMI_TX_STEP1,
				    MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_SET,
				    MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_CLR,
				    MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_STA),
		},
	},
	[MT8188_POWER_DOMAIN_ADSP_AO] = {
		.sta_mask = BIT(10),
		.ctl_offs = 0x35C,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_2_ADSP_AO_STEP1,
				    MT8188_TOP_AXI_PROT_EN_2_SET,
				    MT8188_TOP_AXI_PROT_EN_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_2_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_2_ADSP_AO_STEP2,
				    MT8188_TOP_AXI_PROT_EN_2_SET,
				    MT8188_TOP_AXI_PROT_EN_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_2_STA),
		},
	},
	[MT8188_POWER_DOMAIN_ADSP_INFRA] = {
		.sta_mask = BIT(9),
		.ctl_offs = 0x358,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_2_ADSP_INFRA_STEP1,
				    MT8188_TOP_AXI_PROT_EN_2_SET,
				    MT8188_TOP_AXI_PROT_EN_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_2_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_2_ADSP_INFRA_STEP2,
				    MT8188_TOP_AXI_PROT_EN_2_SET,
				    MT8188_TOP_AXI_PROT_EN_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_2_STA),
		},
	},
	[MT8188_POWER_DOMAIN_ADSP] = {
		.sta_mask = BIT(8),
		.ctl_offs = 0x354,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_2_ADSP_STEP1,
				    MT8188_TOP_AXI_PROT_EN_2_SET,
				    MT8188_TOP_AXI_PROT_EN_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_2_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_2_ADSP_STEP2,
				    MT8188_TOP_AXI_PROT_EN_2_SET,
				    MT8188_TOP_AXI_PROT_EN_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_2_STA),
		},
	},
	[MT8188_POWER_DOMAIN_AUDIO] = {
		.sta_mask = BIT(6),
		.ctl_offs = 0x34C,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_2_AUDIO_STEP1,
				    MT8188_TOP_AXI_PROT_EN_2_SET,
				    MT8188_TOP_AXI_PROT_EN_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_2_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_2_AUDIO_STEP2,
				    MT8188_TOP_AXI_PROT_EN_2_SET,
				    MT8188_TOP_AXI_PROT_EN_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_2_STA),
		},
	},
	[MT8188_POWER_DOMAIN_AUDIO_ASRC] = {
		.sta_mask = BIT(7),
		.ctl_offs = 0x350,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_2_AUDIO_ASRC_STEP1,
				    MT8188_TOP_AXI_PROT_EN_2_SET,
				    MT8188_TOP_AXI_PROT_EN_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_2_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_2_AUDIO_ASRC_STEP2,
				    MT8188_TOP_AXI_PROT_EN_2_SET,
				    MT8188_TOP_AXI_PROT_EN_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_2_STA),
		},
	},
	[MT8188_POWER_DOMAIN_VPPSYS0] = {
		.sta_mask = BIT(11),
		.ctl_offs = 0x360,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_VPPSYS0_STEP1,
				    MT8188_TOP_AXI_PROT_EN_SET,
				    MT8188_TOP_AXI_PROT_EN_CLR,
				    MT8188_TOP_AXI_PROT_EN_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_2_VPPSYS0_STEP2,
				    MT8188_TOP_AXI_PROT_EN_MM_2_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_2_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_VPPSYS0_STEP3,
				    MT8188_TOP_AXI_PROT_EN_SET,
				    MT8188_TOP_AXI_PROT_EN_CLR,
				    MT8188_TOP_AXI_PROT_EN_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_2_VPPSYS0_STEP4,
				    MT8188_TOP_AXI_PROT_EN_MM_2_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_2_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_SUB_INFRA_VDNR_VPPSYS0_STEP5,
				    MT8188_TOP_AXI_PROT_EN_SUB_INFRA_VDNR_SET,
				    MT8188_TOP_AXI_PROT_EN_SUB_INFRA_VDNR_CLR,
				    MT8188_TOP_AXI_PROT_EN_SUB_INFRA_VDNR_STA),
		},
	},
	[MT8188_POWER_DOMAIN_VDOSYS0] = {
		.sta_mask = BIT(13),
		.ctl_offs = 0x368,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_VDOSYS0_STEP1,
				    MT8188_TOP_AXI_PROT_EN_MM_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_VDOSYS0_STEP2,
				    MT8188_TOP_AXI_PROT_EN_SET,
				    MT8188_TOP_AXI_PROT_EN_CLR,
				    MT8188_TOP_AXI_PROT_EN_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_SUB_INFRA_VDNR_VDOSYS0_STEP3,
				    MT8188_TOP_AXI_PROT_EN_SUB_INFRA_VDNR_SET,
				    MT8188_TOP_AXI_PROT_EN_SUB_INFRA_VDNR_CLR,
				    MT8188_TOP_AXI_PROT_EN_SUB_INFRA_VDNR_STA),
		},
	},
	[MT8188_POWER_DOMAIN_VDOSYS1] = {
		.sta_mask = BIT(14),
		.ctl_offs = 0x36C,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_VDOSYS1_STEP1,
				    MT8188_TOP_AXI_PROT_EN_MM_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_VDOSYS1_STEP2,
				    MT8188_TOP_AXI_PROT_EN_MM_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_2_VDOSYS1_STEP3,
				    MT8188_TOP_AXI_PROT_EN_MM_2_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_2_STA),
		},
	},
	[MT8188_POWER_DOMAIN_DP_TX] = {
		.sta_mask = BIT(16),
		.ctl_offs = 0x374,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_DP_TX_STEP1,
				    MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_SET,
				    MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_CLR,
				    MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_STA),
		},
	},
	[MT8188_POWER_DOMAIN_EDP_TX] = {
		.sta_mask = BIT(17),
		.ctl_offs = 0x378,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_EDP_TX_STEP1,
				    MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_SET,
				    MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_CLR,
				    MT8188_TOP_AXI_PROT_EN_INFRA_VDNR_STA),
		},
	},
	[MT8188_POWER_DOMAIN_VPPSYS1] = {
		.sta_mask = BIT(12),
		.ctl_offs = 0x364,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_VPPSYS1_STEP1,
				    MT8188_TOP_AXI_PROT_EN_MM_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_VPPSYS1_STEP2,
				    MT8188_TOP_AXI_PROT_EN_MM_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_2_VPPSYS1_STEP3,
				    MT8188_TOP_AXI_PROT_EN_MM_2_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_2_STA),
		},
	},
	[MT8188_POWER_DOMAIN_WPE] = {
		.sta_mask = BIT(15),
		.ctl_offs = 0x370,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_2_WPE_STEP1,
				    MT8188_TOP_AXI_PROT_EN_MM_2_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_2_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_2_WPE_STEP2,
				    MT8188_TOP_AXI_PROT_EN_MM_2_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_2_STA),
		},
	},
	[MT8188_POWER_DOMAIN_VDEC0] = {
		.sta_mask = BIT(19),
		.ctl_offs = 0x380,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_VDEC0_STEP1,
				    MT8188_TOP_AXI_PROT_EN_MM_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_2_VDEC0_STEP2,
				    MT8188_TOP_AXI_PROT_EN_MM_2_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_2_STA),
		},
	},
	[MT8188_POWER_DOMAIN_VDEC1] = {
		.sta_mask = BIT(20),
		.ctl_offs = 0x384,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_VDEC1_STEP1,
				    MT8188_TOP_AXI_PROT_EN_MM_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_VDEC1_STEP2,
				    MT8188_TOP_AXI_PROT_EN_MM_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_STA),
		},
	},
	[MT8188_POWER_DOMAIN_VENC] = {
		.sta_mask = BIT(22),
		.ctl_offs = 0x38C,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_VENC_STEP1,
				    MT8188_TOP_AXI_PROT_EN_MM_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_VENC_STEP2,
				    MT8188_TOP_AXI_PROT_EN_MM_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_2_VENC_STEP3,
				    MT8188_TOP_AXI_PROT_EN_MM_2_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_2_STA),
		},
	},
	[MT8188_POWER_DOMAIN_IMG_VCORE] = {
		.sta_mask = BIT(28),
		.ctl_offs = 0x3A4,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_IMG_VCORE_STEP1,
				    MT8188_TOP_AXI_PROT_EN_MM_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_IMG_VCORE_STEP2,
				    MT8188_TOP_AXI_PROT_EN_MM_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_2_IMG_VCORE_STEP3,
				    MT8188_TOP_AXI_PROT_EN_MM_2_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_2_STA),
		},
	},
	[MT8188_POWER_DOMAIN_IMG_MAIN] = {
		.sta_mask = BIT(29),
		.ctl_offs = 0x3A8,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_2_IMG_MAIN_STEP1,
				    MT8188_TOP_AXI_PROT_EN_MM_2_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_2_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_2_IMG_MAIN_STEP2,
				    MT8188_TOP_AXI_PROT_EN_MM_2_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_2_STA),
		},
	},
	[MT8188_POWER_DOMAIN_DIP] = {
		.sta_mask = BIT(30),
		.ctl_offs = 0x3AC,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8188_POWER_DOMAIN_IPE] = {
		.sta_mask = BIT(31),
		.ctl_offs = 0x3B0,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8188_POWER_DOMAIN_CAM_VCORE] = {
		.sta_mask = BIT(27),
		.ctl_offs = 0x3A0,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_CAM_VCORE_STEP1,
				    MT8188_TOP_AXI_PROT_EN_MM_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_2_CAM_VCORE_STEP2,
				    MT8188_TOP_AXI_PROT_EN_2_SET,
				    MT8188_TOP_AXI_PROT_EN_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_2_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_1_CAM_VCORE_STEP3,
				    MT8188_TOP_AXI_PROT_EN_1_SET,
				    MT8188_TOP_AXI_PROT_EN_1_CLR,
				    MT8188_TOP_AXI_PROT_EN_1_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_CAM_VCORE_STEP4,
				    MT8188_TOP_AXI_PROT_EN_MM_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_2_CAM_VCORE_STEP5,
				    MT8188_TOP_AXI_PROT_EN_MM_2_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_2_STA),
		},
	},
	[MT8188_POWER_DOMAIN_CAM_MAIN] = {
		.sta_mask = BIT(24),
		.ctl_offs = 0x394,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_2_CAM_MAIN_STEP1,
				    MT8188_TOP_AXI_PROT_EN_MM_2_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_2_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_2_CAM_MAIN_STEP2,
				    MT8188_TOP_AXI_PROT_EN_2_SET,
				    MT8188_TOP_AXI_PROT_EN_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_2_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_MM_2_CAM_MAIN_STEP3,
				    MT8188_TOP_AXI_PROT_EN_MM_2_SET,
				    MT8188_TOP_AXI_PROT_EN_MM_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_MM_2_STA),
			BUS_PROT_WR(MT8188_TOP_AXI_PROT_EN_2_CAM_MAIN_STEP4,
				    MT8188_TOP_AXI_PROT_EN_2_SET,
				    MT8188_TOP_AXI_PROT_EN_2_CLR,
				    MT8188_TOP_AXI_PROT_EN_2_STA),
		},
	},
	[MT8188_POWER_DOMAIN_CAM_SUBA] = {
		.sta_mask = BIT(25),
		.ctl_offs = 0x398,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8188_POWER_DOMAIN_CAM_SUBB] = {
		.sta_mask = BIT(26),
		.ctl_offs = 0x39C,
		.pwr_sta_offs = 0x16C,
		.pwr_sta2nd_offs = 0x170,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
};

MTK_SCP_SOC_DATA(mt8188, mt8188_scp_domain);

static const struct udevice_id mt8188_power_domain_ids[] = {
	{
		.compatible = "mediatek,mt8188-power-controller",
		.data = (ulong)&mt8188_scp_soc_data,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mt8188_power_domain) = {
	.name = "mt8188_power_domain",
	.id = UCLASS_POWER_DOMAIN,
	.ops = &mtk_power_domain_ops,
	.probe = mtk_power_controller_probe,
	.of_match = mt8188_power_domain_ids,
	.priv_auto = sizeof(struct mtk_scpsys),
};
