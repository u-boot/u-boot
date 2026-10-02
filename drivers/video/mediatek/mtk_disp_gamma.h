/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Masson <jmasson@baylibre.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#ifndef _MTK_DISP_GAMMA_H
#define _MTK_DISP_GAMMA_H

struct display_timing;
struct udevice;

/**
 * mtk_gamma_config() - configure the gamma block
 * @dev: gamma device
 * @timing: display timing, giving the frame size
 *
 * Set the frame size of the gamma block.
 */
void mtk_gamma_config(struct udevice *dev, const struct display_timing *timing);

/**
 * mtk_gamma_start() - start the gamma block
 * @dev: gamma device
 *
 * Start the gamma block.
 */
void mtk_gamma_start(struct udevice *dev);

#endif /* _MTK_DISP_GAMMA_H */
