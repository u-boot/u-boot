// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Linaro Ltd.
 */

#include <asm/gpio.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/list.h>
#include <scmi_protocols.h>

struct scmi_gpio_range {
	u32 base;
	u32 offset;
	u32 npins;
	struct list_head list;
};

static int bank_cnt;

struct scmi_gpio_priv {
	struct udevice *pin_dev;
	struct list_head gpio_ranges;
	char *bank_name;
	u32 num_pins;
	u16 *pins;
};

static int scmi_gpio_request(struct udevice *dev, unsigned int offset, const char *label)
{
	struct scmi_gpio_priv *priv = dev_get_priv(dev);
	int pin;
	int ret;

	if (offset >= priv->num_pins)
		return -EINVAL;
	pin = priv->pins[offset];

	ret = scmi_pinctrl_request(priv->pin_dev, SCMI_PIN, pin);
	if (ret == -EOPNOTSUPP)
		ret = 0;
	if (ret)
		dev_err(dev, "%s(): request failed: %d\n", __func__, ret);
	return ret;
}

static int scmi_gpio_rfree(struct udevice *dev, unsigned int offset)
{
	struct scmi_gpio_priv *priv = dev_get_priv(dev);
	int pin;
	int ret;

	if (offset >= priv->num_pins)
		return -EINVAL;
	pin = priv->pins[offset];

	ret = scmi_pinctrl_release(priv->pin_dev, SCMI_PIN, pin);
	if (ret == -EOPNOTSUPP)
		ret = 0;
	if (ret)
		dev_err(dev, "%s(): release failed: %d\n", __func__, ret);
	return ret;
}

static int scmi_gpio_set_flags(struct udevice *dev, unsigned int offset, ulong flags)
{
	struct scmi_gpio_priv *priv = dev_get_priv(dev);
	const int MAX_FLAGS = 10;
	u32 configs[MAX_FLAGS * 2];
	int cnt = 0;
	u32 pin;

	if (offset >= priv->num_pins)
		return -EINVAL;
	pin = priv->pins[offset];

	if (flags & GPIOD_IS_OUT) {
		configs[cnt++] = SCMI_PIN_OUTPUT_MODE;
		configs[cnt++] = 1;
		configs[cnt++] = SCMI_PIN_OUTPUT_VALUE;
		if (flags & GPIOD_IS_OUT_ACTIVE)
			configs[cnt++] = 1;
		else
			configs[cnt++] = 0;
	}
	if (flags & GPIOD_IS_IN) {
		configs[cnt++] = SCMI_PIN_INPUT_MODE;
		configs[cnt++] = 1;
	}
	if (flags & GPIOD_OPEN_DRAIN) {
		configs[cnt++] = SCMI_PIN_DRIVE_OPEN_DRAIN;
		configs[cnt++] = 1;
	}
	if (flags & GPIOD_OPEN_SOURCE) {
		configs[cnt++] = SCMI_PIN_DRIVE_OPEN_SOURCE;
		configs[cnt++] = 1;
	}
	if (flags & GPIOD_PULL_UP) {
		configs[cnt++] = SCMI_PIN_BIAS_PULL_UP;
		configs[cnt++] = 1;
	}
	if (flags & GPIOD_PULL_DOWN) {
		configs[cnt++] = SCMI_PIN_BIAS_PULL_DOWN;
		configs[cnt++] = 1;
	}
	/* TODO: handle GPIOD_ACTIVE_LOW and GPIOD_IS_AF flags */

	return scmi_pinctrl_settings_configure(priv->pin_dev, SCMI_PIN, pin,
					       cnt / 2, &configs[0]);
}

