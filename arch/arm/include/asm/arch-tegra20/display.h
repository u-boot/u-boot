/*
 *  (C) Copyright 2010
 *  NVIDIA Corporation <www.nvidia.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __ASM_ARCH_TEGRA_DISPLAY_H
#define __ASM_ARCH_TEGRA_DISPLAY_H

#include <asm/arch/dc.h>
#include <fdtdec.h>

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

/* Information about the display controller */
struct fdt_disp_config {
	int valid;			/* config is valid */
	int width;			/* width in pixels */
	int height;			/* height in pixels */
	int bpp;			/* number of bits per pixel */

	/*
	 * log2 of number of bpp, in general, unless it bpp is 24 in which
	 * case this field holds 24 also! This is a U-Boot thing.
	 */
	int log2_bpp;
	struct disp_ctlr *disp;		/* Display controller to use */
	fdt_addr_t frame_buffer;	/* Address of frame buffer */
	unsigned pixel_clock;		/* Pixel clock in Hz */
	uint horiz_timing[FDT_LCD_TIMING_COUNT];	/* Horizontal timing */
	uint vert_timing[FDT_LCD_TIMING_COUNT];		/* Vertical timing */
	int panel_node;			/* node offset of panel information */
};

/* Information about the LCD panel */
struct fdt_panel_config {
	int pwm_channel;		/* PWM channel to use for backlight */
	enum lcd_cache_t cache_type;

	struct fdt_gpio_state backlight_en;	/* GPIO for backlight enable */
	struct fdt_gpio_state lvds_shutdown;	/* GPIO for lvds shutdown */
	struct fdt_gpio_state backlight_vdd;	/* GPIO for backlight vdd */
	struct fdt_gpio_state panel_vdd;	/* GPIO for panel vdd */
	/*
	 * Panel required timings
	 * Timing 1: delay between panel_vdd-rise and data-rise
	 * Timing 2: delay between data-rise and backlight_vdd-rise
	 * Timing 3: delay between backlight_vdd and pwm-rise
	 * Timing 4: delay between pwm-rise and backlight_en-rise
	 */
	uint panel_timings[FDT_LCD_TIMINGS];
};

/**
 * Register a new display based on device tree configuration.
 *
 * The frame buffer can be positioned by U-Boot or overriden by the fdt.
 * You should pass in the U-Boot address here, and check the contents of
 * struct fdt_disp_config to see what was actually chosen.
 *
 * @param blob			Device tree blob
 * @param default_lcd_base	Default address of LCD frame buffer
 * @return 0 if ok, -1 on error (unsupported bits per pixel)
 */
int tegra_display_probe(const void *blob, void *default_lcd_base);

/**
 * Return the current display configuration
 *
 * @return pointer to display configuration, or NULL if there is no valid
 * config
 */
struct fdt_disp_config *tegra_display_get_config(void);

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
