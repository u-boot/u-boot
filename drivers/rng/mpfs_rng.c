// SPDX-License-Identifier: GPL-2.0+
/*
 * Microchip's PolarFire SoC (MPFS) System Controller Driver
 *
 * Copyright (C) 2026 Microchip Technology Inc. All rights reserved.
 *
 * Author: Jamie Gibbons <jamie.gibbons@microchip.com>
 *
 */

#include <dm.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/printk.h>
#include <mailbox.h>
#include <malloc.h>
#include <mpfs-mailbox.h>
#include <rng.h>
#include <string.h>

#define CMD_OPCODE						0x21
#define CMD_DATA_SIZE					0U
#define CMD_DATA						NULL
#define MBOX_OFFSET						0U
#define RESP_OFFSET						0U
#define RNG_RESP_BYTES					32U

/**
 * struct mpfs_rng_priv - Structure representing System Controller data.
 * @mpfs_syscontroller_priv:	System Controller
 */
struct mpfs_rng_priv {
	struct mpfs_syscontroller_priv *sys_controller;
};

static int mpfs_rng_read(struct udevice *dev, void *data, size_t len)
{
	struct mpfs_rng_priv *rng_priv = dev_get_priv(dev);
	u32 response_msg[RNG_RESP_BYTES / sizeof(u32)];
	size_t count = 0, copy_size;
	int ret;

	struct mpfs_mss_response response = {
		.resp_status = 0U,
		.resp_msg = (u32 *)response_msg,
		.resp_size = RNG_RESP_BYTES,
	};
	struct mpfs_mss_msg msg = {
		.cmd_opcode = CMD_OPCODE,
		.cmd_data_size = CMD_DATA_SIZE,
		.response = &response,
		.cmd_data = CMD_DATA,
		.mbox_offset = MBOX_OFFSET,
		.resp_offset = RESP_OFFSET,
	};

	while (count < len) {
		ret = mpfs_syscontroller_run_service(rng_priv->sys_controller, &msg);
		if (ret)
			return ret;

		ret = mpfs_syscontroller_recv_response(rng_priv->sys_controller, &msg, 1000);
		if (ret)
			return ret;

		copy_size = (len - count > RNG_RESP_BYTES) ? RNG_RESP_BYTES : (len - count);
		memcpy((u8 *)data + count, response_msg, copy_size);
		count += copy_size;
	}

	return 0;
}

static int mpfs_rng_probe(struct udevice *dev)
{
	struct mpfs_rng_priv *rng_priv = dev_get_priv(dev);

	rng_priv->sys_controller = mpfs_syscontroller_get(dev->parent);
	if (IS_ERR(rng_priv->sys_controller)) {
		dev_err(dev, "Failed to get system controller\n");
		return PTR_ERR(rng_priv->sys_controller);
	}

	return 0;
}

static const struct dm_rng_ops mpfs_rng_ops = {
	.read = mpfs_rng_read,
};

U_BOOT_DRIVER(mpfs_rng) = {
	.name           = "mpfs_rng",
	.id             = UCLASS_RNG,
	.probe          = mpfs_rng_probe,
	.priv_auto	    = sizeof(struct mpfs_rng_priv),
	.ops            = &mpfs_rng_ops,
};
