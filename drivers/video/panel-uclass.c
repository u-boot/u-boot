// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_PANEL

#include <dm.h>
#include <panel.h>

int panel_enable_backlight(struct udevice *dev)
{
	struct panel_ops *ops = panel_get_ops(dev);

	if (!ops->enable_backlight)
		return -ENOSYS;

	return ops->enable_backlight(dev);
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

static void drm_mode_to_display_timing(const struct drm_display_mode *mode,
				       struct display_timing *timing)
{
	timing->pixelclock.typ = mode->clock * 1000; /* kHz to Hz */
	timing->hactive.typ = mode->hdisplay;
	timing->hfront_porch.typ = mode->hsync_start - mode->hdisplay;
	timing->hsync_len.typ = mode->hsync_end - mode->hsync_start;
	timing->hback_porch.typ = mode->htotal - mode->hsync_end;
	timing->vactive.typ = mode->vdisplay;
	timing->vfront_porch.typ = mode->vsync_start - mode->vdisplay;
	timing->vsync_len.typ = mode->vsync_end - mode->vsync_start;
	timing->vback_porch.typ = mode->vtotal - mode->vsync_end;

	/* DRM_MODE_FLAG_* defines are already mapped to u-boot DISPLAY_FLAGS */
	timing->flags = mode->flags;
}

int panel_get_display_timing(struct udevice *dev,
			     struct display_timing *timings)
{
	struct panel_ops *ops = panel_get_ops(dev);
	const struct drm_display_mode *modes;
	int ret = -ENOSYS;

	if (ops->get_display_timing) {
		ret = ops->get_display_timing(dev, timings);
		if (ret != -ENODEV)
			return ret;
	}

	if (!ops->get_modes)
		return ret;

	ret = ops->get_modes(dev, &modes);
	if (ret < 0)
		return ret;
	else if (ret == 0)
		return -ENODEV;

	drm_mode_to_display_timing(&modes[0], timings);
	return 0;
}

UCLASS_DRIVER(panel) = {
	.id		= UCLASS_PANEL,
	.name		= "panel",
};
