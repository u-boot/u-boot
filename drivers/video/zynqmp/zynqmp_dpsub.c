// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 Xilinx Inc.
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <errno.h>
#include <video.h>
#include <dm/device_compat.h>

#define WIDTH	640
#define HEIGHT	480

/**
 * struct zynqmp_dpsub_priv - Private structure
 * @dev: Device uclass for video_ops
 */
struct zynqmp_dpsub_priv {
	struct udevice *dev;
};

static int zynqmp_dpsub_probe(struct udevice *dev)
{
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct zynqmp_dpsub_priv *priv = dev_get_priv(dev);

	uc_priv->bpix = VIDEO_BPP16;
	uc_priv->xsize = WIDTH;
	uc_priv->ysize = HEIGHT;
	uc_priv->rot = 0;

	priv->dev = dev;

	/* Only placeholder for power domain driver */
	return 0;
}

static int zynqmp_dpsub_bind(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->size = WIDTH * HEIGHT * 16;

	return 0;
}

static const struct video_ops zynqmp_dpsub_ops = {
};

static const struct udevice_id zynqmp_dpsub_ids[] = {
	{ .compatible = "xlnx,zynqmp-dpsub-1.7" },
	{ }
};

U_BOOT_DRIVER(zynqmp_dpsub_video) = {
	.name = "zynqmp_dpsub_video",
	.id = UCLASS_VIDEO,
	.of_match = zynqmp_dpsub_ids,
	.ops = &zynqmp_dpsub_ops,
	.plat_auto = sizeof(struct video_uc_plat),
	.bind = zynqmp_dpsub_bind,
	.probe = zynqmp_dpsub_probe,
	.priv_auto = sizeof(struct zynqmp_dpsub_priv),
};
