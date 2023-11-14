// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm SDM845 pinctrl
 *
 * (C) Copyright 2021 Dzmitry Sankouski <dsankouski@gmail.com>
 *
 */

#include <common.h>
#include <dm.h>

#include "pinctrl-qcom.h"

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{"qup9", 1},
	{"gpio", 0},
};

static const char *sdm845_get_function_name(struct udevice *dev,
					     unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *sdm845_get_pin_name(struct udevice *dev,
					unsigned int selector)
{
	snprintf(pin_name, MAX_PIN_NAME_LEN, "GPIO_%u", selector);
	return pin_name;
}

static unsigned int sdm845_get_function_mux(unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static struct msm_pinctrl_data sdm845_data = {
	.pin_count = 150,
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = sdm845_get_function_name,
	.get_function_mux = sdm845_get_function_mux,
	.get_pin_name = sdm845_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,sdm845-pinctrl", .data = (ulong)&sdm845_data },
	{ /* Sentinal */ }
};

U_BOOT_DRIVER(pinctrl_sdm845) = {
	.name		= "pinctrl_sdm845",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
};
