// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <p2sb.h>
#include <pch.h>
#include <pci.h>
#include <syscon.h>
#include <asm/cpu.h>
#include <asm/gpio.h>
#include <asm/intel_pinctrl.h>
#include <asm/intel_pinctrl_defs.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/arch/gpio.h>
#include <dt-bindings/gpio/x86-gpio.h>

static int intel_gpio_direction_input(struct udevice *dev, uint offset)
{
	struct udevice *pinctrl = dev_get_parent(dev);
	uint config_offset = intel_pinctrl_get_config_reg_addr(pinctrl, offset);

	pcr_clrsetbits32(pinctrl, config_offset,
			 PAD_CFG0_MODE_MASK | PAD_CFG0_TX_STATE |
				  PAD_CFG0_RX_DISABLE,
			 PAD_CFG0_MODE_GPIO | PAD_CFG0_TX_DISABLE);

	return 0;
}

static int intel_gpio_direction_output(struct udevice *dev, uint offset,
				       int value)
{
	struct udevice *pinctrl = dev_get_parent(dev);
	uint config_offset = intel_pinctrl_get_config_reg_addr(pinctrl, offset);

	pcr_clrsetbits32(pinctrl, config_offset,
			 PAD_CFG0_MODE_MASK | PAD_CFG0_RX_STATE |
				  PAD_CFG0_TX_DISABLE | PAD_CFG0_TX_STATE,
			 PAD_CFG0_MODE_GPIO | PAD_CFG0_RX_DISABLE |
				  (value ? PAD_CFG0_TX_STATE : 0));

	return 0;
}

static int intel_gpio_get_value(struct udevice *dev, uint offset)
{
	struct udevice *pinctrl = dev_get_parent(dev);
	uint mode, rx_tx;
	u32 reg;

	reg = intel_pinctrl_get_config_reg(pinctrl, offset);
	mode = (reg & PAD_CFG0_MODE_MASK) >> PAD_CFG0_MODE_SHIFT;
	if (!mode) {
		rx_tx = reg & (PAD_CFG0_TX_DISABLE | PAD_CFG0_RX_DISABLE);
		if (rx_tx == PAD_CFG0_TX_DISABLE)
			return reg & PAD_CFG0_RX_STATE ? 1 : 0;
		else if (rx_tx == PAD_CFG0_RX_DISABLE)
			return reg & PAD_CFG0_TX_STATE ? 1 : 0;
	}

	return 0;
}

static int intel_gpio_set_value(struct udevice *dev, unsigned offset, int value)
{
	struct udevice *pinctrl = dev_get_parent(dev);
	uint config_offset = intel_pinctrl_get_config_reg_addr(pinctrl, offset);

	pcr_clrsetbits32(pinctrl, config_offset, PAD_CFG0_TX_STATE,
			 value ? PAD_CFG0_TX_STATE : 0);

	return 0;
}

static int intel_gpio_get_function(struct udevice *dev, uint offset)
{
	struct udevice *pinctrl = dev_get_parent(dev);
	uint mode, rx_tx;
	u32 reg;

	reg = intel_pinctrl_get_config_reg(pinctrl, offset);
	mode = (reg & PAD_CFG0_MODE_MASK) >> PAD_CFG0_MODE_SHIFT;
	if (!mode) {
		rx_tx = reg & (PAD_CFG0_TX_DISABLE | PAD_CFG0_RX_DISABLE);
		if (rx_tx == PAD_CFG0_TX_DISABLE)
			return GPIOF_INPUT;
		else if (rx_tx == PAD_CFG0_RX_DISABLE)
			return GPIOF_OUTPUT;
	}

	return GPIOF_FUNC;
}

static int intel_gpio_xlate(struct udevice *orig_dev, struct gpio_desc *desc,
			    struct ofnode_phandle_args *args)
{
	struct udevice *pinctrl, *dev;
	int gpio, ret;

	/*
	 * GPIO numbers are global in the device tree so it doesn't matter
	 * which one is used
	 */
	gpio = args->args[0];
	ret = intel_pinctrl_get_pad(gpio, &pinctrl, &desc->offset);
	if (ret)
		return log_msg_ret("bad", ret);
	device_find_first_child(pinctrl, &dev);
	if (!dev)
		return log_msg_ret("no child", -ENOENT);
	desc->flags = args->args[1] & GPIO_ACTIVE_LOW ? GPIOD_ACTIVE_LOW : 0;
	desc->dev = dev;

	return 0;
}

static int intel_gpio_probe(struct udevice *dev)
{
	return 0;
}

static int intel_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct gpio_dev_priv *upriv = dev_get_uclass_priv(dev);
	struct intel_pinctrl_priv *pinctrl_priv = dev_get_priv(dev->parent);
	const struct pad_community *comm = pinctrl_priv->comm;

	upriv->gpio_count = comm->last_pad - comm->first_pad + 1;
	upriv->bank_name = dev->name;

	return 0;
}

static const struct dm_gpio_ops gpio_intel_ops = {
	.direction_input	= intel_gpio_direction_input,
	.direction_output	= intel_gpio_direction_output,
	.get_value		= intel_gpio_get_value,
	.set_value		= intel_gpio_set_value,
	.get_function		= intel_gpio_get_function,
	.xlate			= intel_gpio_xlate,
};

static const struct udevice_id intel_intel_gpio_ids[] = {
	{ .compatible = "intel,gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_intel) = {
	.name	= "gpio_intel",
	.id	= UCLASS_GPIO,
	.of_match = intel_intel_gpio_ids,
	.ops	= &gpio_intel_ops,
	.ofdata_to_platdata	= intel_gpio_ofdata_to_platdata,
	.probe	= intel_gpio_probe,
};
