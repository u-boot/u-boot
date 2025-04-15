// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017 Rob Clark
 */

#include <dm.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <log.h>
#include <video.h>
#include <asm/global_data.h>

static int simple_video_probe(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	ofnode node = dev_ofnode(dev);
	const char *format;
	int ret;
	fdt_addr_t base;
	fdt_size_t size;
	u32 width, height, rot;

	base = dev_read_addr_size(dev, &size);
	if (base == FDT_ADDR_T_NONE) {
		debug("%s: Failed to decode memory region\n", __func__);
		return -EINVAL;
	}

	debug("%s: base=%llx, size=%llu\n",
	      __func__, (unsigned long long)base, (unsigned long long)size);

	/*
	 * TODO is there some way to reserve the framebuffer
	 * region so it isn't clobbered?
	 */
	plat->base = base;
	plat->size = size;

	video_set_flush_dcache(dev, true);

	debug("%s: Query resolution...\n", __func__);

	ret = ofnode_read_u32(node, "width", &width);
	ret = ret ?: ofnode_read_u32(node, "height", &height);
	if (ret || !width || !height) {
		log_err("%s: invalid width or height: %d\n", __func__, ret);
		return ret ?: -EINVAL;
	}
	ofnode_read_u32(node, "rot", &rot);
	uc_priv->rot = rot;
	uc_priv->xsize = width;
	uc_priv->ysize = height;

	format = ofnode_read_string(node, "format");
	debug("%s: %dx%d@%s\n", __func__, uc_priv->xsize, uc_priv->ysize, format);

	if (!format) {
		log_err("%s: please add required property \"format\"\n", __func__);
		return -EINVAL;
	}

	if (strcmp(format, "r5g6b5") == 0) {
		uc_priv->bpix = VIDEO_BPP16;
	} else if (strcmp(format, "a8b8g8r8") == 0 ||
		   strcmp(format, "x8b8g8r8") == 0) {
		uc_priv->bpix = VIDEO_BPP32;
		uc_priv->format = VIDEO_X8B8G8R8;
	} else if (strcmp(format, "a8r8g8b8") == 0 ||
		   strcmp(format, "x8r8g8b8") == 0) {
		uc_priv->bpix = VIDEO_BPP32;
		uc_priv->format = VIDEO_X8R8G8B8;
	} else if (strcmp(format, "a2r10g10b10") == 0 ||
		   strcmp(format, "x2r10g10b10") == 0) {
		uc_priv->bpix = VIDEO_BPP32;
		uc_priv->format = VIDEO_X2R10G10B10;
	} else {
		log_err("%s: invalid format: %s\n", __func__, format);
		return -EINVAL;
	}

	return 0;
}

static const struct udevice_id simple_video_ids[] = {
	{ .compatible = "simple-framebuffer" },
	{ }
};

U_BOOT_DRIVER(simple_video) = {
	.name	= "simple_video",
	.id	= UCLASS_VIDEO,
	.of_match = simple_video_ids,
	.probe	= simple_video_probe,
};
