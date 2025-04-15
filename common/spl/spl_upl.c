// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <alist.h>
#include <bloblist.h>
#include <dm.h>
#include <image.h>
#include <mapmem.h>
#include <serial.h>
#include <spl.h>
#include <upl.h>
#include <video.h>
#include <asm/global_data.h>
#include <dm/read.h>
#include <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

struct upl s_upl;

void upl_set_fit_addr(ulong fit)
{
	struct upl *upl = &s_upl;

	upl->fit = fit;
}

void upl_set_fit_info(ulong fit, int conf_offset, ulong entry_addr)
{
	struct upl *upl = &s_upl;

	upl->fit = fit;
	upl->conf_offset = conf_offset;
	log_debug("upl: add fit %lx conf %x\n", fit, conf_offset);
}

int _upl_add_image(int node, ulong load_addr, ulong size, const char *desc)
{
	struct upl *upl = &s_upl;
	struct upl_image img;

	img.load = load_addr;
	img.size = size;
	img.offset = node;
	img.description = desc;
	if (!alist_add(&upl->image, img))
		return -ENOMEM;
	log_debug("upl: add image %s at %lx size %lx\n", desc, load_addr, size);

	return 0;
}

static int write_serial(struct upl_serial *ser)
{
	struct udevice *dev = gd->cur_serial_dev;
	struct serial_device_info info;
	struct memregion region;
	int ret;

	if (!dev)
		return log_msg_ret("ser", -ENOENT);
	ret = serial_getinfo(dev, &info);
	if (ret)
		return log_msg_ret("inf", ret);

	ser->compatible = ofnode_read_string(dev_ofnode(dev), "compatible");
	ser->clock_frequency = info.clock;
	ser->current_speed = gd->baudrate;
	region.base = info.addr;
	region.size = info.size;
	alist_init_struct(&ser->reg, struct memregion);
	if (!alist_add(&ser->reg, region))
		return -ENOMEM;
	ser->reg_io_shift = info.reg_shift;
	ser->reg_offset = info.reg_offset;
	ser->reg_io_width = info.reg_width;
	ser->virtual_reg = 0;
	ser->access_type = info.addr_space;

	return 0;
}

static int write_graphics(struct upl_graphics *gra)
{
	struct video_uc_plat *plat;
	struct video_priv *priv;
	struct memregion region;
	struct udevice *dev;

	alist_init_struct(&gra->reg, struct memregion);
	uclass_find_first_device(UCLASS_VIDEO, &dev);
	if (!dev || !device_active(dev))
		return log_msg_ret("vid", -ENOENT);

	plat = dev_get_uclass_plat(dev);
	region.base = plat->base;
	region.size = plat->size;
	if (!alist_add(&gra->reg, region))
		return log_msg_ret("reg", -ENOMEM);

	priv = dev_get_uclass_priv(dev);
	gra->width = priv->xsize;
	gra->height = priv->ysize;
	gra->stride = priv->line_length;	/* private field */
	switch (priv->format) {
	case VIDEO_RGBA8888:
	case VIDEO_X8R8G8B8:
		gra->format = UPLGF_ARGB32;
		break;
	case VIDEO_X8B8G8R8:
		gra->format = UPLGF_ABGR32;
		break;
	case VIDEO_X2R10G10B10:
		log_debug("device '%s': VIDEO_X2R10G10B10 not supported\n",
			  dev->name);
		return log_msg_ret("for", -EPROTO);
	case VIDEO_UNKNOWN:
		log_debug("device '%s': Unknown video format\n", dev->name);
		return log_msg_ret("for", -EPROTO);
	}

	return 0;
}

int spl_write_upl_handoff(struct spl_image_info *spl_image)
{
	struct upl *upl = &s_upl;
	struct abuf buf;
	ofnode root;
	void *ptr;
	int ret;

	log_debug("UPL: Writing handoff - image_count=%d\n", upl->image.count);
	upl->addr_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->size_cells = IS_ENABLED(CONFIG_PHYS_64BIT) ? 2 : 1;
	upl->bootmode = UPLBM_DEFAULT;
	ret = write_serial(&upl->serial);
	if (ret)
		return log_msg_ret("ser", ret);
	ret = write_graphics(&upl->graphics);
	if (ret && ret != -ENOENT)
		return log_msg_ret("gra", ret);

	root = ofnode_root();
	ret = upl_write_handoff(upl, root, true);
	if (ret)
		return log_msg_ret("wr", ret);

	ret = oftree_to_fdt(oftree_default(), &buf);
	if (ret)
		return log_msg_ret("fdt", ret);
	log_debug("FDT size %zx\n", abuf_size(&buf));

	ptr = bloblist_add(BLOBLISTT_CONTROL_FDT, abuf_size(&buf), 0);
	if (!ptr)
		return log_msg_ret("blo", -ENOENT);
	memcpy(ptr, abuf_data(&buf), abuf_size(&buf));

	return 0;
}

void spl_upl_init(void)
{
	upl_init(&s_upl);
}
