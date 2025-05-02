// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm qcm2290 pinctrl
 *
 * (C) Copyright 2024 Linaro Ltd.
 *
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{ "qup4", 1 },
	{ "gpio", 0 },
};

static const char *qcm2290_get_function_name(struct udevice *dev, unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *qcm2290_get_pin_name(struct udevice *dev, unsigned int selector)
{
	static const char *const special_pins_names[] = {
		"sdc1_rclk", "sdc1_clk", "sdc1_cmd",  "sdc1_data",
		"sdc2_clk",  "sdc2_cmd", "sdc2_data",
	};

	if (selector >= 127 && selector <= 133)
		snprintf(pin_name, MAX_PIN_NAME_LEN, special_pins_names[selector - 127]);
	else
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);

	return pin_name;
}

static int qcm2290_get_function_mux(__maybe_unused unsigned int pin, unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

struct msm_pinctrl_data qcm2290_data = {
	.pin_data = {
		.pin_count = 134,
		.special_pins_start = 127,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = qcm2290_get_function_name,
	.get_function_mux = qcm2290_get_function_mux,
	.get_pin_name = qcm2290_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{
		.compatible = "qcom,qcm2290-tlmm",
		.data = (ulong)&qcm2290_data
	},
	{ /* Sentinel */ } };

U_BOOT_DRIVER(pinctrl_qcm2290) = {
	.name = "pinctrl_qcm2290",
	.id = UCLASS_NOP,
	.of_match = msm_pinctrl_ids,
	.ops = &msm_pinctrl_ops,
	.bind = msm_pinctrl_bind,
};
