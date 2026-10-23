/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Masson <jmasson@baylibre.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#ifndef _MTK_DISP_CCORR_H
#define _MTK_DISP_CCORR_H

struct display_timing;
struct udevice;

/**
 * mtk_ccorr_config() - configure the ccorr block
 * @dev: ccorr device
 * @timing: display timing, giving the frame size
 *
 * Set the frame size of the color correction block.
 */
void mtk_ccorr_config(struct udevice *dev, const struct display_timing *timing);

/**
 * mtk_ccorr_start() - start the ccorr block
 * @dev: ccorr device
 *
 * Start the color correction block.
 */
void mtk_ccorr_start(struct udevice *dev);

#endif /* _MTK_DISP_CCORR_H */
