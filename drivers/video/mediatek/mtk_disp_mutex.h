/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#ifndef _MTK_DISP_MUTEX_H
#define _MTK_DISP_MUTEX_H

struct udevice;

/**
 * mt8188_mutex_config_hdmi() - set up the mutex of the MT8188 HDMI pipeline
 * @dev: vdosys1 mutex device
 *
 * Put all the components of the RDMA -> DPI1 pipeline under one mutex
 * instance, and start and end their frames on the DPI1 ones.
 */
void mt8188_mutex_config_hdmi(struct udevice *dev);

#endif /* _MTK_DISP_MUTEX_H */
