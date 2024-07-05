/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _TEGRA_DC_H
#define _TEGRA_DC_H

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

/* arch-tegra/dc exists only because T124 uses it */
#include <asm/arch-tegra/dc.h>

#define TEGRA_DSI_A		"dsi@54300000"
#define TEGRA_DSI_B		"dsi@54400000"

struct tegra_dc_plat {
	struct udevice *dev;		/* Display controller device */
	struct dc_ctlr *dc;		/* Display controller regmap */
	int pipe;			/* DC number: 0 for A, 1 for B */
	ulong scdiv;			/* Shift clock divider */
};

/* This holds information about a window which can be displayed */
struct disp_ctl_win {
	enum win_color_depth_id fmt;	/* Color depth/format */
	unsigned int bpp;		/* Bits per pixel */
	phys_addr_t phys_addr;		/* Physical address in memory */
	unsigned int x;			/* Horizontal address offset (bytes) */
	unsigned int y;			/* Veritical address offset (bytes) */
	unsigned int w;			/* Width of source window */
	unsigned int h;			/* Height of source window */
	unsigned int stride;		/* Number of bytes per line */
	unsigned int out_x;		/* Left edge of output window (col) */
	unsigned int out_y;		/* Top edge of output window (row) */
	unsigned int out_w;		/* Width of output window in pixels */
	unsigned int out_h;		/* Height of output window in pixels */
};

#endif /* _TEGRA_DC_H */
