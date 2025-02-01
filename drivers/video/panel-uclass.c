// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_PANEL

#include <dm.h>
#include <panel.h>

int panel_enable(struct udevice *dev)
{
	struct panel_ops *ops = panel_get_ops(dev);

	if (!ops->enable)
		return -ENOSYS;

	return ops->enable(dev);
}

/**
 * panel_set_backlight - Set brightness for the panel backlight
 *
 * @dev:	Panel device containing the backlight to update
 * @percent:	Brightness value (0=off, 1=min brightness,
 *		100=full brightness)
 * Return: 0 if OK, -ve on error
 */
int panel_set_backlight(struct udevice *dev, int percent)
{
	struct panel_ops *ops = panel_get_ops(dev);

	if (!ops->set_backlight)
		return -ENOSYS;

	return ops->set_backlight(dev, percent);
}

int panel_get_display_timing(struct udevice *dev,
			     struct display_timing *timings)
{
	struct panel_ops *ops = panel_get_ops(dev);

	if (!ops->get_display_timing)
		return -ENOSYS;

	return ops->get_display_timing(dev, timings);
}

UCLASS_DRIVER(panel) = {
	.id		= UCLASS_PANEL,
	.name		= "panel",
};
