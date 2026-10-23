/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Masson <jmasson@baylibre.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#ifndef _MTK_DDP_H
#define _MTK_DDP_H

#include <linux/compiler.h>
#include <linux/types.h>

/**
 * mt8365_ddp_ovl_to_dsi() - route the MT8365 mmsys muxes from the OVL to the DSI
 * @mmsys_base: mmsys config register window
 *
 * Go through the color, ccorr, aal, gamma and dither blocks.
 */
void mt8365_ddp_ovl_to_dsi(void __iomem *mmsys_base);

/**
 * mt8366_ddp_ovl_to_dsi() - route the MT8366 mmsys muxes from the OVL to the DSI
 * @mmsys_base: mmsys config register window
 *
 * Go straight from the rdma to the DSI, through the crossbars.
 */
void mt8366_ddp_ovl_to_dsi(void __iomem *mmsys_base);

/**
 * mt8366_ddp_ovl_to_dvo() - route the MT8366 mmsys muxes from the OVL to the DVO
 * @mmsys_base: mmsys config register window
 *
 * Take the mt8366_ddp_ovl_to_dsi() path up to the output crossbar, which
 * selects the DVO instead of the DSI.
 */
void mt8366_ddp_ovl_to_dvo(void __iomem *mmsys_base);

#endif /* _MTK_DDP_H */
