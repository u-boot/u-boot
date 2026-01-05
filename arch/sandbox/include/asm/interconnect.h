/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025 Linaro Limited
 */

#ifndef __SANDBOX_INTERCONNECT_H
#define __SANDBOX_INTERCONNECT_H

struct udevice;

int sandbox_interconnect_get_bw(struct udevice *dev, u64 *avg, u64 *peak);
int sandbox_interconnect_test_get(struct udevice *dev, char *name);
int sandbox_interconnect_test_get_index(struct udevice *dev, int index);
int sandbox_interconnect_test_enable(struct udevice *dev);
int sandbox_interconnect_test_disable(struct udevice *dev);
int sandbox_interconnect_test_set_bw(struct udevice *dev, u32 avg_bw, u32 peak_bw);
int sandbox_interconnect_test_put(struct udevice *dev);

#endif
