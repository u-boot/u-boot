// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Linaro Ltd.
 */

#define LOG_CATEGORY UCLASS_PINCTRL

#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/pinctrl.h>
#include <linux/bitfield.h>
#include <linux/compat.h>
#include <scmi_agent.h>
#include <scmi_agent-uclass.h>
#include <scmi_protocols.h>

static int map_config_param_to_scmi(u32 config_param)
{
	switch (config_param) {
	case PIN_CONFIG_BIAS_BUS_HOLD:
		return SCMI_PIN_BIAS_BUS_HOLD;
	case PIN_CONFIG_BIAS_DISABLE:
		return SCMI_PIN_BIAS_DISABLE;
	case PIN_CONFIG_BIAS_HIGH_IMPEDANCE:
		return SCMI_PIN_BIAS_HIGH_IMPEDANCE;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		return SCMI_PIN_BIAS_PULL_DOWN;
	case PIN_CONFIG_BIAS_PULL_PIN_DEFAULT:
		return SCMI_PIN_BIAS_PULL_DEFAULT;
	case PIN_CONFIG_BIAS_PULL_UP:
		return SCMI_PIN_BIAS_PULL_UP;
	case PIN_CONFIG_DRIVE_OPEN_DRAIN:
		return SCMI_PIN_DRIVE_OPEN_DRAIN;
	case PIN_CONFIG_DRIVE_OPEN_SOURCE:
		return SCMI_PIN_DRIVE_OPEN_SOURCE;
	case PIN_CONFIG_DRIVE_PUSH_PULL:
		return SCMI_PIN_DRIVE_PUSH_PULL;
	case PIN_CONFIG_DRIVE_STRENGTH:
		return SCMI_PIN_DRIVE_STRENGTH;
	case PIN_CONFIG_INPUT_DEBOUNCE:
		return SCMI_PIN_INPUT_DEBOUNCE;
	case PIN_CONFIG_INPUT_ENABLE:
		return SCMI_PIN_INPUT_MODE;
	case PIN_CONFIG_INPUT_SCHMITT:
		return SCMI_PIN_INPUT_SCHMITT;
	case PIN_CONFIG_LOW_POWER_MODE:
		return SCMI_PIN_LOW_POWER_MODE;
	case PIN_CONFIG_OUTPUT_ENABLE:
		return SCMI_PIN_OUTPUT_MODE;
	case PIN_CONFIG_OUTPUT:
		return SCMI_PIN_OUTPUT_VALUE;
	case PIN_CONFIG_POWER_SOURCE:
		return SCMI_PIN_POWER_SOURCE;
	case PIN_CONFIG_SLEW_RATE:
		return SCMI_PIN_SLEW_RATE;
	}

	return -EINVAL;
}

int scmi_pinctrl_protocol_attrs(struct udevice *dev, int *num_pins,
				int *num_groups, int *num_functions)
{
	struct scmi_pinctrl_protocol_attrs_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_PINCTRL,
		.message_id = SCMI_PROTOCOL_ATTRIBUTES,
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (out.status)
		return scmi_to_linux_errno(out.status);

	if (num_groups)
		*num_groups = FIELD_GET(GENMASK(31, 16), out.attr_low);
	if (num_pins)
		*num_pins = FIELD_GET(GENMASK(15, 0), out.attr_low);
	if (num_functions)
		*num_functions = FIELD_GET(GENMASK(15, 0), out.attr_high);

	return 0;
}

int scmi_pinctrl_attrs(struct udevice *dev, enum select_type select_type,
		       unsigned int selector, bool *gpio, unsigned int *count,
		       char *name)
{
	struct scmi_pinctrl_attrs_in in;
	struct scmi_pinctrl_attrs_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_PINCTRL,
		.message_id = SCMI_PINCTRL_ATTRIBUTES,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	in.select_type = select_type;
	in.id = selector;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (out.status)
		return scmi_to_linux_errno(out.status);

	if (gpio)
		*gpio = FIELD_GET(BIT(17), out.attr);
	if (count)
		*count = FIELD_GET(GENMASK(15, 0), out.attr);
	if (name)
		strncpy(name, out.name, sizeof(out.name));

	return 0;
}

int scmi_pinctrl_list_associations(struct udevice *dev,
				   enum select_type select_type,
				   unsigned int selector,
				   unsigned short *output,
				   unsigned short num_out)
{
	struct scmi_pinctrl_list_associations_in in;
	struct scmi_pinctrl_list_associations_out *out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_PINCTRL,
		.message_id = SCMI_PINCTRL_LIST_ASSOCIATIONS,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
	};
	size_t out_sz = sizeof(*out) + num_out * sizeof(out->array[0]);
	unsigned int count;
	int ret = -EINVAL;

	out = kzalloc(out_sz, GFP_KERNEL);
	if (!out)
		return -ENOMEM;

	msg.out_msg = (u8 *)out;
	msg.out_msg_sz = out_sz;
	in.select_type = select_type;
	in.id = selector;
	in.index = 0;

	while (num_out > 0) {
		ret = devm_scmi_process_msg(dev, &msg);
		if (ret)
			goto free;
		if (out->status) {
			ret = scmi_to_linux_errno(out->status);
			goto free;
		}

		count = FIELD_GET(GENMASK(11, 0), out->flags);
		if (count > num_out)
			return -EINVAL;
		memcpy(&output[in.index], out->array, count * sizeof(u16));
		num_out -= count;
		in.index += count;
	}
