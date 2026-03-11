// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Linaro Ltd.
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/pinctrl.h>
#include <scmi_agent.h>
#include <scmi_agent-uclass.h>
#include <scmi_protocols.h>

static const struct pinconf_param pinctrl_scmi_conf_params[] = {
	{ "bias-bus-hold", PIN_CONFIG_BIAS_BUS_HOLD, 0},
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-high-impedance", PIN_CONFIG_BIAS_HIGH_IMPEDANCE, 0 },
	{ "bias-pull-down", PIN_CONFIG_BIAS_PULL_DOWN, 0 },
	{ "bias-pull-pin-default", PIN_CONFIG_BIAS_PULL_PIN_DEFAULT, 0 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 0 },
	{ "drive-open-drain", PIN_CONFIG_DRIVE_OPEN_DRAIN, 0 },
	{ "drive-open-source", PIN_CONFIG_DRIVE_OPEN_SOURCE, 0 },
	{ "drive-push-pull", PIN_CONFIG_DRIVE_PUSH_PULL, 0 },
	{ "drive-strength", PIN_CONFIG_DRIVE_STRENGTH, 0 },
	{ "input-debounce", PIN_CONFIG_INPUT_DEBOUNCE, 0 },
	{ "input-enable", PIN_CONFIG_INPUT_ENABLE, 1 },
	{ "input-schmitt", PIN_CONFIG_INPUT_SCHMITT, 0 },
	{ "low-power-mode", PIN_CONFIG_LOW_POWER_MODE, 0 },
	{ "output-mode", PIN_CONFIG_OUTPUT_ENABLE, 0 },
	{ "output-value", PIN_CONFIG_OUTPUT, 0 },
	{ "power-source", PIN_CONFIG_POWER_SOURCE, 0 },
	{ "slew-rate", PIN_CONFIG_SLEW_RATE, 0 },
	/* The SCMI spec also include "default", "pull-mode" and "input-value */
};

static bool valid_selector(struct udevice *dev, enum select_type select_type, u32 selector)
{
	struct pinctrl_scmi_priv *priv = dev_get_priv(dev);

	if (select_type == SCMI_PIN)
		return selector < priv->num_pins;
	if (select_type == SCMI_GROUP)
		return selector < priv->num_groups;
	if (select_type == SCMI_FUNCTION)
		return selector < priv->num_functions;

	return false;
}

static int pinctrl_scmi_get_pins_count(struct udevice *dev)
{
	struct pinctrl_scmi_priv *priv = dev_get_priv(dev);

	return priv->num_pins;
}

static int pinctrl_scmi_get_groups_count(struct udevice *dev)
{
	struct pinctrl_scmi_priv *priv = dev_get_priv(dev);

	return priv->num_groups;
}

static int pinctrl_scmi_get_functions_count(struct udevice *dev)
{
	struct pinctrl_scmi_priv *priv = dev_get_priv(dev);

	return priv->num_functions;
}

static const char *pinctrl_scmi_get_pin_name(struct udevice *dev, unsigned int selector)
{
	struct pinctrl_scmi_priv *priv = dev_get_priv(dev);

	if (selector >= priv->num_pins)
		return NULL;

	return (const char *)priv->pin_info[selector].name;
}

static const char *pinctrl_scmi_get_group_name(struct udevice *dev, unsigned int selector)
{
	struct pinctrl_scmi_priv *priv = dev_get_priv(dev);

	if (selector >= priv->num_groups)
		return NULL;

	return (const char *)priv->group_info[selector].name;
}

static const char *pinctrl_scmi_get_function_name(struct udevice *dev, unsigned int selector)
{
	struct pinctrl_scmi_priv *priv = dev_get_priv(dev);

	if (selector >= priv->num_functions)
		return NULL;

	return (const char *)priv->function_info[selector].name;
}

static int pinctrl_scmi_pinmux_set(struct udevice *dev, u32 pin, u32 function)
{
	struct pinctrl_scmi_priv *priv = dev_get_priv(dev);

	if (pin >= priv->num_pins || function >= priv->num_functions)
		return -EINVAL;

	return scmi_pinctrl_set_function(dev, SCMI_PIN, pin, function);
}

static int pinctrl_scmi_pinmux_group_set(struct udevice *dev, u32 group, u32 function)
{
	struct pinctrl_scmi_priv *priv = dev_get_priv(dev);

	if (group >= priv->num_groups || function >= priv->num_functions)
		return -EINVAL;

	return scmi_pinctrl_set_function(dev, SCMI_GROUP, group, function);
}

