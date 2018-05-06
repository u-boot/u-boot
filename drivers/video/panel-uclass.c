// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <panel.h>

int panel_enable_backlight(struct udevice *dev)
{
	struct panel_ops *ops = panel_get_ops(dev);

	if (!ops->enable_backlight)
		return -ENOSYS;

	return ops->enable_backlight(dev);
}

UCLASS_DRIVER(panel) = {
	.id		= UCLASS_PANEL,
	.name		= "panel",
};
