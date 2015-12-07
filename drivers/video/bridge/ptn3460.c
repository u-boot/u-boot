/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <video_bridge.h>

static int ptn3460_attach(struct udevice *dev)
{
	int ret;

	debug("%s: %s\n", __func__, dev->name);
	ret = video_bridge_set_active(dev, true);
	if (ret)
		return ret;

	return 0;
}

struct video_bridge_ops ptn3460_ops = {
	.attach = ptn3460_attach,
};

static const struct udevice_id ptn3460_ids[] = {
	{ .compatible = "nxp,ptn3460", },
	{ }
};

U_BOOT_DRIVER(parade_ptn3460) = {
	.name	= "nmp_ptn3460",
	.id	= UCLASS_VIDEO_BRIDGE,
	.of_match = ptn3460_ids,
	.ops	= &ptn3460_ops,
};