static int pinctrl_scmi_set_state(struct udevice *dev, struct udevice *config)
{
	struct pinctrl_scmi_priv *priv = dev_get_priv(dev);
	/* batch the setup into 20 lines at a go (there are 5 u32s in a config) */
	const int batch_count = 20 * 5;
	u32 prev_type = -1u;
	u32 prev_selector;
	u32 *configs;
	const u32 *prop;
	int offset, cnt, len;
	int ret = 0;

	prop = dev_read_prop(config, "pinmux", &len);
	if (!prop)
		return 0;

	if (len % sizeof(u32) * 5) {
		dev_err(dev, "invalid pin configuration: len=%d\n", len);
		return -FDT_ERR_BADSTRUCTURE;
	}

	configs = kcalloc(batch_count, sizeof(u32), GFP_KERNEL);
	if (!configs)
		return -ENOMEM;

	offset = 0;
	cnt = 0;
	while (offset + 4 < len / sizeof(u32)) {
		u32 select_type  = fdt32_to_cpu(prop[offset]);
		u32 selector     = fdt32_to_cpu(prop[offset + 1]);
		u32 function     = fdt32_to_cpu(prop[offset + 2]);
		u32 config_type  = fdt32_to_cpu(prop[offset + 3]);
		u32 config_value = fdt32_to_cpu(prop[offset + 4]);

		if (select_type > SCMI_GROUP ||
		    !valid_selector(dev, select_type, selector) ||
		    (function != SCMI_PINCTRL_FUNCTION_NONE &&
		     function > priv->num_functions)) {
			dev_err(dev, "invalid pinctrl data (%u %u %u %u %u)\n",
				select_type, selector, function, config_type,
				config_value);
			ret = -EINVAL;
			goto free;
		}

		if (function != SCMI_PINCTRL_FUNCTION_NONE) {
			if (cnt) {
				ret = scmi_pinctrl_settings_configure(dev,
								      prev_type,
								      prev_selector,
								      cnt / 2, configs);
				if (ret)
					goto free;
				prev_type = -1u;
				cnt = 0;
			}
			scmi_pinctrl_set_function(dev, select_type, selector, function);
			offset += 5;
			continue;
		}

		if (cnt == batch_count)
			goto set;

		if (prev_type == -1u)
			goto store;

		if (select_type == prev_type && selector == prev_selector)
			goto store;
set:
		ret = scmi_pinctrl_settings_configure(dev, prev_type, prev_selector,
						      cnt / 2, configs);
		if (ret)
			goto free;
		cnt = 0;
store:
		prev_type = select_type;
		prev_selector = selector;
		configs[cnt++] = config_type;
		configs[cnt++] = config_value;
		offset += 5;
	}

	if (cnt)
		ret = scmi_pinctrl_settings_configure(dev, prev_type, prev_selector,
						      cnt / 2, configs);
free:
	kfree(configs);
	if (ret)
		dev_err(dev, "set_state() failed: %d\n", ret);

	return ret;
}

static int get_pin_muxing(struct udevice *dev, unsigned int selector,
			  char *buf, int size)
{
	u32 value;
	int ret;

	ret = scmi_pinctrl_settings_get_one(dev, SCMI_PIN, selector,
					    SCMI_PIN_INPUT_VALUE, &value);
	if (ret) {
		dev_err(dev, "settings_get() failed: %d\n", ret);
		return ret;
	}

	snprintf(buf, size, "%d", value);
	return 0;
}

static int pinctrl_scmi_pinconf_set(struct udevice *dev, u32 pin, u32 param, u32 argument)
{
	return scmi_pinctrl_settings_configure_one(dev, SCMI_PIN, pin, param, argument);
}

static int pinctrl_scmi_pinconf_group_set(struct udevice *dev, u32 group, u32 param, u32 argument)
{
	return scmi_pinctrl_settings_configure_one(dev, SCMI_GROUP, group, param, argument);
}

static struct pinctrl_ops scmi_pinctrl_ops = {
	.get_pins_count = pinctrl_scmi_get_pins_count,
	.get_pin_name = pinctrl_scmi_get_pin_name,

	.get_groups_count = pinctrl_scmi_get_groups_count,
	.get_group_name = pinctrl_scmi_get_group_name,

