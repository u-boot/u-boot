// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2026 MediaTek Inc.
 * Author: Irving-ch Lin <irving-ch.lin@mediatek.com>
 */

#include <dm.h>
#include <dt-bindings/power/mediatek,mt8366-power.h>
#include <linux/bitops.h>

#include "mtk-power-domain.h"

#define MT8366_SPM_PWR_STATUS			0x16c
#define MT8366_SPM_PWR_STATUS_2ND		0x170

#define MT8366_TOP_AXI_PROT_EN_STA1		0x0228
#define MT8366_TOP_AXI_PROT_EN_STA1_1		0x0258
#define MT8366_TOP_AXI_PROT_EN_SET		0x02a0
#define MT8366_TOP_AXI_PROT_EN_CLR		0x02a4
#define MT8366_TOP_AXI_PROT_EN_1_SET		0x02a8
#define MT8366_TOP_AXI_PROT_EN_1_CLR		0x02ac
#define MT8366_TOP_AXI_PROT_EN_MM_SET		0x02d4
#define MT8366_TOP_AXI_PROT_EN_MM_CLR		0x02d8
#define MT8366_TOP_AXI_PROT_EN_MM_STA1		0x02ec
#define MT8366_TOP_AXI_PROT_EN_SET_2		0x0714
#define MT8366_TOP_AXI_PROT_EN_CLR_2		0x0718
#define MT8366_TOP_AXI_PROT_EN_STA1_2		0x0724
#define MT8366_TOP_AXI_PROT_EN_ADSP_SET		0x0758
#define MT8366_TOP_AXI_PROT_EN_ADSP_CLR		0x075c
#define MT8366_TOP_AXI_PROT_EN_STA1_ADSP	0x0764

#define MT8366_TOP_AXI_PROT_EN_CONN		(BIT(13) | BIT(18))
#define MT8366_TOP_AXI_PROT_EN_CONN_2ND		(BIT(14))
#define MT8366_TOP_AXI_PROT_EN_1_CONN		(BIT(10))
#define MT8366_TOP_AXI_PROT_EN_1_MFG1		(BIT(21))
#define MT8366_TOP_AXI_PROT_EN_2_MFG1		(BIT(5) | BIT(6))
#define MT8366_TOP_AXI_PROT_EN_MFG1		(BIT(21) | BIT(22))
#define MT8366_TOP_AXI_PROT_EN_2_MFG1_2ND	(BIT(7))
#define MT8366_TOP_AXI_PROT_EN_MM_VDEC		(BIT(24))
#define MT8366_TOP_AXI_PROT_EN_MM_VDEC_2ND	(BIT(25))
#define MT8366_TOP_AXI_PROT_EN_MM_VENC		(BIT(26))
#define MT8366_TOP_AXI_PROT_EN_MM_VENC_2ND	(BIT(27))
#define MT8366_TOP_AXI_PROT_EN_2_AUDIO		(BIT(3))
#define MT8366_TOP_AXI_PROT_EN_2_AUDIO_ACK	(BIT(4) | BIT(10) | BIT(13))
#define MT8366_TOP_AXI_PROT_EN_MM_CAM		(BIT(0) | BIT(2))
#define MT8366_TOP_AXI_PROT_EN_MM_CAM_2ND	(BIT(1) | BIT(3))

