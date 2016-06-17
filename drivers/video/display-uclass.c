/*
 * Copyright 2014 Google Inc.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <display.h>
#include <edid.h>
#include <errno.h>

int display_read_edid(struct udevice *dev, u8 *buf, int buf_size)
{
	struct dm_display_ops *ops = display_get_ops(dev);

	if (!ops || !ops->read_edid)
		return -ENOSYS;
	return ops->read_edid(dev, buf, buf_size);
}

int display_enable(struct udevice *dev, int panel_bpp,
			const struct display_timing *timing)
{
	struct dm_display_ops *ops = display_get_ops(dev);

	if (!ops || !ops->enable)
		return -ENOSYS;
	return ops->enable(dev, panel_bpp, timing);
}

int display_read_timing(struct udevice *dev, struct display_timing *timing)
{
	struct dm_display_ops *ops = display_get_ops(dev);
	int panel_bits_per_colour;
	u8 buf[EDID_EXT_SIZE];
	int ret;

	if (!ops || !ops->read_edid)
		return -ENOSYS;
	ret = ops->read_edid(dev, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	return edid_get_timing(buf, ret, timing, &panel_bits_per_colour);
}

UCLASS_DRIVER(display) = {
	.id		= UCLASS_DISPLAY,
	.name		= "display",
	.per_device_platdata_auto_alloc_size	= sizeof(struct display_plat),
};
