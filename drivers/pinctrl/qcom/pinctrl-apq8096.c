// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm APQ8096 pinctrl
 *
 * (C) Copyright 2019 Ramon Fried <ramon.fried@gmail.com>
 *
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");
static const char * const msm_pinctrl_pins[] = {
	"sdc1_clk",
	"sdc1_cmd",
	"sdc1_data",
	"sdc2_clk",
	"sdc2_cmd",
	"sdc2_data",
	"sdc1_rclk",
};

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{"blsp_uart8", 2},
};

static const char *apq8096_get_function_name(struct udevice *dev,
					     unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *apq8096_get_pin_name(struct udevice *dev,
					unsigned int selector)
{
	if (selector < 150) {
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);
		return pin_name;
	} else {
		return msm_pinctrl_pins[selector - 150];
	}
}

static int apq8096_get_function_mux(__maybe_unused unsigned int pin,
				    unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static const struct msm_pinctrl_data apq8096_data = {
	.pin_data = {
		.pin_count = 157,
		.special_pins_start = 150,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = apq8096_get_function_name,
	.get_function_mux = apq8096_get_function_mux,
	.get_pin_name = apq8096_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,msm8996-pinctrl", .data = (ulong)&apq8096_data },
	{ /* Sentinal */ }
};

U_BOOT_DRIVER(pinctrl_apq8096) = {
	.name		= "pinctrl_apq8096",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
};