	.get_functions_count = pinctrl_scmi_get_functions_count,
	.get_function_name = pinctrl_scmi_get_function_name,

	.pinmux_set = pinctrl_scmi_pinmux_set,
	.pinmux_group_set = pinctrl_scmi_pinmux_group_set,

	.pinconf_num_params = ARRAY_SIZE(pinctrl_scmi_conf_params),
	.pinconf_params = pinctrl_scmi_conf_params,

	.pinconf_set = pinctrl_scmi_pinconf_set,
	.pinconf_group_set = pinctrl_scmi_pinconf_group_set,
	.set_state = pinctrl_scmi_set_state,
	.get_pin_muxing = get_pin_muxing,
};

static int scmi_pinctrl_probe(struct udevice *dev)
{
	struct pinctrl_scmi_priv *priv = dev_get_priv(dev);
	int ret;
	int i;

	ret = devm_scmi_of_get_channel(dev);
	if (ret) {
		dev_err(dev, "get_channel() failed: %d\n", ret);
		return ret;
	}

	ret = scmi_pinctrl_protocol_attrs(dev, &priv->num_pins,
					  &priv->num_groups,
					  &priv->num_functions);
	if (ret) {
		dev_err(dev, "failed to get protocol attributes: %d\n", ret);
		return ret;
	}

	priv->pin_info = devm_kcalloc(dev, priv->num_pins,
				      sizeof(*priv->pin_info), GFP_KERNEL);
	priv->group_info = devm_kcalloc(dev, priv->num_groups,
					sizeof(*priv->group_info), GFP_KERNEL);
	priv->function_info = devm_kcalloc(dev, priv->num_functions,
					   sizeof(*priv->function_info), GFP_KERNEL);
	if (!priv->pin_info || !priv->group_info || !priv->function_info)
		return -ENOMEM;

	for (i = 0; i < priv->num_pins; i++) {
		ret = scmi_pinctrl_attrs(dev, SCMI_PIN, i, NULL, NULL,
					 priv->pin_info[i].name);
		if (ret)
			return ret;
	}

	for (i = 0; i < priv->num_groups; i++) {
		ret = scmi_pinctrl_attrs(dev, SCMI_GROUP, i, NULL,
					 &priv->group_info[i].num_pins,
					 priv->group_info[i].name);
		if (ret) {
			dev_err(dev, "loading group %d failed: %d\n", i, ret);
			return ret;
		}
		priv->group_info[i].pins = devm_kcalloc(dev,
							priv->group_info[i].num_pins,
							sizeof(*priv->group_info[i].pins),
							GFP_KERNEL);
		if (!priv->group_info[i].pins)
			return -ENOMEM;

		ret = scmi_pinctrl_list_associations(dev, SCMI_GROUP, i,
						     priv->group_info[i].pins,
						     priv->group_info[i].num_pins);
		if (ret) {
			dev_err(dev, "list association %d failed for group: %d\n", i, ret);
			return ret;
		}
	}

	for (i = 0; i < priv->num_functions; i++) {
		ret = scmi_pinctrl_attrs(dev, SCMI_FUNCTION, i, NULL,
					 &priv->function_info[i].num_groups,
					 priv->function_info[i].name);
		if (ret) {
			dev_err(dev, "loading function %d failed: %d\n", i, ret);
			return ret;
		}
		priv->function_info[i].groups = devm_kcalloc(dev,
					priv->function_info[i].num_groups,
					sizeof(*priv->function_info[i].groups),
					GFP_KERNEL);
		if (!priv->function_info[i].groups)
			return -ENOMEM;

		ret = scmi_pinctrl_list_associations(dev, SCMI_FUNCTION, i,
						     priv->function_info[i].groups,
						     priv->function_info[i].num_groups);
		if (ret) {
			dev_err(dev, "list association %d failed for function: %d\n", i, ret);
			return ret;
		}
	}

	return 0;
}

U_BOOT_DRIVER(pinctrl_scmi) = {
	.name = "scmi_pinctrl",
	.id = UCLASS_PINCTRL,
	.ops = &scmi_pinctrl_ops,
	.probe = scmi_pinctrl_probe,
	.priv_auto = sizeof(struct pinctrl_scmi_priv),
};

static struct scmi_proto_match match[] = {
	{ .proto_id = SCMI_PROTOCOL_ID_PINCTRL },
	{ /* Sentinel */ }
};

U_BOOT_SCMI_PROTO_DRIVER(pinctrl_scmi, match);

