/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef _PANEL_H
#define _PANEL_H

struct panel_priv {
	bool enabled;
};

struct panel_ops {
	/**
	 * enable() - Enable the panel
	 *
	 * @dev:	Panel device to enable
	 * @return 0 if OK, -ve on error
	 */
	int (*enable)(struct udevice *dev);

	/**
	 * disable() - Disable the panel
	 *
	 * @dev:	Panel device to disable
	 * @return 0 if OK, -ve on error
	 */
	int (*disable)(struct udevice *dev);

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
	 * @tim:	Place to put timings
	 * @return 0 if OK, -ve on error
	 */
	int (*get_display_timing)(struct udevice *dev,
				  struct display_timing *timing);

	/**
	 * set_rotation() - Set the panel rotation
	 *
	 * @dev:	Panel device
	 * @rotation: rotation
	 * @return 0 if OK, -ve on error
	 */
	int (*set_rotation)(struct udevice *dev, int rotation);

	/**
	 * get_rotation() - Get the panel rotation
	 *
	 * @dev:	Panel device
	 * @return rotation
	 */
	int (*get_rotation)(struct udevice *dev);
};

#define panel_get_ops(dev)	((struct panel_ops *)(dev)->driver->ops)

/**
 * panel_enable() - Enable the panel
 *
 * @dev:	Panel device to enable
 * Return: 0 if OK, -ve on error
 */
int panel_enable(struct udevice *dev);

/**
 * panel_disable() - Disable the panel
 *
 * @dev:	Panel device to enable
 * Return: 0 if OK, -ve on error
 */
int panel_disable(struct udevice *dev);

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
 * Return: 0 if OK, -ve on error
 */
int panel_get_display_timing(struct udevice *dev,
			     struct display_timing *timing);


/**
 * panel_set_rotation() - Set the panel rotation
 *
 * @dev:	Panel device
 * @rotation: rotation
 * Return: 0 if OK, -ve on error
 */
int panel_set_rotation(struct udevice *dev, int rotation);

/**
 * panel_get_rotation() - Get the panel rotation
 *
 * @dev:	Panel device
 * Return: rotation
 */
int panel_get_rotation(struct udevice *dev);
#endif
