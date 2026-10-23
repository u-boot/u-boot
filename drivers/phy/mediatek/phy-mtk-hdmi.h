/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2018 MediaTek Inc.
 * Copyright (c) 2026 BayLibre, SAS
 * Author: Chunhui Dai <chunhui.dai@mediatek.com>
 */

#ifndef _MTK_HDMI_PHY_H
#define _MTK_HDMI_PHY_H

#include <clk.h>
#include <generic-phy.h>

struct mtk_hdmi_phy;

/*
 * The Linux driver models the HDMI PLL as a clock provider (struct clk_hw /
 * struct clk_ops) and the 5V output as a regulator. U-Boot has no consumer for
 * either, so the PLL clk_ops are folded into the callbacks below (keeping the
 * kernel prepare/unprepare/set_rate/determine_rate naming) and the 5V control
 * is exposed as power_control(), driven directly from the PHY power ops.
 */
struct mtk_hdmi_phy_conf {
	int (*pll_prepare)(struct mtk_hdmi_phy *hdmi_phy);
	void (*pll_unprepare)(struct mtk_hdmi_phy *hdmi_phy);
	int (*pll_set_rate)(struct mtk_hdmi_phy *hdmi_phy, unsigned long rate);
	void (*pll_determine_rate)(struct mtk_hdmi_phy *hdmi_phy,
				   unsigned long rate);
	void (*hdmi_phy_enable_tmds)(struct mtk_hdmi_phy *hdmi_phy);
	void (*hdmi_phy_disable_tmds)(struct mtk_hdmi_phy *hdmi_phy);
	int (*hdmi_phy_configure)(struct phy *phy, void *opts);
	void (*power_control)(struct mtk_hdmi_phy *hdmi_phy, bool enable);
};

struct mtk_hdmi_phy {
	struct udevice *dev;
	void __iomem *regs;
	const struct mtk_hdmi_phy_conf *conf;
	struct clk pll_ref;
	unsigned long pll_rate;
	bool tmds_over_340M;
};

extern const struct mtk_hdmi_phy_conf mtk_hdmi_phy_8195_conf;

#endif /* _MTK_HDMI_PHY_H */
