/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * MediaTek SMI local arbiter (larb) API
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#ifndef _MTK_SMI_H
#define _MTK_SMI_H

struct udevice;

/**
 * mtk_smi_larb_enable() - ungate an SMI local arbiter
 * @dev: larb device
 *
 * Enable the clocks of the larb so that the DMA master behind it can reach
 * DRAM. Calls are refcounted, so several masters can share one larb.
 *
 * Return: 0 on success, or a negative error code
 */
int mtk_smi_larb_enable(struct udevice *dev);

/**
 * mtk_smi_larb_disable() - gate an SMI local arbiter
 * @dev: larb device
 *
 * Drop one reference taken by mtk_smi_larb_enable(), and disable the larb
 * clocks when it was the last one.
 *
 * Return: 0 on success, -EINVAL if the larb is not enabled, or another
 * negative error code
 */
int mtk_smi_larb_disable(struct udevice *dev);

#endif /* _MTK_SMI_H */
