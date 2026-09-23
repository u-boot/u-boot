// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 * Copyright (C) 2026 BayLibre, SAS
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#include <dm.h>
#include <dt-bindings/power/mt2701-power.h>
#include <linux/bitops.h>

#include "mtk-power-domain.h"

static const struct mtk_scp_domain_data mt2701_scp_domain[] = {
	[MT2701_POWER_DOMAIN_CONN] = {
		.sta_mask = PWR_STATUS_CONN,
		.ctl_offs = SPM_CONN_PWR_CON,
		.bus_prot_mask = BIT(8) | BIT(2),
	},
	[MT2701_POWER_DOMAIN_DISP] = {
		.sta_mask = PWR_STATUS_DISP,
		.ctl_offs = SPM_DIS_PWR_CON,
		.sram_pdn_bits = GENMASK(11, 8),
		.bus_prot_mask = BIT(2),
	},
	[MT2701_POWER_DOMAIN_MFG] = {
		.sta_mask = PWR_STATUS_MFG,
		.ctl_offs = SPM_MFG_PWR_CON,
		.sram_pdn_bits = GENMASK(11, 8),
		.sram_pdn_ack_bits = GENMASK(12, 12),
	},
	[MT2701_POWER_DOMAIN_VDEC] = {
		.sta_mask = PWR_STATUS_VDEC,
		.ctl_offs = SPM_VDE_PWR_CON,
		.sram_pdn_bits = GENMASK(11, 8),
		.sram_pdn_ack_bits = GENMASK(12, 12),
	},
	[MT2701_POWER_DOMAIN_ISP] = {
		.sta_mask = PWR_STATUS_ISP,
		.ctl_offs = SPM_ISP_PWR_CON,
		.sram_pdn_bits = GENMASK(11, 8),
		.sram_pdn_ack_bits = GENMASK(13, 12),
	},
	[MT2701_POWER_DOMAIN_BDP] = {
		.sta_mask = PWR_STATUS_BDP,
		.ctl_offs = SPM_BDP_PWR_CON,
		.sram_pdn_bits = GENMASK(11, 8),
	},
	[MT2701_POWER_DOMAIN_ETH] = {
		.sta_mask = PWR_STATUS_ETH,
		.ctl_offs = SPM_ETH_PWR_CON,
		.sram_pdn_bits = GENMASK(11, 8),
		.sram_pdn_ack_bits = GENMASK(15, 12),
	},
	[MT2701_POWER_DOMAIN_HIF] = {
		.sta_mask = PWR_STATUS_HIF,
		.ctl_offs = SPM_HIF_PWR_CON,
		.sram_pdn_bits = GENMASK(11, 8),
		.sram_pdn_ack_bits = GENMASK(15, 12),
	},
	[MT2701_POWER_DOMAIN_IFR_MSC] = {
		.sta_mask = PWR_STATUS_IFR_MSC,
		.ctl_offs = SPM_IFR_MSC_PWR_CON,
	},
};

MTK_SCP_SOC_DATA(mt2701, mt2701_scp_domain);

static const struct udevice_id mt2701_power_domain_ids[] = {
	{
		.compatible = "mediatek,mt2701-scpsys",
		.data = (ulong)&mt2701_scp_soc_data,
	},
	{
		.compatible = "mediatek,mt7623-scpsys",
		.data = (ulong)&mt2701_scp_soc_data,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mt2701_power_domain) = {
	.name = "mt2701_power_domain",
	.id = UCLASS_POWER_DOMAIN,
	.ops = &mtk_power_domain_ops,
	.probe = mtk_scpsys_probe,
	.of_match = mt2701_power_domain_ids,
	.priv_auto = sizeof(struct mtk_scpsys),
};
