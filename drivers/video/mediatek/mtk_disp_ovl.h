/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Masson <jmasson@baylibre.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#ifndef _MTK_DISP_OVL_H
#define _MTK_DISP_OVL_H

struct display_timing;
struct udevice;
struct video_uc_plat;

/**
 * mtk_ovl_config() - configure an overlay
 * @dev: overlay device
 * @timing: display timing, giving the frame size
 *
 * Set the region of interest over an opaque black background, and reset
 * the overlay.
 */
void mtk_ovl_config(struct udevice *dev, const struct display_timing *timing);

/**
 * mtk_ovl_layer() - scan out the framebuffer on layer 0
 * @dev: overlay device
 * @plat: video uclass platform data, holding the framebuffer base address
 * @timing: display timing, giving the frame size
 *
 * Point layer 0 at the XRGB8888 framebuffer, take its larb port out of the
 * IOMMU, and enable the layer.
 *
 * Return: 0 on success, -ERANGE if the framebuffer is above 4 GiB
 */
int mtk_ovl_layer(struct udevice *dev, const struct video_uc_plat *plat,
		  const struct display_timing *timing);

/**
 * mtk_ovl_start() - start an overlay
 * @dev: overlay device
 */
void mtk_ovl_start(struct udevice *dev);

#endif /* _MTK_DISP_OVL_H */
