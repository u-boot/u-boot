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

/**
 * Perform the next stage of the LCD init if it is time to do so.
 *
 * LCD init can be time-consuming because of the number of delays we need
 * while waiting for the backlight power supply, etc. This function can
 * be called at various times during U-Boot operation to advance the
 * initialization of the LCD to the next stage if sufficient time has
 * passed since the last stage. It keeps track of what stage it is up to
 * and the time that it is permitted to move to the next stage.
 *
 * The final call should have wait=1 to complete the init.
 *
 * @param blob	fdt blob containing LCD information
 * @param wait	1 to wait until all init is complete, and then return
 *		0 to return immediately, potentially doing nothing if it is
 *		not yet time for the next init.
 */
int tegra_lcd_check_next_stage(const void *blob, int wait);

/**
 * Set up the maximum LCD size so we can size the frame buffer.
 *
 * @param blob	fdt blob containing LCD information
 */
void tegra_lcd_early_init(const void *blob);

#endif /*__ASM_ARCH_TEGRA_DISPLAY_H*/
