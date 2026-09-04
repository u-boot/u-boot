// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2025 MediaTek Inc.
 * Copyright (C) 2026 BayLibre, SAS.
 * Authors: Qiqi Wang <qiqi.wang@mediatek.com>
 *          David Lechner <dlechner@baylibre.com>
 */

#include <dm.h>
#include <dt-bindings/power/mediatek,mt8189-power.h>
#include <linux/bitops.h>

#include "mtk-power-domain.h"

/* SPM power status register offsets */
#define MT8189_SPM_PWR_STATUS			0x0f40
#define MT8189_SPM_PWR_STATUS_2ND		0x0f44
#define MT8189_SPM_PWR_STATUS_MSB		0x0f48
#define MT8189_SPM_PWR_STATUS_MSB_2ND		0x0f4c
#define MT8189_SPM_XPU_PWR_STATUS		0x0f50
#define MT8189_SPM_XPU_PWR_STATUS_2ND		0x0f54

/* Infra bus protection register offsets */
#define MT8189_PROT_EN_MMSYS_STA_0_SET		0x0c14
#define MT8189_PROT_EN_MMSYS_STA_0_CLR		0x0c18
#define MT8189_PROT_EN_MMSYS_STA_0_RDY		0x0c1c
#define MT8189_PROT_EN_MMSYS_STA_1_SET		0x0c24
#define MT8189_PROT_EN_MMSYS_STA_1_CLR		0x0c28
#define MT8189_PROT_EN_MMSYS_STA_1_RDY		0x0c2c
#define MT8189_PROT_EN_INFRASYS_STA_0_SET	0x0c44
#define MT8189_PROT_EN_INFRASYS_STA_0_CLR	0x0c48
#define MT8189_PROT_EN_INFRASYS_STA_0_RDY	0x0c4c
#define MT8189_PROT_EN_INFRASYS_STA_1_SET	0x0c54
#define MT8189_PROT_EN_INFRASYS_STA_1_CLR	0x0c58
#define MT8189_PROT_EN_INFRASYS_STA_1_RDY	0x0c5c
#define MT8189_PROT_EN_PERISYS_STA_0_SET	0x0c84
#define MT8189_PROT_EN_PERISYS_STA_0_CLR	0x0c88
#define MT8189_PROT_EN_PERISYS_STA_0_RDY	0x0c8c
#define MT8189_PROT_EN_MCU_STA_0_SET		0x0c94
#define MT8189_PROT_EN_MCU_STA_0_CLR		0x0c98
#define MT8189_PROT_EN_MCU_STA_0_RDY		0x0c9c
#define MT8189_PROT_EN_MD_STA_0_SET		0x0ca4
#define MT8189_PROT_EN_MD_STA_0_CLR		0x0ca8
#define MT8189_PROT_EN_MD_STA_0_RDY		0x0cac

