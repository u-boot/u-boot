/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Masson <jmasson@baylibre.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#ifndef _MTK_DISP_COLOR_H
#define _MTK_DISP_COLOR_H

struct display_timing;
struct udevice;

/**
 * mtk_color_config() - configure the color block
 * @dev: color device
 * @timing: display timing, giving the frame size
 *
 * Set the frame size of the color block.
 */
void mtk_color_config(struct udevice *dev, const struct display_timing *timing);

/**
 * mtk_color_start() - start the color block
 * @dev: color device
 *
 * Start the color block with every color adjustment bypassed.
 */
void mtk_color_start(struct udevice *dev);

#endif /* _MTK_DISP_COLOR_H */
