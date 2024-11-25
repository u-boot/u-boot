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
#include <i2c.h>
#include <linux/delay.h>
#include <power/regulator.h>

#define USB5744_COMMAND_ATTACH		0x0056
#define USB5744_COMMAND_ATTACH_LSB	0xAA
#define USB5744_CONFIG_REG_ACCESS	0x0037
#define USB5744_CONFIG_REG_ACCESS_LSB	0x99

struct onboard_hub {
	struct udevice *vdd;
	struct gpio_desc *reset_gpio;
};

struct onboard_hub_data {
	unsigned long reset_us;
	unsigned long power_on_delay_us;
	int (*init)(struct udevice *dev);
};

static int usb5744_i2c_init(struct udevice *dev)
{
	/*
	 *  Prevent the MCU from the putting the HUB in suspend mode through register write.
	 *  The BYPASS_UDC_SUSPEND bit (Bit 3) of the RuntimeFlags2 register at address
	 *  0x411D controls this aspect of the hub.
	 *  Format to write to hub registers via SMBus- 2D 00 00 05 00 01 41 1D 08
	 *  Byte 0: Address of slave 2D
	 *  Byte 1: Memory address 00
	 *  Byte 2: Memory address 00
	 *  Byte 3: Number of bytes to write to memory
	 *  Byte 4: Write configuration register (00)
	 *  Byte 5: Write the number of data bytes (01- 1 data byte)
	 *  Byte 6: LSB of register address 0x41
	 *  Byte 7: MSB of register address 0x1D
	 *  Byte 8: value to be written to the register
	 */
	u8 data_buf[8] = {0x0, 0x5, 0x0, 0x1, 0x41, 0x1D, 0x08};
	u8 config_reg_access_buf = USB5744_CONFIG_REG_ACCESS;
	struct udevice *i2c_bus = NULL, *i2c_dev;
	struct ofnode_phandle_args phandle;
	u8 buf = USB5744_COMMAND_ATTACH;
	struct dm_i2c_chip *i2c_chip;
	int ret, slave_addr;

	ret = dev_read_phandle_with_args(dev, "i2c-bus", NULL, 0, 0, &phandle);
	if (ret) {
		dev_err(dev, "i2c-bus not specified\n");
		return ret;
	}

	ret = device_get_global_by_ofnode(ofnode_get_parent(phandle.node), &i2c_bus);
	if (ret) {
		dev_err(dev, "Failed to get i2c node, err: %d\n", ret);
		return ret;
	}

	ret = ofnode_read_u32(phandle.node, "reg", &slave_addr);
	if (ret)
		return ret;

	ret = i2c_get_chip(i2c_bus, slave_addr, 1, &i2c_dev);
	if (ret) {
		dev_err(dev, "%s: can't find i2c chip device for addr 0x%x\n", __func__,
			slave_addr);
		return ret;
	}

	i2c_chip = dev_get_parent_plat(i2c_dev);
	if (!i2c_chip) {
		dev_err(dev, "parent platform data not found\n");
		return -EINVAL;
	}

	i2c_chip->flags &= ~DM_I2C_CHIP_WR_ADDRESS;
	/* SMBus write command */
	ret = dm_i2c_write(i2c_dev, 0, (uint8_t *)&data_buf, 8);
	if (ret) {
		dev_err(dev, "data_buf i2c_write failed, err:%d\n", ret);
		return ret;
	}

	/* Configuration register access command */
	ret = dm_i2c_write(i2c_dev, USB5744_CONFIG_REG_ACCESS_LSB,
			   &config_reg_access_buf, 2);
	if (ret) {
		dev_err(dev, "config_reg_access i2c_write failed, err: %d\n", ret);
		return ret;
	}

	/* USB Attach with SMBus */
	ret = dm_i2c_write(i2c_dev, USB5744_COMMAND_ATTACH_LSB, &buf, 2);
	if (ret) {
		dev_err(dev, "usb_attach i2c_write failed, err: %d\n", ret);
		return ret;
	}

	return 0;
}

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
	struct onboard_hub_data *data =
		(struct onboard_hub_data *)dev_get_driver_data(dev);
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

	ret = usb_onboard_hub_reset(dev);
	if (ret)
		return ret;

	if (data->init) {
		ret = data->init(dev);
		if (ret) {
			dev_err(dev, "onboard i2c init failed: %d\n", ret);
			goto err;
		}
	}
	return 0;
err:
	dm_gpio_set_value(hub->reset_gpio, 0);
	return ret;
}

static int usb_onboard_hub_bind(struct udevice *dev)
{
	struct ofnode_phandle_args phandle;
	const void *fdt = gd->fdt_blob;
	int ret, off;

	ret = dev_read_phandle_with_args(dev, "peer-hub", NULL, 0, 0, &phandle);
	if (ret)  {
		dev_err(dev, "peer-hub not specified\n");
		return ret;
	}

	off = ofnode_to_offset(phandle.node);
	ret = fdt_node_check_compatible(fdt, off, "usb424,5744");
	if (!ret)
		return 0;

	return -ENODEV;
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

static const struct onboard_hub_data usb5744_data = {
	.init = usb5744_i2c_init,
	.power_on_delay_us = 1000,
	.reset_us = 5,
};

static const struct udevice_id usb_onboard_hub_ids[] = {
	/* Use generic usbVID,PID dt-bindings (usb-device.yaml) */
	{	.compatible = "usb424,2514",	/* USB2514B USB 2.0 */
		.data = (ulong)&usb2514_data,
	}, {
		.compatible = "usb424,2744",	/* USB2744 USB 2.0 */
		.data = (ulong)&usb5744_data,
	}, {
		.compatible = "usb424,5744",	/* USB5744 USB 3.0 */
		.data = (ulong)&usb5744_data,
	}
};

U_BOOT_DRIVER(usb_onboard_hub) = {
	.name	= "usb_onboard_hub",
	.id	= UCLASS_USB_HUB,
	.bind   = usb_onboard_hub_bind,
	.probe = usb_onboard_hub_probe,
	.remove = usb_onboard_hub_remove,
	.of_match = usb_onboard_hub_ids,
	.priv_auto = sizeof(struct onboard_hub),
};
