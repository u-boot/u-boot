/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Masson <jmasson@baylibre.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#ifndef _MTK_DISP_DITHER_H
#define _MTK_DISP_DITHER_H

struct display_timing;
struct udevice;

/**
 * mtk_dither_config() - configure the dither block
 * @dev: dither device
 * @timing: display timing, giving the frame size
 *
 * Set the frame size of the dither block, and dither its 10-bit input down
 * to 8 bits per component.
 */
void mtk_dither_config(struct udevice *dev, const struct display_timing *timing);

/**
 * mtk_dither_start() - start the dither block
 * @dev: dither device
 *
 * Start the dither block.
 */
void mtk_dither_start(struct udevice *dev);

#endif /* _MTK_DISP_DITHER_H */
