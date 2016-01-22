/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <backlight.h>

int backlight_enable(struct udevice *dev)
{
	const struct backlight_ops *ops = backlight_get_ops(dev);

	if (!ops->enable)
		return -ENOSYS;

	return ops->enable(dev);
}

UCLASS_DRIVER(backlight) = {
	.id		= UCLASS_PANEL_BACKLIGHT,
	.name		= "backlight",
};