/* Bus protection bits */
#define MT8189_PROT_EN_INFRASYS_STA_0_CONN	BIT(8)
#define MT8189_PROT_EN_INFRASYS_STA_1_CONN	BIT(12)
#define MT8189_PROT_EN_INFRASYS_STA_1_MFG1	BIT(20)
#define MT8189_PROT_EN_MCU_STA_0_CONN		BIT(1)
#define MT8189_PROT_EN_MCU_STA_0_CONN_2ND	BIT(0)
#define MT8189_PROT_EN_MD_STA_0_MFG1		(BIT(0) | BIT(2))
#define MT8189_PROT_EN_MD_STA_0_MFG1_2ND	BIT(4)
#define MT8189_PROT_EN_MM_INFRA_IGN		BIT(1)
#define MT8189_PROT_EN_MM_INFRA_2_IGN		BIT(0)
#define MT8189_PROT_EN_MMSYS_STA_0_CAM_MAIN	GENMASK(31, 30)
#define MT8189_PROT_EN_MMSYS_STA_1_CAM_MAIN	GENMASK(10, 9)
#define MT8189_PROT_EN_MMSYS_STA_0_DISP		GENMASK(1, 0)
#define MT8189_PROT_EN_MMSYS_STA_0_ISP_IMG1	BIT(3)
#define MT8189_PROT_EN_MMSYS_STA_1_ISP_IMG1	BIT(7)
#define MT8189_PROT_EN_MMSYS_STA_0_ISP_IPE	BIT(2)
#define MT8189_PROT_EN_MMSYS_STA_1_ISP_IPE	BIT(8)
#define MT8189_PROT_EN_MMSYS_STA_0_MDP0		BIT(18)
#define MT8189_PROT_EN_MMSYS_STA_1_MM_INFRA	GENMASK(3, 2)
#define MT8189_PROT_EN_MMSYS_STA_1_MM_INFRA_2ND	GENMASK(15, 7)
#define MT8189_PROT_EN_MMSYS_STA_0_VDE0		BIT(20)
#define MT8189_PROT_EN_MMSYS_STA_1_VDE0		BIT(13)
#define MT8189_PROT_EN_MMSYS_STA_0_VEN0		BIT(12)
#define MT8189_PROT_EN_MMSYS_STA_1_VEN0		BIT(12)
#define MT8189_PROT_EN_PERISYS_STA_0_AUDIO	BIT(6)
#define MT8189_PROT_EN_PERISYS_STA_0_SSUSB	BIT(7)

/*
 * Three things the Linux driver does for these domains are not modelled
 * here, because this driver only ever powers up the domains that the U-Boot
 * device tree references: the EMICFG GALS sleep protection step that MFG1
 * needs, the domain supply regulator that MFG0 and MFG1 need
 * (MTK_SCPD_DOMAIN_SUPPLY in Linux), and the SRAM isolation with an inverted
 * SRAM power-down bit that ADSP_TOP_DORMANT and EDP_TX_DORMANT need.
 */

