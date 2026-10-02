/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * MediaTek MT8188 DPI Support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#ifndef _MTK_DPI_H
#define _MTK_DPI_H

struct display_timing;
struct udevice;

/**
 * mtk_dpi_hw_enable() - start the DPI output
 * @dev: DPI device
 */
void mtk_dpi_hw_enable(struct udevice *dev);

/**
 * mtk_dpi_hw_disable() - stop the DPI output
 * @dev: DPI device
 */
void mtk_dpi_hw_disable(struct udevice *dev);

/**
 * mtk_dpi_config() - configure the DPI for a display timing
 * @dev: DPI device
 * @timing: display timing to output, as 8-bit RGB
 */
void mtk_dpi_config(struct udevice *dev, const struct display_timing *timing);

#endif /* _MTK_DPI_H */
