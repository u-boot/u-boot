/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Masson <jmasson@baylibre.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#ifndef _MTK_DISP_RDMA_H
#define _MTK_DISP_RDMA_H

struct display_timing;
struct udevice;

/**
 * mtk_rdma_config() - configure an rdma
 * @dev: rdma device
 * @timing: display timing, giving the frame size
 *
 * Set the frame size and the FIFO thresholds of the rdma, which passes the
 * overlay output through without reading DRAM itself.
 */
void mtk_rdma_config(struct udevice *dev, const struct display_timing *timing);

/**
 * mtk_rdma_start() - start an rdma
 * @dev: rdma device
 */
void mtk_rdma_start(struct udevice *dev);

#endif /* _MTK_DISP_RDMA_H */
