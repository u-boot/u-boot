// SPDX-License-Identifier: GPL-2.0+
/*
 * TLMM driver for Qualcomm APQ8016, APQ8096
 *
 * (C) Copyright 2018 Ramon Fried <ramon.fried@gmail.com>
 *
 */

#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <asm/gpio.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <linux/bug.h>
#include <mach/gpio.h>

#include "pinctrl-qcom.h"

struct msm_pinctrl_priv {
	phys_addr_t base;
	struct msm_pinctrl_data *data;
};

#define GPIO_CONFIG_REG(priv, x) \
	(qcom_pin_offset((priv)->data->pin_data.pin_offsets, x))

#define GPIO_IN_OUT_REG(priv, x) \
	(GPIO_CONFIG_REG(priv, x) + 0x4)

#define TLMM_GPIO_PULL_MASK	GENMASK(1, 0)
#define TLMM_FUNC_SEL_MASK	GENMASK(5, 2)
#define TLMM_DRV_STRENGTH_MASK	GENMASK(8, 6)
#define TLMM_GPIO_OUTPUT_MASK	BIT(1)
#define TLMM_GPIO_OE_MASK	BIT(9)

/* GPIO register shifts. */
#define GPIO_OUT_SHIFT		1

static const struct pinconf_param msm_conf_params[] = {
	{ "drive-strength", PIN_CONFIG_DRIVE_STRENGTH, 2 },
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 3 },
	{ "bias-pull-down", PIN_CONFIG_BIAS_PULL_UP, 1 },
	{ "output-high", PIN_CONFIG_OUTPUT, 1, },
	{ "output-low", PIN_CONFIG_OUTPUT, 0, },
};

static int msm_get_functions_count(struct udevice *dev)
{
	struct msm_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->data->functions_count;
}

static int msm_get_pins_count(struct udevice *dev)
{
	struct msm_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->data->pin_data.pin_count;
}

static const char *msm_get_function_name(struct udevice *dev,
					 unsigned int selector)
{
	struct msm_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->data->get_function_name(dev, selector);
}

static int msm_pinctrl_probe(struct udevice *dev)
{
	struct msm_pinctrl_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	priv->data = (struct msm_pinctrl_data *)dev_get_driver_data(dev);

	return priv->base == FDT_ADDR_T_NONE ? -EINVAL : 0;
}

static const char *msm_get_pin_name(struct udevice *dev, unsigned int selector)
{
	struct msm_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->data->get_pin_name(dev, selector);
}

static int msm_pinmux_set(struct udevice *dev, unsigned int pin_selector,
			  unsigned int func_selector)
{
	struct msm_pinctrl_priv *priv = dev_get_priv(dev);
	u32 func = priv->data->get_function_mux(pin_selector, func_selector);

	/* Always NOP for special pins, assume they're in the correct state */
	if (qcom_is_special_pin(&priv->data->pin_data, pin_selector))
		return 0;

	clrsetbits_le32(priv->base + GPIO_CONFIG_REG(priv, pin_selector),
			TLMM_FUNC_SEL_MASK | TLMM_GPIO_OE_MASK, func << 2);
	return 0;
}

static int msm_pinconf_set_special(struct msm_pinctrl_priv *priv, unsigned int pin_selector,
				   unsigned int param, unsigned int argument)
{
	unsigned int offset = pin_selector - priv->data->pin_data.special_pins_start;
	const struct msm_special_pin_data *data;

	if (!priv->data->pin_data.special_pins_data)
		return 0;

	data = &priv->data->pin_data.special_pins_data[offset];

	switch (param) {
	case PIN_CONFIG_DRIVE_STRENGTH:
		argument = (argument / 2) - 1;
		clrsetbits_le32(priv->base + data->ctl_reg,
				GENMASK(2, 0) << data->drv_bit,
				argument << data->drv_bit);
		break;
	case PIN_CONFIG_BIAS_DISABLE:
		clrbits_le32(priv->base + data->ctl_reg,
			     TLMM_GPIO_PULL_MASK << data->pull_bit);
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		clrsetbits_le32(priv->base + data->ctl_reg,
				TLMM_GPIO_PULL_MASK << data->pull_bit,
				argument << data->pull_bit);
		break;
	default:
		return 0;
	}

	return 0;
}

