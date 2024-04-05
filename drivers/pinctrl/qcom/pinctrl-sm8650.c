// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm sm8650 pinctrl
 *
 * (C) Copyright 2024 Linaro Ltd.
 *
 */

#include <common.h>
#include <dm.h>

#include "pinctrl-qcom.h"

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{"qup2_se7", 1},
	{"gpio", 0},
};

static const char *sm8650_get_function_name(struct udevice *dev,
						 unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *sm8650_get_pin_name(struct udevice *dev,
					unsigned int selector)
{
	static const char *special_pins_names[] = {
		"ufs_reset",
		"sdc2_clk",
		"sdc2_cmd",
		"sdc2_data",
	};

	if (selector >= 210 && selector <= 213)
		snprintf(pin_name, MAX_PIN_NAME_LEN, special_pins_names[selector - 210]);
	else
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);

	return pin_name;
}

static unsigned int sm8650_get_function_mux(__maybe_unused unsigned int pin,
					    unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static struct msm_pinctrl_data sm8650_data = {
	.pin_data = {
		.pin_count = 214,
		.special_pins_start = 210,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = sm8650_get_function_name,
	.get_function_mux = sm8650_get_function_mux,
	.get_pin_name = sm8650_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,sm8650-tlmm", .data = (ulong)&sm8650_data },
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(pinctrl_sm8650) = {
	.name		= "pinctrl_sm8650",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
};

