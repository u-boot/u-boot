/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
};

#define panel_get_ops(dev)	((struct panel_ops *)(dev)->driver->ops)

/**
 * panel_enable_backlight() - Enable the panel backlight
 *
 * @dev:	Panel device containing the backlight to enable
 * @return 0 if OK, -ve on error
 */
int panel_enable_backlight(struct udevice *dev);

#endif
