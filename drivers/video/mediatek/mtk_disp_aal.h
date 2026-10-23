/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Masson <jmasson@baylibre.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#ifndef _MTK_DISP_AAL_H
#define _MTK_DISP_AAL_H

struct display_timing;
struct udevice;

/**
 * mtk_aal_config() - configure the aal block
 * @dev: aal device
 * @timing: display timing, giving the frame size
 *
 * Set the input and output frame sizes of the adaptive ambient light block.
 */
void mtk_aal_config(struct udevice *dev, const struct display_timing *timing);

/**
 * mtk_aal_start() - start the aal block
 * @dev: aal device
 *
 * Start the adaptive ambient light block.
 */
void mtk_aal_start(struct udevice *dev);

#endif /* _MTK_DISP_AAL_H */
