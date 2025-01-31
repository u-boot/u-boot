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
	struct panel_priv *priv = dev_get_uclass_priv(dev);
	int err;

	if (!ops->enable)
		return -ENOSYS;

	if (priv->enabled)
		return 0;

	err = ops->enable(dev);
	if (err)
		return err;

	priv->enabled = true;

	return 0;
}

int panel_disable(struct udevice *dev)
{
	struct panel_ops *ops = panel_get_ops(dev);
	struct panel_priv *priv = dev_get_uclass_priv(dev);
	int err;

	if (!ops->disable)
		return -ENOSYS;

	if (!priv->enabled)
		return 0;

	err = ops->disable(dev);
	if (err)
		return err;

	priv->enabled = false;

	return 0;
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

int panel_set_rotation(struct udevice *dev, int rotation)
{
	struct panel_ops *ops = panel_get_ops(dev);

	if (!ops->set_rotation)
		return -ENOSYS;

	return ops->set_rotation(dev, rotation);
}

int panel_get_rotation(struct udevice *dev)
{
	struct panel_ops *ops = panel_get_ops(dev);

	if (!ops->get_rotation)
		return -ENOSYS;

	return ops->get_rotation(dev);
}

UCLASS_DRIVER(panel) = {
	.id = UCLASS_PANEL,
	.name = "panel",
	.per_device_auto = sizeof(struct panel_priv),
};