static int scmi_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct scmi_gpio_priv *priv = dev_get_priv(dev);
	u32 value;
	int pin;
	int ret;

	if (offset >= priv->num_pins)
		return -EINVAL;
	pin = priv->pins[offset];

	ret = scmi_pinctrl_settings_get_one(priv->pin_dev, SCMI_PIN, pin,
					    SCMI_PIN_INPUT_VALUE, &value);
	if (ret) {
		dev_err(dev, "settings_get_one() failed: %d\n", ret);
		return ret;
	}

	return value;
}

static int scmi_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct scmi_gpio_priv *priv = dev_get_priv(dev);
	u32 value;
	int pin;
	int ret;

	if (offset >= priv->num_pins)
		return -EINVAL;
	pin = priv->pins[offset];

	ret = scmi_pinctrl_settings_get_one(priv->pin_dev, SCMI_PIN, pin,
					    SCMI_PIN_INPUT_MODE,
					    &value);
	if (ret) {
		dev_err(dev, "settings_get() failed %d\n", ret);
		return ret;
	}

	if (value)
		return GPIOF_INPUT;
	return GPIOF_OUTPUT;
}

static const struct dm_gpio_ops scmi_gpio_ops = {
	.request	= scmi_gpio_request,
	.rfree		= scmi_gpio_rfree,
	.set_flags	= scmi_gpio_set_flags,
	.get_value	= scmi_gpio_get_value,
	.get_function	= scmi_gpio_get_function,
};

static int scmi_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct scmi_gpio_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args args;
	struct scmi_gpio_range *range;
	int index = 0;
	int ret, i;

	INIT_LIST_HEAD(&priv->gpio_ranges);

	for (;; index++) {
		ret = dev_read_phandle_with_args(dev, "gpio-ranges",
						 NULL, 3, index, &args);
		if (ret)
			break;

		if (index == 0) {
			ret = uclass_get_device_by_ofnode(UCLASS_PINCTRL,
							  args.node,
							  &priv->pin_dev);
			if (ret) {
				dev_err(dev, "failed to find pinctrl device: %d\n", ret);
				return ret;
			}
		}

		range = devm_kmalloc(dev, sizeof(*range), GFP_KERNEL);
		if (!range)
			return -ENOMEM;

		range->base = args.args[0];
		if (range->base != priv->num_pins) {
			dev_err(dev, "no gaps allowed in between pins %d vs %d\n",
				priv->num_pins, range->base);
			return -EINVAL;
		}
		range->offset = args.args[1];
		range->npins = args.args[2];
		priv->num_pins += args.args[2];
		list_add_tail(&range->list, &priv->gpio_ranges);
	}

	if (priv->num_pins == 0) {
		dev_err(dev, "failed to registier pin-groups\n");
		return -EINVAL;
	}

	priv->pins = devm_kzalloc(dev, priv->num_pins * sizeof(u16), GFP_KERNEL);
	if (!priv->pins)
		return -ENOMEM;

	list_for_each_entry(range, &priv->gpio_ranges, list) {
		for (i = 0; i < range->npins; i++)
			priv->pins[range->base + i] = range->offset + i;
	}

	ret = snprintf(NULL, 0, "gpio_scmi%d_", bank_cnt);
	uc_priv->bank_name = devm_kzalloc(dev, ret + 1, GFP_KERNEL);
	if (!uc_priv->bank_name)
		return -ENOMEM;
	snprintf((char *)uc_priv->bank_name, ret + 1, "gpio_scmi%d_", bank_cnt);
	bank_cnt++;

	uc_priv->gpio_count = priv->num_pins;

	return 0;
}

static const struct udevice_id scmi_gpio_match[] = {
	{ .compatible = "scmi-pinctrl-gpio" },
	{ }
};

U_BOOT_DRIVER(scmi_pinctrl_gpio) = {
	.name	= "scmi_pinctrl_gpio",
	.id	= UCLASS_GPIO,
	.of_match = scmi_gpio_match,
	.probe	= scmi_gpio_probe,
	.priv_auto = sizeof(struct scmi_gpio_priv),
	.ops	= &scmi_gpio_ops,
};

