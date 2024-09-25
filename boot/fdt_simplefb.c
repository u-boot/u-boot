// SPDX-License-Identifier: GPL-2.0+
/*
 * Simplefb device tree support
 *
 * (C) Copyright 2015
 * Stephen Warren <swarren@wwwdotorg.org>
 */

#include <dm.h>
#include <fdt_support.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <video.h>
#include <spl.h>
#include <bloblist.h>

DECLARE_GLOBAL_DATA_PTR;

static int fdt_simplefb_configure_node(void *blob, int off)
{
	int xsize, ysize;
	int bpix; /* log2 of bits per pixel */
	const char *name;
	ulong fb_base;
	struct video_uc_plat *plat;
	struct video_priv *uc_priv;
	struct udevice *dev;
	int ret;

	if (IS_ENABLED(CONFIG_SPL_VIDEO_HANDOFF) && xpl_phase() > PHASE_SPL) {
		struct video_handoff *ho;

		ho = bloblist_find(BLOBLISTT_U_BOOT_VIDEO, sizeof(*ho));
		if (!ho)
			return log_msg_ret("Missing video bloblist", -ENOENT);

		xsize = ho->xsize;
		ysize = ho->ysize;
		bpix = ho->bpix;
		fb_base = ho->fb;
	} else {
		ret = uclass_first_device_err(UCLASS_VIDEO, &dev);
		if (ret)
			return ret;
		uc_priv = dev_get_uclass_priv(dev);
		plat = dev_get_uclass_plat(dev);
		xsize = uc_priv->xsize;
		ysize = uc_priv->ysize;
		bpix = uc_priv->bpix;
		fb_base = plat->base;
	}

	switch (bpix) {
	case 4: /* VIDEO_BPP16 */
		name = "r5g6b5";
		break;
	case 5: /* VIDEO_BPP32 */
		name = "a8r8g8b8";
		break;
	default:
		return -EINVAL;
	}

	return fdt_setup_simplefb_node(blob, off, fb_base, xsize, ysize,
				       xsize * (1 << bpix) / 8, name);
}

int fdt_simplefb_add_node(void *blob)
{
	static const char compat[] = "simple-framebuffer";
	static const char disabled[] = "disabled";
	int off, ret;

	off = fdt_add_subnode(blob, 0, "framebuffer");
	if (off < 0)
		return -1;

	ret = fdt_setprop(blob, off, "status", disabled, sizeof(disabled));
	if (ret < 0)
		return -1;

	ret = fdt_setprop(blob, off, "compatible", compat, sizeof(compat));
	if (ret < 0)
		return -1;

	return fdt_simplefb_configure_node(blob, off);
}

/**
 * fdt_simplefb_enable_existing_node() - enable simple-framebuffer DT node
 *
 * @blob:	device-tree
 * Return:	0 on success, non-zero otherwise
 */
static int fdt_simplefb_enable_existing_node(void *blob)
{
	int off;

	off = fdt_node_offset_by_compatible(blob, -1, "simple-framebuffer");
	if (off < 0)
		return -1;

	return fdt_simplefb_configure_node(blob, off);
}

int fdt_simplefb_enable_and_mem_rsv(void *blob)
{
	int ret;

	/* nothing to do when video is not active */
	if (!video_is_active())
		return 0;

	ret = fdt_simplefb_enable_existing_node(blob);
	if (ret)
		return ret;

	return fdt_add_fb_mem_rsv(blob);
}
