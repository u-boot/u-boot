// SPDX-License-Identifier: GPL-2.0
/*
 * Verified Boot for Embedded (VBE) loading firmware phases
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY LOGC_BOOT

#include <binman_sym.h>
#include <bloblist.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <bootstage.h>
#include <display_options.h>
#include <dm.h>
#include <image.h>
#include <log.h>
#include <mapmem.h>
#include <memalign.h>
#include <mmc.h>
#include <spl.h>
#include <vbe.h>
#include <dm/device-internal.h>
#include "vbe_abrec.h"
#include "vbe_common.h"

binman_sym_declare(ulong, spl_a, image_pos);
binman_sym_declare(ulong, spl_b, image_pos);
binman_sym_declare(ulong, spl_recovery, image_pos);

binman_sym_declare(ulong, spl_a, size);
binman_sym_declare(ulong, spl_b, size);
binman_sym_declare(ulong, spl_recovery, size);

binman_sym_declare(ulong, u_boot_a, image_pos);
binman_sym_declare(ulong, u_boot_b, image_pos);
binman_sym_declare(ulong, u_boot_recovery, image_pos);

binman_sym_declare(ulong, u_boot_a, size);
binman_sym_declare(ulong, u_boot_b, size);
binman_sym_declare(ulong, u_boot_recovery, size);

binman_sym_declare(ulong, vpl, image_pos);
binman_sym_declare(ulong, vpl, size);

static const char *const pick_names[] = {"A", "B", "Recovery"};

/**
 * abrec_read_bootflow_fw() - Create a bootflow for firmware
 *
 * Locates and loads the firmware image (FIT) needed for the next phase. The FIT
 * should ideally use external data, to reduce the amount of it that needs to be
 * read.
 *
 * @bdev: bootdev device containing the firmwre
 * @meth: VBE abrec bootmeth
 * @blow: Place to put the created bootflow, on success
 * @return 0 if OK, -ve on error
 */
int abrec_read_bootflow_fw(struct udevice *dev, struct bootflow *bflow)
{
	struct udevice *media = dev_get_parent(bflow->dev);
	struct udevice *meth = bflow->method;
	struct abrec_priv *priv = dev_get_priv(meth);
	ulong len, load_addr;
	struct udevice *blk;
	int ret;

	log_debug("media=%s\n", media->name);
	ret = blk_get_from_parent(media, &blk);
	if (ret)
		return log_msg_ret("med", ret);

	ret = vbe_read_fit(blk, priv->area_start + priv->skip_offset,
			   priv->area_size, NULL, &load_addr, &len, &bflow->name);
	if (ret)
		return log_msg_ret("vbe", ret);

	/* set up the bootflow with the info we obtained */
	bflow->blk = blk;
	bflow->buf = map_sysmem(load_addr, len);
	bflow->size = len;

	return 0;
}

static int abrec_run_vpl(struct udevice *blk, struct spl_image_info *image,
			 struct vbe_handoff *handoff)
{
	uint flags, tries, prev_result;
	struct abrec_priv priv;
	struct abrec_state state;
	enum vbe_pick_t pick;
	uint try_count;
	ulong offset, size;
	ulong ub_offset, ub_size;
	ofnode node;
	int ret;

	node = vbe_get_node();
	if (!ofnode_valid(node))
		return log_msg_ret("nod", -EINVAL);

	ret = abrec_read_priv(node, &priv);
	if (ret)
		return log_msg_ret("pri", ret);

	ret = abrec_read_nvdata(&priv, blk, &state);
	if (ret)
		return log_msg_ret("sta", ret);

	prev_result = state.try_result;
	try_count = state.try_count;

	if (state.recovery) {
		pick = VBEP_RECOVERY;

	/* if we are trying B but ran out of tries, use A */
	} else if ((prev_result == VBETR_TRYING) && !tries) {
		pick = VBEP_A;
		state.try_result = VBETR_BAD;

	/* if requested, try B */
	} else if (flags & VBEF_TRY_B) {
		pick = VBEP_B;

		/* decrement the try count if not already zero */
		if (try_count)
			try_count--;
		state.try_result = VBETR_TRYING;
	} else {
		pick = VBEP_A;
	}
	state.try_count = try_count;

