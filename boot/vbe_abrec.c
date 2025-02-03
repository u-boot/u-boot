// SPDX-License-Identifier: GPL-2.0+
/*
 * Verified Boot for Embedded (VBE) 'simple' method
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY LOGC_BOOT

#include <dm.h>
#include <memalign.h>
#include <mmc.h>
#include <dm/ofnode.h>
#include "vbe_abrec.h"

int abrec_read_priv(ofnode node, struct abrec_priv *priv)
{
	memset(priv, '\0', sizeof(*priv));
	if (ofnode_read_u32(node, "area-start", &priv->area_start) ||
	    ofnode_read_u32(node, "area-size", &priv->area_size) ||
	    ofnode_read_u32(node, "version-offset", &priv->version_offset) ||
	    ofnode_read_u32(node, "version-size", &priv->version_size) ||
	    ofnode_read_u32(node, "state-offset", &priv->state_offset) ||
	    ofnode_read_u32(node, "state-size", &priv->state_size))
		return log_msg_ret("read", -EINVAL);
	ofnode_read_u32(node, "skip-offset", &priv->skip_offset);
	priv->storage = strdup(ofnode_read_string(node, "storage"));
	if (!priv->storage)
		return log_msg_ret("str", -EINVAL);

	return 0;
}

int abrec_read_nvdata(struct abrec_priv *priv, struct udevice *blk,
		      struct abrec_state *state)
{
	ALLOC_CACHE_ALIGN_BUFFER(u8, buf, MMC_MAX_BLOCK_LEN);
	const struct vbe_nvdata *nvd = (struct vbe_nvdata *)buf;
	uint flags;
	int ret;

	ret = vbe_read_nvdata(blk, priv->area_start + priv->state_offset,
			      priv->state_size, buf);
	if (ret == -EPERM) {
		memset(buf, '\0', MMC_MAX_BLOCK_LEN);
		log_warning("Starting with empty state\n");
	} else if (ret) {
		return log_msg_ret("nv", ret);
	}

	state->fw_vernum = nvd->fw_vernum;
	flags = nvd->flags;
	state->try_count = flags & VBEF_TRY_COUNT_MASK;
	state->try_b = flags & VBEF_TRY_B;
	state->recovery = flags & VBEF_RECOVERY;
	state->pick = (flags & VBEF_PICK_MASK) >> VBEF_PICK_SHIFT;

	return 0;
}

int abrec_read_state(struct udevice *dev, struct abrec_state *state)
{
	struct abrec_priv *priv = dev_get_priv(dev);
	struct udevice *blk;
	int ret;

	ret = vbe_get_blk(priv->storage, &blk);
	if (ret)
		return log_msg_ret("blk", ret);

	ret = vbe_read_version(blk, priv->area_start + priv->version_offset,
			       state->fw_version, MAX_VERSION_LEN);
	if (ret)
		return log_msg_ret("ver", ret);
	log_debug("version=%s\n", state->fw_version);

	ret = abrec_read_nvdata(priv, blk, state);
	if (ret)
		return log_msg_ret("nvd", ret);

	return 0;
}