static int msm_pinconf_set(struct udevice *dev, unsigned int pin_selector,
			   unsigned int param, unsigned int argument)
{
	struct msm_pinctrl_priv *priv = dev_get_priv(dev);

	if (qcom_is_special_pin(&priv->data->pin_data, pin_selector))
		return msm_pinconf_set_special(priv, pin_selector, param, argument);

	switch (param) {
	case PIN_CONFIG_DRIVE_STRENGTH:
		argument = (argument / 2) - 1;
		clrsetbits_le32(priv->base + GPIO_CONFIG_REG(priv, pin_selector),
				TLMM_DRV_STRENGTH_MASK, argument << 6);
		break;
	case PIN_CONFIG_BIAS_DISABLE:
		clrbits_le32(priv->base + GPIO_CONFIG_REG(priv, pin_selector),
			     TLMM_GPIO_PULL_MASK);
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		clrsetbits_le32(priv->base + GPIO_CONFIG_REG(priv, pin_selector),
				TLMM_GPIO_PULL_MASK, argument);
		break;
	case PIN_CONFIG_OUTPUT:
		writel(argument << GPIO_OUT_SHIFT,
		       priv->base + GPIO_IN_OUT_REG(priv, pin_selector));
		setbits_le32(priv->base + GPIO_CONFIG_REG(priv, pin_selector),
			     TLMM_GPIO_OE_MASK);
		break;
	default:
		return 0;
	}

	return 0;
}

struct pinctrl_ops msm_pinctrl_ops = {
	.get_pins_count = msm_get_pins_count,
	.get_pin_name = msm_get_pin_name,
	.set_state = pinctrl_generic_set_state,
	.pinmux_set = msm_pinmux_set,
	.pinconf_num_params = ARRAY_SIZE(msm_conf_params),
	.pinconf_params = msm_conf_params,
	.pinconf_set = msm_pinconf_set,
	.get_functions_count = msm_get_functions_count,
	.get_function_name = msm_get_function_name,
};

int msm_pinctrl_bind(struct udevice *dev)
{
	ofnode node = dev_ofnode(dev);
	struct msm_pinctrl_data *data = (struct msm_pinctrl_data *)dev_get_driver_data(dev);
	struct driver *drv;
	struct udevice *pinctrl_dev;
	const char *name;
	int ret;

	if (!data->pin_data.special_pins_start)
		dev_warn(dev, "Special pins start index not defined!\n");

	drv = lists_driver_lookup_name("pinctrl_qcom");
	if (!drv)
		return -ENOENT;

	ret = device_bind_with_driver_data(dev_get_parent(dev), drv, ofnode_get_name(node), (ulong)data,
					   dev_ofnode(dev), &pinctrl_dev);
	if (ret)
		return ret;

	ofnode_get_property(node, "gpio-controller", &ret);
	if (ret < 0)
		return 0;

	/* Get the name of gpio node */
	name = ofnode_get_name(node);
	if (!name)
		return -EINVAL;

	drv = lists_driver_lookup_name("gpio_msm");
	if (!drv) {
		printf("Can't find gpio_msm driver\n");
		return -ENODEV;
	}

	/* Bind gpio device as a child of the pinctrl device */
	ret = device_bind_with_driver_data(pinctrl_dev, drv,
					   name, (ulong)&data->pin_data, node, NULL);
	if (ret) {
		device_unbind(pinctrl_dev);
		return ret;
	}

	return 0;
}

U_BOOT_DRIVER(pinctrl_qcom) = {
	.name		= "pinctrl_qcom",
	.id		= UCLASS_PINCTRL,
	.priv_auto	= sizeof(struct msm_pinctrl_priv),
	.ops		= &msm_pinctrl_ops,
	.probe		= msm_pinctrl_probe,
};