static const struct mtk_scp_domain_data mt8366_scp_domain[] = {
	[MT8366_POWER_DOMAIN_CONN] = {
		.sta_mask = BIT(0),
		.ctl_offs = 0x304,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.bp_infracfg = {
			BUS_PROT_WR_IGN(MT8366_TOP_AXI_PROT_EN_CONN,
					MT8366_TOP_AXI_PROT_EN_SET,
					MT8366_TOP_AXI_PROT_EN_CLR,
					MT8366_TOP_AXI_PROT_EN_STA1),
			BUS_PROT_WR_IGN(MT8366_TOP_AXI_PROT_EN_CONN_2ND,
					MT8366_TOP_AXI_PROT_EN_SET,
					MT8366_TOP_AXI_PROT_EN_CLR,
					MT8366_TOP_AXI_PROT_EN_STA1),
			BUS_PROT_WR_IGN(MT8366_TOP_AXI_PROT_EN_1_CONN,
					MT8366_TOP_AXI_PROT_EN_1_SET,
					MT8366_TOP_AXI_PROT_EN_1_CLR,
					MT8366_TOP_AXI_PROT_EN_STA1_1),
		},
	},
	[MT8366_POWER_DOMAIN_MFG0] = {
		.sta_mask = BIT(1),
		.ctl_offs = 0x308,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8366_POWER_DOMAIN_MFG1] = {
		.sta_mask = BIT(2),
		.ctl_offs = 0x30c,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR_IGN(MT8366_TOP_AXI_PROT_EN_1_MFG1,
					MT8366_TOP_AXI_PROT_EN_1_SET,
					MT8366_TOP_AXI_PROT_EN_1_CLR,
					MT8366_TOP_AXI_PROT_EN_STA1_1),
			BUS_PROT_WR_IGN(MT8366_TOP_AXI_PROT_EN_2_MFG1,
					MT8366_TOP_AXI_PROT_EN_SET_2,
					MT8366_TOP_AXI_PROT_EN_CLR_2,
					MT8366_TOP_AXI_PROT_EN_STA1_2),
			BUS_PROT_WR_IGN(MT8366_TOP_AXI_PROT_EN_MFG1,
					MT8366_TOP_AXI_PROT_EN_SET,
					MT8366_TOP_AXI_PROT_EN_CLR,
					MT8366_TOP_AXI_PROT_EN_STA1),
			BUS_PROT_WR_IGN(MT8366_TOP_AXI_PROT_EN_2_MFG1_2ND,
					MT8366_TOP_AXI_PROT_EN_SET_2,
					MT8366_TOP_AXI_PROT_EN_CLR_2,
					MT8366_TOP_AXI_PROT_EN_STA1_2),
		},
	},
	[MT8366_POWER_DOMAIN_MFG2] = {
		.sta_mask = BIT(3),
		.ctl_offs = 0x310,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8366_POWER_DOMAIN_MFG3] = {
		.sta_mask = BIT(4),
		.ctl_offs = 0x314,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8366_POWER_DOMAIN_VDEC] = {
		.sta_mask = BIT(23),
		.ctl_offs = 0x340,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR_IGN(MT8366_TOP_AXI_PROT_EN_MM_VDEC,
					MT8366_TOP_AXI_PROT_EN_MM_SET,
					MT8366_TOP_AXI_PROT_EN_MM_CLR,
					MT8366_TOP_AXI_PROT_EN_MM_STA1),
			BUS_PROT_WR_IGN(MT8366_TOP_AXI_PROT_EN_MM_VDEC_2ND,
					MT8366_TOP_AXI_PROT_EN_MM_SET,
					MT8366_TOP_AXI_PROT_EN_MM_CLR,
					MT8366_TOP_AXI_PROT_EN_MM_STA1),
		},
	},
	[MT8366_POWER_DOMAIN_VENC] = {
		.sta_mask = BIT(24),
		.ctl_offs = 0x348,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR_IGN(MT8366_TOP_AXI_PROT_EN_MM_VENC,
					MT8366_TOP_AXI_PROT_EN_MM_SET,
					MT8366_TOP_AXI_PROT_EN_MM_CLR,
					MT8366_TOP_AXI_PROT_EN_MM_STA1),
			BUS_PROT_WR_IGN(MT8366_TOP_AXI_PROT_EN_MM_VENC_2ND,
					MT8366_TOP_AXI_PROT_EN_MM_SET,
					MT8366_TOP_AXI_PROT_EN_MM_CLR,
					MT8366_TOP_AXI_PROT_EN_MM_STA1),
		},
	},
	[MT8366_POWER_DOMAIN_AUDIO] = {
		.sta_mask = BIT(7),
		.ctl_offs = 0x358,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR_IGN_STA_MASK(MT8366_TOP_AXI_PROT_EN_2_AUDIO,
						 MT8366_TOP_AXI_PROT_EN_2_AUDIO_ACK,
						 MT8366_TOP_AXI_PROT_EN_ADSP_SET,
						 MT8366_TOP_AXI_PROT_EN_ADSP_CLR,
						 MT8366_TOP_AXI_PROT_EN_STA1_ADSP),
		},
	},
	[MT8366_POWER_DOMAIN_CAM] = {
		.sta_mask = BIT(13),
		.ctl_offs = 0x35c,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
		.bp_infracfg = {
			BUS_PROT_WR_IGN(MT8366_TOP_AXI_PROT_EN_MM_CAM,
					MT8366_TOP_AXI_PROT_EN_MM_SET,
					MT8366_TOP_AXI_PROT_EN_MM_CLR,
					MT8366_TOP_AXI_PROT_EN_MM_STA1),
			BUS_PROT_WR_IGN(MT8366_TOP_AXI_PROT_EN_MM_CAM_2ND,
					MT8366_TOP_AXI_PROT_EN_MM_SET,
					MT8366_TOP_AXI_PROT_EN_MM_CLR,
					MT8366_TOP_AXI_PROT_EN_MM_STA1),
		},
	},
	[MT8366_POWER_DOMAIN_CAM_RAWA] = {
		.sta_mask = BIT(14),
		.ctl_offs = 0x360,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8366_POWER_DOMAIN_DISP] = {
		.sta_mask = BIT(19),
		.ctl_offs = 0x3f0,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8366_POWER_DOMAIN_DP_TX] = {
		.sta_mask = BIT(9),
		.ctl_offs = 0x3ac,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8366_POWER_DOMAIN_PCIE] = {
		.sta_mask = BIT(12),
		.ctl_offs = 0x3d4,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8366_POWER_DOMAIN_ISP_IMG1] = {
		.sta_mask = BIT(15),
		.ctl_offs = 0x3e0,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8366_POWER_DOMAIN_ISP_IMG2] = {
		.sta_mask = BIT(16),
		.ctl_offs = 0x3e4,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8366_POWER_DOMAIN_ISP_IPE] = {
		.sta_mask = BIT(17),
		.ctl_offs = 0x3e8,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8366_POWER_DOMAIN_MMLSYS_SHUTDOWN] = {
		.sta_mask = BIT(18),
		.ctl_offs = 0x3ec,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8366_POWER_DOMAIN_EPD] = {
		.sta_mask = BIT(20),
		.ctl_offs = 0x3f4,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
	[MT8366_POWER_DOMAIN_CSI_RX] = {
		.sta_mask = BIT(25),
		.ctl_offs = 0x408,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
	},
	[MT8366_POWER_DOMAIN_VADSP_AO] = {
		.sta_mask = BIT(28),
		.ctl_offs = 0x414,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
	},
	[MT8366_POWER_DOMAIN_VADSP_INFRA] = {
		.sta_mask = BIT(27),
		.ctl_offs = 0x410,
		.pwr_sta_offs = MT8366_SPM_PWR_STATUS,
		.pwr_sta2nd_offs = MT8366_SPM_PWR_STATUS_2ND,
		.sram_pdn_bits = BIT(8),
		.sram_pdn_ack_bits = BIT(12),
	},
};

MTK_SCP_SOC_DATA(mt8366, mt8366_scp_domain);

static const struct udevice_id mt8366_power_domain_ids[] = {
	{
		.compatible = "mediatek,mt8366-power-controller",
		.data = (ulong)&mt8366_scp_soc_data,
	},
	{ }
};

U_BOOT_DRIVER(mt8366_power_domain) = {
	.name = "mt8366_power_domain",
	.id = UCLASS_POWER_DOMAIN,
	.ops = &mtk_power_domain_ops,
	.probe = mtk_power_controller_probe,
	.of_match = mt8366_power_domain_ids,
	.priv_auto = sizeof(struct mtk_scpsys),
};
