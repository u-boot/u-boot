/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef _PANEL_H
#define _PANEL_H

#include <video.h>
#include <fdtdec.h>

/* DRM mode flags mapped to U-Boot DISPLAY_FLAGS for direct compatibility */
#define DRM_MODE_FLAG_NHSYNC		DISPLAY_FLAGS_HSYNC_LOW
#define DRM_MODE_FLAG_PHSYNC		DISPLAY_FLAGS_HSYNC_HIGH
#define DRM_MODE_FLAG_NVSYNC		DISPLAY_FLAGS_VSYNC_LOW
#define DRM_MODE_FLAG_PVSYNC		DISPLAY_FLAGS_VSYNC_HIGH
#define DRM_MODE_FLAG_INTERLACE		DISPLAY_FLAGS_INTERLACED
#define DRM_MODE_FLAG_DBLSCAN		DISPLAY_FLAGS_DOUBLESCAN
#define DRM_MODE_FLAG_DBLCLK		DISPLAY_FLAGS_DOUBLECLK

/**
 * struct drm_display_mode - DRM kernel-internal display mode structure
 *			     simplified for U-Boot
 * @hdisplay: horizontal display size
 * @hsync_start: horizontal sync start
 * @hsync_end: horizontal sync end
 * @htotal: horizontal total size
 * @vdisplay: vertical display size
 * @vsync_start: vertical sync start
 * @vsync_end: vertical sync end
 * @vtotal: vertical total size
 *
 * The horizontal and vertical timings are defined per the following diagram.
 *
 * ::
 *
 *
 *               Active                 Front           Sync           Back
 *              Region                 Porch                          Porch
 *     <-----------------------><----------------><-------------><-------------->
 *       //////////////////////|
 *      ////////////////////// |
 *     //////////////////////  |..................               ................
 *                                                _______________
 *     <----- [hv]display ----->
 *     <------------- [hv]sync_start ------------>
 *     <--------------------- [hv]sync_end --------------------->
 *     <-------------------------------- [hv]total ----------------------------->*
 */
struct drm_display_mode {
	unsigned int clock; /* in kHz */

	u16 hdisplay;
	u16 hsync_start;
	u16 hsync_end;
	u16 htotal;
	u16 vdisplay;
	u16 vsync_start;
	u16 vsync_end;
	u16 vtotal;

	u32 flags;
};

struct panel_ops {
	/**
	 * enable_backlight() - Enable the panel backlight
	 *
	 * @dev:	Panel device containing the backlight to enable
	 * @return 0 if OK, -ve on error
	 */
	int (*enable_backlight)(struct udevice *dev);

	/**
	 * set_backlight - Set panel backlight brightness
	 *
	 * @dev:	Panel device containing the backlight to update
	 * @percent:	Brightness value (0 to 100, or BACKLIGHT_... value)
	 * @return 0 if OK, -ve on error
	 */
	int (*set_backlight)(struct udevice *dev, int percent);

	/**
	 * get_timings() - Get display timings from panel.
	 *
	 * @dev:	Panel device containing the display timings
	 * @timing:	Pointer to the timing for storing
	 * @return 0 if OK, -ve on error
	 */
	int (*get_display_timing)(struct udevice *dev,
				  struct display_timing *timing);

	/**
	 * get_modes() - Get display modes from panel
	 *
	 * Returns an array of display modes supported by the panel.
	 * Similar to Linux's drm_panel_funcs->get_modes().
	 *
	 * @dev:	Panel device
	 * @modes:	Pointer to an array of modes
	 * @return number of modes if OK, -ve on error
	 */
	int (*get_modes)(struct udevice *dev,
			 const struct drm_display_mode **modes);
};

#define panel_get_ops(dev)	((struct panel_ops *)(dev)->driver->ops)

/**
 * panel_enable_backlight() - Enable/disable the panel backlight
 *
 * @dev:	Panel device containing the backlight to enable
 * @enable:	true to enable the backlight, false to dis
 * Return: 0 if OK, -ve on error
 */
int panel_enable_backlight(struct udevice *dev);

/**
 * panel_set_backlight - Set brightness for the panel backlight
 *
 * @dev:	Panel device containing the backlight to update
 * @percent:	Brightness value (0 to 100, or BACKLIGHT_... value)
 * Return: 0 if OK, -ve on error
 */
int panel_set_backlight(struct udevice *dev, int percent);

/**
 * panel_get_display_timing() - Get display timings from panel.
 *
 * @dev:	Panel device containing the display timings
 * @timing:	Pointer to the timing for storing
 * Return: 0 if OK, -ve on error
 */
int panel_get_display_timing(struct udevice *dev,
			     struct display_timing *timing);

#endif
