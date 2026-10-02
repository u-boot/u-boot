/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * MediaTek Video Disp Merge Support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#ifndef _MTK_DISP_MERGE_H
#define _MTK_DISP_MERGE_H

#include <linux/types.h>

#include "mtk_disp_comp.h"

/**
 * struct mtk_disp_merge_priv - private data of a merge component
 * @comp: common component state, must be the first member: the shared
 *	  component helpers cast the private data of every pipeline component
 *	  to struct mtk_disp_comp_priv
 * @fifo_en: buffer the input through the merge SRAMs, from the
 *	     "mediatek,merge-fifo-en" property
 * @mute_support: the merge has a mute control, from the
 *		  "mediatek,merge-mute" property
 */
struct mtk_disp_merge_priv {
	struct mtk_disp_comp_priv comp;
	bool fifo_en;
	bool mute_support;
};

/**
 * mtk_disp_merge_config() - configure a merge component
 * @dev: merge device
 * @l_w: width of the left input
 * @r_w: width of the right input, or 0 when there is none
 * @h: height of both inputs
 *
 * With a right input the two inputs are merged side by side, otherwise the
 * single input is only buffered through the merge SRAMs.
 *
 * A single input with no fifo just goes straight through, one pixel in and
 * two out. The HDMI pipeline never takes that path (merge3 has a right input
 * and merge5 declares mediatek,merge-fifo-en), so it is untested and only
 * here for a future single-pixel user.
 */
void mtk_disp_merge_config(struct udevice *dev, u16 l_w, u16 r_w, u16 h);

#endif /* _MTK_DISP_MERGE_H */
