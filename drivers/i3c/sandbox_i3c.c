// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

#include <dm.h>
#include <errno.h>
#include <i3c.h>
#include <linux/i3c/master.h>

struct sandbox_i3c_priv {
	struct i3c_priv_xfer i3c_xfers;
};

static int sandbox_i3c_priv_read(struct udevice *dev, u32 dev_number,
				 u8 *buf, u32 buf_size)
{
	struct sandbox_i3c_priv *priv = dev_get_priv(dev);
	struct i3c_priv_xfer i3c_xfers;

	i3c_xfers = priv->i3c_xfers;
	i3c_xfers.data.in = buf;
	i3c_xfers.len = buf_size;

	return 0;
}

static int sandbox_i3c_priv_write(struct udevice *dev, u32 dev_number,
				  u8 *buf, u32 buf_size)
{
	struct sandbox_i3c_priv *priv = dev_get_priv(dev);
	struct i3c_priv_xfer i3c_xfers;

	i3c_xfers = priv->i3c_xfers;
	i3c_xfers.data.out = buf;
	i3c_xfers.len = buf_size;

	return 0;
}

static const struct dm_i3c_ops sandbox_i3c_ops = {
	.read = sandbox_i3c_priv_read,
	.write = sandbox_i3c_priv_write,
};

static const struct udevice_id sandbox_i3c_ids[] = {
	{ .compatible = "sandbox,i3c" },
	{ }
};

U_BOOT_DRIVER(i3c_sandbox) = {
	.name = "i3c_sandbox",
	.id = UCLASS_I3C,
	.of_match = sandbox_i3c_ids,
	.ops    = &sandbox_i3c_ops,
};
