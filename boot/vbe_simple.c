// SPDX-License-Identifier: GPL-2.0+
/*
 * Verified Boot for Embedded (VBE) 'simple' method
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY LOGC_BOOT

#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <dm.h>
#include <log.h>
#include <memalign.h>
#include <mmc.h>
#include <vbe.h>
#include <dm/device-internal.h>
#include <dm/ofnode.h>
#include "vbe_simple.h"

static int simple_read_nvdata(const struct simple_priv *priv,
			      struct udevice *blk, struct simple_state *state)
{
	ALLOC_CACHE_ALIGN_BUFFER(u8, buf, MMC_MAX_BLOCK_LEN);
	const struct vbe_nvdata *nvd;
	int ret;

	ret = vbe_read_nvdata(blk, priv->area_start + priv->state_offset,
			      priv->state_size, buf);
	if (ret)
		return log_msg_ret("nv", ret);

	nvd = (struct vbe_nvdata *)buf;
	state->fw_vernum = nvd->fw_vernum;

	log_debug("version=%s\n", state->fw_version);

	return 0;
}

int vbe_simple_read_state(struct udevice *dev, struct simple_state *state)
{
	struct simple_priv *priv = dev_get_priv(dev);
	struct udevice *blk;
	int ret;

	ret = vbe_get_blk(priv->storage, &blk);
	if (ret)
		return log_msg_ret("blk", ret);

	ret = vbe_read_version(blk, priv->area_start + priv->version_offset,
			       state->fw_version, MAX_VERSION_LEN);
	if (ret)
		return log_msg_ret("ver", ret);

	ret = simple_read_nvdata(priv, blk, state);
	if (ret)
		return log_msg_ret("nvd", ret);

	return 0;
}

static int vbe_simple_get_state_desc(struct udevice *dev, char *buf,
				     int maxsize)
{
	struct simple_state state;
	int ret;

	ret = vbe_simple_read_state(dev, &state);
	if (ret)
		return log_msg_ret("read", ret);

	if (maxsize < 30)
		return -ENOSPC;
	snprintf(buf, maxsize, "Version: %s\nVernum: %x/%x", state.fw_version,
		 state.fw_vernum >> FWVER_KEY_SHIFT,
		 state.fw_vernum & FWVER_FW_MASK);

	return 0;
}

static int vbe_simple_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	int ret;

	if (CONFIG_IS_ENABLED(BOOTMETH_VBE_SIMPLE_FW)) {
		if (vbe_phase() == VBE_PHASE_FIRMWARE) {
			ret = vbe_simple_read_bootflow_fw(dev, bflow);
			if (ret)
				return log_msg_ret("fw", ret);
			return 0;
		}
	}

	return -EINVAL;
}

static int vbe_simple_read_file(struct udevice *dev, struct bootflow *bflow,
				const char *file_path, ulong addr,
				enum bootflow_img_t type, ulong *sizep)
{
	int ret;

	if (vbe_phase() == VBE_PHASE_OS) {
		ret = bootmeth_common_read_file(dev, bflow, file_path, addr,
						type, sizep);
		if (ret)
			return log_msg_ret("os", ret);
	}

	/* To be implemented */
	return -EINVAL;
}

static struct bootmeth_ops bootmeth_vbe_simple_ops = {
	.get_state_desc	= vbe_simple_get_state_desc,
	.read_bootflow	= vbe_simple_read_bootflow,
	.read_file	= vbe_simple_read_file,
};

static int bootmeth_vbe_simple_probe(struct udevice *dev)
{
	struct simple_priv *priv = dev_get_priv(dev);

	memset(priv, '\0', sizeof(*priv));
	if (dev_read_u32(dev, "area-start", &priv->area_start) ||
	    dev_read_u32(dev, "area-size", &priv->area_size) ||
	    dev_read_u32(dev, "version-offset", &priv->version_offset) ||
	    dev_read_u32(dev, "version-size", &priv->version_size) ||
	    dev_read_u32(dev, "state-offset", &priv->state_offset) ||
	    dev_read_u32(dev, "state-size", &priv->state_size))
		return log_msg_ret("read", -EINVAL);
	dev_read_u32(dev, "skip-offset", &priv->skip_offset);
	priv->storage = strdup(dev_read_string(dev, "storage"));
	if (!priv->storage)
		return log_msg_ret("str", -EINVAL);

	return 0;
}

static int bootmeth_vbe_simple_bind(struct udevice *dev)
{
	struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->desc = IS_ENABLED(CONFIG_BOOTSTD_FULL) ?
		"VBE simple" : "vbe-simple";
	plat->flags = BOOTMETHF_GLOBAL;

	return 0;
}

#if CONFIG_IS_ENABLED(OF_REAL)
static const struct udevice_id generic_simple_vbe_simple_ids[] = {
	{ .compatible = "fwupd,vbe-simple" },
	{ }
};
#endif

U_BOOT_DRIVER(vbe_simple) = {
	.name	= "vbe_simple",
	.id	= UCLASS_BOOTMETH,
	.of_match = of_match_ptr(generic_simple_vbe_simple_ids),
	.ops	= &bootmeth_vbe_simple_ops,
	.bind	= bootmeth_vbe_simple_bind,
	.probe	= bootmeth_vbe_simple_probe,
	.flags	= DM_FLAG_PRE_RELOC,
	.priv_auto	= sizeof(struct simple_priv),
};
