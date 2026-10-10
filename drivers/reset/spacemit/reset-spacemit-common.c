// SPDX-License-Identifier: GPL-2.0+
/*
 * SpacemiT reset controller — common implementation (U-Boot)
 */

#include <asm/io.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <malloc.h>
#include <reset-uclass.h>

#include "reset-spacemit-common.h"

static int spacemit_reset_xfer(struct reset_ctl *rst, bool assert)
{
	struct spacemit_reset_priv *priv = dev_get_priv(rst->dev);
	const struct spacemit_reset_data *e;
	u32 v;

	if (rst->id >= priv->table_size)
		return -EINVAL;

	e = &priv->table[rst->id];
	if (e->assert_mask == 0 && e->deassert_mask == 0)
		return -EINVAL;

	v = readl(priv->base + e->offset);
	v &= ~(e->assert_mask | e->deassert_mask);
	v |= assert ? e->assert_mask : e->deassert_mask;
	writel(v, priv->base + e->offset);

	return 0;
}

static int spacemit_reset_assert(struct reset_ctl *rst)
{
	return spacemit_reset_xfer(rst, true);
}

static int spacemit_reset_deassert(struct reset_ctl *rst)
{
	return spacemit_reset_xfer(rst, false);
}

static int spacemit_reset_request(struct reset_ctl *rst)
{
	struct spacemit_reset_priv *priv = dev_get_priv(rst->dev);

	return rst->id < priv->table_size ? 0 : -EINVAL;
}

const struct reset_ops spacemit_reset_ops = {
	.request	= spacemit_reset_request,
	.rst_assert	= spacemit_reset_assert,
	.rst_deassert	= spacemit_reset_deassert,
};

int spacemit_reset_probe(struct udevice *dev)
{
	struct spacemit_reset_priv *priv = dev_get_priv(dev);

	priv->base = (void __iomem *)dev_remap_addr(dev);
	if (!priv->base)
		return -ENODEV;

	return 0;
}

int spacemit_reset_bind(struct udevice *parent, const char *drv_name,
			const struct spacemit_reset_data *table,
			size_t table_size)
{
	struct spacemit_reset_priv *priv;
	struct udevice *rst_dev;
	int ret;

	ret = device_bind_driver_to_node(parent, drv_name, "reset",
					 dev_ofnode(parent), &rst_dev);
	if (ret)
		return ret;

	priv = malloc(sizeof(*priv));
	if (!priv) {
		device_unbind(rst_dev);
		return -ENOMEM;
	}
	priv->table = table;
	priv->table_size = table_size;
	dev_set_priv(rst_dev, priv);

	return 0;
}
