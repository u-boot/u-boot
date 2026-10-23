/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * MediaTek MT8189 Digital Video Output (DVO) support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: David Lechner <dlechner@baylibre.com>
 */

#ifndef _MTK_DVO_H
#define _MTK_DVO_H

struct display_timing;
struct udevice;

/**
 * mtk_dvo_config() - program the DVO for a display mode
 * @dev: DVO device
 * @timing: display timing to output
 *
 * Retune the pixel clock for the mode, then program the timing generator
 * and the display buffer watermarks. The output is left disabled.
 *
 * Return: 0 on success, a negative error code otherwise
 */
int mtk_dvo_config(struct udevice *dev, const struct display_timing *timing);

/**
 * mtk_dvo_hw_enable() - start the DVO output
 * @dev: DVO device
 */
void mtk_dvo_hw_enable(struct udevice *dev);

#endif /* _MTK_DVO_H */
