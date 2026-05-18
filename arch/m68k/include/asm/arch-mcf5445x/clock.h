/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ColdFire clock support
 *
 * Copyright 2026 Kernelspace.
 * Angelo Dureghello <angelo@kernel-space.org>
 */

#ifndef __CLOCK_H
#define __CLOCK_H

/* Stub to use fsl/nxp drivers. */
enum mxc_clock {
	MXC_ESDHC_CLK,
};

int mxc_get_clock(enum mxc_clock clk);

#endif /* __CLOCK_H */
