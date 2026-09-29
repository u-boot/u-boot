/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2019 MediaTek Inc.
 * Copyright (c) 2026 BayLibre, SAS
 * Author: Jitao Shi <jitao.shi@mediatek.com>
 */

#ifndef _PHY_MTK_MIPI_DSI_H
#define _PHY_MTK_MIPI_DSI_H

#include <clk.h>
#include <linux/compiler.h>
#include <linux/types.h>

struct mtk_mipi_tx;
struct udevice;

/*
 * The Linux driver models the MIPI TX PLL as a clock provider (struct clk_hw /
 * struct clk_ops) fed to the DSI host as its "hs" clock. U-Boot has no consumer
 * for that clock, so the PLL clk_ops are folded into the callbacks below
 * (keeping the kernel determine_rate/enable/disable naming) and are driven
 * directly from the PHY ops.
 */
struct mtk_mipitx_data {
	u64 (*pll_determine_rate)(struct mtk_mipi_tx *mipi_tx, u64 rate);
	int (*pll_enable)(struct mtk_mipi_tx *mipi_tx);
	void (*pll_disable)(struct mtk_mipi_tx *mipi_tx);
	void (*mipi_tx_enable_signal)(struct mtk_mipi_tx *mipi_tx);
	void (*mipi_tx_disable_signal)(struct mtk_mipi_tx *mipi_tx);
};

struct mtk_mipi_tx {
	struct udevice *dev;
	void __iomem *regs;
	struct clk ref_clk;
	u64 data_rate;
	u32 mipitx_drive;
	u32 rt_code[5];
	const struct mtk_mipitx_data *driver_data;
};

extern const struct mtk_mipitx_data mt8183_mipitx_data;

#endif /* _PHY_MTK_MIPI_DSI_H */
