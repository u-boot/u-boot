/*
 * Copyright 2014 Google Inc.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <displayport.h>
#include <errno.h>

int display_port_read_edid(struct udevice *dev, u8 *buf, int buf_size)
{
	struct dm_display_port_ops *ops = display_port_get_ops(dev);

	if (!ops || !ops->read_edid)
		return -ENOSYS;
	return ops->read_edid(dev, buf, buf_size);
}

int display_port_enable(struct udevice *dev, int panel_bpp,
			const struct display_timing *timing)
{
	struct dm_display_port_ops *ops = display_port_get_ops(dev);

	if (!ops || !ops->enable)
		return -ENOSYS;
	return ops->enable(dev, panel_bpp, timing);
}

UCLASS_DRIVER(display_port) = {
	.id		= UCLASS_DISPLAY_PORT,
	.name		= "display_port",
};
