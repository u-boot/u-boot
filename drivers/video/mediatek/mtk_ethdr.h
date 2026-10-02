/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * MediaTek Video ETHDR support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#ifndef _MTK_ETHDR_H
#define _MTK_ETHDR_H

#include <linux/types.h>

#include "mtk_disp_comp.h"

/**
 * struct mtk_ethdr_priv - private data of the ethdr component
 * @comp: common component state, holding the "mixer" register window. It
 *	  must be the first member: the shared component helpers cast the
 *	  private data of every pipeline component to struct mtk_disp_comp_priv
 * @vdo_be: HDR video back-end ("vdo_be") register window
 */
struct mtk_ethdr_priv {
	struct mtk_disp_comp_priv comp;
	void __iomem *vdo_be;
};

/**
 * mtk_ethdr_config() - configure the ethdr mixer
 * @dev: ethdr device
 * @width: width of the frame
 * @height: height of the frame
 *
 * Put the HDR video back-end in bypass and set up the mixer to output its
 * layer 2 alone, over a black background.
 */
void mtk_ethdr_config(struct udevice *dev, u16 width, u16 height);

#endif /* _MTK_ETHDR_H */
