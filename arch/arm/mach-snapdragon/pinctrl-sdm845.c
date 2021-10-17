// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm SDM845 pinctrl
 *
 * (C) Copyright 2021 Dzmitry Sankouski <dsankouski@gmail.com>
 *
 */

#include "pinctrl-snapdragon.h"
#include <common.h>

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

struct msm_pinctrl_data sdm845_data = {
	.pin_count = 150,
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = sdm845_get_function_name,
	.get_function_mux = sdm845_get_function_mux,
	.get_pin_name = sdm845_get_pin_name,
};
