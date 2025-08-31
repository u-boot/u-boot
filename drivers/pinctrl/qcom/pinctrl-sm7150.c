// SPDX-License-Identifier: BSD-3-Clause
/*
 * Pinctrl driver for Qualcomm SM7150
 *
 * (C) Copyright 2025 Danila Tikhonov <danila@jiaxyga.com>
 * (C) Copyright 2025 Jens Reidel <adrian@mainlining.org>
 *
 * Based on Linux Kernel driver
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define WEST	0x00000000
#define NORTH	0x00400000
#define SOUTH	0x00800000

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{ "qup12", 1 },
	{ "gpio",  0 },
	{ "sdc2_clk", 0 } /* special pin GPIO124 */
};

#define SDC_QDSD_PINGROUP(pg_name, ctl, pull, drv)	\
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

static const struct msm_special_pin_data msm_special_pins_data[] = {
	[0] = UFS_RESET("ufs_reset", 0x9f000),
	[1] = SDC_QDSD_PINGROUP("sdc1_rclk", WEST + 0x9a000, 15, 0),
	[2] = SDC_QDSD_PINGROUP("sdc1_clk", WEST + 0x9a000, 13, 6),
	[3] = SDC_QDSD_PINGROUP("sdc1_cmd", WEST + 0x9a000, 11, 3),
	[4] = SDC_QDSD_PINGROUP("sdc1_data", WEST + 0x9a000, 9, 0),
	[5] = SDC_QDSD_PINGROUP("sdc2_clk", SOUTH + 0x98000, 14, 6),
	[6] = SDC_QDSD_PINGROUP("sdc2_cmd", SOUTH + 0x98000, 11, 3),
	[7] = SDC_QDSD_PINGROUP("sdc2_data", SOUTH + 0x98000, 9, 0),
};

static const unsigned int sm7150_pin_offsets[] = {
	[0] = SOUTH,	[1] = SOUTH,	[2] = SOUTH,	[3] = SOUTH,
	[4] = NORTH,	[5] = NORTH,	[6] = NORTH,	[7] = NORTH,
	[8] = NORTH,	[9] = NORTH,	[10] = NORTH,	[11] = NORTH,
	[12] = SOUTH,	[13] = SOUTH,	[14] = SOUTH,	[15] = SOUTH,
	[16] = SOUTH,	[17] = SOUTH,	[18] = SOUTH,	[19] = SOUTH,
	[20] = SOUTH,	[21] = SOUTH,	[22] = SOUTH,	[23] = SOUTH,
	[24] = SOUTH,	[25] = SOUTH,	[26] = SOUTH,	[27] = SOUTH,
	[28] = SOUTH,	[29] = NORTH,	[30] = SOUTH,	[31] = WEST,
	[32] = NORTH,	[33] = NORTH,	[34] = SOUTH,	[35] = SOUTH,
	[36] = SOUTH,	[37] = SOUTH,	[38] = SOUTH,	[39] = SOUTH,
	[40] = SOUTH,	[41] = SOUTH,	[42] = NORTH,	[43] = NORTH,
	[44] = NORTH,	[45] = NORTH,	[46] = NORTH,	[47] = NORTH,
	[48] = WEST,	[49] = WEST,	[50] = WEST,	[51] = WEST,
	[52] = WEST,	[53] = WEST,	[54] = WEST,	[55] = WEST,
	[56] = WEST,	[57] = WEST,	[58] = WEST,	[59] = NORTH,
	[60] = NORTH,	[61] = NORTH,	[62] = NORTH,	[63] = NORTH,
	[64] = NORTH,	[65] = NORTH,	[66] = NORTH,	[67] = NORTH,
	[68] = NORTH,	[69] = NORTH,	[70] = NORTH,	[71] = NORTH,
	[72] = NORTH,	[73] = NORTH,	[74] = WEST,	[75] = WEST,
	[76] = WEST,	[77] = WEST,	[78] = WEST,	[79] = WEST,
	[80] = WEST,	[81] = WEST,	[82] = WEST,	[83] = WEST,
	[84] = WEST,	[85] = WEST,	[86] = NORTH,	[87] = NORTH,
	[88] = NORTH,	[89] = NORTH,	[90] = NORTH,	[91] = NORTH,
	[92] = NORTH,	[93] = NORTH,	[94] = SOUTH,	[95] = WEST,
	[96] = WEST,	[97] = WEST,	[98] = WEST,	[99] = WEST,
	[100] = WEST,	[101] = NORTH,	[102] = NORTH,	[103] = NORTH,
	[104] = WEST,	[105] = NORTH,	[106] = NORTH,	[107] = WEST,
	[108] = SOUTH,	[109] = SOUTH,	[110] = NORTH,	[111] = NORTH,
	[112] = NORTH,	[113] = NORTH,	[114] = NORTH,	[115] = NORTH,
	[116] = NORTH,	[117] = NORTH,	[118] = NORTH,
};

static const char *sm7150_get_function_name(struct udevice *dev, unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *sm7150_get_pin_name(struct udevice *dev,
				       unsigned int selector)
{
	if (selector >= 119 && selector <= 126)
		snprintf(pin_name, MAX_PIN_NAME_LEN,
			 msm_special_pins_data[selector - 119].name);
	else
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);

	return pin_name;
}

static int sm7150_get_function_mux(__maybe_unused unsigned int pin, unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static struct msm_pinctrl_data sm7150_data = {
	.pin_data = {
		.pin_offsets = sm7150_pin_offsets,
		.pin_count = 126,
		.special_pins_start = 119,
		.special_pins_data = msm_special_pins_data,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = sm7150_get_function_name,
	.get_function_mux = sm7150_get_function_mux,
	.get_pin_name = sm7150_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,sm7150-tlmm", .data = (ulong)&sm7150_data },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(pinctrl_sm7150) = {
	.name		= "pinctrl_sm7150",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
};
