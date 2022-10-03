// SPDX-License-Identifier: GPL-2.0+
/*
 * TLMM driver for Qualcomm IPQ40xx
 *
 * (C) Copyright 2018 Ramon Fried <ramon.fried@gmail.com>
 *
 * Copyright (c) 2020 Sartura Ltd.
 *
 * Author: Robert Marko <robert.marko@sartura.hr>
 *
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include "pinctrl-snapdragon.h"

struct msm_pinctrl_priv {
	phys_addr_t base;
	struct msm_pinctrl_data *data;
};

#define GPIO_CONFIG_OFFSET(x)         ((x) * 0x1000)
#define TLMM_GPIO_PULL_MASK GENMASK(1, 0)
#define TLMM_FUNC_SEL_MASK GENMASK(5, 2)
#define TLMM_DRV_STRENGTH_MASK GENMASK(8, 6)
#define TLMM_GPIO_DISABLE BIT(9)

static const struct pinconf_param msm_conf_params[] = {
	{ "drive-strength", PIN_CONFIG_DRIVE_STRENGTH, 2 },
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 2 },
};

static int msm_get_functions_count(struct udevice *dev)
{
	struct msm_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->data->functions_count;
}

static int msm_get_pins_count(struct udevice *dev)
{
	struct msm_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->data->pin_count;
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

	priv->base = devfdt_get_addr(dev);
	priv->data = (struct msm_pinctrl_data *)dev->driver_data;

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

	clrsetbits_le32(priv->base + GPIO_CONFIG_OFFSET(pin_selector),
			TLMM_FUNC_SEL_MASK | TLMM_GPIO_DISABLE,
			priv->data->get_function_mux(func_selector) << 2);
	return 0;
}

static int msm_pinconf_set(struct udevice *dev, unsigned int pin_selector,
			   unsigned int param, unsigned int argument)
{
	struct msm_pinctrl_priv *priv = dev_get_priv(dev);

	switch (param) {
	case PIN_CONFIG_DRIVE_STRENGTH:
		clrsetbits_le32(priv->base + GPIO_CONFIG_OFFSET(pin_selector),
				TLMM_DRV_STRENGTH_MASK, argument << 6);
		break;
	case PIN_CONFIG_BIAS_DISABLE:
		clrbits_le32(priv->base + GPIO_CONFIG_OFFSET(pin_selector),
			     TLMM_GPIO_PULL_MASK);
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		clrsetbits_le32(priv->base + GPIO_CONFIG_OFFSET(pin_selector),
			     TLMM_GPIO_PULL_MASK, argument);
		break;
	default:
		return 0;
	}

	return 0;
}

static int msm_pinctrl_bind(struct udevice *dev)
{
	ofnode node = dev_ofnode(dev);
	const char *name;
	int ret;

	ofnode_get_property(node, "gpio-controller", &ret);
	if (ret < 0)
		return 0;

	/* Get the name of gpio node */
	name = ofnode_get_name(node);
	if (!name)
		return -EINVAL;

	/* Bind gpio node */
	ret = device_bind_driver_to_node(dev, "gpio_msm",
					 name, node, NULL);
	if (ret)
		return ret;

	dev_dbg(dev, "bind %s\n", name);

	return 0;
}

static struct pinctrl_ops msm_pinctrl_ops = {
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

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,ipq4019-pinctrl", .data = (ulong)&ipq4019_data },
	{ }
};

U_BOOT_DRIVER(pinctrl_snapdraon) = {
	.name		= "pinctrl_msm",
	.id		= UCLASS_PINCTRL,
	.of_match	= msm_pinctrl_ids,
	.priv_auto	= sizeof(struct msm_pinctrl_priv),
	.ops		= &msm_pinctrl_ops,
	.probe		= msm_pinctrl_probe,
	.bind		= msm_pinctrl_bind,
};