	switch (pick) {
	case VBEP_A:
		offset = binman_sym(ulong, spl_a, image_pos);
		size = binman_sym(ulong, spl_a, size);
		ub_offset = binman_sym(ulong, u_boot_a, image_pos);
		ub_size = binman_sym(ulong, u_boot_a, size);
		break;
	case VBEP_B:
		offset = binman_sym(ulong, spl_b, image_pos);
		size = binman_sym(ulong, spl_b, size);
		ub_offset = binman_sym(ulong, u_boot_b, image_pos);
		ub_size = binman_sym(ulong, u_boot_b, size);
		break;
	case VBEP_RECOVERY:
		offset = binman_sym(ulong, spl_recovery, image_pos);
		size = binman_sym(ulong, spl_recovery, size);
		ub_offset = binman_sym(ulong, u_boot_recovery, image_pos);
		ub_size = binman_sym(ulong, u_boot_recovery, size);
		break;
	}
	log_debug("pick=%d, offset=%lx size=%lx\n", pick, offset, size);
	log_info("VBE: Firmware pick %s at %lx\n", pick_names[pick], offset);

	ret = vbe_read_fit(blk, offset, size, image, NULL, NULL, NULL);
	if (ret)
		return log_msg_ret("vbe", ret);
	handoff->offset = ub_offset;
	handoff->size = ub_size;
	handoff->pick = pick;
	image->load_addr = spl_get_image_text_base();
	image->entry_point = image->load_addr;

	return 0;
}

static int abrec_run_spl(struct udevice *blk, struct spl_image_info *image,
			 struct vbe_handoff *handoff)
{
	int ret;

	log_info("VBE: Firmware pick %s at %lx\n", pick_names[handoff->pick],
		 handoff->offset);
	ret = vbe_read_fit(blk, handoff->offset, handoff->size, image, NULL,
			   NULL, NULL);
	if (ret)
		return log_msg_ret("vbe", ret);
	image->load_addr = spl_get_image_text_base();
	image->entry_point = image->load_addr;

	return 0;
}

static int abrec_load_from_image(struct spl_image_info *image,
				 struct spl_boot_device *bootdev)
{
	struct vbe_handoff *handoff;
	int ret;

	printf("load: %s\n", ofnode_read_string(ofnode_root(), "model"));
	if (xpl_phase() != PHASE_VPL && xpl_phase() != PHASE_SPL &&
	    xpl_phase() != PHASE_TPL)
		return -ENOENT;

	ret = bloblist_ensure_size(BLOBLISTT_VBE, sizeof(struct vbe_handoff),
				   0, (void **)&handoff);
	if (ret)
		return log_msg_ret("ro", ret);

	if (USE_BOOTMETH) {
		struct udevice *meth, *bdev;
		struct abrec_priv *priv;
		struct bootflow bflow;

		vbe_find_first_device(&meth);
		if (!meth)
			return log_msg_ret("vd", -ENODEV);
		log_debug("vbe dev %s\n", meth->name);
		ret = device_probe(meth);
		if (ret)
			return log_msg_ret("probe", ret);

		priv = dev_get_priv(meth);
		log_debug("abrec %s\n", priv->storage);
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
		image->flags = SPL_SANDBOXF_ARG_IS_BUF;
		image->arg = bflow.buf;
		image->size = bflow.size;
		log_debug("Image: %s at %p size %x\n", bflow.name, bflow.buf,
			  bflow.size);

		/* this is not used from now on, so free it */
		bootflow_free(&bflow);
	} else {
		struct udevice *media;
		struct udevice *blk;

		ret = uclass_get_device_by_seq(UCLASS_MMC, 1, &media);
		if (ret)
			return log_msg_ret("vdv", ret);
		ret = blk_get_from_parent(media, &blk);
		if (ret)
			return log_msg_ret("med", ret);

		if (xpl_phase() == PHASE_TPL) {
			ulong offset, size;

			offset = binman_sym(ulong, vpl, image_pos);
			size = binman_sym(ulong, vpl, size);
			log_debug("VPL at offset %lx size %lx\n", offset, size);
			ret = vbe_read_fit(blk, offset, size, image, NULL,
					   NULL, NULL);
			if (ret)
				return log_msg_ret("vbe", ret);
		} else if (xpl_phase() == PHASE_VPL) {
			ret = abrec_run_vpl(blk, image, handoff);
		} else {
			ret = abrec_run_spl(blk, image, handoff);
		}
	}

	/* Record that VBE was used in this phase */
	handoff->phases |= 1 << xpl_phase();

	return 0;
}
SPL_LOAD_IMAGE_METHOD("vbe_abrec", 5, BOOT_DEVICE_VBE,
		      abrec_load_from_image);
