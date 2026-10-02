/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * MediaTek Video MDP RDMA Support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#ifndef _MTK_MDP_RDMA_H
#define _MTK_MDP_RDMA_H

#include <linux/types.h>

struct udevice;
struct video_uc_plat;
struct display_timing;

/**
 * mtk_mdp_rdma_config() - configure an RDMA to read one half of the frame
 * @dev: RDMA device
 * @plat: video uclass platform data, holding the framebuffer base address
 * @timing: display timing, giving the frame size
 * @pitch: framebuffer line length, in bytes
 * @apply_offset: read the right half of the frame instead of the left one
 *
 * Each of the two RDMAs of the pipeline reads one half of the XRGB8888
 * framebuffer and feeds it to the mixer.
 *
 * Return: 0 on success, -EINVAL if the framebuffer is above 4 GiB or its
 * pitch is too large
 */
int mtk_mdp_rdma_config(struct udevice *dev, struct video_uc_plat *plat,
			const struct display_timing *timing,
			unsigned int pitch, bool apply_offset);

#endif /* _MTK_MDP_RDMA_H */
