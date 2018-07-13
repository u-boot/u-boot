/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef _PANEL_H
#define _PANEL_H

struct panel_ops {
	/**
	 * enable_backlight() - Enable the panel backlight
	 *
	 * @dev:	Panel device containing the backlight to enable
	 * @return 0 if OK, -ve on error
	 */
	int (*enable_backlight)(struct udevice *dev);
	/**
	 * get_timings() - Get display timings from panel.
	 *
	 * @dev:	Panel device containing the display timings
	 * @tim:	Place to put timings
	 * @return 0 if OK, -ve on error
	 */
	int (*get_display_timing)(struct udevice *dev,
				  struct display_timing *timing);
};

#define panel_get_ops(dev)	((struct panel_ops *)(dev)->driver->ops)

/**
 * panel_enable_backlight() - Enable the panel backlight
 *
 * @dev:	Panel device containing the backlight to enable
 * @return 0 if OK, -ve on error
 */
int panel_enable_backlight(struct udevice *dev);

/**
 * panel_get_display_timing() - Get display timings from panel.
 *
 * @dev:	Panel device containing the display timings
 * @return 0 if OK, -ve on error
 */
int panel_get_display_timing(struct udevice *dev,
			     struct display_timing *timing);

#endif
