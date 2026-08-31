/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 MediaTek Inc.
 * Copyright (C) 2026 BayLibre, SAS
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#ifndef __MTK_POWER_DOMAIN_H
#define __MTK_POWER_DOMAIN_H

#include <clk.h>
#include <power-domain-uclass.h>
#include <linux/bitops.h>
#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/types.h>

struct udevice;

#define SPM_EN			(0xb16 << 16 | 0x1)
#define SPM_VDE_PWR_CON		0x0210
#define SPM_MFG_PWR_CON		0x0214
#define SPM_ISP_PWR_CON		0x0238
#define SPM_DIS_PWR_CON		0x023c
#define SPM_CONN_PWR_CON	0x0280
#define SPM_BDP_PWR_CON		0x029c
#define SPM_ETH_PWR_CON		0x02a0
#define SPM_HIF_PWR_CON		0x02a4
#define SPM_IFR_MSC_PWR_CON	0x02a8
#define SPM_ETHSYS_PWR_CON	0x2e0
#define SPM_HIF0_PWR_CON	0x2e4
#define SPM_HIF1_PWR_CON	0x2e8
#define SPM_PWR_STATUS		0x60c
#define SPM_PWR_STATUS_2ND	0x610

#define PWR_RST_B_BIT		BIT(0)
#define PWR_ISO_BIT		BIT(1)
#define PWR_ON_BIT		BIT(2)
#define PWR_ON_2ND_BIT		BIT(3)
#define PWR_CLK_DIS_BIT		BIT(4)

#define PWR_STATUS_CONN		BIT(1)
#define PWR_STATUS_DISP		BIT(3)
#define PWR_STATUS_MFG		BIT(4)
#define PWR_STATUS_ISP		BIT(5)
#define PWR_STATUS_VDEC		BIT(7)
#define PWR_STATUS_BDP		BIT(14)
#define PWR_STATUS_ETH		BIT(15)
#define PWR_STATUS_HIF		BIT(16)
#define PWR_STATUS_IFR_MSC	BIT(17)
#define PWR_STATUS_ETHSYS	BIT(24)
#define PWR_STATUS_HIF0		BIT(25)
#define PWR_STATUS_HIF1		BIT(26)

/* Infrasys configuration */
#define INFRA_TOPDCM_CTRL	0x10
#define INFRA_TOPAXI_PROT_EN	0x220
#define INFRA_TOPAXI_PROT_STA1	0x228

#define DCM_TOP_EN		BIT(0)

#define SPM_MAX_BUS_PROT_DATA	6

struct mtk_scp_domain;

struct mtk_scpsys_bus_prot_data {
	u32 bus_prot_mask;
	u32 bus_prot_set;
	u32 bus_prot_clr;
	u32 bus_prot_sta;
	bool bus_prot_reg_update;
	bool ignore_clr_ack;
};

struct mtk_scp_domain_data {
	u32 sta_mask;
	int ctl_offs;
	u32 sram_pdn_bits;
	u32 sram_pdn_ack_bits;
	u32 bus_prot_mask;
	const struct mtk_scpsys_bus_prot_data bp_infracfg[SPM_MAX_BUS_PROT_DATA];
	int pwr_sta_offs;
	int pwr_sta2nd_offs;
};

struct mtk_scp_soc_data {
	const struct mtk_scp_domain_data *data;
	int num_domains;
};

#define MTK_SCP_SOC_DATA(_name, _domains)			\
static const struct mtk_scp_soc_data _name##_scp_soc_data = {	\
	.data = _domains,					\
	.num_domains = ARRAY_SIZE(_domains),			\
}

struct mtk_scp_domain {
	const struct mtk_scp_domain_data *data;
	void __iomem *infracfg;
	struct clk_bulk clks;
	struct clk_bulk subsys_clks;
	bool has_pd;
	struct power_domain parent_pd;
};

struct mtk_scpsys {
	void __iomem *base;
	void __iomem *infracfg;
	const struct mtk_scp_soc_data *soc_data;
	struct mtk_scp_domain *domains;
};

#define _BUS_PROT(_mask, _set, _clr, _sta, _update, _ignore) {	\
	.bus_prot_mask = (_mask),				\
	.bus_prot_set = (_set),					\
	.bus_prot_clr = (_clr),					\
	.bus_prot_sta = (_sta),					\
	.bus_prot_reg_update = (_update),			\
	.ignore_clr_ack = (_ignore),				\
}

#define BUS_PROT_WR(_mask, _set, _clr, _sta)			\
	_BUS_PROT(_mask, _set, _clr, _sta, false, false)

int mtk_scpsys_probe(struct udevice *dev);
int mtk_power_controller_probe(struct udevice *dev);

extern const struct power_domain_ops mtk_power_domain_ops;

#endif /* __MTK_POWER_DOMAIN_H */
