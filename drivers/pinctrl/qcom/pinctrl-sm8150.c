// SPDX-License-Identifier: BSD-3-Clause
/*
 * Qualcomm SM8150 pinctrl and GPIO driver
 *
 * Volodymyr Babchuk <volodymyr_babchuk@epam.com>
 * Copyright (c) 2024 EPAM Systems.
 *
 * (C) Copyright 2024 Julius Lehmann <lehmanju@devpi.de>
 *
 * Based on similar U-Boot drivers. Constants were taken from the Linux driver
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define WEST	0x100000
#define EAST	0x500000
#define NORTH	0x900000
#define SOUTH	0xd00000

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{ "qup2", 1 },
	{ "gpio", 0 },
};

static const unsigned int sm8150_pin_offsets[] = {
	[0]   = SOUTH, [1]   = SOUTH, [2]   = SOUTH, [3]   = SOUTH,
	[4]   = SOUTH, [5]   = SOUTH, [6]   = SOUTH, [7]   = SOUTH,
	[8]   = NORTH, [9]   = NORTH, [10]  = NORTH, [11]  = NORTH,
	[12]  = NORTH, [13]  = NORTH, [14]  = NORTH, [15]  = NORTH,
	[16]  = NORTH, [17]  = NORTH, [18]  = NORTH, [19]  = NORTH,
	[20]  = NORTH, [21]  = EAST,  [22]  = EAST,  [23]  = EAST,
	[24]  = EAST,  [25]  = EAST,  [26]  = EAST,  [27]  = EAST,
	[28]  = EAST,  [29]  = EAST,  [30]  = EAST,  [31]  = NORTH,
	[32]  = NORTH, [33]  = NORTH, [34]  = NORTH, [35]  = NORTH,
	[36]  = NORTH, [37]  = NORTH, [38]  = SOUTH, [39]  = NORTH,
	[40]  = NORTH, [41]  = NORTH, [42]  = NORTH, [43]  = EAST,
	[44]  = EAST,  [45]  = EAST,  [46]  = EAST,  [47]  = EAST,
	[48]  = EAST,  [49]  = EAST,  [50]  = EAST,  [51]  = SOUTH,
	[52]  = SOUTH, [53]  = SOUTH, [54]  = SOUTH, [55]  = SOUTH,
	[56]  = SOUTH, [57]  = SOUTH, [58]  = SOUTH, [59]  = SOUTH,
	[60]  = SOUTH, [61]  = SOUTH, [62]  = SOUTH, [63]  = SOUTH,
	[64]  = SOUTH, [65]  = SOUTH, [66]  = SOUTH, [67]  = SOUTH,
	[68]  = SOUTH, [69]  = SOUTH, [70]  = SOUTH, [71]  = SOUTH,
	[72]  = SOUTH, [73]  = SOUTH, [74]  = SOUTH, [75]  = SOUTH,
	[76]  = SOUTH, [77]  = SOUTH, [78]  = SOUTH, [79]  = SOUTH,
	[80]  = SOUTH, [81]  = SOUTH, [82]  = SOUTH, [83]  = NORTH,
	[84]  = NORTH, [85]  = NORTH, [86]  = NORTH, [87]  = EAST,
	[88]  = NORTH, [89]  = NORTH, [90]  = NORTH, [91]  = NORTH,
	[92]  = NORTH, [93]  = NORTH, [94]  = NORTH, [95]  = NORTH,
	[96]  = NORTH, [97]  = NORTH, [98]  = SOUTH, [99]  = SOUTH,
	[100] = SOUTH, [101] = SOUTH, [102] = NORTH, [103] = NORTH,
	[104] = NORTH, [105] = WEST,  [106] = WEST,  [107] = WEST,
	[108] = WEST,  [109] = WEST,  [110] = WEST,  [111] = WEST,
	[112] = WEST,  [113] = WEST,  [114] = SOUTH, [115] = SOUTH,
	[116] = SOUTH, [117] = SOUTH, [118] = SOUTH, [119] = SOUTH,
	[120] = SOUTH, [121] = SOUTH, [122] = SOUTH, [123] = SOUTH,
	[124] = SOUTH, [125] = WEST,  [126] = SOUTH, [127] = SOUTH,
	[128] = SOUTH, [129] = SOUTH, [130] = SOUTH, [131] = SOUTH,
	[132] = SOUTH, [133] = SOUTH, [134] = SOUTH, [135] = SOUTH,
	[136] = SOUTH, [137] = SOUTH, [138] = SOUTH, [139] = SOUTH,
	[140] = SOUTH, [141] = SOUTH, [142] = SOUTH, [143] = SOUTH,
	[144] = SOUTH, [145] = SOUTH, [146] = SOUTH, [147] = SOUTH,
	[148] = SOUTH, [149] = SOUTH, [150] = SOUTH, [151] = SOUTH,
	[152] = SOUTH, [153] = SOUTH, [154] = SOUTH, [155] = WEST,
	[156] = WEST,  [157] = WEST,  [158] = WEST,  [159] = WEST,
	[160] = WEST,  [161] = WEST,  [162] = WEST,  [163] = WEST,
	[164] = WEST,  [165] = WEST,  [166] = WEST,  [167] = WEST,
	[168] = WEST,  [169] = NORTH, [170] = NORTH, [171] = NORTH,
	[172] = NORTH, [173] = NORTH, [174] = NORTH,
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

#define UFS_RESET(pg_name, offset)        \
	{                                 \
		.name = pg_name,	  \
		.ctl_reg = offset,	  \
		.io_reg = offset + 0x04,  \
		.pull_bit = 3,		  \
		.drv_bit = 0,             \
		.oe_bit = -1,             \
		.in_bit = -1,             \
		.out_bit = 0,             \
	}

static const struct msm_special_pin_data msm_special_pins_data[] = {
	[0] = UFS_RESET("ufs_reset", SOUTH + 0xb6000),
	[1] = SDC_QDSD_PINGROUP("sdc2_clk", NORTH + 0xb2000, 14, 6),
	[2] = SDC_QDSD_PINGROUP("sdc2_cmd", NORTH + 0xb2000, 11, 3),
	[3] = SDC_QDSD_PINGROUP("sdc2_data", NORTH + 0xb2000, 9, 0),
};

static const char *sm8150_get_function_name(struct udevice *dev,
					    unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *sm8150_get_pin_name(struct udevice *dev,
				       unsigned int selector)
{
	if (selector >= 175 && selector <= 178)
		snprintf(pin_name, MAX_PIN_NAME_LEN,
			 msm_special_pins_data[selector - 175].name);
	else
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);

	return pin_name;
}

static int sm8150_get_function_mux(__maybe_unused unsigned int pin,
				   unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static struct msm_pinctrl_data sm8150_data = {
	.pin_data = {
		.pin_offsets = sm8150_pin_offsets,
		.pin_count = 179,
		.special_pins_start = 175,
		.special_pins_data = msm_special_pins_data,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = sm8150_get_function_name,
	.get_function_mux = sm8150_get_function_mux,
	.get_pin_name = sm8150_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,sm8150-pinctrl", .data = (ulong)&sm8150_data },
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(pinctrl_sm8150) = {
	.name		= "pinctrl_sm8150",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
};
