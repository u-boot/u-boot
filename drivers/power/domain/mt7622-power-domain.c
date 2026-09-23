// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 * Copyright (C) 2026 BayLibre, SAS
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#include <dm.h>
#include <dt-bindings/power/mt7622-power.h>
#include <linux/bitops.h>

#include "mtk-power-domain.h"

static const struct mtk_scp_domain_data mt7622_scp_domain[] = {
	[MT7622_POWER_DOMAIN_ETHSYS] = {
		.sta_mask = PWR_STATUS_ETHSYS,
		.ctl_offs = SPM_ETHSYS_PWR_CON,
		.sram_pdn_bits = GENMASK(11, 8),
		.sram_pdn_ack_bits = GENMASK(15, 12),
		.bus_prot_mask = (BIT(3) | BIT(17)),
	},
	[MT7622_POWER_DOMAIN_HIF0] = {
		.sta_mask = PWR_STATUS_HIF0,
		.ctl_offs = SPM_HIF0_PWR_CON,
		.sram_pdn_bits = GENMASK(11, 8),
		.sram_pdn_ack_bits = GENMASK(15, 12),
		.bus_prot_mask = GENMASK(25, 24),
	},
	[MT7622_POWER_DOMAIN_HIF1] = {
		.sta_mask = PWR_STATUS_HIF1,
		.ctl_offs = SPM_HIF1_PWR_CON,
		.sram_pdn_bits = GENMASK(11, 8),
		.sram_pdn_ack_bits = GENMASK(15, 12),
		.bus_prot_mask = GENMASK(28, 26),
	},
};

MTK_SCP_SOC_DATA(mt7622, mt7622_scp_domain);

static const struct udevice_id mt7622_power_domain_ids[] = {
	{
		.compatible = "mediatek,mt7622-scpsys",
		.data = (ulong)&mt7622_scp_soc_data,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mt7622_power_domain) = {
	.name = "mt7622_power_domain",
	.id = UCLASS_POWER_DOMAIN,
	.ops = &mtk_power_domain_ops,
	.probe = mtk_scpsys_probe,
	.of_match = mt7622_power_domain_ids,
	.priv_auto = sizeof(struct mtk_scpsys),
};
