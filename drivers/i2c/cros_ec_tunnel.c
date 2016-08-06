/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <cros_ec.h>
#include <errno.h>
#include <i2c.h>

static int cros_ec_i2c_set_bus_speed(struct udevice *dev, unsigned int speed)
{
	return 0;
}

static int cros_ec_i2c_xfer(struct udevice *dev, struct i2c_msg *msg,
			    int nmsgs)
{
	return cros_ec_i2c_tunnel(dev->parent, msg, nmsgs);
}

static const struct dm_i2c_ops cros_ec_i2c_ops = {
	.xfer		= cros_ec_i2c_xfer,
	.set_bus_speed	= cros_ec_i2c_set_bus_speed,
};

static const struct udevice_id cros_ec_i2c_ids[] = {
	{ .compatible = "google,cros-ec-i2c-tunnel" },
	{ }
};

U_BOOT_DRIVER(cros_ec_tunnel) = {
	.name	= "cros_ec_tunnel",
	.id	= UCLASS_I2C,
	.of_match = cros_ec_i2c_ids,
	.ops	= &cros_ec_i2c_ops,
};
