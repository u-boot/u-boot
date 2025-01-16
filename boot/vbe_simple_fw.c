// SPDX-License-Identifier: GPL-2.0
/*
 * Verified Boot for Embedded (VBE) loading firmware phases
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY LOGC_BOOT

#include <bloblist.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <bootstage.h>
#include <dm.h>
#include <image.h>
#include <log.h>
#include <mapmem.h>
#include <memalign.h>
#include <mmc.h>
#include <spl.h>
#include <vbe.h>
#include <dm/device-internal.h>
#include "vbe_simple.h"

/**
 * vbe_simple_read_bootflow_fw() - Create a bootflow for firmware
 *
 * Locates and loads the firmware image (FIT) needed for the next phase. The FIT
 * should ideally use external data, to reduce the amount of it that needs to be
 * read.
 *
 * @bdev: bootdev device containing the firmwre
 * @meth: VBE simple bootmeth
 * @blow: Place to put the created bootflow, on success
 * @return 0 if OK, -ve on error
 */
int vbe_simple_read_bootflow_fw(struct udevice *dev, struct bootflow *bflow)
{
	struct udevice *media = dev_get_parent(bflow->dev);
	struct udevice *meth = bflow->method;
	struct simple_priv *priv = dev_get_priv(meth);
	ulong len, load_addr;
	struct udevice *blk;
	int ret;

	log_debug("media=%s\n", media->name);
	ret = blk_get_from_parent(media, &blk);
	if (ret)
		return log_msg_ret("med", ret);
	log_debug("blk=%s\n", blk->name);

	ret = vbe_read_fit(blk, priv->area_start + priv->skip_offset,
			   priv->area_size, NULL, &load_addr, &len,
			   &bflow->name);

	/* set up the bootflow with the info we obtained */
	bflow->blk = blk;
	bflow->buf = map_sysmem(load_addr, len);
	bflow->size = len;

	return 0;
}

static int simple_load_from_image(struct spl_image_info *spl_image,
				  struct spl_boot_device *bootdev)
{
	struct udevice *meth, *bdev;
	struct simple_priv *priv;
	struct bootflow bflow;
	struct vbe_handoff *handoff;
	int ret;

	if (xpl_phase() != PHASE_VPL && xpl_phase() != PHASE_SPL)
		return -ENOENT;

	ret = bloblist_ensure_size(BLOBLISTT_VBE, sizeof(struct vbe_handoff),
				   0, (void **)&handoff);
	if (ret)
		return log_msg_ret("ro", ret);

	vbe_find_first_device(&meth);
	if (!meth)
		return log_msg_ret("vd", -ENODEV);
	log_debug("vbe dev %s\n", meth->name);
	ret = device_probe(meth);
	if (ret)
		return log_msg_ret("probe", ret);

	priv = dev_get_priv(meth);
	log_debug("simple %s\n", priv->storage);
	ret = bootdev_find_by_label(priv->storage, &bdev, NULL);
	if (ret)
		return log_msg_ret("bd", ret);
	log_debug("bootdev %s\n", bdev->name);

	bootflow_init(&bflow, bdev, meth);
	ret = bootmeth_read_bootflow(meth, &bflow);
	log_debug("\nfw ret=%d\n", ret);
	if (ret)
		return log_msg_ret("rd", ret);

	/* jump to the image */
	spl_image->flags = SPL_SANDBOXF_ARG_IS_BUF;
	spl_image->arg = bflow.buf;
	spl_image->size = bflow.size;
	log_debug("Image: %s at %p size %x\n", bflow.name, bflow.buf,
		  bflow.size);

	/* this is not used from now on, so free it */
	bootflow_free(&bflow);

	/* Record that VBE was used in this phase */
	handoff->phases |= 1 << xpl_phase();

	return 0;
}
SPL_LOAD_IMAGE_METHOD("vbe_simple", 5, BOOT_DEVICE_VBE,
		      simple_load_from_image);
