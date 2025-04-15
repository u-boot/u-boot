// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm x1e80100 pinctrl
 *
 * (C) Copyright 2024 Linaro Ltd.
 *
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{"qup2_se5", 1},
	{"pcie3_clk", 1},
	{"pcie4_clk", 1},
	{"pcie5_clk", 1},
	{"pcie6a_clk", 1},
	{"pcie6b_clk", 1},
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

#define UFS_RESET(pg_name, ctl)				\
	{					        \
		.name = pg_name,			\
		.ctl_reg = ctl,				\
		.io_reg = ctl + 0x4,			\
		.pull_bit = 3,				\
		.drv_bit = 0,				\
		.oe_bit = -1,				\
		.in_bit = -1,				\
		.out_bit = 0,				\
	}

static const struct msm_special_pin_data msm_special_pins_data[] = {
	[0] = UFS_RESET("ufs_reset", 0xf9000),
	[1] = SDC_QDSD_PINGROUP("sdc2_clk", 0xf2000, 14, 6),
	[2] = SDC_QDSD_PINGROUP("sdc2_cmd", 0xf2000, 11, 3),
	[3] = SDC_QDSD_PINGROUP("sdc2_data", 0xf2000, 9, 0),
};

static const char *x1e80100_get_function_name(struct udevice *dev,
					      unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *x1e80100_get_pin_name(struct udevice *dev,
					 unsigned int selector)
{
	if (selector >= 238 && selector <= 241)
		snprintf(pin_name, MAX_PIN_NAME_LEN,
			 msm_special_pins_data[selector - 238].name);
	else
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);

	return pin_name;
}

static int x1e80100_get_function_mux(__maybe_unused unsigned int pin,
				     unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static struct msm_pinctrl_data x1e80100_data = {
	.pin_data = {
		.pin_count = 242,
		.special_pins_start = 238,
		.special_pins_data = msm_special_pins_data,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = x1e80100_get_function_name,
	.get_function_mux = x1e80100_get_function_mux,
	.get_pin_name = x1e80100_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,x1e80100-tlmm", .data = (ulong)&x1e80100_data },
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(pinctrl_x1e80100) = {
	.name		= "pinctrl_x1e80100",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
};

