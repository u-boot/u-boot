// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 - 2026, Advanced Micro Devices, Inc.
 *
 * Michal Simek <michal.simek@amd.com>
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <asm/gpio.h>
#include <linux/delay.h>

struct gpio_delay_desc {
	struct gpio_desc real_gpio;
	u32 ramp_up_us;
	u32 ramp_down_us;
};

struct gpio_delay_priv {
	struct gpio_delay_desc *descs;
};

static int gpio_delay_direction_input(struct udevice *dev, unsigned int offset)
{
	return -ENOSYS;
}

static int gpio_delay_get_value(struct udevice *dev, unsigned int offset)
{
	return -ENOSYS;
}

static int gpio_delay_set_value(struct udevice *dev, unsigned int offset,
				int value)
{
	struct gpio_delay_priv *priv = dev_get_priv(dev);
	struct gpio_delay_desc *desc = &priv->descs[offset];
	u32 wait;
	int ret;

	dev_dbg(dev, "gpio %d set to %d\n", offset, value);

	ret = dm_gpio_set_value(&desc->real_gpio, value);
	if (ret) {
		dev_err(dev, "Failed to set gpio %d, value %d\n", offset, value);
		return ret;
	}

	if (value)
		wait = desc->ramp_up_us;
	else
		wait = desc->ramp_down_us;

	udelay(wait);

	dev_dbg(dev, "waited for %d us\n", wait);

	return 0;
}

static int gpio_delay_direction_output(struct udevice *dev, unsigned int offset,
				       int value)
{
	return gpio_delay_set_value(dev, offset, value);
}

static int gpio_delay_xlate(struct udevice *dev, struct gpio_desc *desc,
			    struct ofnode_phandle_args *args)
{
	struct gpio_delay_priv *priv = dev_get_priv(dev);

	if (args->args_count < 3)
		return -EINVAL;

	if (args->args[0] >= 32)
		return -EINVAL;

	struct gpio_delay_desc *d = &priv->descs[args->args[0]];

	d->ramp_up_us = args->args[1];
	d->ramp_down_us = args->args[2];

	dev_dbg(dev, "pin: %d, ramp_up_us: %d, ramp_down_us: %d\n",
		args->args[0], d->ramp_up_us, d->ramp_down_us);

	return 0;
}

static const struct dm_gpio_ops gpio_delay_ops = {
	.direction_output = gpio_delay_direction_output,
	.direction_input = gpio_delay_direction_input,
	.get_value = gpio_delay_get_value,
	.set_value = gpio_delay_set_value,
	.xlate = gpio_delay_xlate,
};

static int gpio_delay_probe(struct udevice *dev)
{
	struct gpio_delay_priv *priv = dev_get_priv(dev);
	struct gpio_delay_desc *d;
	ofnode node = dev_ofnode(dev);
	int i = 0, ret, ngpio;

	ngpio = gpio_get_list_count(dev, "gpios");
	if (ngpio < 0)
		return ngpio;

	dev_dbg(dev, "gpios: %d\n", ngpio);

	priv->descs = devm_kmalloc_array(dev, ngpio, sizeof(*d), GFP_KERNEL);
	if (!priv->descs)
		return -ENOMEM;

	/* Request all GPIOs described in the controller node */
	for (i = 0; i < ngpio; i++) {
		d = &priv->descs[i];
		ret = gpio_request_by_name_nodev(node, "gpios", i,
						 &d->real_gpio, 0);
		if (ret)
			return ret;
	}

	return 0;
}

static const struct udevice_id gpio_delay_ids[] = {
	{ .compatible = "gpio-delay" },
	{ }
};

U_BOOT_DRIVER(gpio_delay) = {
	.name = "gpio-delay",
	.id = UCLASS_GPIO,
	.of_match = gpio_delay_ids,
	.ops = &gpio_delay_ops,
	.priv_auto = sizeof(struct gpio_delay_priv),
	.probe = gpio_delay_probe,
};
