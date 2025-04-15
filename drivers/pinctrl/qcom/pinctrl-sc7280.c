// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm sc7280 pinctrl
 *
 * (C) Copyright 2024 Linaro Ltd.
 *
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define WEST 0x00000000
#define SOUTH 0x00400000
#define NORTH 0x00800000

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{ "qup05", 1 },
	{ "gpio", 0 },
	{ "pcie1_clkreqn", 3},
};
#define SDC_PINGROUP(pg_name, ctl, pull, drv)		\
	{						\
		.name = pg_name,			\
		.ctl_reg = ctl,				\
		.io_reg = 0,				\
		.pull_bit = pull,			\
		.drv_bit = drv,				\
		.oe_bit = -1,				\
		.in_bit = -1,				\
		.out_bit = -1,				\
	}

#define UFS_RESET(pg_name, offset)			\
	{						\
		.name = pg_name,			\
		.ctl_reg = offset,			\
		.io_reg = offset + 0x4,			\
		.pull_bit = 3,				\
		.drv_bit = 0,				\
		.oe_bit = -1,				\
		.in_bit = -1,				\
		.out_bit = 0,				\
	}

static const struct msm_special_pin_data sc7280_special_pins_data[] = {
	[0] = UFS_RESET("ufs_reset", SOUTH + 0xbe000),
	[1] = SDC_PINGROUP("sdc1_rclk", 0xb3004, 0, 6),
	[2] = SDC_PINGROUP("sdc1_clk", 0xb3000, 13, 6),
	[3] = SDC_PINGROUP("sdc1_cmd", 0xb3000, 11, 3),
	[4] = SDC_PINGROUP("sdc1_data", 0xb3000, 9, 0),
	[5] = SDC_PINGROUP("sdc2_clk", 0xb4000, 14, 6),
	[6] = SDC_PINGROUP("sdc2_cmd", 0xb4000, 11, 3),
	[7] = SDC_PINGROUP("sdc2_data", 0xb4000, 9, 0),
};

static const char *sc7280_get_function_name(struct udevice *dev, unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *sc7280_get_pin_name(struct udevice *dev, unsigned int selector)
{
	if (selector >= 175 && selector <= 182)
		snprintf(pin_name, MAX_PIN_NAME_LEN,
			 sc7280_special_pins_data[selector - 175].name);
	else
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);

	return pin_name;
}

static int sc7280_get_function_mux(__maybe_unused unsigned int pin, unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static struct msm_pinctrl_data sc7280_data = {
	.pin_data = {
		.pin_count = 183,
		.special_pins_start = 175,
		.special_pins_data = sc7280_special_pins_data,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = sc7280_get_function_name,
	.get_function_mux = sc7280_get_function_mux,
	.get_pin_name = sc7280_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{
		.compatible = "qcom,sc7280-pinctrl",
		.data = (ulong)&sc7280_data
	},
	{ /* Sentinel */ } };

U_BOOT_DRIVER(pinctrl_sc7280) = {
	.name = "pinctrl_sc7280",
	.id = UCLASS_NOP,
	.of_match = msm_pinctrl_ids,
	.ops = &msm_pinctrl_ops,
	.bind = msm_pinctrl_bind,
};
