// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm sm8550 pinctrl
 *
 * (C) Copyright 2024 Linaro Ltd.
 *
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{"qup1_se7", 1},
	{"gpio", 0},
};

#define SDC_QDSD_PINGROUP(pg_name, ctl, pull, drv)	\
	{					        \
		.name = pg_name,			\
		.ctl_reg = ctl,				\
		.io_reg = 0,				\
		.pull_bit = pull,			\
		.drv_bit = drv,				\
		.oe_bit = -1,				\
		.in_bit = -1,				\
		.out_bit = -1,				\
	}

#define UFS_RESET(pg_name, ctl, io)			\
	{					        \
		.name = pg_name,			\
		.ctl_reg = ctl,				\
		.io_reg = io,				\
		.pull_bit = 3,				\
		.drv_bit = 0,				\
		.oe_bit = -1,				\
		.in_bit = -1,				\
		.out_bit = 0,				\
	}

static const struct msm_special_pin_data msm_special_pins_data[] = {
	[0] = UFS_RESET("ufs_reset", 0xde000, 0xde004),
	[1] = SDC_QDSD_PINGROUP("sdc2_clk", 0xd6000, 14, 6),
	[2] = SDC_QDSD_PINGROUP("sdc2_cmd", 0xd6000, 11, 3),
	[3] = SDC_QDSD_PINGROUP("sdc2_data", 0xd6000, 9, 0),
};

static const char *sm8550_get_function_name(struct udevice *dev,
						 unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *sm8550_get_pin_name(struct udevice *dev,
					unsigned int selector)
{
	if (selector >= 210 && selector <= 213)
		snprintf(pin_name, MAX_PIN_NAME_LEN,
			 msm_special_pins_data[selector - 210].name);
	else
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);

	return pin_name;
}

static unsigned int sm8550_get_function_mux(__maybe_unused unsigned int pin,
					    unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static struct msm_pinctrl_data sm8550_data = {
	.pin_data = {
		.pin_count = 214,
		.special_pins_start = 210,
		.special_pins_data = msm_special_pins_data,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = sm8550_get_function_name,
	.get_function_mux = sm8550_get_function_mux,
	.get_pin_name = sm8550_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,sm8550-tlmm", .data = (ulong)&sm8550_data },
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(pinctrl_sm8550) = {
	.name		= "pinctrl_sm8550",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
};