free:
	kfree(out);
	return ret;
}

#define SCMI_PINCTRL_CONFIG_SETTINGS_FUNCTION -2u

int scmi_pinctrl_settings_get_one(struct udevice *dev, enum select_type select_type,
				  unsigned int selector,
				  u32 config_type, u32 *value)
{
	struct scmi_pinctrl_settings_get_in in;
	struct scmi_pinctrl_settings_get_out *out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_PINCTRL,
		.message_id = SCMI_PINCTRL_SETTINGS_GET,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
	};
	size_t out_sz = sizeof(*out) + (sizeof(u32) * 2);
	u32 num_configs;
	int ret;

	if (config_type == SCMI_PINCTRL_CONFIG_SETTINGS_ALL) {
		/* FIXME: implement */
		return -EIO;
	}

	out = kzalloc(out_sz, GFP_KERNEL);
	if (!out)
		return -ENOMEM;

	msg.out_msg = (u8 *)out;
	msg.out_msg_sz = out_sz;
	in.id = selector;
	in.attr = 0;
	if (config_type == SCMI_PINCTRL_CONFIG_SETTINGS_FUNCTION)
		in.attr = FIELD_PREP(GENMASK(19, 18), 2);
	in.attr |= FIELD_PREP(GENMASK(17, 16), select_type);
	if (config_type != SCMI_PINCTRL_CONFIG_SETTINGS_FUNCTION)
		in.attr |= FIELD_PREP(GENMASK(7, 0), config_type);

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		goto free;
	if (out->status) {
		ret = scmi_to_linux_errno(out->status);
		goto free;
	}
	num_configs = FIELD_GET(GENMASK(7, 0), out->num_configs);
	if (out->num_configs == 0) {
		*value = out->function_selected;
		goto free;
	}
	if (num_configs != 1) {
		ret = -EINVAL;
		goto free;
	}

	*value = out->configs[1];
free:
	kfree(out);
	return ret;
}

static int scmi_pinctrl_settings_configure_helper(struct udevice *dev,
						  enum select_type select_type,
						  unsigned int selector,
						  u32 function_id,
						  u16 num_configs, u32 *configs)
{
	struct scmi_pinctrl_settings_configure_in *in;
	struct scmi_pinctrl_settings_configure_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_PINCTRL,
		.message_id = SCMI_PINCTRL_SETTINGS_CONFIGURE,
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	size_t in_sz = sizeof(*in) + (num_configs * sizeof(u32) * 2);
	int ret;

	in = kzalloc(in_sz, GFP_KERNEL);
	if (!in)
		return -ENOMEM;

	msg.in_msg = (u8 *)in;
	msg.in_msg_sz = in_sz;
	in->id = selector;
	in->function_id = function_id;
	in->attr = 0;
	in->attr |= FIELD_PREP(GENMASK(9, 2), num_configs);
	in->attr |= FIELD_PREP(GENMASK(1, 0), select_type);
	memcpy(in->configs, configs, num_configs * sizeof(u32) * 2);

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		goto free;
	if (out.status) {
		ret = scmi_to_linux_errno(out.status);
		goto free;
	}
free:
	kfree(in);
	return ret;
}

int scmi_pinctrl_settings_configure(struct udevice *dev, enum select_type select_type,
				    unsigned int selector, u16 num_configs,
				    u32 *configs)
{
	return scmi_pinctrl_settings_configure_helper(dev, select_type,
						      selector,
						      SCMI_PINCTRL_FUNCTION_NONE,
						      num_configs, configs);
}

int scmi_pinctrl_settings_configure_one(struct udevice *dev, enum select_type select_type,
					unsigned int selector,
					u32 param, u32 argument)
{
	u32 config_value[2];
	int scmi_config;

	/* see stmfx_pinctrl_conf_set() */
	scmi_config = map_config_param_to_scmi(param);
	if (scmi_config < 0)
		return scmi_config;

	config_value[0] = scmi_config;
	config_value[1] = argument;

	return scmi_pinctrl_settings_configure(dev, select_type, selector, 1,
					       &config_value[0]);
}

int scmi_pinctrl_set_function(struct udevice *dev, enum select_type select_type,
			      unsigned int selector, u32 function_id)
{
	return scmi_pinctrl_settings_configure_helper(dev, select_type, selector,
						      function_id, 0, NULL);
}

int scmi_pinctrl_request(struct udevice *dev, enum select_type select_type,
			 unsigned int selector)
{
	struct scmi_pinctrl_request_in in;
	struct scmi_pinctrl_request_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_PINCTRL,
		.message_id = SCMI_PINCTRL_REQUEST,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	in.id = selector;
	in.flags = FIELD_PREP(GENMASK(1, 0), select_type);

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (out.status)
		return scmi_to_linux_errno(out.status);

	return 0;
}

int scmi_pinctrl_release(struct udevice *dev, enum select_type select_type,
			 unsigned int selector)
{
	struct scmi_pinctrl_release_in in;
	struct scmi_pinctrl_release_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_PINCTRL,
		.message_id = SCMI_PINCTRL_RELEASE,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	in.id = selector;
	in.flags = FIELD_PREP(GENMASK(1, 0), select_type);

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (out.status)
		return scmi_to_linux_errno(out.status);

	return 0;
}

