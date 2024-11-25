// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for onboard USB hubs
 *
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * Mostly inspired by Linux kernel v6.1 onboard_usb_hub driver
 */

#include <asm/gpio.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <power/regulator.h>

struct onboard_hub {
	struct udevice *vdd;
	struct gpio_desc *reset_gpio;
};

struct onboard_hub_data {
	unsigned long reset_us;
	unsigned long power_on_delay_us;
};

int usb_onboard_hub_reset(struct udevice *dev)
{
	struct onboard_hub_data *data =
		(struct onboard_hub_data *)dev_get_driver_data(dev);
	struct onboard_hub *hub = dev_get_priv(dev);
	int ret;

	hub->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_IS_OUT);

	/* property is optional, don't return error! */
	if (!hub->reset_gpio)
		return 0;

	ret = dm_gpio_set_value(hub->reset_gpio, 1);
	if (ret)
		return ret;

	udelay(data->reset_us);

	ret = dm_gpio_set_value(hub->reset_gpio, 0);
	if (ret)
		return ret;

	udelay(data->power_on_delay_us);

	return 0;
}

static int usb_onboard_hub_probe(struct udevice *dev)
{
	struct onboard_hub *hub = dev_get_priv(dev);
	int ret;

	ret = device_get_supply_regulator(dev, "vdd-supply", &hub->vdd);
	if (ret && ret != -ENOENT) {
		dev_err(dev, "can't get vdd-supply: %d\n", ret);
		return ret;
	}

	if (hub->vdd) {
		ret = regulator_set_enable_if_allowed(hub->vdd, true);
		if (ret && ret != -ENOSYS) {
			dev_err(dev, "can't enable vdd-supply: %d\n", ret);
			return ret;
		}
	}

	return usb_onboard_hub_reset(dev);
}

static int usb_onboard_hub_remove(struct udevice *dev)
{
	struct onboard_hub *hub = dev_get_priv(dev);
	int ret;

	if (hub->reset_gpio)
		dm_gpio_free(hub->reset_gpio->dev, hub->reset_gpio);

	ret = regulator_set_enable_if_allowed(hub->vdd, false);
	if (ret)
		dev_err(dev, "can't disable vdd-supply: %d\n", ret);

	return ret;
}

static const struct onboard_hub_data usb2514_data = {
	.power_on_delay_us = 500,
	.reset_us = 1,
};

static const struct udevice_id usb_onboard_hub_ids[] = {
	/* Use generic usbVID,PID dt-bindings (usb-device.yaml) */
	{	.compatible = "usb424,2514",	/* USB2514B USB 2.0 */
		.data = (ulong)&usb2514_data,
	}
};

U_BOOT_DRIVER(usb_onboard_hub) = {
	.name	= "usb_onboard_hub",
	.id	= UCLASS_USB_HUB,
	.probe = usb_onboard_hub_probe,
	.remove = usb_onboard_hub_remove,
	.of_match = usb_onboard_hub_ids,
	.priv_auto = sizeof(struct onboard_hub),
};
