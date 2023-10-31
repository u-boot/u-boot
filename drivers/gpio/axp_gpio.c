// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Hans de Goede <hdegoede@redhat.com>
 *
 * X-Powers AXP Power Management ICs gpio driver
 */

#include <asm/arch/pmic_bus.h>
#include <asm/gpio.h>
#include <axp_pmic.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <errno.h>
#include <sunxi_gpio.h>

#define AXP_GPIO_PREFIX			"AXP0-"
#define AXP_GPIO_COUNT			4

static int axp_gpio_set_value(struct udevice *dev, unsigned pin, int val);

static u8 axp_get_gpio_ctrl_reg(unsigned pin)
{
	switch (pin) {
	case 0: return AXP_GPIO0_CTRL;
	case 1: return AXP_GPIO1_CTRL;
#ifdef AXP_GPIO2_CTRL
	case 2: return AXP_GPIO2_CTRL;
#endif
#ifdef AXP_GPIO3_CTRL
	case 3: return AXP_GPIO3_CTRL;
#endif
	}
	return 0;
}

static int axp_gpio_direction_input(struct udevice *dev, unsigned pin)
{
	u8 reg;

	reg = axp_get_gpio_ctrl_reg(pin);
	if (reg == 0)
		return -EINVAL;

	return pmic_bus_write(reg, AXP_GPIO_CTRL_INPUT);
}

static int axp_gpio_direction_output(struct udevice *dev, unsigned pin,
				     int val)
{
	u8 reg;

	reg = axp_get_gpio_ctrl_reg(pin);
	if (reg == 0)
		return -EINVAL;

	return pmic_bus_write(reg, val ? AXP_GPIO_CTRL_OUTPUT_HIGH :
					 AXP_GPIO_CTRL_OUTPUT_LOW);
}

static int axp_gpio_get_value(struct udevice *dev, unsigned pin)
{
	u8 reg, val, mask;
	int ret;

	reg = axp_get_gpio_ctrl_reg(pin);
	if (reg == 0)
		return -EINVAL;

	ret = pmic_bus_read(AXP_GPIO_STATE, &val);
	if (ret)
		return ret;

	mask = 1 << (pin + AXP_GPIO_STATE_OFFSET);

	return (val & mask) ? 1 : 0;
}

static int axp_gpio_set_value(struct udevice *dev, unsigned pin, int val)
{
	u8 reg;

	reg = axp_get_gpio_ctrl_reg(pin);
	if (reg == 0)
		return -EINVAL;

	return pmic_bus_write(reg, val ? AXP_GPIO_CTRL_OUTPUT_HIGH :
					 AXP_GPIO_CTRL_OUTPUT_LOW);
}

static const struct dm_gpio_ops gpio_axp_ops = {
	.direction_input	= axp_gpio_direction_input,
	.direction_output	= axp_gpio_direction_output,
	.get_value		= axp_gpio_get_value,
	.set_value		= axp_gpio_set_value,
};

static int gpio_axp_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	/* Tell the uclass how many GPIOs we have */
	uc_priv->bank_name = AXP_GPIO_PREFIX;
	uc_priv->gpio_count = AXP_GPIO_COUNT;

	return 0;
}

U_BOOT_DRIVER(gpio_axp) = {
	.name	= "gpio_axp",
	.id	= UCLASS_GPIO,
	.ops	= &gpio_axp_ops,
	.probe	= gpio_axp_probe,
};

int axp_gpio_init(void)
{
	struct udevice *dev;
	int ret;

	ret = pmic_bus_init();
	if (ret)
		return ret;

	/* There is no devicetree support for the axp yet, so bind directly */
	ret = device_bind_driver(dm_root(), "gpio_axp", "AXP-gpio", &dev);
	if (ret)
		return ret;

	return 0;
}
