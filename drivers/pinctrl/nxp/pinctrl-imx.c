// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Peng Fan <van.freenix@gmail.com>
 */

#include <malloc.h>
#include <mapmem.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/err.h>
#include <dm.h>
#include <dm/pinctrl.h>

#include "pinctrl-imx.h"

DECLARE_GLOBAL_DATA_PTR;

int imx_pinctrl_set_state_common(struct udevice *dev, struct udevice *config,
				 int pin_size, u32 **pin_data, int *npins)
{
	ofnode node = dev_ofnode(config);
	const struct fdt_property *prop;
	int size;

	dev_dbg(dev, "%s: %s\n", __func__, config->name);

	prop = ofnode_get_property(node, "fsl,pins", &size);
	if (!prop) {
		dev_err(dev, "No fsl,pins property in node %s\n", config->name);
		return -EINVAL;
	}

	if (!size || size % pin_size) {
		dev_err(dev, "Invalid fsl,pins property in node %s\n",
			config->name);
		return -EINVAL;
	}

	*pin_data = devm_kzalloc(dev, size, 0);
	if (!*pin_data)
		return -ENOMEM;

	if (ofnode_read_u32_array(node, "fsl,pins", *pin_data, size >> 2)) {
		dev_err(dev, "Error reading pin data.\n");
		devm_kfree(dev, *pin_data);
		return -EINVAL;
	}

	*npins = size / pin_size;

	return 0;
}

int imx_pinctrl_probe_common(struct udevice *dev)
{
	struct imx_pinctrl_soc_info *info =
		(struct imx_pinctrl_soc_info *)dev_get_driver_data(dev);
	struct imx_pinctrl_priv *priv = dev_get_priv(dev);

	if (!info) {
		dev_err(dev, "wrong pinctrl info\n");
		return -EINVAL;
	}

	priv->dev = dev;
	priv->info = info;

	return 0;
}
