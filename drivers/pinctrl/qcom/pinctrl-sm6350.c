// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm sm6350 pinctrl
 *
 * (C) Copyright 2024 Linaro Ltd.
 * (C) Copyright 2025 Luca Weiss <luca.weiss@fairphone.com>
 *
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{"qup13_f2", 1},
	{"gpio", 0},
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

static const struct msm_special_pin_data sm6350_special_pins_data[] = {
	[0] = UFS_RESET("ufs_reset", 0xae000),
	[1] = SDC_PINGROUP("sdc1_rclk", 0xa1000, 15, 0),
	[2] = SDC_PINGROUP("sdc1_clk", 0xa0000, 13, 6),
	[3] = SDC_PINGROUP("sdc1_cmd", 0xa0000, 11, 3),
	[4] = SDC_PINGROUP("sdc1_data", 0xa0000, 9, 0),
	[5] = SDC_PINGROUP("sdc2_clk", 0xa2000, 14, 6),
	[6] = SDC_PINGROUP("sdc2_cmd", 0xa2000, 11, 3),
	[7] = SDC_PINGROUP("sdc2_data", 0xa2000, 9, 0),
};

static const char *sm6350_get_function_name(struct udevice *dev,
					    unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *sm6350_get_pin_name(struct udevice *dev,
				       unsigned int selector)
{
	if (selector >= 156 && selector <= 163)
		snprintf(pin_name, MAX_PIN_NAME_LEN,
			 sm6350_special_pins_data[selector - 156].name);
	else
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);

	return pin_name;
}

static int sm6350_get_function_mux(__maybe_unused unsigned int pin,
				   unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static struct msm_pinctrl_data sm6350_data = {
	.pin_data = {
		.pin_count = 164,
		.special_pins_start = 156,
		.special_pins_data = sm6350_special_pins_data,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = sm6350_get_function_name,
	.get_function_mux = sm6350_get_function_mux,
	.get_pin_name = sm6350_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,sm6350-tlmm", .data = (ulong)&sm6350_data },
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(pinctrl_sm6350) = {
	.name		= "pinctrl_sm6350",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
};
