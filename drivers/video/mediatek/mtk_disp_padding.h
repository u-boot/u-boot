/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * MediaTek Video Disp Padding Support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#ifndef _MTK_DISP_PADDING_H
#define _MTK_DISP_PADDING_H

struct udevice;

/**
 * mtk_disp_padding_config() - put a padding component in bypass
 * @dev: padding device
 */
void mtk_disp_padding_config(struct udevice *dev);

#endif /* _MTK_DISP_PADDING_H */
