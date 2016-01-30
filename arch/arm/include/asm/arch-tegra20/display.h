/*
 *  (C) Copyright 2010
 *  NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_TEGRA_DISPLAY_H
#define __ASM_ARCH_TEGRA_DISPLAY_H

#include <asm/arch-tegra/dc.h>
#include <fdtdec.h>
#include <asm/gpio.h>

/* This holds information about a window which can be displayed */
struct disp_ctl_win {
	enum win_color_depth_id fmt;	/* Color depth/format */
	unsigned	bpp;		/* Bits per pixel */
	phys_addr_t	phys_addr;	/* Physical address in memory */
	unsigned	x;		/* Horizontal address offset (bytes) */
	unsigned	y;		/* Veritical address offset (bytes) */
	unsigned	w;		/* Width of source window */
	unsigned	h;		/* Height of source window */
	unsigned	stride;		/* Number of bytes per line */
	unsigned	out_x;		/* Left edge of output window (col) */
	unsigned	out_y;		/* Top edge of output window (row) */
	unsigned	out_w;		/* Width of output window in pixels */
	unsigned	out_h;		/* Height of output window in pixels */
};

#define FDT_LCD_TIMINGS	4

enum {
	FDT_LCD_TIMING_REF_TO_SYNC,
	FDT_LCD_TIMING_SYNC_WIDTH,
	FDT_LCD_TIMING_BACK_PORCH,
	FDT_LCD_TIMING_FRONT_PORCH,

	FDT_LCD_TIMING_COUNT,
};

enum lcd_cache_t {
	FDT_LCD_CACHE_OFF		= 0,
	FDT_LCD_CACHE_WRITE_THROUGH	= 1 << 0,
	FDT_LCD_CACHE_WRITE_BACK	= 1 << 1,
	FDT_LCD_CACHE_FLUSH		= 1 << 2,
	FDT_LCD_CACHE_WRITE_BACK_FLUSH	= FDT_LCD_CACHE_WRITE_BACK |
						FDT_LCD_CACHE_FLUSH,
};

#endif /*__ASM_ARCH_TEGRA_DISPLAY_H*/