static const struct mtk_scp_domain_data mt8189_scp_domain[] = {
	[MT8189_POWER_DOMAIN_CONN] = {
		.sta_mask = BIT(1),
		.ctl_offs = 0xe04,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_2ND,
		.bp_infracfg = {
			BUS_PROT_WR_IGN(MT8189_PROT_EN_MCU_STA_0_CONN,
					MT8189_PROT_EN_MCU_STA_0_SET,
					MT8189_PROT_EN_MCU_STA_0_CLR,
					MT8189_PROT_EN_MCU_STA_0_RDY),
			BUS_PROT_WR_IGN(MT8189_PROT_EN_INFRASYS_STA_1_CONN,
					MT8189_PROT_EN_INFRASYS_STA_1_SET,
					MT8189_PROT_EN_INFRASYS_STA_1_CLR,
					MT8189_PROT_EN_INFRASYS_STA_1_RDY),
			BUS_PROT_WR_IGN(MT8189_PROT_EN_MCU_STA_0_CONN_2ND,
					MT8189_PROT_EN_MCU_STA_0_SET,
					MT8189_PROT_EN_MCU_STA_0_CLR,
					MT8189_PROT_EN_MCU_STA_0_RDY),
			BUS_PROT_WR_IGN(MT8189_PROT_EN_INFRASYS_STA_0_CONN,
					MT8189_PROT_EN_INFRASYS_STA_0_SET,
					MT8189_PROT_EN_INFRASYS_STA_0_CLR,
					MT8189_PROT_EN_INFRASYS_STA_0_RDY),
		},
	},
	[MT8189_POWER_DOMAIN_AUDIO] = {
		.sta_mask = BIT(6),
		.ctl_offs = 0xe18,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR_IGN(MT8189_PROT_EN_PERISYS_STA_0_AUDIO,
					MT8189_PROT_EN_PERISYS_STA_0_SET,
					MT8189_PROT_EN_PERISYS_STA_0_CLR,
					MT8189_PROT_EN_PERISYS_STA_0_RDY),
		},
	},
	[MT8189_POWER_DOMAIN_ADSP_TOP_DORMANT] = {
		.sta_mask = BIT(7),
		.ctl_offs = 0xe1c,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(9),
		.sram_pdn_ack_bits = BIT(13),
	},
	[MT8189_POWER_DOMAIN_ADSP_INFRA] = {
		.sta_mask = BIT(8),
		.ctl_offs = 0xe20,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_2ND,
	},
	[MT8189_POWER_DOMAIN_ADSP_AO] = {
		.sta_mask = BIT(9),
		.ctl_offs = 0xe24,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_2ND,
	},
	[MT8189_POWER_DOMAIN_MM_INFRA] = {
		.sta_mask = BIT(30),
		.ctl_offs = 0xe78,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR_IGN(MT8189_PROT_EN_MMSYS_STA_1_MM_INFRA,
					MT8189_PROT_EN_MMSYS_STA_1_SET,
					MT8189_PROT_EN_MMSYS_STA_1_CLR,
					MT8189_PROT_EN_MMSYS_STA_1_RDY),
			BUS_PROT_WR_IGN(MT8189_PROT_EN_MMSYS_STA_1_MM_INFRA_2ND,
					MT8189_PROT_EN_MMSYS_STA_1_SET,
					MT8189_PROT_EN_MMSYS_STA_1_CLR,
					MT8189_PROT_EN_MMSYS_STA_1_RDY),
			BUS_PROT_WR_IGN_SUBCLK(MT8189_PROT_EN_MM_INFRA_IGN,
					       MT8189_PROT_EN_MMSYS_STA_1_SET,
					       MT8189_PROT_EN_MMSYS_STA_1_CLR,
					       MT8189_PROT_EN_MMSYS_STA_1_RDY),
			BUS_PROT_WR_IGN_SUBCLK(MT8189_PROT_EN_MM_INFRA_2_IGN,
					       MT8189_PROT_EN_MMSYS_STA_1_SET,
					       MT8189_PROT_EN_MMSYS_STA_1_CLR,
					       MT8189_PROT_EN_MMSYS_STA_1_RDY),
		},
	},
	[MT8189_POWER_DOMAIN_ISP_IMG1] = {
		.sta_mask = BIT(10),
		.ctl_offs = 0xe28,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR_IGN(MT8189_PROT_EN_MMSYS_STA_0_ISP_IMG1,
					MT8189_PROT_EN_MMSYS_STA_0_SET,
					MT8189_PROT_EN_MMSYS_STA_0_CLR,
					MT8189_PROT_EN_MMSYS_STA_0_RDY),
			BUS_PROT_WR_IGN(MT8189_PROT_EN_MMSYS_STA_1_ISP_IMG1,
					MT8189_PROT_EN_MMSYS_STA_1_SET,
					MT8189_PROT_EN_MMSYS_STA_1_CLR,
					MT8189_PROT_EN_MMSYS_STA_1_RDY),
		},
	},
	[MT8189_POWER_DOMAIN_ISP_IMG2] = {
		.sta_mask = BIT(11),
		.ctl_offs = 0xe2c,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8189_POWER_DOMAIN_ISP_IPE] = {
		.sta_mask = BIT(12),
		.ctl_offs = 0xe30,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR_IGN(MT8189_PROT_EN_MMSYS_STA_0_ISP_IPE,
					MT8189_PROT_EN_MMSYS_STA_0_SET,
					MT8189_PROT_EN_MMSYS_STA_0_CLR,
					MT8189_PROT_EN_MMSYS_STA_0_RDY),
			BUS_PROT_WR_IGN(MT8189_PROT_EN_MMSYS_STA_1_ISP_IPE,
					MT8189_PROT_EN_MMSYS_STA_1_SET,
					MT8189_PROT_EN_MMSYS_STA_1_CLR,
					MT8189_PROT_EN_MMSYS_STA_1_RDY),
		},
	},
	[MT8189_POWER_DOMAIN_VDE0] = {
		.sta_mask = BIT(14),
		.ctl_offs = 0xe38,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR_IGN(MT8189_PROT_EN_MMSYS_STA_0_VDE0,
					MT8189_PROT_EN_MMSYS_STA_0_SET,
					MT8189_PROT_EN_MMSYS_STA_0_CLR,
					MT8189_PROT_EN_MMSYS_STA_0_RDY),
			BUS_PROT_WR_IGN(MT8189_PROT_EN_MMSYS_STA_1_VDE0,
					MT8189_PROT_EN_MMSYS_STA_1_SET,
					MT8189_PROT_EN_MMSYS_STA_1_CLR,
					MT8189_PROT_EN_MMSYS_STA_1_RDY),
		},
	},
	[MT8189_POWER_DOMAIN_VEN0] = {
		.sta_mask = BIT(16),
		.ctl_offs = 0xe40,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR_IGN(MT8189_PROT_EN_MMSYS_STA_0_VEN0,
					MT8189_PROT_EN_MMSYS_STA_0_SET,
					MT8189_PROT_EN_MMSYS_STA_0_CLR,
					MT8189_PROT_EN_MMSYS_STA_0_RDY),
			BUS_PROT_WR_IGN(MT8189_PROT_EN_MMSYS_STA_1_VEN0,
					MT8189_PROT_EN_MMSYS_STA_1_SET,
					MT8189_PROT_EN_MMSYS_STA_1_CLR,
					MT8189_PROT_EN_MMSYS_STA_1_RDY),
		},
	},
	[MT8189_POWER_DOMAIN_CAM_MAIN] = {
		.sta_mask = BIT(18),
		.ctl_offs = 0xe48,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR_IGN(MT8189_PROT_EN_MMSYS_STA_0_CAM_MAIN,
					MT8189_PROT_EN_MMSYS_STA_0_SET,
					MT8189_PROT_EN_MMSYS_STA_0_CLR,
					MT8189_PROT_EN_MMSYS_STA_0_RDY),
			BUS_PROT_WR_IGN(MT8189_PROT_EN_MMSYS_STA_1_CAM_MAIN,
					MT8189_PROT_EN_MMSYS_STA_1_SET,
					MT8189_PROT_EN_MMSYS_STA_1_CLR,
					MT8189_PROT_EN_MMSYS_STA_1_RDY),
		},
	},
	[MT8189_POWER_DOMAIN_CAM_SUBA] = {
		.sta_mask = BIT(20),
		.ctl_offs = 0xe50,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8189_POWER_DOMAIN_CAM_SUBB] = {
		.sta_mask = BIT(21),
		.ctl_offs = 0xe54,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8189_POWER_DOMAIN_MDP0] = {
		.sta_mask = BIT(26),
		.ctl_offs = 0xe68,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR_IGN(MT8189_PROT_EN_MMSYS_STA_0_MDP0,
					MT8189_PROT_EN_MMSYS_STA_0_SET,
					MT8189_PROT_EN_MMSYS_STA_0_CLR,
					MT8189_PROT_EN_MMSYS_STA_0_RDY),
		},
	},
	[MT8189_POWER_DOMAIN_DISP] = {
		.sta_mask = BIT(28),
		.ctl_offs = 0xe70,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR_IGN(MT8189_PROT_EN_MMSYS_STA_0_DISP,
					MT8189_PROT_EN_MMSYS_STA_0_SET,
					MT8189_PROT_EN_MMSYS_STA_0_CLR,
					MT8189_PROT_EN_MMSYS_STA_0_RDY),
		},
	},
	[MT8189_POWER_DOMAIN_DP_TX] = {
		.sta_mask = BIT(0),
		.ctl_offs = 0xe80,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS_MSB,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_MSB_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8189_POWER_DOMAIN_CSI_RX] = {
		.sta_mask = BIT(7),
		.ctl_offs = 0xe9c,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS_MSB,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_MSB_2ND,
	},
	[MT8189_POWER_DOMAIN_SSUSB] = {
		.sta_mask = BIT(10),
		.ctl_offs = 0xea8,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS_MSB,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_MSB_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR_IGN(MT8189_PROT_EN_PERISYS_STA_0_SSUSB,
					MT8189_PROT_EN_PERISYS_STA_0_SET,
					MT8189_PROT_EN_PERISYS_STA_0_CLR,
					MT8189_PROT_EN_PERISYS_STA_0_RDY),
		},
	},
	[MT8189_POWER_DOMAIN_MFG0] = {
		.sta_mask = BIT(1),
		.ctl_offs = 0xeb4,
		.pwr_sta_offs = MT8189_SPM_XPU_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_XPU_PWR_STATUS_2ND,
	},
	[MT8189_POWER_DOMAIN_MFG1] = {
		.sta_mask = BIT(2),
		.ctl_offs = 0xeb8,
		.pwr_sta_offs = MT8189_SPM_XPU_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_XPU_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR_IGN(MT8189_PROT_EN_INFRASYS_STA_1_MFG1,
					MT8189_PROT_EN_INFRASYS_STA_1_SET,
					MT8189_PROT_EN_INFRASYS_STA_1_CLR,
					MT8189_PROT_EN_INFRASYS_STA_1_RDY),
			BUS_PROT_WR_IGN(MT8189_PROT_EN_MD_STA_0_MFG1,
					MT8189_PROT_EN_MD_STA_0_SET,
					MT8189_PROT_EN_MD_STA_0_CLR,
					MT8189_PROT_EN_MD_STA_0_RDY),
			BUS_PROT_WR_IGN(MT8189_PROT_EN_MD_STA_0_MFG1_2ND,
					MT8189_PROT_EN_MD_STA_0_SET,
					MT8189_PROT_EN_MD_STA_0_CLR,
					MT8189_PROT_EN_MD_STA_0_RDY),
		},
	},
	[MT8189_POWER_DOMAIN_MFG2] = {
		.sta_mask = BIT(3),
		.ctl_offs = 0xebc,
		.pwr_sta_offs = MT8189_SPM_XPU_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_XPU_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8189_POWER_DOMAIN_MFG3] = {
		.sta_mask = BIT(4),
		.ctl_offs = 0xec0,
		.pwr_sta_offs = MT8189_SPM_XPU_PWR_STATUS,
		.pwr_sta2nd_offs = MT8189_SPM_XPU_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8189_POWER_DOMAIN_EDP_TX_DORMANT] = {
		.sta_mask = BIT(12),
		.ctl_offs = 0xf70,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS_MSB,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_MSB_2ND,
		.sram_pdn_bits = BIT(9),
	},
	[MT8189_POWER_DOMAIN_PCIE] = {
		.sta_mask = BIT(13),
		.ctl_offs = 0xf74,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS_MSB,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_MSB_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8189_POWER_DOMAIN_PCIE_PHY] = {
		.sta_mask = BIT(14),
		.ctl_offs = 0xf78,
		.pwr_sta_offs = MT8189_SPM_PWR_STATUS_MSB,
		.pwr_sta2nd_offs = MT8189_SPM_PWR_STATUS_MSB_2ND,
	},
};

MTK_SCP_SOC_DATA(mt8189, mt8189_scp_domain);

static const struct udevice_id mt8189_power_domain_ids[] = {
	{
		.compatible = "mediatek,mt8189-power-controller",
		.data = (ulong)&mt8189_scp_soc_data,
	},
	{ }
};

U_BOOT_DRIVER(mt8189_power_domain) = {
	.name = "mt8189_power_domain",
	.id = UCLASS_POWER_DOMAIN,
	.ops = &mtk_power_domain_ops,
	.probe = mtk_power_controller_probe,
	.of_match = mt8189_power_domain_ids,
	.priv_auto = sizeof(struct mtk_scpsys),
};
