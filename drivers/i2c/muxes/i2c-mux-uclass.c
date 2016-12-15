/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <dm/lists.h>
#include <dm/root.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct i2c_mux: Information the uclass stores about an I2C mux
 *
 * @selected:	Currently selected mux, or -1 for none
 * @i2c_bus: I2C bus to use for communcation
 */
struct i2c_mux {
	int selected;
	struct udevice *i2c_bus;
};

/**
 * struct i2c_mux_bus: Information about each bus the mux controls
 *
 * @channel: Channel number used to select this bus
 */
struct i2c_mux_bus {
	uint channel;
};

/* Find out the mux channel number */
static int i2c_mux_child_post_bind(struct udevice *dev)
{
	struct i2c_mux_bus *plat = dev_get_parent_platdata(dev);
	int channel;

	channel = fdtdec_get_int(gd->fdt_blob, dev->of_offset, "reg", -1);
	if (channel < 0)
		return -EINVAL;
	plat->channel = channel;

	return 0;
}

/* Find the I2C buses selected by this mux */
static int i2c_mux_post_bind(struct udevice *mux)
{
	const void *blob = gd->fdt_blob;
	int ret;
	int offset;

	debug("%s: %s\n", __func__, mux->name);
	/*
	 * There is no compatible string in the sub-nodes, so we must manually
	 * bind these
	 */
	for (offset = fdt_first_subnode(blob, mux->of_offset);
	     offset > 0;
	     offset = fdt_next_subnode(blob, offset)) {
		struct udevice *dev;
		const char *name;

		name = fdt_get_name(blob, offset, NULL);
		ret = device_bind_driver_to_node(mux, "i2c_mux_bus_drv", name,
						 offset, &dev);
		debug("   - bind ret=%d, %s\n", ret, dev ? dev->name : NULL);
		if (ret)
			return ret;
	}

	return 0;
}

/* Set up the mux ready for use */
static int i2c_mux_post_probe(struct udevice *mux)
{
	struct i2c_mux *priv = dev_get_uclass_priv(mux);
	int ret;

	debug("%s: %s\n", __func__, mux->name);
	priv->selected = -1;

	ret = uclass_get_device_by_phandle(UCLASS_I2C, mux, "i2c-parent",
					   &priv->i2c_bus);
	if (ret)
		return ret;
	debug("%s: bus=%p/%s\n", __func__, priv->i2c_bus, priv->i2c_bus->name);

	return 0;
}

int i2c_mux_select(struct udevice *dev)
{
	struct i2c_mux_bus *plat = dev_get_parent_platdata(dev);
	struct udevice *mux = dev->parent;
	struct i2c_mux_ops *ops = i2c_mux_get_ops(mux);

	if (!ops->select)
		return -ENOSYS;

	return ops->select(mux, dev, plat->channel);
}

int i2c_mux_deselect(struct udevice *dev)
{
	struct i2c_mux_bus *plat = dev_get_parent_platdata(dev);
	struct udevice *mux = dev->parent;
	struct i2c_mux_ops *ops = i2c_mux_get_ops(mux);

	if (!ops->deselect)
		return -ENOSYS;

	return ops->deselect(mux, dev, plat->channel);
}

static int i2c_mux_bus_set_bus_speed(struct udevice *dev, unsigned int speed)
{
	struct udevice *mux = dev->parent;
	struct i2c_mux *priv = dev_get_uclass_priv(mux);
	int ret, ret2;

	ret = i2c_mux_select(dev);
	if (ret)
		return ret;
	ret = dm_i2c_set_bus_speed(priv->i2c_bus, speed);
	ret2 = i2c_mux_deselect(dev);

	return ret ? ret : ret2;
}

static int i2c_mux_bus_probe(struct udevice *dev, uint chip_addr,
			     uint chip_flags)
{
	struct udevice *mux = dev->parent;
	struct i2c_mux *priv = dev_get_uclass_priv(mux);
	struct dm_i2c_ops *ops = i2c_get_ops(priv->i2c_bus);
	int ret, ret2;

	debug("%s: %s, bus %s\n", __func__, dev->name, priv->i2c_bus->name);
	if (!ops->probe_chip)
		return -ENOSYS;
	ret = i2c_mux_select(dev);
	if (ret)
		return ret;
	ret = ops->probe_chip(priv->i2c_bus, chip_addr, chip_flags);
	ret2 = i2c_mux_deselect(dev);

	return ret ? ret : ret2;
}

static int i2c_mux_bus_xfer(struct udevice *dev, struct i2c_msg *msg,
			    int nmsgs)
{
	struct udevice *mux = dev->parent;
	struct i2c_mux *priv = dev_get_uclass_priv(mux);
	struct dm_i2c_ops *ops = i2c_get_ops(priv->i2c_bus);
	int ret, ret2;

	debug("%s: %s, bus %s\n", __func__, dev->name, priv->i2c_bus->name);
	if (!ops->xfer)
		return -ENOSYS;
	ret = i2c_mux_select(dev);
	if (ret)
		return ret;
	ret = ops->xfer(priv->i2c_bus, msg, nmsgs);
	ret2 = i2c_mux_deselect(dev);

	return ret ? ret : ret2;
}

static const struct dm_i2c_ops i2c_mux_bus_ops = {
	.xfer		= i2c_mux_bus_xfer,
	.probe_chip	= i2c_mux_bus_probe,
	.set_bus_speed	= i2c_mux_bus_set_bus_speed,
};

U_BOOT_DRIVER(i2c_mux_bus) = {
	.name		= "i2c_mux_bus_drv",
	.id		= UCLASS_I2C,
	.ops	= &i2c_mux_bus_ops,
};

UCLASS_DRIVER(i2c_mux) = {
	.id		= UCLASS_I2C_MUX,
	.name		= "i2c_mux",
	.post_bind	= i2c_mux_post_bind,
	.post_probe	= i2c_mux_post_probe,
	.per_device_auto_alloc_size = sizeof(struct i2c_mux),
	.per_child_platdata_auto_alloc_size = sizeof(struct i2c_mux_bus),
	.child_post_bind = i2c_mux_child_post_bind,
};
